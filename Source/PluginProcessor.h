#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"

struct SerpentPreset { const char* name; const char* category; const char* data; };

class SerpentProcessor : public juce::AudioProcessor
{
public:
    SerpentProcessor();
    ~SerpentProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Serpent"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- used by the editor
    static int numPresets();
    static const SerpentPreset& preset (int i);
    static juce::StringArray categories();
    void applyPresetData (const char* data);
    void mutate (float amount, bool includeRhythm);
    bool savePatch (const juce::File& f);
    bool loadPatch (const juce::File& f);

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    sp::Engine engine;
    juce::String patchName { "Init" };
    std::atomic<int> presetChangeCounter { 0 };
    std::atomic<bool> killNotes { false };

    // oscilloscope ring
    static constexpr int SCOPE = 2048;
    std::array<float, SCOPE> scope {};
    std::atomic<int> scopeWrite { 0 };
    std::atomic<double> uiBpm { 124.0 };

private:
    std::array<std::atomic<float>*, sp::NUM_P> raw {};
    std::array<juce::RangedAudioParameter*, sp::NUM_P> params {};
    std::array<float, sp::NUM_P> eff {};
    int currentProgram = 0;
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SerpentProcessor)
};
