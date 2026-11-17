#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VocalfireAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit VocalfireAudioProcessorEditor(VocalfireAudioProcessor&);
    ~VocalfireAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VocalfireAudioProcessor& audioProcessor;

    juce::ComboBox presetBox;
    juce::TextButton randomButton { "RANDOM PRESET" };
    juce::TextButton saveButton { "SAVE PRESET" };
    juce::TextButton loadButton { "LOAD PRESET" };

    juce::Slider inputSlider, outputSlider;
    juce::Slider distortionSlider, reverbSlider;
    juce::Slider delaySlider, feedbackSlider;
    juce::Slider cutoffSlider;

    juce::ComboBox keyBox, scaleBox;

    using SliderAttachment =
        juce::AudioProcessorValueTreeState::SliderAttachment;

    using ComboAttachment =
        juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<SliderAttachment> distortionAttachment;
    std::unique_ptr<SliderAttachment> reverbAttachment;
    std::unique_ptr<SliderAttachment> delayAttachment;
    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> cutoffAttachment;

    std::unique_ptr<ComboAttachment> keyAttachment;
    std::unique_ptr<ComboAttachment> scaleAttachment;

    void applyPreset(int presetIndex);
    void randomPreset();
    void savePreset();
    void loadPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        VocalfireAudioProcessorEditor)
};
