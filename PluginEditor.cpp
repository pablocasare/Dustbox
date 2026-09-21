#include "PluginEditor.h"

using namespace dustbox;

// ============================================================ look and feel
ModernLookAndFeel::ModernLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, ui::bg);
    setColour (juce::Label::textColourId, ui::ink);
    setColour (juce::TextEditor::backgroundColourId, ui::card);
    setColour (juce::TextEditor::textColourId, ui::ink);
    setColour (juce::TextEditor::outlineColourId, ui::line);
    setColour (juce::TextEditor::focusedOutlineColourId, ui::accent);
    setColour (juce::TextEditor::highlightColourId, ui::accent.withAlpha (0.25f));
    setColour (juce::CaretComponent::caretColourId, ui::accent);
    setColour (juce::ComboBox::textColourId, ui::ink);
    setColour (juce::ComboBox::arrowColourId, ui::muted);
    setColour (juce::PopupMenu::backgroundColourId, ui::lcd);
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffe8e6e1));
    setColour (juce::PopupMenu::headerTextColourId, ui::accent);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ui::accent);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::textColourOffId, ui::ink);
    setColour (juce::TextButton::textColourOnId, ui::card);
    setColour (juce::Slider::textBoxTextColourId, ui::muted);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId, ui::muted);
}

void ModernLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                          float pos, float a0, float a1, juce::Slider& s)
{
    const auto b = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const float ringR = r - 2.f;

    juce::Path track;
    track.addCentredArc (c.x, c.y, ringR, ringR, 0.f, a0, a1, true);
    g.setColour (ui::track);
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // bipolar controls (eq gain, pitch) fill from the centre
    float from = a0;
    if (s.getMinimum() < 0.0 && s.getMaximum() > 0.0)
        from = a0 + (float) s.valueToProportionOfLength (0.0) * (a1 - a0);
    if (std::abs (ang - from) > 0.01f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, ringR, ringR, 0.f, juce::jmin (from, ang), juce::jmax (from, ang), true);
        g.setColour (s.isEnabled() ? ui::accent : ui::muted);
        g.strokePath (val, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const float kr = r * 0.6f;
    g.setColour (ui::ink);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f);
    g.setColour (ui::card);
    const juce::Point<float> p1 (c.x + std::sin (ang) * kr * 0.25f, c.y - std::cos (ang) * kr * 0.25f);
    const juce::Point<float> p2 (c.x + std::sin (ang) * kr * 0.82f, c.y - std::cos (ang) * kr * 0.82f);
    g.drawLine ({ p1, p2 }, 2.f);
}

void ModernLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    const auto area = b.getLocalBounds().toFloat();
    const auto pill = juce::Rectangle<float> (30.f, 16.f).withCentre (area.getCentre());
    const bool on = b.getToggleState();
    g.setColour (on ? ui::accent : ui::track.darker (over ? 0.08f : 0.f));
    g.fillRoundedRectangle (pill, 8.f);
    const float d = 12.f;
    const float kx = on ? pill.getRight() - d - 2.f : pill.getX() + 2.f;
    g.setColour (juce::Colours::white);
    g.fillEllipse (kx, pill.getY() + 2.f, d, d);
}

void ModernLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                              bool over, bool down)
{
    const auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool accent = b.getProperties()["accent"];
    const bool tab    = b.getProperties()["tab"];
    juce::Colour fill = ui::card;
    if (accent)                          fill = ui::accent;
    else if (tab && b.getToggleState())  fill = ui::ink;
    if (over)  fill = fill.brighter (accent ? 0.08f : 0.f).darker (accent ? 0.f : 0.03f);
    if (down)  fill = fill.darker (0.1f);
    g.setColour (fill);
    g.fillRoundedRectangle (r, 6.f);
    if (! accent && ! (tab && b.getToggleState()))
    {
        g.setColour (ui::line);
        g.drawRoundedRectangle (r, 6.f, 1.f);
    }
}

void ModernLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    const auto r = juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f);
    g.setColour (ui::card);
    g.fillRoundedRectangle (r, 6.f);
    g.setColour (box.hasKeyboardFocus (true) ? ui::accent : ui::line);
    g.drawRoundedRectangle (r, 6.f, 1.f);
    juce::Path chev;
    const float cx = (float) w - 13.f, cy = (float) h * 0.5f;
    chev.startNewSubPath (cx - 4.f, cy - 2.f);
    chev.lineTo (cx, cy + 2.f);
    chev.lineTo (cx + 4.f, cy - 2.f);
    g.setColour (ui::muted);
    g.strokePath (chev, juce::PathStrokeType (1.5f));
}

void ModernLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 0, box.getWidth() - 28, box.getHeight());
    label.setFont (getComboBoxFont (box));
}

juce::Label* ModernLookAndFeel::createSliderTextBox (juce::Slider& s)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (s);
    l->setFont (ui::font (11.f));
    l->setColour (juce::Label::textColourId, ui::muted);
    l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    l->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    return l;
}

// ============================================================ cells
KnobCell::KnobCell (juce::AudioProcessorValueTreeState& s, int i)
{
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, Card::cellW, 14);
    // set directly: the cell is built before it's parented, so it can't rely on the editor's look-and-feel yet
    slider.setColour (juce::Slider::textBoxTextColourId, ui::muted);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxHighlightColourId, ui::accent.withAlpha (0.2f));
    slider.setDoubleClickReturnValue (true, kFloatSpecs[i].def);
    addAndMakeVisible (slider);
    caption.setText (kFloatSpecs[i].label, juce::dontSendNotification);
    caption.setJustificationType (juce::Justification::centred);
    caption.setFont (ui::font (12.f, true));
    addAndMakeVisible (caption);
    attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, kFloatSpecs[i].id, slider);
}
void KnobCell::resized()
{
    auto r = getLocalBounds();
    caption.setBounds (r.removeFromBottom (16));
    slider.setBounds (r);
}

ChoiceCell::ChoiceCell (juce::AudioProcessorValueTreeState& s, int i)
{
    box.addItemList (juce::StringArray::fromTokens (kChoiceSpecs[i].items, "|", ""), 1);
    addAndMakeVisible (box);
    caption.setText (kChoiceSpecs[i].label, juce::dontSendNotification);
    caption.setJustificationType (juce::Justification::centred);
    caption.setFont (ui::font (12.f, true));
    addAndMakeVisible (caption);
    attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (s, kChoiceSpecs[i].id, box);
}
void ChoiceCell::resized()
{
    auto r = getLocalBounds();
    caption.setBounds (r.removeFromBottom (16));
    box.setBounds (r.withSizeKeepingCentre (r.getWidth() - 6, 26).translated (0, -6));
}

// ============================================================ card
void Card::addPower (juce::AudioProcessorValueTreeState& s, const char* id)
{
    power = std::make_unique<juce::ToggleButton>();
    power->setTooltip ("on / off");
    addAndMakeVisible (*power);
    powerAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, id, *power);
}
static int widthOf (const juce::Component* c) { return dynamic_cast<const ChoiceCell*> (c) != nullptr ? Card::choiceW : Card::cellW; }
int Card::preferredWidth() const
{
    int w = pad * 2;
    for (auto* c : cells) w += widthOf (c);
    return juce::jmax (112, w);
}
void Card::refreshEnabled()
{
    const bool on = power == nullptr || power->getToggleState();
    for (auto* c : cells) { c->setAlpha (on ? 1.f : 0.4f); }
}
void Card::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (ui::card);
    g.fillRoundedRectangle (r, 10.f);
    g.setColour (ui::line);
    g.drawRoundedRectangle (r, 10.f, 1.f);
    g.setColour (ui::ink);
    g.setFont (ui::font (13.f, true));
    g.drawText (title, pad, 8, getWidth() - pad * 2 - 40, 16, juce::Justification::centredLeft);
}
void Card::resized()
{
    if (power) power->setBounds (getWidth() - pad - 34, 6, 36, 22);
    int x = pad;
    for (auto* c : cells) { const int w = widthOf (c); c->setBounds (x, header, w, cellH); x += w; }
}

// ============================================================ editor
static const char* const kCharGroups[][2] = {
    { "mtone", "tone" }, { "mlen", "length" }, { "mfx", "effects" } };
static const char* const kSynthGroups[][2] = {
    { "osc", "oscillators" }, { "flt", "filter" }, { "env", "envelopes" }, { "mot", "motion" }, { "out", "output" } };
static const char* const kFxGroups[][3] = {
    { "eq", "eq", "eqOn" }, { "crush", "crush", "crushOn" }, { "phaser", "phaser", "phaserOn" },
    { "chorus", "chorus", "chorusOn" }, { "pump", "pump", "pumpOn" }, { "comp", "compressor", "compOn" },
    { "delay", "delay", "delayOn" }, { "reverb", "reverb", "reverbOn" } };

DustBoxEditor::DustBoxEditor (DustBoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);

    // ---- cards -------------------------------------------------------------
    auto fillCard = [this] (Card* card, const char* group)
    {
        for (int i = 0; i < kNumChoiceParams; ++i)
            if (! kChoiceSpecs[i].toggle && juce::String (kChoiceSpecs[i].group) == group)
                card->addAndMakeVisible (card->cells.add (new ChoiceCell (proc.apvts, i)));
        for (int i = 0; i < kNumFloatParams; ++i)
            if (juce::String (kFloatSpecs[i].group) == group)
                card->addAndMakeVisible (card->cells.add (new KnobCell (proc.apvts, i)));
    };
    for (auto& gname : kCharGroups)
    {
        auto* c = charCards.add (new Card (gname[1]));
        fillCard (c, gname[0]);
        addAndMakeVisible (c);
    }
    for (auto& gname : kSynthGroups)
    {
        auto* c = synthCards.add (new Card (gname[1]));
        fillCard (c, gname[0]);
        addAndMakeVisible (c);
    }
    for (auto& gname : kFxGroups)
    {
        auto* c = fxCards.add (new Card (gname[1]));
        c->addPower (proc.apvts, gname[2]);
        fillCard (c, gname[0]);
        addChildComponent (c);
    }

    // ---- preset browser ------------------------------------------------------
    categoryBox.addItem ("all sounds", 1);
    for (int i = 0; i < kNumCategories; ++i) categoryBox.addItem (juce::String (kCategories[i]).toLowerCase(), i + 2);
    categoryBox.setSelectedId (1, juce::dontSendNotification);
    categoryBox.onChange = [this] { rebuildPresetList(); };
    addAndMakeVisible (categoryBox);

    presetBox.setTextWhenNothingSelected ("choose a preset");
    presetBox.onChange = [this] { if (presetBox.getSelectedId() > 0) choosePreset (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);
    rebuildPresetList();

    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (+1); };
    for (auto* b : { &prevBtn, &nextBtn, &loadBtn, &saveBtn }) addAndMakeVisible (b);

    loadBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Load a patch", juce::File(), "*.json");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                const auto f = fc.getResult();
                if (f.existsAsFile() && proc.loadPatchJson (f)) presetBox.setText (proc.patchName, juce::dontSendNotification);
                refreshReadout();
            });
    };
    saveBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Save this patch",
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                .getChildFile (juce::File::createLegalFileName (proc.patchName) + ".json"), "*.json");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
            [this] (const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File()) return;
                if (! f.hasFileExtension ("json")) f = f.withFileExtension ("json");
                proc.patchInfo = proc.savePatchJson (f) ? "saved " + f.getFileName() : "couldn't write that file";
                refreshReadout();
            });
    };

    // ---- vibe --------------------------------------------------------------
    vibe.setTextToShowWhenEmpty ("describe a sound: rolling pumping bass, dusty chord stab, silky rnb keys...", ui::muted);
    vibe.setFont (ui::font (15.f));
    vibe.setIndents (12, 0);
    vibe.setJustification (juce::Justification::centredLeft);
    vibe.onReturnKey = [this] { makeBtn.triggerClick(); };
    addAndMakeVisible (vibe);

    makeBtn.getProperties().set ("accent", true);
    makeBtn.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    makeBtn.onClick = [this]
    {
        const auto t = vibe.getText().trim();
        if (t.isEmpty()) return;
        proc.makeFromVibe (t);
        presetBox.setText (proc.patchName, juce::dontSendNotification);
        refreshReadout();
    };
    addAndMakeVisible (makeBtn);

    // ---- page tabs -------------------------------------------------------------
    for (auto* t : { &charTab, &synthTab, &fxTab })
    {
        t->getProperties().set ("tab", true);
        t->setClickingTogglesState (false);
        addAndMakeVisible (t);
    }
    charTab.onClick  = [this] { showPage (0); };
    synthTab.onClick = [this] { showPage (1); };
    fxTab.onClick    = [this] { showPage (2); };
    resetBtn.onClick = [this] { proc.resetCharacter(); };
    addAndMakeVisible (resetBtn);

    // ---- keyboard ----------------------------------------------------------------
    keyboard.setAvailableRange (36, 96);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, ui::card);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, ui::ink);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, ui::line);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, ui::accent);
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, ui::accent.withAlpha (0.18f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, ui::card);
    addAndMakeVisible (keyboard);

    showPage (0);
    refreshReadout();
    setResizable (true, true);
    setResizeLimits (1000, 700, 1800, 1200);
    setSize (1060, 740);
    startTimerHz (30);
}

DustBoxEditor::~DustBoxEditor() { stopTimer(); setLookAndFeel (nullptr); }

// ---- preset browsing -------------------------------------------------------------
void DustBoxEditor::rebuildPresetList()
{
    const int cat = categoryBox.getSelectedId() - 2;          // -1 means all
    presetBox.clear (juce::dontSendNotification);
    visiblePresets.clear();
    auto* root = presetBox.getRootMenu();
    for (int c = 0; c < kNumCategories; ++c)
    {
        if (cat >= 0 && c != cat) continue;
        juce::PopupMenu sub;
        int count = 0;
        for (int i = 0; i < kNumPresets; ++i)
            if (kPresets[i].category == c)
            {
                if (cat >= 0) presetBox.addItem (kPresets[i].name, i + 1);   // one category: flat list
                else          sub.addItem (i + 1, kPresets[i].name);         // everything: a flyout per category
                visiblePresets.add (i);
                ++count;
            }
        if (cat < 0) root->addSubMenu (juce::String (kCategories[c]) + "   (" + juce::String (count) + ")", sub);
    }
    const int cur = proc.getCurrentProgram();
    if (visiblePresets.contains (cur)) presetBox.setSelectedId (cur + 1, juce::dontSendNotification);
    else                               presetBox.setText (proc.patchName, juce::dontSendNotification);
}

void DustBoxEditor::choosePreset (int index)
{
    proc.setCurrentProgram (index);
    presetBox.setSelectedId (index + 1, juce::dontSendNotification);
    refreshReadout();
}

void DustBoxEditor::stepPreset (int delta)
{
    if (visiblePresets.isEmpty()) return;
    int pos = visiblePresets.indexOf (proc.getCurrentProgram());
    pos = pos < 0 ? 0 : (pos + delta + visiblePresets.size()) % visiblePresets.size();
    choosePreset (visiblePresets[pos]);
}

void DustBoxEditor::showPage (int p)
{
    page = p;
    charTab.setToggleState (p == 0, juce::dontSendNotification);
    synthTab.setToggleState (p == 1, juce::dontSendNotification);
    fxTab.setToggleState (p == 2, juce::dontSendNotification);
    for (auto* b : { &charTab, &synthTab, &fxTab })
        b->setColour (juce::TextButton::textColourOffId, b->getToggleState() ? ui::card : ui::ink);
    for (auto* c : charCards)  c->setVisible (p == 0);
    for (auto* c : synthCards) c->setVisible (p == 1);
    for (auto* c : fxCards)    c->setVisible (p == 2);
    resetBtn.setVisible (p == 0);
    repaint();
}

void DustBoxEditor::refreshReadout() { repaint (lcdArea); }

// ---- periodic ------------------------------------------------------------------------
void DustBoxEditor::timerCallback()
{
    for (auto* c : fxCards) c->refreshEnabled();

    const int w = proc.scopeWrite.load();
    scopePath.clear();
    const auto a = scopeArea.toFloat();
    const int N = DustBoxProcessor::kScopeSize;
    for (int i = 0; i < N; ++i)
    {
        const float v = juce::jlimit (-1.f, 1.f, proc.scope[(size_t) ((w + i) % N)]);
        const float x = a.getX() + a.getWidth() * (float) i / (float) (N - 1);
        const float y = a.getCentreY() - v * a.getHeight() * 0.48f;
        if (i == 0) scopePath.startNewSubPath (x, y); else scopePath.lineTo (x, y);
    }
    repaint (lcdArea);
}

// ---- drawing ---------------------------------------------------------------------------
void DustBoxEditor::paint (juce::Graphics& g)
{
    g.fillAll (ui::bg);

    // wordmark
    g.setColour (ui::ink);
    g.setFont (ui::font (24.f, true));
    g.drawText ("dust box", 20, 16, 140, 32, juce::Justification::centredLeft);
    g.setColour (ui::accent);
    g.fillEllipse (132.f, 22.f, 7.f, 7.f);

    // lcd
    const auto l = lcdArea.toFloat();
    g.setColour (ui::lcd);
    g.fillRoundedRectangle (l, 12.f);

    auto text = lcdArea.reduced (20, 14);
    text.removeFromRight (scopeArea.getWidth() + 24);
    g.setColour (ui::lcdDim);
    g.setFont (ui::font (11.f, true));
    g.drawText ("patch", text.removeFromTop (14), juce::Justification::centredLeft);
    g.setColour (ui::lcdText);
    g.setFont (ui::font (26.f, true));
    g.drawText (proc.patchName.toLowerCase(), text.removeFromTop (34), juce::Justification::centredLeft, true);
    g.setColour (ui::lcdDim);
    g.setFont (ui::font (12.f));
    g.drawText (proc.patchInfo.toLowerCase(), text.removeFromTop (18), juce::Justification::centredLeft, true);

    g.setColour (ui::lcdDim.withAlpha (0.35f));
    g.drawHorizontalLine (scopeArea.getCentreY(), (float) scopeArea.getX(), (float) scopeArea.getRight());
    g.setColour (ui::lcdText);
    g.strokePath (scopePath, juce::PathStrokeType (1.6f));
    g.setColour (ui::lcdDim);
    g.setFont (ui::font (11.f));
    g.drawText (juce::String (proc.hostBpm(), 1) + " bpm   /   " + juce::String (proc.activeVoiceCount()) + " voices",
                scopeArea.withY (lcdArea.getBottom() - 24).withHeight (14), juce::Justification::centredRight);
}

void DustBoxEditor::layoutCards (juce::OwnedArray<Card>& cards, juce::Rectangle<int> area)
{
    const int gap = 12, h = Card::header + Card::cellH + 10;
    int x = area.getX(), y = area.getY();
    // flow into rows, then stretch each row's cards to fill the width
    juce::Array<Card*> row;
    auto flush = [&]
    {
        if (row.isEmpty()) return;
        int used = 0; for (auto* c : row) used += c->preferredWidth();
        const int spare = area.getWidth() - used - gap * (row.size() - 1);
        int cx = area.getX();
        for (auto* c : row)
        {
            const int w = c->preferredWidth() + spare * c->preferredWidth() / juce::jmax (1, used);
            c->setBounds (cx, y, w, h);
            cx += w + gap;
        }
        row.clear(); y += h + gap; x = area.getX();
    };
    for (auto* c : cards)
    {
        if (x + c->preferredWidth() > area.getRight() && ! row.isEmpty()) flush();
        row.add (c); x += c->preferredWidth() + gap;
    }
    flush();
}

void DustBoxEditor::resized()
{
    auto r = getLocalBounds().reduced (20, 16);

    auto top = r.removeFromTop (32);
    top.removeFromLeft (160);
    saveBtn.setBounds (top.removeFromRight (64)); top.removeFromRight (8);
    loadBtn.setBounds (top.removeFromRight (64)); top.removeFromRight (20);
    nextBtn.setBounds (top.removeFromRight (32)); top.removeFromRight (6);
    prevBtn.setBounds (top.removeFromRight (32)); top.removeFromRight (8);
    presetBox.setBounds (top.removeFromRight (juce::jmin (320, top.getWidth() - 170))); top.removeFromRight (8);
    categoryBox.setBounds (top.removeFromRight (juce::jmin (170, top.getWidth())));

    r.removeFromTop (14);
    lcdArea = r.removeFromTop (96);
    scopeArea = lcdArea.reduced (20, 16).removeFromRight (280).withTrimmedBottom (14);

    r.removeFromTop (14);
    auto vibeRow = r.removeFromTop (42);
    makeBtn.setBounds (vibeRow.removeFromRight (96)); vibeRow.removeFromRight (8);
    vibe.setBounds (vibeRow);

    r.removeFromTop (16);
    auto tabs = r.removeFromTop (30);
    charTab.setBounds (tabs.removeFromLeft (96)); tabs.removeFromLeft (6);
    synthTab.setBounds (tabs.removeFromLeft (84)); tabs.removeFromLeft (6);
    fxTab.setBounds (tabs.removeFromLeft (84));
    resetBtn.setBounds (tabs.removeFromRight (140));

    auto keys = r.removeFromBottom (72);
    keyboard.setBounds (keys);
    keyboard.setKeyWidth ((float) keys.getWidth() / 36.f);

    r.removeFromTop (12);
    layoutCards (charCards, r);
    layoutCards (synthCards, r);
    layoutCards (fxCards, r);
}
