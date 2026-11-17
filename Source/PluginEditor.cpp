#include "PluginEditor.h"

static const juce::Colour bg(0xff09070f), panel(0xff171020), purple(0xffb45cff), pale(0xffeadbff);

VocalfireAudioProcessorEditor::VocalfireAudioProcessorEditor(VocalfireAudioProcessor& p)
    : AudioProcessorEditor(&p), audio(p)
{
    setSize(720, 480);
    auto setupLabel = [](juce::Label& l, const juce::String& t, float size, juce::Colour c)
    {
        l.setText(t, juce::dontSendNotification);
        l.setFont(juce::Font(size, juce::Font::bold));
        l.setColour(juce::Label::textColourId, c);
    };
    setupLabel(title, "VOCALFIRE", 30.f, pale); addAndMakeVisible(title);
    setupLabel(subtitle, "VOCAL PROCESSOR  //  V1.0 ALPHA", 11.f, purple); addAndMakeVisible(subtitle);
    setupLabel(pitchReadout, "PITCH DETECTOR  •  waiting for signal", 13.f, pale); addAndMakeVisible(pitchReadout);

    auto setupSlider = [this, &setupLabel](juce::Slider& s, juce::Label& l, const juce::String& name)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
        s.setColour(juce::Slider::rotarySliderFillColourId, purple);
        s.setColour(juce::Slider::thumbColourId, pale);
        s.setColour(juce::Slider::textBoxTextColourId, pale);
        s.setColour(juce::Slider::textBoxBackgroundColourId, panel);
        addAndMakeVisible(s);
        setupLabel(l, name, 12.f, pale); l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };
    setupSlider(input, inputLabel, "INPUT");
    setupSlider(output, outputLabel, "OUTPUT");
    setupSlider(reverb, reverbLabel, "REVERB");
    setupSlider(delay, delayLabel, "DELAY");
    setupSlider(cutoff, cutoffLabel, "LOW CUT");
    setupSlider(pitch, pitchLabel, "PITCH AMOUNT");

    auto& state = audio.parameters;
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "input", input));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "output", output));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "reverb", reverb));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "delay", delay));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "cutoff", cutoff));
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(state, "pitch", pitch));

    key.addItemList({"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 1);
    scale.addItemList({"Chromatic","Major","Minor"}, 1);
    addAndMakeVisible(key); addAndMakeVisible(scale);
    key.setColour(juce::ComboBox::backgroundColourId, panel);
    scale.setColour(juce::ComboBox::backgroundColourId, panel);
    key.setColour(juce::ComboBox::textColourId, pale);
    scale.setColour(juce::ComboBox::textColourId, pale);
    keyAttachment = std::make_unique<ComboAttachment>(state, "key", key);
    scaleAttachment = std::make_unique<ComboAttachment>(state, "scale", scale);
    setupLabel(keyLabel, "KEY", 11.f, purple); addAndMakeVisible(keyLabel);
    setupLabel(scaleLabel, "SCALE", 11.f, purple); addAndMakeVisible(scaleLabel);
    startTimerHz(8);
}

void VocalfireAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(purple.withAlpha(0.28f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(10), 18.f, 1.5f);
    g.setColour(panel);
    g.fillRoundedRectangle(juce::Rectangle<float>(22, 100, 676, 350), 14.f);
    g.setColour(purple.withAlpha(0.7f));
    g.fillRect(22, 93, 676, 2);
    g.setColour(juce::Colour(0xff38234d));
    g.drawRoundedRectangle(juce::Rectangle<float>(35, 120, 650, 205), 10.f, 1.f);
}

void VocalfireAudioProcessorEditor::resized()
{
    title.setBounds(30, 18, 300, 42);
    subtitle.setBounds(32, 60, 300, 22);
    pitchReadout.setBounds(350, 32, 340, 32);
    const int xs[] = {65, 170, 275, 380, 485, 590};
    juce::Slider* sliders[] = {&input,&output,&reverb,&delay,&cutoff,&pitch};
    juce::Label* labels[] = {&inputLabel,&outputLabel,&reverbLabel,&delayLabel,&cutoffLabel,&pitchLabel};
    for (int i=0;i<6;++i) { labels[i]->setBounds(xs[i]-35, 135, 100, 22); sliders[i]->setBounds(xs[i]-45, 160, 120, 140); }
    keyLabel.setBounds(120, 355, 70, 20); key.setBounds(120, 380, 120, 30);
    scaleLabel.setBounds(285, 355, 70, 20); scale.setBounds(285, 380, 150, 30);
}

void VocalfireAudioProcessorEditor::timerCallback()
{
    const float hz = audio.detectedFrequency();
    if (hz > 0.0f)
    {
        const int midi = juce::roundToInt(audio.detectedMidiNote());
        const juce::String note = juce::MidiMessage::getMidiNoteName(midi, true, true, 4);
        pitchReadout.setText("PITCH DETECTOR  •  " + note + "  /  " + juce::String(hz, 1) + " Hz", juce::dontSendNotification);
    }
}
