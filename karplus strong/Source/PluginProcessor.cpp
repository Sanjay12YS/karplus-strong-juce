#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KarplusstrongAudioProcessor::KarplusstrongAudioProcessor()
: AudioProcessor (BusesProperties()
                 .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
, apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout KarplusstrongAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Required params (assignment ranges)
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "delay", "Delay (s)",
        juce::NormalisableRange<float>(0.0f, 0.02f, 0.00001f),
        0.005f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay",
        juce::NormalisableRange<float>(0.8f, 0.999f, 0.0001f),
        0.99f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "width", "Width (s)",
        juce::NormalisableRange<float>(0.0f, 0.02f, 0.00001f),
        0.01f));

    // Bonus 1: low-pass cutoff in feedback
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "lpcutoff", "LP Cutoff (Hz)",
        juce::NormalisableRange<float>(200.0f, 12000.0f, 1.0f, 0.5f),
        4000.0f));

    // Bonus 2: burst type
    params.push_back (std::make_unique<juce::AudioParameterChoice>(
        "bursttype", "Burst Type",
        juce::StringArray { "Noise", "Sine", "Triangle", "Square", "Saw" },
        0));

    return { params.begin(), params.end() };
}

//==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
bool KarplusstrongAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Output must be mono or stereo
    const auto out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}
#endif

void KarplusstrongAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    // Allocate a delay buffer long enough for max delay (0.02 s) + block safety
    delayBufferLength = (int) std::ceil (0.02 * sampleRate) + samplesPerBlock + 4;
    delayBuffer.setSize (juce::jmax (1, getTotalNumOutputChannels()), delayBufferLength);
    delayBuffer.clear();

    writePos = 0;
    burstSamplesRemaining = 0;
    burstGain = 0.0f;
    oscPhase = 0.0f;

    lpState.assign ((size_t) juce::jmax (1, getTotalNumOutputChannels()), 0.0f);
}

void KarplusstrongAudioProcessor::triggerPluck()
{
    const auto sr = (float) getSampleRate();

    // width can be 0 by spec -> clamp in code to avoid divide-by-zero
    const float widthSec = juce::jmax (0.0001f, apvts.getRawParameterValue("width")->load());
    burstSamplesRemaining = (int) std::ceil (widthSec * sr);

    burstGain = 1.0f;

    // Reset oscillator phase for consistent attacks (optional)
    oscPhase = 0.0f;
}

static inline float triangleFromPhase01 (float p01)
{
    // p01 in [0,1)
    // triangle in [-1, 1]
    return 2.0f * std::abs (2.0f * (p01 - std::floor (p01 + 0.5f))) - 1.0f;
}

void KarplusstrongAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels(); // IMPORTANT: output-safe even with 0 inputs

    // Safety: if host gives no channels, nothing to do
    if (numChannels <= 0 || delayBuffer.getNumChannels() <= 0)
        return;

    buffer.clear(); // This is a synth-like effect (we generate audio)

    const float sr = (float) getSampleRate();

    // Read parameters
    float delaySec = apvts.getRawParameterValue("delay")->load();
    float decay    = apvts.getRawParameterValue("decay")->load();
    float cutoffHz = apvts.getRawParameterValue("lpcutoff")->load();
    int burstType  = (int) apvts.getRawParameterValue("bursttype")->load();

    // Clamp delay to avoid zero/negative, and also avoid readPos == writePos issues
    delaySec = juce::jlimit (0.00005f, 0.02f, delaySec);
    int delaySamples = juce::jlimit (1, delayBufferLength - 2, (int) std::round (delaySec * sr));

    // One-pole LP coefficient
    cutoffHz = juce::jlimit (20.0f, 20000.0f, cutoffHz);
    const float a = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * cutoffHz / sr);

    // Oscillator increment (800 Hz)
    const float oscFreq = 800.0f;
    const float phaseInc = oscFreq / sr;

    for (int i = 0; i < numSamples; ++i)
    {
        const int readPos = (writePos - delaySamples + delayBufferLength) % delayBufferLength;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* delayData = delayBuffer.getWritePointer (juce::jmin (ch, delayBuffer.getNumChannels() - 1));
            float delayed = delayData[readPos];

            // Excitation (burst input)
            float in = 0.0f;

            if (burstSamplesRemaining > 0 && burstGain > 0.0f)
            {
                float v = 0.0f;

                if (burstType == 0) // Noise
                {
                    v = rng.nextFloat() * 2.0f - 1.0f;
                }
                else
                {
                    float p = oscPhase; // [0,1)

                    if (burstType == 1)      v = std::sin (2.0f * juce::MathConstants<float>::pi * p); // Sine
                    else if (burstType == 2) v = triangleFromPhase01 (p);                               // Triangle
                    else if (burstType == 3) v = (p < 0.5f ? 1.0f : -1.0f);                               // Square
                    else if (burstType == 4) v = 2.0f * p - 1.0f;                                        // Saw

                    // advance phase once per sample (per block; same phase used for all channels)
                    if (ch == 0)
                    {
                        oscPhase += phaseInc;
                        if (oscPhase >= 1.0f) oscPhase -= 1.0f;
                    }
                }

                in = burstGain * v;

                // Linear decay of burstGain across burst duration
                burstSamplesRemaining--;

                // Smoothly ramp the burst down to zero by the end
                if (burstSamplesRemaining <= 0)
                    burstGain = 0.0f;
                else
                {
                    // decay burstGain a little each sample (fast)
                    burstGain = juce::jmax (0.0f, burstGain - (1.0f / (0.01f * sr)));
                }
            }

            // Bonus 1: Low-pass filter in feedback path
            const size_t idx = (size_t) juce::jmin (ch, (int)lpState.size() - 1);
            lpState[idx] = lpState[idx] + a * (delayed - lpState[idx]);
            const float filteredFeedback = lpState[idx];

            // Karplus-Strong write: excitation + filtered feedback
            const float y = in + decay * filteredFeedback;

            delayData[writePos] = y;

            // Output: you can output delayed or y; delayed often sounds more “string-like”
            buffer.getWritePointer(ch)[i] = delayed;
        }

        writePos++;
        if (writePos >= delayBufferLength)
            writePos = 0;
    }
}

//==============================================================================
juce::AudioProcessorEditor* KarplusstrongAudioProcessor::createEditor()
{
    return new KarplusstrongAudioProcessorEditor (*this);
}

void KarplusstrongAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void KarplusstrongAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KarplusstrongAudioProcessor();
}
