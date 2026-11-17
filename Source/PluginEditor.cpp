#include "PluginEditor.h"

namespace
{
    const juce::Colour background(0xff09070f);
    const juce::Colour panel(0xff171321);
    const juce::Colour panelEdge(0xff493064);
    const juce::Colour purple(0xffb45cff);
    const juce::Colour hotPurple(0xffe0a8ff);
    const juce::Colour text(0xfff4eaff);
    const juce::Colour muted(0xff9b8aa9);
}

VocalfireAudioProcessorEditor::VocalfireAudioProcessorEditor(VocalfireAudioProcessor& p)
    : AudioProcessorEditor(&p), audio(p)
{
    setSize(980, 650);

    auto setupLabel = [](juce::Label& label, const juce::String& name, float size, juce::Colour colour)
    {
        label.setText(name, juce::dontSendNotification);
        label.setFont(juce::Font(size, juce::Font::bold));
        label.setColour(juce::Label::textColourId, colour);
        label.setJustificationType(juce::Justification::centred);
    };

    setupLabel(title, "VOCALFIRE", 31.0f, text);
    addAndMakeVisible(title);
    setupLabel(subtitle, "MODULAR VOCAL LAB  //  V2.0 DEVELOPMENT", 11.0f, purple);
    addAndMakeVisible(subtitle);
    setupLabel(pitchReadout, "PITCH MONITOR  •  WAITING FOR SIGNAL", 13.0f, hotPurple);
    addAndMakeVisible(pitchReadout);

    auto setupSlider = [this, &setupLabel](juce::Slider& slider, juce::Label& label, const juce::String& name)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
        slider.setColour(juce::Slider::rotarySliderFillColourId, purple);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, panelEdge);
        slider.setColour(juce::Slider::thumbColourId, text);
        slider.setColour(juce::Slider::textBoxTextColourId, text);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, panel);
        slider.setColour(juce::Slider::textBoxOutlineColourId, panelEdge);
        addAndMakeVisible(slider);
        setupLabel(label, name, 11.0f, text);
        addAndMakeVisible(label);
    };

    setupSlider(input, inputLabel, "INPUT");
    setupSlider(output, outputLabel, "OUTPUT");
    setupSlider(distortion, distortionLabel, "DISTORTION");
    setupSlider(reverb, reverbLabel, "REVERB");
    setupSlider(delay, delayLabel, "DELAY");
    setupSlider(cutoff, cutoffLabel, "LOW CUT");
    setupSlider(pitch, pitchLabel, "PITCH MONITOR");

    auto& state = audio.parameters;
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "input", input));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "output", output));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "distortion", distortion));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "reverb", reverb));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "delay", delay));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "cutoff", cutoff));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "pitch", pitch));

    key.addItemList({ "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 1);
    scale.addItemList({ "Chromatic","Major","Minor" }, 1);
    key.setColour(juce::ComboBox::backgroundColourId, panel);
    scale.setColour(juce::ComboBox::backgroundColourId, panel);
    key.setColour(juce::ComboBox::textColourId, text);
    scale.setColour(juce::ComboBox::textColourId, text);
    key.setColour(juce::ComboBox::outlineColourId, panelEdge);
    scale.setColour(juce::ComboBox::outlineColourId, panelEdge);
    addAndMakeVisible(key);
    addAndMakeVisible(scale);
    keyAttachment = std::make_unique<ComboAttachment>(state, "key", key);
    scaleAttachment = std::make_unique<ComboAttachment>(state, "scale", scale);
    setupLabel(keyLabel, "KEY", 10.0f, purple);
    setupLabel(scaleLabel, "SCALE", 10.0f, purple);
    addAndMakeVisible(keyLabel);
    addAndMakeVisible(scaleLabel);

    startTimerHz(8);
}

void VocalfireAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);

    auto bounds = getLocalBounds().toFloat();
    g.setColour(purple.withAlpha(0.35f));
    g.drawRoundedRectangle(bounds.reduced(8.0f), 20.0f, 1.5f);

    // Header
    g.setColour(panel);
    g.fillRoundedRectangle(juce::Rectangle<float>(20, 96, 940, 58), 16.0f);
    g.setColour(panelEdge);
    g.drawRoundedRectangle(juce::Rectangle<float>(20, 96, 940, 58), 16.0f, 1.0f);

    // Four modular bays inspired by the supplied hardware-rack reference.
    const juce::Rectangle<float> bays[] = {
        { 22, 178, 218, 390 }, { 258, 178, 218, 390 },
        { 494, 178, 218, 390 }, { 730, 178, 228, 390 }
    };
    const juce::String names[] = { "01  //  LEVEL", "02  //  DISTORTION", "03  //  SPACE", "04  //  PITCH LAB" };

    for (int i = 0; i < 4; ++i)
    {
        g.setColour(panel);
        g.fillRoundedRectangle(bays[i], 18.0f);
        g.setColour(panelEdge);
        g.drawRoundedRectangle(bays[i], 18.0f, 1.2f);
        g.setColour(purple.withAlpha(0.8f));
        g.fillRoundedRectangle(juce::Rectangle<float>(bays[i].getX() + 14, bays[i].getY() + 13, bays[i].getWidth() - 28, 3), 2.0f);
        g.setColour(hotPurple);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(names[i], bays[i].toNearestInt().withHeight(38).withY((int)bays[i].getY() + 23), juce::Justification::centred);
    }

    // Soft neon connector lines between modules.
    g.setColour(purple.withAlpha(0.75f));
    g.drawLine(240.0f, 370.0f, 258.0f, 370.0f, 3.0f);
    g.drawLine(476.0f, 370.0f, 494.0f, 370.0f, 3.0f);
    g.drawLine(712.0f, 370.0f, 730.0f, 370.0f, 3.0f);

    g.setColour(muted);
    g.setFont(juce::Font(10.0f));
    g.drawText("DRAG-AND-DROP MODULE CHAIN  •  MORE MODULES COMING IN THE NEXT BUILDS",
               juce::Rectangle<int>(24, 590, 932, 24), juce::Justification::centred);
}

void VocalfireAudioProcessorEditor::resized()
{
    title.setBounds(28, 20, 300, 42);
    subtitle.setBounds(30, 59, 350, 20);
    pitchReadout.setBounds(580, 108, 350, 32);

    inputLabel.setBounds(38, 250, 90, 22);
    input.setBounds(36, 276, 94, 126);
    outputLabel.setBounds(132, 250, 90, 22);
    output.setBounds(130, 276, 94, 126);

    distortionLabel.setBounds(312, 260, 110, 22);
    distortion.setBounds(305, 292, 124, 150);

    reverbLabel.setBounds(520, 250, 90, 22);
    reverb.setBounds(515, 276, 94, 126);
    delayLabel.setBounds(614, 250, 90, 22);
    delay.setBounds(610, 276, 94, 126);

    cutoffLabel.setBounds(748, 250, 90, 22);
    cutoff.setBounds(745, 276, 94, 126);
    pitchLabel.setBounds(840, 250, 100, 22);
    pitch.setBounds(840, 276, 100, 126);

    keyLabel.setBounds(770, 440, 70, 18);
    key.setBounds(755, 462, 92, 30);
    scaleLabel.setBounds(855, 440, 80, 18);
    scale.setBounds(850, 462, 96, 30);
}

void VocalfireAudioProcessorEditor::timerCallback()
{
    const float hz = audio.detectedFrequency();
    if (hz > 0.0f)
    {
        const int midi = juce::roundToInt(audio.detectedMidiNote());
        const auto note = juce::MidiMessage::getMidiNoteName(midi, true, true, 4);
        pitchReadout.setText("PITCH MONITOR  •  " + note + "  /  " + juce::String(hz, 1) + " Hz",
                             juce::dontSendNotification);
    }
}
