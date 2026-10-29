#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using namespace sp;

SerpentProcessor::SerpentProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SERPENT", createLayout())
{
    for (int i = 0; i < NUM_P; ++i)
    {
        raw[(size_t) i] = apvts.getRawParameterValue (kParams[i].id);
        params[(size_t) i] = apvts.getParameter (kParams[i].id);
        jassert (raw[(size_t) i] != nullptr);
    }
    setCurrentProgram (0);
}

bool SerpentProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SerpentProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    keyboardState.reset();
}

void SerpentProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    if (killNotes.exchange (false)) engine.allNotesOff();
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    for (int i = 0; i < NUM_P; ++i) eff[(size_t) i] = raw[(size_t) i]->load();

    // XY pad: push the chosen targets upward from where the preset put them
    static const int xyMap[] = { cutoff, reso, a_wave, b_wave, fm, dist, rev_mix, dly_mix, detune, gate_dep, l_amt, penv };
    auto applyXY = [this] (int targetChoice, float amount)
    {
        if (amount <= 0.0f) return;
        const int t = xyMap[juce::jlimit (0, 11, targetChoice)];
        auto* prm = params[(size_t) t];
        const float n = juce::jlimit (0.0f, 1.0f, prm->convertTo0to1 (eff[(size_t) t]) + amount);
        eff[(size_t) t] = prm->convertFrom0to1 (n);
    };
    applyXY ((int) eff[xy_xd], eff[xy_x]);
    applyXY ((int) eff[xy_yd], eff[xy_y]);

    Clock clk;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) clk.bpm = *b;
            if (auto q = pos->getPpqPosition()) clk.ppq = *q;
            clk.playing = pos->getIsPlaying();
        }
    uiBpm.store (clk.bpm);

    engine.process (buffer, midi, eff.data(), clk);

    // scope tap
    const float* L = buffer.getReadPointer (0);
    const float* R = buffer.getNumChannels() > 1 ? buffer.getReadPointer (1) : L;
    int w = scopeWrite.load();
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        scope[(size_t) w] = 0.5f * (L[i] + R[i]);
        w = (w + 1) % SCOPE;
    }
    scopeWrite.store (w);
    midi.clear();
}

//==============================================================================
int SerpentProcessor::numPresets() { return kNumPresets; }
const SerpentPreset& SerpentProcessor::preset (int i) { return kPresets[juce::jlimit (0, kNumPresets - 1, i)]; }

juce::StringArray SerpentProcessor::categories()
{
    juce::StringArray c;
    for (int i = 0; i < kNumPresets; ++i) c.addIfNotAlreadyThere (kPresets[i].category);
    return c;
}

void SerpentProcessor::applyPresetData (const char* data)
{
    // everything back to default first, then the preset's values
    for (int i = 0; i < NUM_P; ++i)
        params[(size_t) i]->setValueNotifyingHost (params[(size_t) i]->convertTo0to1 (kParams[i].def));

    auto tokens = juce::StringArray::fromTokens (data, ";", "");
    for (auto& t : tokens)
    {
        const auto key = t.upToFirstOccurrenceOf ("=", false, false).trim();
        const auto val = t.fromFirstOccurrenceOf ("=", false, false).trim();
        if (key.isEmpty()) continue;
        if (auto* prm = apvts.getParameter (key))
            prm->setValueNotifyingHost (prm->convertTo0to1 (val.getFloatValue()));
    }
    killNotes.store (true);
    ++presetChangeCounter;
}

int SerpentProcessor::getNumPrograms() { return kNumPresets; }

void SerpentProcessor::setCurrentProgram (int index)
{
    currentProgram = juce::jlimit (0, kNumPresets - 1, index);
    applyPresetData (kPresets[currentProgram].data);
    patchName = kPresets[currentProgram].name;
    ++presetChangeCounter;
}

const juce::String SerpentProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, kNumPresets)
        ? juce::String (kPresets[index].category) + " - " + kPresets[index].name : juce::String();
}

void SerpentProcessor::mutate (float amount, bool includeRhythm)
{
    static const int skip[] = { vol, xy_x, xy_y, xy_xd, xy_yd, mono, gate_on, arp_on, arp_mode, arp_div, arp_oct,
                                chord, ftype, a_oct, b_oct, a_semi, b_semi, l_sync, l_retrig, dly_pp, pump_div,
                                dist_t, l_dest, gate_div, locut, hicut, uni };
    for (int i = 0; i < NUM_P; ++i)
    {
        if (std::find (std::begin (skip), std::end (skip), i) != std::end (skip)) continue;
        const bool isStep = i >= g0 && i <= g15;
        if (isStep && ! includeRhythm) continue;
        auto* prm = params[(size_t) i];
        if (isStep)
        {
            if (rng.nextFloat() < amount) prm->setValueNotifyingHost (prm->getValue() > 0.5f ? 0.0f : 1.0f);
            continue;
        }
        if (kParams[i].type == PT::Choice || kParams[i].type == PT::Bool) continue;
        // only nudge things that are already doing something, so mutations stay musical
        const float cur = prm->getValue();
        const float jitter = (rng.nextFloat() * 2.0f - 1.0f) * amount;
        prm->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, cur + jitter * (i == cutoff ? 0.6f : 0.35f)));
    }
    if (! patchName.endsWith ("*")) patchName << " *";
    ++presetChangeCounter;
}

bool SerpentProcessor::savePatch (const juce::File& f)
{
    auto state = apvts.copyState();
    state.setProperty ("patchName", f.getFileNameWithoutExtension(), nullptr);
    if (auto xml = state.createXml())
        if (xml->writeTo (f)) { patchName = f.getFileNameWithoutExtension(); ++presetChangeCounter; return true; }
    return false;
}

bool SerpentProcessor::loadPatch (const juce::File& f)
{
    if (auto xml = juce::XmlDocument::parse (f))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        if (tree.isValid() && tree.hasType (apvts.state.getType()))
        {
            apvts.replaceState (tree);
            patchName = f.getFileNameWithoutExtension();
            killNotes.store (true);
            ++presetChangeCounter;
            return true;
        }
    }
    return false;
}

void SerpentProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("patchName", patchName, nullptr);
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void SerpentProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        if (tree.isValid() && tree.hasType (apvts.state.getType()))
        {
            apvts.replaceState (tree);
            patchName = tree.getProperty ("patchName", "Patch").toString();
            currentProgram = (int) tree.getProperty ("program", 0);
            ++presetChangeCounter;
        }
    }
}

juce::AudioProcessorEditor* SerpentProcessor::createEditor() { return new SerpentEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SerpentProcessor(); }
