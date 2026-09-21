#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace dustbox;

// Built from the same toNorm/fromNorm as the browser prototype, so a patch
// JSON means exactly the same thing in both.
static juce::NormalisableRange<float> rangeFor (int i)
{
    const auto& s = kFloatSpecs[i];
    return { s.lo, s.hi,
             [i] (float, float, float n) { return fromNorm (i, n); },
             [i] (float, float, float v) { return toNorm (i, v); },
             [i] (float lo, float hi, float v)
             { return juce::jlimit (lo, hi, kFloatSpecs[i].stepped ? std::round (v) : v); } };
}

// What the knob readouts and the host's automation lanes display.
static juce::String formatValue (int i, float v)
{
    const juce::String u (kFloatSpecs[i].unit);
    if (u == " Hz")
    {
        if (v >= 1000.f) return juce::String (v / 1000.f, v >= 10000.f ? 1 : 2) + "k";
        if (v >= 100.f)  return juce::String (juce::roundToInt (v)) + " Hz";
        return juce::String (v, v < 10.f ? 2 : 1) + " Hz";            // lfo and phaser rates live down here
    }
    if (u == " s")   return v < 1.f ? juce::String (juce::roundToInt (v * 1000.f)) + " ms" : juce::String (v, 2) + " s";
    if (u == " ms")  return v < 10.f ? juce::String (v, 1) + " ms" : juce::String (juce::roundToInt (v)) + " ms";
    if (u == " dB")  return (v > 0.05f ? "+" : "") + juce::String (v, 1) + " dB";
    if (u == " st")  return (v > 0.f ? "+" : "") + juce::String (juce::roundToInt (v)) + " st";
    if (u == " ct")  return juce::String (juce::roundToInt (v)) + " ct";
    if (u == ":1")   return juce::String (v, 1) + ":1";
    if (u == " oct") return juce::String (v, 1) + " oct";
    if (i == pResonance) return juce::String (v, 1);
    return juce::String (juce::roundToInt (v * 100.f)) + "%";
}

juce::AudioProcessorValueTreeState::ParameterLayout DustBoxProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < kNumFloatParams; ++i)
    {
        const auto& s = kFloatSpecs[i];
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { s.id, 1 }, s.label, rangeFor (i), s.def,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (
                [i] (float v, int) { return formatValue (i, v); })));
    }
    for (int i = 0; i < kNumChoiceParams; ++i)
    {
        const auto& s = kChoiceSpecs[i];
        if (s.toggle)
            layout.add (std::make_unique<juce::AudioParameterBool> (
                juce::ParameterID { s.id, 1 }, s.label, s.def != 0));
        else
            layout.add (std::make_unique<juce::AudioParameterChoice> (
                juce::ParameterID { s.id, 1 }, s.label,
                juce::StringArray::fromTokens (s.items, "|", ""), s.def));
    }
    return layout;
}

DustBoxProcessor::DustBoxProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "DUSTBOX", createLayout())
{
    for (int i = 0; i < kNumFloatParams; ++i)
        floatRefs[(size_t) i] = apvts.getRawParameterValue (kFloatSpecs[i].id);
    for (int i = 0; i < kNumChoiceParams; ++i)
        choiceRefs[(size_t) i] = apvts.getRawParameterValue (kChoiceSpecs[i].id);
    setCurrentProgram (0);
}

void DustBoxProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    keyboardState.reset();
}

bool DustBoxProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

Patch DustBoxProcessor::readPatch() const
{
    Patch p = Patch::defaults();
    for (int i = 0; i < kNumFloatParams; ++i)  p.v[i] = floatRefs[(size_t) i]->load();
    for (int i = 0; i < kNumChoiceParams; ++i) p.c[i] = (int) std::lround (choiceRefs[(size_t) i]->load());
    p.clampAll();
    return p;
}

void DustBoxProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const int numSamples = buffer.getNumSamples();

    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);
    engine.setPatch (readPatch());

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            const double bpm = pos->getBpm().orFallback (lastBpm.load());
            lastBpm = bpm;
            engine.setTransport (bpm, pos->getPpqPosition().orFallback (0.0), pos->getIsPlaying());
        }

    int last = 0;
    auto render = [&] (int from, int to)
    {
        if (to > from)
            engine.process (buffer.getWritePointer (0, from), buffer.getWritePointer (1, from), to - from);
    };
    for (const auto meta : midi)
    {
        const int pos = juce::jlimit (0, numSamples, meta.samplePosition);
        render (last, pos);
        last = pos;
        const auto m = meta.getMessage();
        if (m.isNoteOn())                                   engine.noteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff())                             engine.noteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff())    engine.allNotesOff (m.isAllSoundOff());
    }
    render (last, numSamples);

    // feed the scope, decimated so ~1024 points cover roughly 20 ms
    const float* l = buffer.getReadPointer (0);
    const float* r = buffer.getReadPointer (1);
    int w = scopeWrite.load();
    for (int i = 0; i < numSamples; i += 2)
    {
        scope[(size_t) w] = 0.5f * (l[i] + r[i]);
        w = (w + 1) % kScopeSize;
    }
    scopeWrite = w;
}

static bool isCharacter (int i) { return i >= pMDark && i <= pMGlued; }

void DustBoxProcessor::applyPatch (const Patch& in, bool keepCharacter)
{
    Patch p = in; p.clampAll();
    for (int i = 0; i < kNumFloatParams; ++i)
        if (keepCharacter && isCharacter (i)) continue;
        else if (auto* par = apvts.getParameter (kFloatSpecs[i].id))
            par->setValueNotifyingHost (par->convertTo0to1 (p.v[i]));
    for (int i = 0; i < kNumChoiceParams; ++i)
        if (auto* par = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (kChoiceSpecs[i].id)))
            par->setValueNotifyingHost (par->convertTo0to1 ((float) p.c[i]));
}

void DustBoxProcessor::resetCharacter()
{
    for (int i = pMDark; i <= pMGlued; ++i)
        if (auto* par = apvts.getParameter (kFloatSpecs[i].id))
            par->setValueNotifyingHost (par->convertTo0to1 (0.f));
}

void DustBoxProcessor::makeFromVibe (const juce::String& text)
{
    const auto r = matchVibe (text.toStdString());
    applyPatch (r.patch);
    patchName = r.name;
    juce::String info = "from ";
    for (size_t i = 0; i < r.sources.size(); ++i) info << (i ? " + " : "") << juce::String (r.sources[i]);
    if (! r.applied.empty())
    {
        info << "  /  ";
        for (size_t i = 0; i < r.applied.size(); ++i) info << (i ? ", " : "") << juce::String (r.applied[i]);
    }
    patchInfo = info;
}

void DustBoxProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, kNumPresets - 1, index);
    currentProgram = index;
    applyPatch (kPresets[index].patch, true);     // browsing keeps your character settings
    patchName = kPresets[index].name;
    patchInfo = juce::String (kCategories[kPresets[index].category]) + "  /  "
              + juce::String (index + 1) + " of " + juce::String (kNumPresets);
}

const juce::String DustBoxProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, kNumPresets) ? juce::String (kPresets[index].name) : juce::String();
}

// ---- patch files: same format the browser prototype exports ---------------
bool DustBoxProcessor::savePatchJson (const juce::File& file) const
{
    const Patch p = readPatch();
    auto* params = new juce::DynamicObject();
    for (int i = 0; i < kNumFloatParams; ++i)
        params->setProperty (kFloatSpecs[i].id, (double) toNorm (i, p.v[i]));
    for (int i = 0; i < kNumChoiceParams; ++i)
    {
        const auto items = juce::StringArray::fromTokens (kChoiceSpecs[i].items, "|", "");
        params->setProperty (kChoiceSpecs[i].id, items[p.c[i]]);
    }
    auto* root = new juce::DynamicObject();
    root->setProperty ("format", "dustbox.patch.v2");
    root->setProperty ("name", patchName);
    root->setProperty ("params", juce::var (params));
    return file.replaceWithText (juce::JSON::toString (juce::var (root)));
}

bool DustBoxProcessor::loadPatchJson (const juce::File& file)
{
    const auto parsed = juce::JSON::parse (file.loadFileAsString());
    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr) return false;
    auto* po = obj->getProperty ("params").getDynamicObject();
    if (po == nullptr) return false;

    // anything the file doesn't mention (e.g. FX in a v1 browser patch) stays off/default
    Patch p = Patch::defaults();
    for (int c = 0; c < kNumChoiceParams; ++c) if (kChoiceSpecs[c].toggle) p.c[c] = 0;

    for (int i = 0; i < kNumFloatParams; ++i)
    {
        const auto v = po->getProperty (kFloatSpecs[i].id);
        if (! v.isVoid()) p.v[i] = fromNorm (i, (float) (double) v);
    }
    for (int i = 0; i < kNumChoiceParams; ++i)
    {
        const auto v = po->getProperty (kChoiceSpecs[i].id);
        if (v.isVoid()) continue;
        if (v.isBool() || v.isInt() || v.isDouble()) { p.c[i] = (int) v; continue; }
        const auto items = juce::StringArray::fromTokens (kChoiceSpecs[i].items, "|", "");
        const int idx = items.indexOf (v.toString());
        if (idx >= 0) p.c[i] = idx;
    }
    // v1 browser patches had chorus and reverb always in the chain
    if (obj->getProperty ("format").toString() == "dustbox.patch.v1")
    {
        p.c[cChorusOn] = p.v[pChorus] > 0.01f ? 1 : 0;
        p.c[cReverbOn] = p.v[pReverb] > 0.01f ? 1 : 0;
    }
    applyPatch (p);
    patchName = obj->getProperty ("name").toString();
    if (patchName.isEmpty()) patchName = file.getFileNameWithoutExtension();
    patchInfo = "loaded " + file.getFileName();
    return true;
}

void DustBoxProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("patchName", patchName, nullptr);
    state.setProperty ("program", currentProgram.load(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void DustBoxProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        const auto tree = juce::ValueTree::fromXml (*xml);
        if (! tree.isValid()) return;
        apvts.replaceState (tree);
        patchName = tree.getProperty ("patchName", "Dust Box").toString();
        currentProgram = (int) tree.getProperty ("program", 0);
        patchInfo = "restored from session";
    }
}

juce::AudioProcessorEditor* DustBoxProcessor::createEditor() { return new DustBoxEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()      { return new DustBoxProcessor(); }
