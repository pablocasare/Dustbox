#include "PluginEditor.h"
#include <complex>
#include <algorithm>
#include <cmath>

using namespace ui;

//==============================================================================
juce::Font ui::font (float h, bool bold)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}

Look::Look()
{
    setColour (juce::ResizableWindow::backgroundColourId, col::bg);
    setColour (juce::ComboBox::textColourId, col::text);
    setColour (juce::ComboBox::arrowColourId, col::dim);
    setColour (juce::PopupMenu::backgroundColourId, col::panel2);
    setColour (juce::PopupMenu::textColourId, col::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, col::violet.withAlpha (0.35f));
    setColour (juce::PopupMenu::highlightedTextColourId, col::text);
    setColour (juce::TextButton::textColourOffId, col::text);
    setColour (juce::TextButton::textColourOnId, col::bg);
    setColour (juce::TextButton::buttonOnColourId, col::violet);
    setColour (juce::TextEditor::backgroundColourId, col::panel2);
    setColour (juce::TextEditor::textColourId, col::text);
    setColour (juce::TextEditor::outlineColourId, col::line);
    setColour (juce::TextEditor::focusedOutlineColourId, col::violet);
    setColour (juce::TextEditor::highlightColourId, col::violet.withAlpha (0.4f));
    setColour (juce::CaretComponent::caretColourId, col::acid);
    setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId, col::line.brighter (0.3f));
    setColour (juce::Label::textColourId, col::text);
    setColour (juce::BubbleComponent::backgroundColourId, col::panel2);
    setColour (juce::BubbleComponent::outlineColourId, col::violet);
    setColour (juce::TooltipWindow::textColourId, col::text);
}

void Look::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    const auto c = s.findColour (juce::Slider::rotarySliderFillColourId);
    const float r = (float) juce::jmin (w, h) * 0.5f - 5.0f;
    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float ang = a0 + pos * (a1 - a0);

    // body
    juce::ColourGradient body (col::panel2.brighter (0.12f), cx, cy - r, col::bg, cx, cy + r, false);
    g.setGradientFill (body);
    g.fillEllipse (cx - r * 0.74f, cy - r * 0.74f, r * 1.48f, r * 1.48f);
    g.setColour (col::line);
    g.drawEllipse (cx - r * 0.74f, cy - r * 0.74f, r * 1.48f, r * 1.48f, 1.0f);

    juce::Path track; track.addCentredArc (cx, cy, r, r, 0.0f, a0, a1, true);
    g.setColour (col::line.brighter (0.15f));
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    float from = a0;
    if (s.getMinimum() < 0.0 && s.getMaximum() > 0.0)
        from = a0 + (float) s.valueToProportionOfLength (0.0) * (a1 - a0);
    if (std::abs (ang - from) > 0.001f)
    {
        juce::Path val; val.addCentredArc (cx, cy, r, r, 0.0f, juce::jmin (from, ang), juce::jmax (from, ang), true);
        g.setColour (c.withAlpha (0.22f));
        g.strokePath (val, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (c);
        g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    const juce::Point<float> p1 (cx + std::sin (ang) * r * 0.25f, cy - std::cos (ang) * r * 0.25f);
    const juce::Point<float> p2 (cx + std::sin (ang) * r * 0.62f, cy - std::cos (ang) * r * 0.62f);
    g.setColour (s.isMouseOverOrDragging() ? c : col::text);
    g.drawLine ({ p1, p2 }, 2.2f);
}

void Look::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    auto b = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setColour (col::panel2);
    g.fillRoundedRectangle (b, 6.0f);
    g.setColour (box.isMouseOver (true) ? col::violet : col::line);
    g.drawRoundedRectangle (b, 6.0f, 1.0f);
    juce::Path arrow;
    const float ax = (float) w - 14.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4, ay - 2, ax + 4, ay - 2, ax, ay + 3);
    g.setColour (col::dim);
    g.fillPath (arrow);
}

void Look::positionComboBoxText (juce::ComboBox& box, juce::Label& l)
{
    l.setBounds (6, 0, box.getWidth() - 22, box.getHeight());
    l.setFont (getComboBoxFont (box));
}

void Look::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    const auto accent = b.findColour (juce::TextButton::buttonOnColourId);
    if (on)
    {
        g.setColour (accent.withAlpha (0.25f));
        g.fillRoundedRectangle (r.expanded (1.5f), 8.0f);
        g.setColour (accent);
        g.fillRoundedRectangle (r, 7.0f);
    }
    else
    {
        g.setColour (down ? col::line : col::panel2);
        g.fillRoundedRectangle (r, 7.0f);
        g.setColour (over ? accent : col::line);
        g.drawRoundedRectangle (r, 7.0f, 1.0f);
    }
}

void Look::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (col::panel2);
    g.setColour (col::line);
    g.drawRect (0, 0, w, h);
}

void Look::drawBubble (juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>& body)
{
    g.setColour (col::panel2);
    g.fillRoundedRectangle (body, 5.0f);
    g.setColour (col::violet);
    g.drawRoundedRectangle (body, 5.0f, 1.0f);
}

//==============================================================================
Knob::Knob (APVTS& s, const char* id, const juce::String& l, juce::Colour c) : label (l)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled (true, true, nullptr, 2000);
    slider.setColour (juce::Slider::rotarySliderFillColourId, c);
    slider.setColour (juce::Slider::textBoxTextColourId, col::text);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    slider.setMouseDragSensitivity (220);
    addAndMakeVisible (slider);
    att = std::make_unique<APVTS::SliderAttachment> (s, id, slider);
    slider.setDoubleClickReturnValue (true, s.getParameter (id)->convertFrom0to1 (s.getParameter (id)->getDefaultValue()));
}
void Knob::paint (juce::Graphics& g)
{
    g.setColour (col::dim);
    g.setFont (font (11.0f));
    g.drawFittedText (label, getLocalBounds().removeFromBottom (16), juce::Justification::centred, 1);
}
void Knob::resized() { slider.setBounds (getLocalBounds().withTrimmedBottom (14)); }

Choice::Choice (APVTS& s, const char* id, const juce::String& l) : label (l)
{
    if (auto* prm = dynamic_cast<juce::AudioParameterChoice*> (s.getParameter (id)))
        box.addItemList (prm->choices, 1);
    addAndMakeVisible (box);
    att = std::make_unique<APVTS::ComboBoxAttachment> (s, id, box);
}
void Choice::paint (juce::Graphics& g)
{
    g.setColour (col::dim);
    g.setFont (font (11.0f));
    g.drawFittedText (label, getLocalBounds().removeFromBottom (16), juce::Justification::centred, 1);
}
void Choice::resized()
{
    auto r = getLocalBounds().withTrimmedBottom (16);
    box.setBounds (r.withSizeKeepingCentre (r.getWidth() - 4, 26));
}

Toggle::Toggle (APVTS& s, const char* id, const juce::String& t, juce::Colour c)
{
    btn.setButtonText (t);
    btn.setClickingTogglesState (true);
    btn.setColour (juce::TextButton::buttonOnColourId, c);
    addAndMakeVisible (btn);
    att = std::make_unique<APVTS::ButtonAttachment> (s, id, btn);
}

void Section::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (col::panel);
    g.fillRoundedRectangle (r, 10.0f);
    g.setColour (col::line);
    g.drawRoundedRectangle (r, 10.0f, 1.0f);
    g.setColour (accent);
    g.fillRoundedRectangle (12.0f, 11.0f, 3.0f, 10.0f, 1.5f);
    g.setFont (font (11.5f, true));
    g.setColour (col::text);
    g.drawText (title, 20, 6, 300, 20, juce::Justification::centredLeft);
}

void Section::resized()
{
    auto r = getLocalBounds().reduced (10, 8).withTrimmedTop (20);
    if (display) { display->setBounds (r.removeFromLeft (displayW).reduced (0, 2)); r.removeFromLeft (8); }
    int total = 0; for (auto& it : items) total += it.second;
    const int spare = juce::jmax (0, r.getWidth() - total);
    const int gap = items.size() > 1 ? juce::jmin (14, spare / (int) items.size()) : 0;
    for (auto& it : items)
    {
        const bool tall = std::find (talls.begin(), talls.end(), it.first) != talls.end();
        it.first->setBounds (r.removeFromLeft (it.second).withSizeKeepingCentre (it.second, tall ? r.getHeight() : juce::jmin (r.getHeight(), 84)));
        r.removeFromLeft (gap);
    }
}

void Page::resized()
{
    auto r = getLocalBounds();
    for (auto& row : rows)
    {
        auto rr = r.removeFromTop (row.second);
        r.removeFromTop (8);
        float tot = 0; for (auto& s : row.first) tot += s.second;
        const int W = rr.getWidth() - 8 * ((int) row.first.size() - 1);
        for (size_t i = 0; i < row.first.size(); ++i)
        {
            const int w = i + 1 == row.first.size() ? rr.getWidth() : (int) ((float) W * row.first[i].second / tot);
            row.first[i].first->setBounds (rr.removeFromLeft (w));
            rr.removeFromLeft (8);
        }
    }
}

//==============================================================================
static void glowPath (juce::Graphics& g, const juce::Path& p, juce::Colour c, float w)
{
    g.setColour (c.withAlpha (0.18f));
    g.strokePath (p, juce::PathStrokeType (w * 3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (c);
    g.strokePath (p, juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

static void screen (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (col::bg);
    g.fillRoundedRectangle (r, 7.0f);
    g.setColour (col::line.withAlpha (0.6f));
    for (int i = 1; i < 4; ++i) g.drawHorizontalLine ((int) (r.getY() + r.getHeight() * (float) i / 4.0f), r.getX() + 4, r.getRight() - 4);
    for (int i = 1; i < 6; ++i) g.drawVerticalLine ((int) (r.getX() + r.getWidth() * (float) i / 6.0f), r.getY() + 4, r.getBottom() - 4);
    g.setColour (col::line);
    g.drawRoundedRectangle (r, 7.0f, 1.0f);
}

WaveDisplay::WaveDisplay (APVTS& s, const char* w, const char* l, juce::Colour c)
    : wave (s.getRawParameterValue (w)), lvl (s.getRawParameterValue (l)), colour (c) {}

void WaveDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    screen (g, r);
    const auto& wt = sp::Wavetables::get();
    const float wv = wave->load();
    const float amp = 0.25f + 0.6f * juce::jlimit (0.0f, 1.0f, lvl->load());
    juce::Path p;
    const int N = 160;
    for (int i = 0; i <= N; ++i)
    {
        const float ph = (float) i / (float) N;
        const float v = wt.read (juce::jlimit (0.0f, 6.999f, wv), std::fmod (ph * 2.0f, 1.0f), 5);
        const float x = r.getX() + 6 + ph * (r.getWidth() - 12);
        const float y = r.getCentreY() - v * amp * r.getHeight() * 0.42f;
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    glowPath (g, p, colour, 1.8f);
    const int s0 = (int) wv; const int s1 = juce::jmin (7, s0 + 1);
    juce::String nm = sp::Wavetables::shapeName (s0);
    if (wv - (float) s0 > 0.05f && s0 < 7) nm << " > " << sp::Wavetables::shapeName (s1);
    g.setColour (col::dim);
    g.setFont (font (10.5f));
    g.drawText (nm, r.reduced (8, 4), juce::Justification::bottomLeft);
}

FilterDisplay::FilterDisplay (APVTS& st) : s (st) {}
void FilterDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    screen (g, r);
    const float fc = s.getRawParameterValue ("cutoff")->load();
    const float rs = s.getRawParameterValue ("reso")->load();
    const int t = (int) s.getRawParameterValue ("ftype")->load();
    const float k = 2.0f - 1.94f * rs;
    juce::Path p, fill;
    const int N = 120;
    for (int i = 0; i <= N; ++i)
    {
        const float f = 20.0f * std::pow (1000.0f, (float) i / (float) N);
        const std::complex<float> sj (0.0f, f / fc);
        auto den = [&] (float kk) { return sj * sj + kk * sj + 1.0f; };
        std::complex<float> H;
        if (t == 0) H = (1.0f / den (k)) * (1.0f / den (1.414f));
        else if (t == 1) H = 1.0f / den (k);
        else if (t == 2) H = (sj / den (k)) * (0.5f + k * 0.5f);
        else H = sj * sj / den (k);
        const float db = juce::jlimit (-40.0f, 24.0f, 20.0f * std::log10 (std::abs (H) + 1.0e-6f));
        const float x = r.getX() + r.getWidth() * (float) i / (float) N;
        const float y = r.getY() + r.getHeight() * (1.0f - (db + 40.0f) / 64.0f);
        if (i == 0) { p.startNewSubPath (x, y); fill.startNewSubPath (x, r.getBottom()); }
        else p.lineTo (x, y);
        fill.lineTo (x, y);
    }
    fill.lineTo (r.getRight(), r.getBottom()); fill.closeSubPath();
    g.setGradientFill (juce::ColourGradient (col::cyan.withAlpha (0.28f), 0, r.getY(), col::cyan.withAlpha (0.0f), 0, r.getBottom(), false));
    g.fillPath (fill);
    glowPath (g, p, col::cyan, 1.8f);
    g.setColour (col::dim); g.setFont (font (10.5f));
    g.drawText (fc >= 1000.0f ? juce::String (fc / 1000.0f, 1) + " kHz" : juce::String ((int) fc) + " Hz", r.reduced (8, 4), juce::Justification::bottomRight);
}

EnvDisplay::EnvDisplay (APVTS& s, const char* a, const char* d, const char* su, const char* rl, juce::Colour c) : colour (c)
{
    p[0] = s.getRawParameterValue (a); p[1] = s.getRawParameterValue (d);
    p[2] = s.getRawParameterValue (su); p[3] = s.getRawParameterValue (rl);
}
void EnvDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    screen (g, r);
    auto seg = [] (float t) { return std::log1p (t * 12.0f) + 0.05f; };
    const float A = seg (p[0]->load()), D = seg (p[1]->load()), S = 0.6f, R = seg (p[3]->load());
    const float tot = A + D + S + R;
    const float su = p[2]->load();
    auto b = r.reduced (6, 8);
    auto X = [&] (float t) { return b.getX() + b.getWidth() * t / tot; };
    auto Y = [&] (float v) { return b.getBottom() - b.getHeight() * v; };
    juce::Path pth;
    pth.startNewSubPath (X (0), Y (0));
    pth.lineTo (X (A), Y (1));
    pth.quadraticTo (X (A + D * 0.25f), Y (su + (1.0f - su) * 0.15f), X (A + D), Y (su));
    pth.lineTo (X (A + D + S), Y (su));
    pth.quadraticTo (X (A + D + S + R * 0.25f), Y (su * 0.15f), X (tot), Y (0));
    juce::Path fill (pth); fill.lineTo (X (0), Y (0)); fill.closeSubPath();
    g.setGradientFill (juce::ColourGradient (colour.withAlpha (0.3f), 0, b.getY(), colour.withAlpha (0.0f), 0, b.getBottom(), false));
    g.fillPath (fill);
    glowPath (g, pth, colour, 1.8f);
}

LfoDisplay::LfoDisplay (APVTS& s, sp::Engine& e) : shape (s.getRawParameterValue ("l_shape")), eng (e) {}
void LfoDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    screen (g, r);
    const int sh = (int) shape->load();
    juce::Path p;
    juce::Random rr (7);
    float shv = 0.0f;
    const int N = 120;
    for (int i = 0; i <= N; ++i)
    {
        float ph = std::fmod ((float) i / (float) N * 2.0f, 1.0f);
        float v = 0;
        switch (sh)
        {
            case 0: v = std::sin (sp::kTwoPi * ph); break;
            case 1: v = 1.0f - 4.0f * std::abs (ph - 0.5f); break;
            case 2: v = 2.0f * ph - 1.0f; break;
            case 3: v = 1.0f - 2.0f * ph; break;
            case 4: v = ph < 0.5f ? 1.0f : -1.0f; break;
            default: if (i % 15 == 0) shv = rr.nextFloat() * 2.0f - 1.0f; v = shv; break;
        }
        const float x = r.getX() + 6 + (r.getWidth() - 12) * (float) i / (float) N;
        const float y = r.getCentreY() - v * r.getHeight() * 0.36f;
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    glowPath (g, p, col::pink, 1.6f);
    const float v = eng.uiLfo.load();
    const float y = r.getCentreY() - v * r.getHeight() * 0.36f;
    g.setColour (col::acid);
    g.fillEllipse (r.getRight() - 14, y - 4, 8, 8);
}

void PumpMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    screen (g, r);
    const float v = eng.uiPump.load();
    auto bar = r.reduced (8).withTrimmedTop ((r.getHeight() - 16) * (1.0f - v));
    g.setGradientFill (juce::ColourGradient (col::acid, 0, r.getY(), col::violet, 0, r.getBottom(), false));
    g.fillRoundedRectangle (bar, 4.0f);
}

void Scope::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    screen (g, r);
    const int S = SerpentProcessor::SCOPE;
    const int w = proc.scopeWrite.load();
    std::vector<float> buf ((size_t) S);
    for (int i = 0; i < S; ++i) buf[(size_t) i] = proc.scope[(size_t) ((w + i) % S)];
    // trigger on a rising zero crossing so the wave stands still
    int start = S / 2;
    for (int i = S / 2; i > 4; --i) if (buf[(size_t) i - 1] < 0.0f && buf[(size_t) i] >= 0.0f) { start = i; break; }
    const int len = juce::jmin (700, S - start);
    float pk = 0.05f;
    for (int i = 0; i < S; ++i) pk = juce::jmax (pk, std::abs (buf[(size_t) i]));
    juce::Path p;
    for (int i = 0; i < len; ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) len;
        const float y = r.getCentreY() - buf[(size_t) (start + i)] / pk * r.getHeight() * 0.4f;
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    g.setGradientFill (juce::ColourGradient (col::violet, r.getX(), 0, col::acid, r.getRight(), 0, false));
    g.strokePath (p, juce::PathStrokeType (1.6f));
}

//==============================================================================
XYPad::XYPad (SerpentProcessor& p) : proc (p)
{
    px = proc.apvts.getParameter ("xy_x");
    py = proc.apvts.getParameter ("xy_y");
    setMouseCursor (juce::MouseCursor::CrosshairCursor);
}

void XYPad::tick()
{
    phase += 0.03f; if (phase > 1.0f) phase -= 1.0f;
    const auto pt = juce::Point<float> (px->getValue(), py->getValue());
    if (trail.empty() || trail.back().getDistanceFrom (pt) > 0.002f) trail.push_back (pt);
    else if (! trail.empty()) trail.pop_front();
    while (trail.size() > 40) trail.pop_front();
    repaint();
}

void XYPad::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    juce::ColourGradient bgGrad (col::violet.withAlpha (0.16f), r.getX(), r.getBottom(), col::acid.withAlpha (0.10f), r.getRight(), r.getY(), false);
    g.setColour (col::bg); g.fillRoundedRectangle (r, 12.0f);
    g.setGradientFill (bgGrad); g.fillRoundedRectangle (r, 12.0f);
    g.setColour (col::line.withAlpha (0.7f));
    for (int i = 1; i < 8; ++i)
    {
        g.drawHorizontalLine ((int) (r.getY() + r.getHeight() * (float) i / 8.0f), r.getX(), r.getRight());
        g.drawVerticalLine ((int) (r.getX() + r.getWidth() * (float) i / 8.0f), r.getY(), r.getBottom());
    }
    g.setColour (col::line); g.drawRoundedRectangle (r.reduced (0.5f), 12.0f, 1.0f);

    auto toPx = [&] (juce::Point<float> v) { return juce::Point<float> (r.getX() + 14 + v.x * (r.getWidth() - 28), r.getBottom() - 14 - v.y * (r.getHeight() - 28)); };
    for (size_t i = 0; i < trail.size(); ++i)
    {
        const float a = (float) i / (float) trail.size();
        const auto q = toPx (trail[i]);
        g.setColour (col::acid.withAlpha (a * 0.35f));
        g.fillEllipse (q.x - 3 * a, q.y - 3 * a, 6 * a, 6 * a);
    }
    const auto c = toPx ({ px->getValue(), py->getValue() });
    g.setColour (col::violet.withAlpha (0.5f));
    g.drawLine (c.x, r.getY() + 8, c.x, r.getBottom() - 8, 0.8f);
    g.drawLine (r.getX() + 8, c.y, r.getRight() - 8, c.y, 0.8f);
    for (int i = 0; i < 3; ++i)
    {
        const float ph = std::fmod (phase + (float) i / 3.0f, 1.0f);
        const float rad = 8.0f + ph * 34.0f;
        g.setColour (col::acid.withAlpha ((1.0f - ph) * 0.35f));
        g.drawEllipse (c.x - rad, c.y - rad, rad * 2, rad * 2, 1.2f);
    }
    g.setColour (col::acid.withAlpha (0.25f)); g.fillEllipse (c.x - 16, c.y - 16, 32, 32);
    g.setColour (col::acid); g.fillEllipse (c.x - 8, c.y - 8, 16, 16);
    g.setColour (col::bg); g.fillEllipse (c.x - 3, c.y - 3, 6, 6);

    auto* xd = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter ("xy_xd"));
    auto* yd = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter ("xy_yd"));
    g.setFont (font (11.0f, true));
    g.setColour (col::text.withAlpha (0.8f));
    g.drawText ("X  " + xd->getCurrentChoiceName().toUpperCase(), r.reduced (12, 8), juce::Justification::bottomRight);
    g.drawText ("Y  " + yd->getCurrentChoiceName().toUpperCase(), r.reduced (12, 8), juce::Justification::topLeft);
}

void XYPad::setFrom (juce::Point<float> pos)
{
    auto r = getLocalBounds().toFloat();
    const float x = juce::jlimit (0.0f, 1.0f, (pos.x - r.getX() - 14) / (r.getWidth() - 28));
    const float y = juce::jlimit (0.0f, 1.0f, (r.getBottom() - 14 - pos.y) / (r.getHeight() - 28));
    px->setValueNotifyingHost (x);
    py->setValueNotifyingHost (y);
}
void XYPad::mouseDown (const juce::MouseEvent& e) { px->beginChangeGesture(); py->beginChangeGesture(); setFrom (e.position); }
void XYPad::mouseDrag (const juce::MouseEvent& e) { setFrom (e.position); }
void XYPad::mouseUp (const juce::MouseEvent&) { px->endChangeGesture(); py->endChangeGesture(); }
void XYPad::mouseDoubleClick (const juce::MouseEvent&)
{
    px->beginChangeGesture(); py->beginChangeGesture();
    px->setValueNotifyingHost (0.0f); py->setValueNotifyingHost (0.0f);
    px->endChangeGesture(); py->endChangeGesture();
}

void StepButton::paintButton (juce::Graphics& g, bool over, bool)
{
    auto r = getLocalBounds().toFloat().reduced (2.0f);
    const bool on = getToggleState();
    const bool beat = index % 4 == 0;
    if (on)
    {
        g.setGradientFill (juce::ColourGradient (col::acid, 0, r.getY(), col::acid.darker (0.5f), 0, r.getBottom(), false));
        g.fillRoundedRectangle (r, 4.0f);
    }
    else
    {
        g.setColour (beat ? col::panel2.brighter (0.08f) : col::panel2);
        g.fillRoundedRectangle (r, 4.0f);
    }
    if (playing)
    {
        g.setColour (col::text.withAlpha (on ? 0.9f : 0.5f));
        g.drawRoundedRectangle (r.expanded (1.0f), 5.0f, 2.0f);
    }
    else if (over)
    {
        g.setColour (col::acid.withAlpha (0.5f));
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
    }
}

//==============================================================================
Browser::Browser (SerpentProcessor& p) : proc (p)
{
    search.setTextToShowWhenEmpty ("search " + juce::String (SerpentProcessor::numPresets()) + " presets...", col::dim);
    search.setFont (font (13.0f));
    search.setIndents (8, 6);
    search.onTextChange = [this] { refilter(); };
    addAndMakeVisible (search);

    juce::StringArray c = SerpentProcessor::categories();
    c.insert (0, "All");
    for (auto& name : c)
    {
        auto* b = cats.add (new juce::TextButton (name));
        b->setClickingTogglesState (true);
        b->setRadioGroupId (77);
        b->setColour (juce::TextButton::buttonOnColourId, col::violet);
        b->onClick = [this, b, name] { if (b->getToggleState()) { cat = name; refilter(); } };
        addAndMakeVisible (b);
    }
    cats[0]->setToggleState (true, juce::dontSendNotification);

    list.setModel (this);
    list.setRowHeight (24);
    list.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (list);
    refilter();
}

void Browser::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (col::panel); g.fillRoundedRectangle (r, 10.0f);
    g.setColour (col::line); g.drawRoundedRectangle (r, 10.0f, 1.0f);
    g.setColour (col::dim); g.setFont (font (10.5f));
    g.drawText (juce::String (filtered.size()) + " SOUNDS", getLocalBounds().reduced (12, 6), juce::Justification::bottomRight);
}

void Browser::resized()
{
    auto r = getLocalBounds().reduced (10);
    search.setBounds (r.removeFromTop (30));
    r.removeFromTop (8);
    const int cols = 2, rowsN = (cats.size() + 1) / 2;
    auto grid = r.removeFromTop (rowsN * 27);
    const int cw = (grid.getWidth() - 6) / cols;
    for (int i = 0; i < cats.size(); ++i)
        cats[i]->setBounds (grid.getX() + (i % cols) * (cw + 6), grid.getY() + (i / cols) * 27, cw, 23);
    r.removeFromTop (8);
    list.setBounds (r.withTrimmedBottom (16));
}

void Browser::refilter()
{
    filtered.clear();
    auto toks = juce::StringArray::fromTokens (search.getText().toLowerCase(), " ", "");
    toks.removeEmptyStrings();
    for (int i = 0; i < SerpentProcessor::numPresets(); ++i)
    {
        const auto& p = SerpentProcessor::preset (i);
        if (cat != "All" && cat != p.category) continue;
        const auto hay = (juce::String (p.name) + " " + p.category).toLowerCase();
        bool ok = true;
        for (auto& t : toks) if (! hay.contains (t)) { ok = false; break; }
        if (ok) filtered.add (i);
    }
    list.updateContent();
    syncSelection();
    repaint();
}

void Browser::syncSelection()
{
    syncing = true;
    const int row = filtered.indexOf (proc.getCurrentProgram());
    if (row >= 0) { list.selectRow (row); }
    else list.deselectAllRows();
    syncing = false;
    list.repaint();
}

void Browser::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (! juce::isPositiveAndBelow (row, filtered.size())) return;
    const int idx = filtered[row];
    const auto& p = SerpentProcessor::preset (idx);
    if (selected)
    {
        g.setColour (col::violet.withAlpha (0.22f));
        g.fillRoundedRectangle (1.0f, 1.0f, (float) w - 2.0f, (float) h - 2.0f, 5.0f);
        g.setColour (col::acid);
        g.fillRoundedRectangle (2.0f, 6.0f, 3.0f, (float) h - 12.0f, 1.5f);
    }
    g.setColour (selected ? col::text : col::text.withAlpha (0.82f));
    g.setFont (font (12.5f, selected));
    const int catW = cat == "All" ? 74 : 0;
    g.drawText (p.name, 12, 0, w - 16 - catW, h, juce::Justification::centredLeft, true);
    if (cat == "All")
    {
        g.setColour (col::dim.withAlpha (0.7f));
        g.setFont (font (10.0f));
        g.drawFittedText (juce::String (p.category).toUpperCase(), w - catW - 6, 0, catW, h, juce::Justification::centredRight, 1, 0.8f);
    }
}

void Browser::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (juce::isPositiveAndBelow (row, filtered.size())) proc.setCurrentProgram (filtered[row]);
}

void Browser::selectedRowsChanged (int row)
{
    if (syncing || ! juce::isPositiveAndBelow (row, filtered.size())) return;
    if (filtered[row] != proc.getCurrentProgram()) proc.setCurrentProgram (filtered[row]);
}

void Browser::step (int d)
{
    if (filtered.isEmpty()) return;
    int row = filtered.indexOf (proc.getCurrentProgram());
    row = row < 0 ? 0 : (row + d + filtered.size()) % filtered.size();
    proc.setCurrentProgram (filtered[row]);
    list.scrollToEnsureRowIsOnscreen (row);
}

void Logo::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    // the snake: one S curve with a head
    const float h = r.getHeight();
    juce::Path s;
    const float x0 = r.getX() + 4, cy = r.getCentreY();
    s.startNewSubPath (x0 + 22, cy - h * 0.30f);
    s.cubicTo (x0 - 2, cy - h * 0.36f, x0 - 2, cy - h * 0.02f, x0 + 13, cy);
    s.cubicTo (x0 + 28, cy + h * 0.02f, x0 + 28, cy + h * 0.36f, x0 + 2, cy + h * 0.30f);
    g.setColour (col::acid.withAlpha (0.25f));
    g.strokePath (s, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setGradientFill (juce::ColourGradient (col::acid, x0, cy - h * 0.3f, col::violet, x0, cy + h * 0.3f, false));
    g.strokePath (s, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (col::acid);
    g.fillEllipse (x0 + 19, cy - h * 0.30f - 4, 8, 8);
    g.setColour (col::bg);
    g.fillEllipse (x0 + 22.5f, cy - h * 0.30f - 1.5f, 2.5f, 2.5f);

    g.setFont (font (25.0f, true));
    juce::GlyphArrangement ga;
    ga.addLineOfText (font (25.0f, true), "SERPENT", x0 + 36, cy + 4);
    juce::Path tp; ga.createPath (tp);
    g.setGradientFill (juce::ColourGradient (col::text, x0 + 36, 0, col::violet.brighter (0.3f), x0 + 170, 0, false));
    g.fillPath (tp);
    g.setColour (col::dim);
    g.setFont (font (10.0f, true));
    g.drawText ("BY SYNTHETIK", (int) x0 + 38, (int) (cy + 7), 160, 14, juce::Justification::centredLeft);
}

//==============================================================================
SerpentEditor::SerpentEditor (SerpentProcessor& p)
    : AudioProcessorEditor (p), proc (p), browser (p), xy (p), scope (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&look);
    juce::LookAndFeel::setDefaultLookAndFeel (&look);
    auto& s = proc.apvts;

    addAndMakeVisible (logo);
    presetName.setFont (font (20.0f, true));
    presetName.setJustificationType (juce::Justification::centred);
    presetCat.setFont (font (10.5f, true));
    presetCat.setColour (juce::Label::textColourId, col::acid);
    presetCat.setJustificationType (juce::Justification::centred);
    info.setFont (font (11.0f));
    info.setColour (juce::Label::textColourId, col::dim);
    info.setJustificationType (juce::Justification::centredLeft);
    for (auto* c : { (juce::Component*) &presetName, (juce::Component*) &presetCat, (juce::Component*) &info }) addAndMakeVisible (c);

    for (auto* b : { &prevBtn, &nextBtn, &mutateBtn, &diceBtn, &saveBtn, &loadBtn }) addAndMakeVisible (b);
    mutateBtn.setColour (juce::TextButton::buttonOnColourId, col::acid);
    diceBtn.setColour (juce::TextButton::buttonOnColourId, col::pink);
    mutateBtn.setTooltip ("Nudge the current sound into a new variation");
    prevBtn.onClick = [this] { browser.step (-1); };
    nextBtn.onClick = [this] { browser.step (1); };
    mutateBtn.onClick = [this] { proc.mutate (0.16f, false); };
    diceBtn.onClick = [this] { proc.mutate (0.45f, true); };
    saveBtn.onClick = [this]
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Serpent Patches");
        dir.createDirectory();
        chooser = std::make_unique<juce::FileChooser> ("Save patch", dir.getChildFile (proc.patchName.trimCharactersAtEnd (" *") + ".serpent"), "*.serpent");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f != juce::File()) proc.savePatch (f.withFileExtension ("serpent")); });
    };
    loadBtn.onClick = [this]
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userMusicDirectory).getChildFile ("Serpent Patches");
        chooser = std::make_unique<juce::FileChooser> ("Load patch", dir, "*.serpent");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc) { auto f = fc.getResult(); if (f.existsAsFile()) proc.loadPatch (f); });
    };

    const char* tabNames[] = { "SYNTH", "MOTION", "FX" };
    for (int i = 0; i < 3; ++i)
    {
        tabBtn[i].setButtonText (tabNames[i]);
        tabBtn[i].setClickingTogglesState (true);
        tabBtn[i].setRadioGroupId (5);
        tabBtn[i].setColour (juce::TextButton::buttonOnColourId, col::violet);
        tabBtn[i].onClick = [this, i] { if (tabBtn[i].getToggleState()) showPage (i); };
        addAndMakeVisible (tabBtn[i]);
        addChildComponent (pages[i]);
    }

    // ------------------------------------------------ SYNTH page
    {
        auto* oA = section ("OSC A", col::violet);
        auto* wa = new WaveDisplay (s, "a_wave", "a_lvl", col::violet); owned.add (wa);
        oA->setDisplay (wa, 140);
        oA->add (knob ("a_wave", "Wave", col::violet), 56);
        oA->add (knob ("a_oct", "Octave", col::violet), 56);
        oA->add (knob ("a_semi", "Semi", col::violet), 56);
        oA->add (knob ("a_fine", "Fine", col::violet), 56);
        oA->add (knob ("a_lvl", "Level", col::violet), 56);
        oA->add (knob ("penv", "Pitch Env", col::pink), 56);
        oA->add (knob ("pdec", "P Decay", col::pink), 56);

        auto* oB = section ("OSC B", col::pink);
        auto* wb = new WaveDisplay (s, "b_wave", "b_lvl", col::pink); owned.add (wb);
        oB->setDisplay (wb, 140);
        oB->add (knob ("b_wave", "Wave", col::pink), 56);
        oB->add (knob ("b_oct", "Octave", col::pink), 56);
        oB->add (knob ("b_semi", "Semi", col::pink), 56);
        oB->add (knob ("b_fine", "Fine", col::pink), 56);
        oB->add (knob ("b_lvl", "Level", col::pink), 56);
        oB->add (knob ("fm", "FM B>A", col::acid), 56);

        auto* vc = section ("VOICE", col::acid);
        vc->add (knob ("uni", "Unison", col::acid), 56);
        vc->add (knob ("detune", "Detune", col::acid), 56);
        vc->add (knob ("spread", "Spread", col::acid), 56);
        vc->add (knob ("sub", "Sub", col::acid), 56);
        vc->add (knob ("noise", "Noise", col::acid), 56);
        vc->add (knob ("glide", "Glide", col::acid), 56);
        vc->add (knob ("vel", "Velocity", col::acid), 56);
        vc->add (toggle ("mono", "MONO", col::acid), 64);

        auto* fl = section ("FILTER", col::cyan);
        auto* fdsp = new FilterDisplay (s); owned.add (fdsp);
        fl->setDisplay (fdsp, 170);
        fl->add (choice ("ftype", "Type"), 82);
        fl->add (knob ("cutoff", "Cutoff", col::cyan), 56);
        fl->add (knob ("reso", "Reso", col::cyan), 56);
        fl->add (knob ("fdrive", "Drive", col::cyan), 56);
        fl->add (knob ("fenv", "Env Amt", col::cyan), 56);
        fl->add (knob ("fkey", "Key Trk", col::cyan), 56);

        auto* ae = section ("AMP ENV", col::violet);
        auto* aed = new EnvDisplay (s, "aa", "ad", "as", "ar", col::violet); owned.add (aed);
        ae->setDisplay (aed, 84);
        ae->add (knob ("aa", "Attack", col::violet), 50);
        ae->add (knob ("ad", "Decay", col::violet), 50);
        ae->add (knob ("as", "Sustain", col::violet), 50);
        ae->add (knob ("ar", "Release", col::violet), 50);

        auto* fe = section ("FILTER ENV", col::cyan);
        auto* fed = new EnvDisplay (s, "fa", "fd", "fs", "fr", col::cyan); owned.add (fed);
        fe->setDisplay (fed, 84);
        fe->add (knob ("fa", "Attack", col::cyan), 50);
        fe->add (knob ("fd", "Decay", col::cyan), 50);
        fe->add (knob ("fs", "Sustain", col::cyan), 50);
        fe->add (knob ("fr", "Release", col::cyan), 50);

        pages[0].addRow ({ { oA, 1.0f } }, 110);
        pages[0].addRow ({ { oB, 1.0f } }, 110);
        pages[0].addRow ({ { vc, 1.0f } }, 100);
        pages[0].addRow ({ { fl, 1.0f } }, 122);
        pages[0].addRow ({ { ae, 1.0f }, { fe, 1.0f } }, 116);
    }

    // ------------------------------------------------ MOTION page
    {
        auto* lf = section ("LFO", col::pink);
        auto* ld = new LfoDisplay (s, proc.engine); owned.add (ld);
        lf->setDisplay (ld, 110);
        lf->add (choice ("l_shape", "Shape"), 84);
        lf->add (choice ("l_dest", "Target"), 84);
        lf->add (toggle ("l_sync", "SYNC", col::pink), 52);
        lf->add (choice ("l_div", "Division"), 76);
        lf->add (knob ("l_rate", "Hz", col::pink), 52);
        lf->add (knob ("l_amt", "Amount", col::pink), 52);
        lf->add (toggle ("l_retrig", "RETRIG", col::pink), 60);

        auto* gs = section ("TRANCE GATE  /  16 STEPS", col::acid);
        gs->add (toggle ("gate_on", "GATE", col::acid), 58);
        gs->add (choice ("gate_div", "Rate"), 70);
        gs->add (knob ("gate_dep", "Depth", col::acid), 52);
        gs->add (knob ("gate_sm", "Smooth", col::acid), 52);
        auto* grid = new juce::Component(); owned.add (grid);
        for (int i = 0; i < 16; ++i)
        {
            auto* b = new StepButton (i); owned.add (b);
            steps.push_back (b);
            grid->addAndMakeVisible (b);
            stepAtt.push_back (std::make_unique<APVTS::ButtonAttachment> (s, "g" + juce::String (i), *b));
        }
        const char* pats[][2] = { { "Roller", "1011101110111011" }, { "Garage", "1101101101101101" }, { "8ths", "1010101010101010" },
                                  { "Skip", "1110111011101110" }, { "Dotted", "1001001001001001" }, { "Stutter", "1111011111110110" },
                                  { "Broken", "1011011010110110" }, { "Offbeat", "0010001000100010" }, { "All", "1111111111111111" } };
        for (auto& pp : pats)
        {
            auto* b = patternBtns.add (new juce::TextButton (pp[0]));
            const juce::String pat (pp[1]);
            b->onClick = [this, pat] { setGatePattern (pat); };
            grid->addAndMakeVisible (b);
        }
        auto* rl = patternBtns.add (new juce::TextButton ("<<")); rl->onClick = [this] { rotateGate (-1); }; grid->addAndMakeVisible (rl);
        auto* rr = patternBtns.add (new juce::TextButton (">>")); rr->onClick = [this] { rotateGate (1); }; grid->addAndMakeVisible (rr);
        gs->add (grid, 360, true);

        auto* ar = section ("ARPEGGIATOR", col::violet);
        ar->add (toggle ("arp_on", "ARP", col::violet), 56);
        ar->add (choice ("arp_mode", "Mode"), 92);
        ar->add (choice ("arp_div", "Rate"), 70);
        ar->add (knob ("arp_oct", "Octaves", col::violet), 52);
        ar->add (knob ("arp_gate", "Gate", col::violet), 52);

        auto* ch = section ("CHORD MEMORY", col::cyan);
        ch->add (choice ("chord", "One finger chords"), 130);

        pages[1].addRow ({ { lf, 1.0f } }, 124);
        pages[1].addRow ({ { gs, 1.0f } }, 220);
        pages[1].addRow ({ { ar, 0.64f }, { ch, 0.36f } }, 110);

        struct GridLayout : juce::ComponentListener
        {
            SerpentEditor* ed; std::vector<ui::StepButton*>* st; juce::OwnedArray<juce::TextButton>* pb;
            void componentMovedOrResized (juce::Component& c, bool, bool) override
            {
                auto r = c.getLocalBounds();
                auto top = r.removeFromTop ((int) (r.getHeight() * 0.58f));
                const float w = (float) top.getWidth() / 16.0f;
                for (int i = 0; i < 16; ++i)
                    (*st)[(size_t) i]->setBounds ((int) ((float) i * w), top.getY(), (int) w, top.getHeight() - 4);
                r.removeFromTop (6);
                const int n = pb->size();
                const int perRow = (n + 1) / 2;
                const int bw = r.getWidth() / perRow;
                for (int i = 0; i < n; ++i)
                    (*pb)[i]->setBounds (r.getX() + (i % perRow) * bw, r.getY() + (i / perRow) * (r.getHeight() / 2), bw - 4, r.getHeight() / 2 - 4);
            }
        };
        auto* gl = new GridLayout(); gl->ed = this; gl->st = &steps; gl->pb = &patternBtns;
        grid->addComponentListener (gl);
        gridLayout.reset (gl);
    }

    // ------------------------------------------------ FX page
    {
        auto* dr = section ("DRIVE", col::pink);
        dr->add (choice ("dist_t", "Type"), 82);
        dr->add (knob ("dist", "Amount", col::pink), 58);
        auto* co = section ("CHORUS", col::violet);
        co->add (knob ("cho_mix", "Mix", col::violet), 58);
        co->add (knob ("cho_rate", "Rate", col::violet), 58);
        co->add (knob ("cho_dep", "Depth", col::violet), 58);
        auto* dl = section ("TEMPO DELAY", col::cyan);
        dl->add (choice ("dly_div", "Time"), 82);
        dl->add (knob ("dly_mix", "Mix", col::cyan), 58);
        dl->add (knob ("dly_fb", "Feedback", col::cyan), 58);
        dl->add (knob ("dly_tone", "Tone", col::cyan), 58);
        dl->add (toggle ("dly_pp", "PING PONG", col::cyan), 86);
        auto* rv = section ("REVERB", col::violet);
        rv->add (knob ("rev_mix", "Mix", col::violet), 58);
        rv->add (knob ("rev_size", "Size", col::violet), 58);
        rv->add (knob ("rev_damp", "Damp", col::violet), 58);
        auto* pu = section ("PUMP  /  SIDECHAIN", col::acid);
        auto* pm = new PumpMeter (proc.engine); owned.add (pm);
        pu->setDisplay (pm, 28);
        pu->add (knob ("pump", "Amount", col::acid), 58);
        pu->add (choice ("pump_div", "Rate"), 70);
        auto* ou = section ("OUTPUT", col::acid);
        ou->add (knob ("locut", "Low Cut", col::acid), 58);
        ou->add (knob ("hicut", "High Cut", col::acid), 58);
        ou->add (knob ("width", "Width", col::acid), 58);
        masterVol = knob ("vol", "Volume", col::acid);
        ou->add (masterVol, 58);

        pages[2].addRow ({ { dr, 0.4f }, { co, 0.6f } }, 120);
        pages[2].addRow ({ { dl, 1.0f } }, 120);
        pages[2].addRow ({ { rv, 0.55f }, { pu, 0.45f } }, 120);
        pages[2].addRow ({ { ou, 1.0f } }, 120);
    }

    addAndMakeVisible (browser);
    addAndMakeVisible (xy);
    addAndMakeVisible (scope);
    xTarget = choice ("xy_xd", "X target"); addAndMakeVisible (xTarget);
    yTarget = choice ("xy_yd", "Y target"); addAndMakeVisible (yTarget);

    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffdcd9e8));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff15151d));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff9a97aa));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, col::violet.withAlpha (0.8f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, col::acid.withAlpha (0.35f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, col::panel2);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId, col::text);
    keyboard.setOctaveForMiddleC (5);   // FL Studio naming
    keyboard.setKeyPressBaseOctave (4);
    addAndMakeVisible (keyboard);

    tabBtn[0].setToggleState (true, juce::dontSendNotification);
    showPage (0);
    setSize (1240, 800);
    startTimerHz (30);
}

SerpentEditor::~SerpentEditor()
{
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

ui::Knob* SerpentEditor::knob (const char* id, const juce::String& l, juce::Colour c) { auto* k = new ui::Knob (proc.apvts, id, l, c); owned.add (k); return k; }
ui::Choice* SerpentEditor::choice (const char* id, const juce::String& l) { auto* k = new ui::Choice (proc.apvts, id, l); owned.add (k); return k; }
ui::Toggle* SerpentEditor::toggle (const char* id, const juce::String& t, juce::Colour c) { auto* k = new ui::Toggle (proc.apvts, id, t, c); owned.add (k); return k; }
ui::Section* SerpentEditor::section (const juce::String& t, juce::Colour c) { auto* k = new ui::Section (t, c); owned.add (k); return k; }

void SerpentEditor::showPage (int i)
{
    page = i;
    for (int k = 0; k < 3; ++k) pages[k].setVisible (k == i);
}

void SerpentEditor::setGatePattern (const juce::String& pat)
{
    for (int i = 0; i < 16; ++i)
        if (auto* prm = proc.apvts.getParameter ("g" + juce::String (i)))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (pat[i] == '1' ? 1.0f : 0.0f);
            prm->endChangeGesture();
        }
    if (auto* on = proc.apvts.getParameter ("gate_on")) on->setValueNotifyingHost (1.0f);
}

void SerpentEditor::rotateGate (int dir)
{
    juce::String pat;
    for (int i = 0; i < 16; ++i)
    {
        const int src = (i - dir + 16) % 16;
        pat << (proc.apvts.getRawParameterValue ("g" + juce::String (src))->load() > 0.5f ? "1" : "0");
    }
    setGatePattern (pat);
}

void SerpentEditor::paint (juce::Graphics& g)
{
    g.fillAll (col::bg);
    // subtle glow behind the top bar
    g.setGradientFill (juce::ColourGradient (col::violet.withAlpha (0.10f), 0, 0, juce::Colours::transparentBlack, 0, 90, false));
    g.fillRect (0, 0, getWidth(), 90);
    g.setColour (col::line);
    g.drawHorizontalLine (64, 0, (float) getWidth());
    // preset box
    auto pb = juce::Rectangle<float> (316, 12, (float) getWidth() - 316 - 324 - 42, 40);
    g.setColour (col::panel2);
    g.fillRoundedRectangle (pb, 8.0f);
    g.setColour (col::line);
    g.drawRoundedRectangle (pb, 8.0f, 1.0f);
}

void SerpentEditor::resized()
{
    const int W = getWidth(), H = getHeight();
    logo.setBounds (16, 8, 240, 48);
    const int pbX = 316, pbW = W - 316 - 324 - 42;
    prevBtn.setBounds (pbX - 38, 16, 32, 32);
    nextBtn.setBounds (pbX + pbW + 6, 16, 32, 32);
    presetCat.setBounds (pbX, 13, pbW, 14);
    presetName.setBounds (pbX, 25, pbW, 26);

    auto right = juce::Rectangle<int> (W - 312, 16, 300, 32);
    const int bw = (300 - 18) / 4;
    mutateBtn.setBounds (right.removeFromLeft (bw)); right.removeFromLeft (6);
    diceBtn.setBounds (right.removeFromLeft (bw)); right.removeFromLeft (6);
    saveBtn.setBounds (right.removeFromLeft (bw)); right.removeFromLeft (6);
    loadBtn.setBounds (right);

    const int top = 76, bottom = H - 96;
    browser.setBounds (12, top, 250, bottom - top);

    const int cx = 274, cw = W - 324 - cx;
    for (int i = 0; i < 3; ++i) tabBtn[i].setBounds (cx + i * 104, top, 98, 30);
    for (auto& pg : pages) pg.setBounds (cx, top + 40, cw, bottom - top - 40);

    auto rc = juce::Rectangle<int> (W - 312, top, 300, bottom - top);
    auto tg = rc.removeFromTop (46);
    xTarget->setBounds (tg.removeFromLeft (148));
    yTarget->setBounds (tg.removeFromRight (148));
    rc.removeFromTop (4);
    xy.setBounds (rc.removeFromTop (300));
    rc.removeFromTop (10);
    info.setBounds (rc.removeFromBottom (18));
    rc.removeFromBottom (4);
    scope.setBounds (rc);

    keyboard.setBounds (12, H - 86, W - 24, 74);
    keyboard.setKeyWidth ((float) (W - 24) / 36.0f);
    keyboard.setLowestVisibleKey (36);
}

void SerpentEditor::timerCallback()
{
    const int c = proc.presetChangeCounter.load();
    if (c != lastCounter)
    {
        lastCounter = c;
        presetName.setText (proc.patchName, juce::dontSendNotification);
        const auto& pr = SerpentProcessor::preset (proc.getCurrentProgram());
        presetCat.setText (proc.patchName.startsWith (pr.name) ? juce::String (pr.category).toUpperCase() : juce::String ("USER PATCH"), juce::dontSendNotification);
        browser.syncSelection();
    }
    const int st = proc.engine.uiGateStep.load();
    for (auto* b : steps)
    {
        const bool pl = b->index == st;
        if (pl != b->playing) { b->playing = pl; b->repaint(); }
    }
    info.setText (juce::String (proc.uiBpm.load(), 1) + " BPM    " + juce::String (proc.engine.uiActiveVoices.load()) + " voices    "
                  + juce::String (SerpentProcessor::numPresets()) + " presets", juce::dontSendNotification);
    xy.tick();
    scope.repaint();
    pages[page].repaint();
}
