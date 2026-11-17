#pragma once
#include <JuceHeader.h>

class VocalfireAudioProcessor : public juce::AudioProcessor
{
public:
    VocalfireAudioProcessor();
    ~VocalfireAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "VOCALFIRE"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    float detectedMidiNote() const noexcept { return detectedNote.load(); }
    float detectedFrequency() const noexcept { return detectedHz.load(); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void analysePitch(const juce::AudioBuffer<float>& buffer);

    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 192000 };
    juce::dsp::IIR::Filter<float> lowCutL, lowCutR;
    juce::AudioBuffer<float> monoAnalysis;
    juce::AudioProcessorValueTreeState::ParameterLayout unusedLayout;
    std::atomic<float> detectedNote { -1.0f }, detectedHz { 0.0f };
    double currentSampleRate = 44100.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalfireAudioProcessor)
};
