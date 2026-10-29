#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <deque>

namespace ui
{
namespace col
{
    const juce::Colour bg      { 0xff09090e };
    const juce::Colour panel   { 0xff111119 };
    const juce::Colour panel2  { 0xff181822 };
    const juce::Colour line    { 0xff262634 };
    const juce::Colour text    { 0xffeceaf5 };
    const juce::Colour dim     { 0xff8a8799 };
    const juce::Colour violet  { 0xffa66bff };
    const juce::Colour acid    { 0xffc8ff3c };
    const juce::Colour pink    { 0xffff4fd8 };
    const juce::Colour cyan    { 0xff3de8ff };
}

juce::Font font (float h, bool bold = false);

class Look : public juce::LookAndFeel_V4
{
public:
    Look();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return font (12.5f); }
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return font (12.0f, true); }
    void drawPopupMenuBackground (juce::Graphics&, int w, int h) override;
    juce::Font getPopupMenuFont() override { return font (13.0f); }
    void drawBubble (juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>&) override;
    juce::Font getSliderPopupFont (juce::Slider&) override { return font (12.0f, true); }
};

using APVTS = juce::AudioProcessorValueTreeState;

struct Knob : juce::Component
{
    Knob (APVTS&, const char* id, const juce::String& label, juce::Colour c);
    void paint (juce::Graphics&) override;
    void resized() override;
    juce::Slider slider;
    juce::String label;
    std::unique_ptr<APVTS::SliderAttachment> att;
};

struct Choice : juce::Component
{
    Choice (APVTS&, const char* id, const juce::String& label);
    void paint (juce::Graphics&) override;
    void resized() override;
    juce::ComboBox box;
    juce::String label;
    std::unique_ptr<APVTS::ComboBoxAttachment> att;
};

struct Toggle : juce::Component
{
    Toggle (APVTS&, const char* id, const juce::String& text, juce::Colour c);
    void resized() override { btn.setBounds (getLocalBounds().withSizeKeepingCentre (getWidth(), 24)); }
    juce::TextButton btn;
    std::unique_ptr<APVTS::ButtonAttachment> att;
};

// panel with a title; lays its items out in one row
class Section : public juce::Component
{
public:
    Section (const juce::String& t, juce::Colour c) : title (t), accent (c) {}
    void add (juce::Component* c, int width, bool tall = false) { items.push_back ({ c, width }); if (tall) talls.push_back (c); addAndMakeVisible (c); }
    void setDisplay (juce::Component* d, int width) { display = d; displayW = width; addAndMakeVisible (d); }
    void paint (juce::Graphics&) override;
    void resized() override;
    juce::String title;
    juce::Colour accent;
private:
    std::vector<std::pair<juce::Component*, int>> items;
    std::vector<juce::Component*> talls;
    juce::Component* display = nullptr; int displayW = 0;
};

class Page : public juce::Component
{
public:
    void addRow (std::vector<std::pair<Section*, float>> row, int height) { rows.push_back ({ row, height }); for (auto& r : row) addAndMakeVisible (r.first); }
    void resized() override;
private:
    std::vector<std::pair<std::vector<std::pair<Section*, float>>, int>> rows;
};

// ---- displays
struct WaveDisplay : juce::Component { WaveDisplay (APVTS& s, const char* waveId, const char* lvlId, juce::Colour c); void paint (juce::Graphics&) override; std::atomic<float>* wave; std::atomic<float>* lvl; juce::Colour colour; };
struct FilterDisplay : juce::Component { explicit FilterDisplay (APVTS&); void paint (juce::Graphics&) override; APVTS& s; };
struct EnvDisplay : juce::Component { EnvDisplay (APVTS&, const char* a, const char* d, const char* su, const char* r, juce::Colour); void paint (juce::Graphics&) override; std::atomic<float>* p[4]; juce::Colour colour; };
struct LfoDisplay : juce::Component { LfoDisplay (APVTS&, sp::Engine&); void paint (juce::Graphics&) override; std::atomic<float>* shape; sp::Engine& eng; };
struct PumpMeter : juce::Component { explicit PumpMeter (sp::Engine& e) : eng (e) {} void paint (juce::Graphics&) override; sp::Engine& eng; };

struct Scope : juce::Component { explicit Scope (SerpentProcessor& p) : proc (p) {} void paint (juce::Graphics&) override; SerpentProcessor& proc; };

class XYPad : public juce::Component
{
public:
    XYPad (SerpentProcessor&);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void tick();
private:
    void setFrom (juce::Point<float>);
    SerpentProcessor& proc;
    juce::RangedAudioParameter *px, *py;
    std::deque<juce::Point<float>> trail;
    float phase = 0.0f;
};

class StepButton : public juce::Button
{
public:
    StepButton (int i) : juce::Button ("step"), index (i) { setClickingTogglesState (true); }
    void paintButton (juce::Graphics&, bool over, bool down) override;
    int index; bool playing = false;
};

class Browser : public juce::Component, private juce::ListBoxModel
{
public:
    explicit Browser (SerpentProcessor&);
    void resized() override;
    void paint (juce::Graphics&) override;
    void step (int delta);
    void refilter();
    void syncSelection();
private:
    int getNumRows() override { return filtered.size(); }
    void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void returnKeyPressed (int row) override { if (juce::isPositiveAndBelow (row, filtered.size())) proc.setCurrentProgram (filtered[row]); }
    void selectedRowsChanged (int row) override;
    SerpentProcessor& proc;
    juce::TextEditor search;
    juce::OwnedArray<juce::TextButton> cats;
    juce::ListBox list;
    juce::Array<int> filtered;
    juce::String cat { "All" };
    bool syncing = false;
};

class Logo : public juce::Component { public: void paint (juce::Graphics&) override; };
} // namespace ui

class SerpentEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SerpentEditor (SerpentProcessor&);
    ~SerpentEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void showPage (int);
    ui::Knob* knob (const char* id, const juce::String& l, juce::Colour c);
    ui::Choice* choice (const char* id, const juce::String& l);
    ui::Toggle* toggle (const char* id, const juce::String& t, juce::Colour c);
    ui::Section* section (const juce::String& t, juce::Colour c);
    void setGatePattern (const juce::String& pattern);
    void rotateGate (int dir);

    SerpentProcessor& proc;
    ui::Look look;
    std::unique_ptr<juce::ComponentListener> gridLayout; // must outlive 'owned'
    juce::OwnedArray<juce::Component> owned;

    ui::Logo logo;
    juce::Label presetName, presetCat, info;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, mutateBtn { "MUTATE" }, diceBtn { "DICE" }, saveBtn { "SAVE" }, loadBtn { "LOAD" };
    juce::TextButton tabBtn[3];
    ui::Page pages[3];
    ui::Browser browser;
    ui::XYPad xy;
    ui::Scope scope;
    ui::Choice *xTarget = nullptr, *yTarget = nullptr;
    ui::Knob* masterVol = nullptr;
    juce::MidiKeyboardComponent keyboard;
    std::vector<ui::StepButton*> steps;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> stepAtt;
    juce::OwnedArray<juce::TextButton> patternBtns;
    std::unique_ptr<juce::FileChooser> chooser;
    int lastCounter = -1, page = 0;
};
