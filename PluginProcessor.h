#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Params.h"
#include "Presets.h"
#include "Matcher.h"
#include "SynthEngine.h"
#include <array>
#include <atomic>

class DustBoxProcessor : public juce::AudioProcessor
{
public:
    DustBoxProcessor();
    ~DustBoxProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Dust Box"; }
    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override { return dustbox::kNumPresets; }
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- editor API --------------------------------------------------------
    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

    void applyPatch (const dustbox::Patch&, bool keepCharacter = false);
    void resetCharacter();
    dustbox::Patch currentPatch() const { return readPatch(); }
    void makeFromVibe (const juce::String& text);
    bool loadPatchJson (const juce::File&);
    bool savePatchJson (const juce::File&) const;

    juce::String patchName { "Rolling Three" };
    juce::String patchInfo;
    int activeVoiceCount() const { return engine.activeVoices(); }
    double hostBpm() const { return lastBpm.load(); }

    // A small ring buffer of recent output, read by the editor's scope.
    static constexpr int kScopeSize = 1024;
    std::array<float, kScopeSize> scope {};
    std::atomic<int> scopeWrite { 0 };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    dustbox::SynthEngine engine;
    std::atomic<int> currentProgram { 0 };
    std::atomic<double> lastBpm { 124.0 };
    std::array<std::atomic<float>*, dustbox::kNumFloatParams>  floatRefs {};
    std::array<std::atomic<float>*, dustbox::kNumChoiceParams> choiceRefs {};

    dustbox::Patch readPatch() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DustBoxProcessor)
};
