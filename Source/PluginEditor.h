#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VocalfireAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VocalfireAudioProcessorEditor(VocalfireAudioProcessor&);
    ~VocalfireAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    VocalfireAudioProcessor& audio;
    juce::Label title, subtitle, pitchReadout;
    juce::Slider input, output, reverb, delay, cutoff, pitch;
    juce::ComboBox key, scale;
    juce::Label inputLabel, outputLabel, reverbLabel, delayLabel, cutoffLabel, pitchLabel, keyLabel, scaleLabel;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::unique_ptr<ComboAttachment> keyAttachment, scaleAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalfireAudioProcessorEditor)
};
