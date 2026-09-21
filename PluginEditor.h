#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

// ---------------------------------------------------------------- palette
namespace ui {
    const juce::Colour bg       { 0xffe6e4de };
    const juce::Colour card     { 0xfff4f3ef };
    const juce::Colour line     { 0xffd3d0c8 };
    const juce::Colour ink      { 0xff141414 };
    const juce::Colour muted    { 0xff86837b };
    const juce::Colour track    { 0xffdad7cf };
    const juce::Colour accent   { 0xffff5a1f };
    const juce::Colour lcd      { 0xff121212 };
    const juce::Colour lcdText  { 0xffff7a45 };
    const juce::Colour lcdDim   { 0xff6f6a62 };

    inline juce::Font font (float h, bool bold = false)
    { return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain)); }
}

// ---------------------------------------------------------------- look and feel
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return ui::font (13.f); }
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return ui::font (13.f, true); }
    juce::Font getPopupMenuFont() override { return ui::font (14.f); }
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Label* createSliderTextBox (juce::Slider&) override;
};

// ---------------------------------------------------------------- controls
struct KnobCell : public juce::Component
{
    juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label caption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    KnobCell (juce::AudioProcessorValueTreeState&, int paramIndex);
    void resized() override;
};

struct ChoiceCell : public juce::Component
{
    juce::ComboBox box;
    juce::Label caption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attach;
    ChoiceCell (juce::AudioProcessorValueTreeState&, int choiceIndex);
    void resized() override;
};

// A titled panel holding cells, with an optional power switch for FX modules.
struct Card : public juce::Component
{
    juce::String title;
    std::unique_ptr<juce::ToggleButton> power;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttach;
    juce::OwnedArray<juce::Component> cells;

    Card (const juce::String& t) : title (t) {}
    void addPower (juce::AudioProcessorValueTreeState&, const char* paramId);
    int  preferredWidth() const;
    void refreshEnabled();
    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int cellW = 64, choiceW = 84, cellH = 96, pad = 12, header = 30;
};

// ---------------------------------------------------------------- editor
class DustBoxEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DustBoxEditor (DustBoxProcessor&);
    ~DustBoxEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildPresetList();
    void choosePreset (int index);
    void stepPreset (int delta);
    void showPage (int page);
    void refreshReadout();
    void layoutCards (juce::OwnedArray<Card>&, juce::Rectangle<int>);

    DustBoxProcessor& proc;
    ModernLookAndFeel lnf;

    juce::ComboBox categoryBox, presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, loadBtn { "load" }, saveBtn { "save" };
    juce::TextEditor vibe;
    juce::TextButton makeBtn { "make" };
    juce::TextButton charTab { "character" }, synthTab { "synth" }, fxTab { "fx" };
    juce::TextButton resetBtn { "reset character" };
    juce::OwnedArray<Card> charCards, synthCards, fxCards;
    juce::MidiKeyboardComponent keyboard;
    std::unique_ptr<juce::FileChooser> chooser;

    juce::Rectangle<int> lcdArea, scopeArea;
    juce::Path scopePath;
    juce::Array<int> visiblePresets;     // preset indices in the current filter
    int page = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DustBoxEditor)
};
