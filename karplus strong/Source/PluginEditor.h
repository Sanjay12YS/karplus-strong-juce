#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class KarplusstrongAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit KarplusstrongAudioProcessorEditor (KarplusstrongAudioProcessor&);
    ~KarplusstrongAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    KarplusstrongAudioProcessor& audioProcessor;

    // UI Components
    juce::TextButton pluckButton { "Pluck" };

    juce::Slider delaySlider, decaySlider, widthSlider, cutoffSlider;
    juce::ComboBox burstTypeBox;

    juce::Label delayLabel, decayLabel, widthLabel, cutoffLabel, burstLabel;

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> delayAttach, decayAttach, widthAttach, cutoffAttach;
    std::unique_ptr<ComboAttachment>  burstAttach;

    void setupSlider (juce::Slider& s);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KarplusstrongAudioProcessorEditor)
};
