#include "PluginEditor.h"

//==============================================================================
KarplusstrongAudioProcessorEditor::KarplusstrongAudioProcessorEditor (KarplusstrongAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (520, 260);

    // Pluck button (momentary)
    addAndMakeVisible (pluckButton);
    pluckButton.onClick = [this]()
    {
        audioProcessor.triggerPluck();
    };

    auto addLabel = [this](juce::Label& lab, const juce::String& text)
    {
        addAndMakeVisible (lab);
        lab.setText (text, juce::dontSendNotification);
        lab.setJustificationType (juce::Justification::centredLeft);
    };

    setupSlider (delaySlider);  addAndMakeVisible (delaySlider);  addLabel (delayLabel,  "Delay (s)");
    setupSlider (decaySlider);  addAndMakeVisible (decaySlider);  addLabel (decayLabel,  "Decay");
    setupSlider (widthSlider);  addAndMakeVisible (widthSlider);  addLabel (widthLabel,  "Width (s)");
    setupSlider (cutoffSlider); addAndMakeVisible (cutoffSlider); addLabel (cutoffLabel, "LP Cutoff (Hz)");

    // Burst type combo
    addAndMakeVisible (burstTypeBox);
    burstTypeBox.addItem ("Noise",    1);
    burstTypeBox.addItem ("Sine",     2);
    burstTypeBox.addItem ("Triangle", 3);
    burstTypeBox.addItem ("Square",   4);
    burstTypeBox.addItem ("Saw",      5);
    burstTypeBox.setSelectedId (1);

    addLabel (burstLabel, "Burst Type");

    // Attachments to APVTS
    delayAttach  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "delay",    delaySlider);
    decayAttach  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "decay",    decaySlider);
    widthAttach  = std::make_unique<SliderAttachment> (audioProcessor.apvts, "width",    widthSlider);
    cutoffAttach = std::make_unique<SliderAttachment> (audioProcessor.apvts, "lpcutoff", cutoffSlider);
    burstAttach  = std::make_unique<ComboAttachment>  (audioProcessor.apvts, "bursttype", burstTypeBox);
}

void KarplusstrongAudioProcessorEditor::setupSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 22);
}

void KarplusstrongAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    g.drawText ("Karplus-Strong Plucked String", 10, 10, getWidth()-20, 24, juce::Justification::centred);

    g.setFont (12.0f);
    g.drawText ("Tip: shorter delay = higher pitch | higher decay = longer sustain", 10, 36, getWidth()-20, 18,
                juce::Justification::centred);
}

void KarplusstrongAudioProcessorEditor::resized()
{
    const int pad = 12;

    pluckButton.setBounds (pad, 60, 120, 30);

    burstLabel.setBounds (pad, 100, 120, 20);
    burstTypeBox.setBounds (pad, 122, 160, 24);

    // Rotary sliders area
    const int y = 70;
    const int w = 110;
    const int h = 110;

    int x = 200;

    delayLabel.setBounds  (x, y - 18, w, 18);
    delaySlider.setBounds (x, y, w, h);

    x += 110;

    decayLabel.setBounds  (x, y - 18, w, 18);
    decaySlider.setBounds (x, y, w, h);

    x += 110;

    widthLabel.setBounds  (x, y - 18, w, 18);
    widthSlider.setBounds (x, y, w, h);

    x += 110;

    cutoffLabel.setBounds  (x, y - 18, w, 18);
    cutoffSlider.setBounds (x, y, w, h);
}
