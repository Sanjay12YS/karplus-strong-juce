#pragma once

#include <JuceHeader.h>

//==============================================================================
class KarplusstrongAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    KarplusstrongAudioProcessor();
    ~KarplusstrongAudioProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Called by UI button (momentary pluck)
    void triggerPluck();

    // Expose APVTS to editor for attachments
    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Delay line
    juce::AudioBuffer<float> delayBuffer;
    int delayBufferLength = 0;
    int writePos = 0;

    // Pluck/burst state
    int burstSamplesRemaining = 0;
    float burstGain = 0.0f;

    // Oscillator for non-noise burst types
    float oscPhase = 0.0f;

    // One-pole low-pass state per channel (feedback filter)
    std::vector<float> lpState;

    // Random generator
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KarplusstrongAudioProcessor)
};
