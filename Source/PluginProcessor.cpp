
#include "PluginEditor.h"
#include <random>

namespace
{
    struct VocalPreset
    {
        const char* name;
        float input;
        float output;
        float distortion;
        float reverb;
        float delay;
        float feedback;
        float cutoff;
        int key;
        int scale;
    };

    const VocalPreset presets[] =
    {
        { "VILLIAN // DARK TRAP",       2.0f, -1.0f, 34.0f, 10.0f,  8.0f, 28.0f, 105.0f,  2, 2 },
        { "FORTUNA812 // SPACE",        1.0f, -1.5f, 18.0f, 38.0f, 24.0f, 36.0f,  95.0f,  9, 2 },
        { "MAYOT // MELODIC",           1.5f, -1.0f, 12.0f, 22.0f, 16.0f, 24.0f,  90.0f,  4, 2 },
        { "OG BUDA // WARM",            2.5f, -2.0f, 28.0f, 14.0f,  5.0f, 20.0f, 115.0f,  7, 2 },
        { "9MICE // BRIGHT",            0.0f, -1.0f, 22.0f, 28.0f, 20.0f, 32.0f, 130.0f,  1, 2 },
        { "DARK PRINCE // LOW",         3.0f, -2.5f, 42.0f,  8.0f,  4.0f, 18.0f,  75.0f,  5, 2 },
        { "ANONYMUS EMBER // AMBIENT",  0.5f, -2.0f, 15.0f, 52.0f, 32.0f, 42.0f, 100.0f,  8, 2 },
        { "MADK1D // EDGE",             2.0f, -1.0f, 48.0f, 12.0f, 12.0f, 30.0f, 120.0f, 10, 2 }
    };

    constexpr int presetCount = sizeof(presets) / sizeof(presets[0]);

    void styleSlider(juce::Slider& slider, const juce::String& name)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 65, 20);
        slider.setName(name);
        slider.setColour(juce::Slider::rotarySliderFillColourId,
                         juce::Colour(0xffa855f7));
        slider.setColour(juce::Slider::rotarySliderOutlineColourId,
                         juce::Colour(0xff302044));
        slider.setColour(juce::Slider::thumbColourId,
                         juce::Colour(0xffe9d5ff));
        slider.setColour(juce::Slider::textBoxTextColourId,
                         juce::Colours::white);
        slider.setColour(juce::Slider::textBoxBackgroundColourId,
                         juce::Colour(0xff171020));
        slider.setColour(juce::Slider::textBoxOutlineColourId,
                         juce::Colour(0xff51316e));
    }
}

VocalfireAudioProcessorEditor::VocalfireAudioProcessorEditor(
    VocalfireAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(760, 520);

    auto& params = audioProcessor.parameters;

    auto setupButton = [](juce::TextButton& button)
    {
        button.setColour(juce::TextButton::buttonColourId,
                         juce::Colour(0xff28143d));
        button.setColour(juce::TextButton::buttonOnColourId,
                         juce::Colour(0xff7135a8));
        button.setColour(juce::TextButton::textColourOffId,
                         juce::Colours::white);
        button.setColour(juce::TextButton::textColourOnId,
                         juce::Colours::white);
    };

    for (int i = 0; i < presetCount; ++i)
        presetBox.addItem(presets[i].name, i + 1);

    presetBox.setSelectedId(1);
    presetBox.setColour(juce::ComboBox::backgroundColourId,
                        juce::Colour(0xff171020));
    presetBox.setColour(juce::ComboBox::textColourId,
                        juce::Colours::white);
    presetBox.setColour(juce::ComboBox::outlineColourId,
                        juce::Colour(0xff7135a8));

    addAndMakeVisible(presetBox);
    presetBox.onChange = [this]
    {
        applyPreset(presetBox.getSelectedId() - 1);
    };

    setupButton(randomButton);
    setupButton(saveButton);
    setupButton(loadButton);

    addAndMakeVisible(randomButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);

    randomButton.onClick = [this] { randomPreset(); };
    saveButton.onClick = [this] { savePreset(); };
    loadButton.onClick = [this] { loadPreset(); };

    styleSlider(inputSlider, "INPUT");
    styleSlider(outputSlider, "OUTPUT");
    styleSlider(distortionSlider, "DISTORTION");
    styleSlider(reverbSlider, "REVERB");
    styleSlider(delaySlider, "DELAY");
    styleSlider(feedbackSlider, "FEEDBACK");
    styleSlider(cutoffSlider, "LOW CUT");

    addAndMakeVisible(inputSlider);
    addAndMakeVisible(outputSlider);
    addAndMakeVisible(distortionSlider);
    addAndMakeVisible(reverbSlider);
    addAndMakeVisible(delaySlider);
    addAndMakeVisible(feedbackSlider);
    addAndMakeVisible(cutoffSlider);

    inputAttachment =
        std::make_unique<SliderAttachment>(params, "input", inputSlider);
    outputAttachment =
        std::make_unique<SliderAttachment>(params, "output", outputSlider);
    distortionAttachment =
        std::make_unique<SliderAttachment>(params, "distortion", distortionSlider);
    reverbAttachment =
        std::make_unique<SliderAttachment>(params, "reverb", reverbSlider);
    delayAttachment =
        std::make_unique<SliderAttachment>(params, "delay", delaySlider);
    feedbackAttachment =
        std::make_unique<SliderAttachment>(params, "feedback", feedbackSlider);
    cutoffAttachment =
        std::make_unique<SliderAttachment>(params, "cutoff", cutoffSlider);

    keyBox.addItem("C", 1);
    keyBox.addItem("C#", 2);
    keyBox.addItem("D", 3);
    keyBox.addItem("D#", 4);
    keyBox.addItem("E", 5);
    keyBox.addItem("F", 6);
    keyBox.addItem("F#", 7);
    keyBox.addItem("G", 8);
    keyBox.addItem("G#", 9);
    keyBox.addItem("A", 10);
    keyBox.addItem("A#", 11);
    keyBox.addItem("B", 12);

    scaleBox.addItem("Major", 1);
    scaleBox.addItem("Minor", 2);
    scaleBox.addItem("Harmonic Minor", 3);

    addAndMakeVisible(keyBox);
    addAndMakeVisible(scaleBox);

    keyAttachment =
        std::make_unique<ComboAttachment>(params, "key", keyBox);
    scaleAttachment =
        std::make_unique<ComboAttachment>(params, "scale", scaleBox);

    keyBox.setColour(juce::ComboBox::backgroundColourId,
                     juce::Colour(0xff171020));
    keyBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    scaleBox.setColour(juce::ComboBox::backgroundColourId,
                       juce::Colour(0xff171020));
    scaleBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
}

VocalfireAudioProcessorEditor::~VocalfireAudioProcessorEditor()
{
}

void VocalfireAudioProcessorEditor::applyPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= presetCount)
        return;

    const auto& preset = presets[presetIndex];
    auto& params = audioProcessor.parameters;

    auto setParameter = [&params](const char* id, float value)
    {
        if (auto* parameter = params.getParameter(id))
        {
            const float normalized =
                parameter->convertTo0to1(value);

            parameter->setValueNotifyingHost(normalized);
        }
    };

    setParameter("input", preset.input);
    setParameter("output", preset.output);
    setParameter("distortion", preset.distortion);
    setParameter("reverb", preset.reverb);
    setParameter("delay", preset.delay);
    setParameter("feedback", preset.feedback);
    setParameter("cutoff", preset.cutoff);

    // Choice parameters use normalized values.
    if (auto* key = params.getParameter("key"))
        key->setValueNotifyingHost(static_cast<float>(preset.key) / 11.0f);

    if (auto* scale = params.getParameter("scale"))
        scale->setValueNotifyingHost(static_cast<float>(preset.scale) / 2.0f);
}

void VocalfireAudioProcessorEditor::randomPreset()
{
    static std::mt19937 randomEngine { std::random_device{}() };
    std::uniform_int_distribution<int> distribution(0, presetCount - 1);

    const int index = distribution(randomEngine);
    presetBox.setSelectedId(index + 1, juce::sendNotification);
    applyPreset(index);
}

void VocalfireAudioProcessorEditor::savePreset()
{
    juce::FileChooser chooser(
        "Save VOCALFIRE preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("My VOCALFIRE Preset.vocalfire"),
        "*.vocalfire");

    if (chooser.browseForFileToSave(true))
    {
        auto state = audioProcessor.parameters.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());

        if (xml != nullptr)
            xml->writeTo(chooser.getResult());
    }
}

void VocalfireAudioProcessorEditor::loadPreset()
{
    juce::FileChooser chooser(
        "Load VOCALFIRE preset",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.vocalfire");

    if (chooser.browseForFileToOpen())
    {
        auto xml = juce::XmlDocument::parse(chooser.getResult());

        if (xml != nullptr)
        {
            auto state = juce::ValueTree::fromXml(*xml);

            if (state.isValid())
                audioProcessor.parameters.replaceState(state);
        }
    }
}

void VocalfireAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff090610));

    g.setColour(juce::Colour(0xffa855f7));
    g.setFont(juce::Font(27.0f, juce::Font::bold));
    g.drawText("VOCALFIRE", 24, 14, 300, 38,
               juce::Justification::left);

    g.setColour(juce::Colour(0xffc084fc));
    g.setFont(11.0f);
    g.drawText("VOCAL PROCESSING SYSTEM // VERSION 2.1",
               27, 49, 350, 18, juce::Justification::left);

    g.setColour(juce::Colour(0xff39204e));
    g.drawRoundedRectangle(15.0f, 78.0f,
                           getWidth() - 30.0f, 105.0f, 10.0f, 1.2f);

    g.drawRoundedRectangle(15.0f, 195.0f,
                           getWidth() - 30.0f, 205.0f, 10.0f, 1.2f);

    g.setColour(juce::Colour(0xffc084fc));
    g.setFont(12.0f);
    g.drawText("ARTIST PRESETS", 28, 84, 180, 20,
               juce::Justification::left);
    g.drawText("VOICE FX MODULES", 28, 201, 180, 20,
               juce::Justification::left);
    g.drawText("KEY / SCALE", 28, 414, 180, 20,
               juce::Justification::left);
}

void VocalfireAudioProcessorEditor::resized()
{
    presetBox.setBounds(28, 111, 280, 30);
    randomButton.setBounds(325, 111, 125, 30);
    saveButton.setBounds(465, 111, 115, 30);
    loadButton.setBounds(595, 111, 115, 30);

    const int sliderWidth = 95;
    const int sliderHeight = 145;
    const int startX = 27;
    const int gap = 9;
    const int y = 235;

    inputSlider.setBounds(startX + (sliderWidth + gap) * 0,
                          y, sliderWidth, sliderHeight);
    outputSlider.setBounds(startX + (sliderWidth + gap) * 1,
                           y, sliderWidth, sliderHeight);
    distortionSlider.setBounds(startX + (sliderWidth + gap) * 2,
                               y, sliderWidth, sliderHeight);
    reverbSlider.setBounds(startX + (sliderWidth + gap) * 3,
                           y, sliderWidth, sliderHeight);
    delaySlider.setBounds(startX + (sliderWidth + gap) * 4,
                          y, sliderWidth, sliderHeight);
    feedbackSlider.setBounds(startX + (sliderWidth + gap) * 5,
                             y, sliderWidth, sliderHeight);
    cutoffSlider.setBounds(startX + (sliderWidth + gap) * 6,
                           y, sliderWidth, sliderHeight);

    keyBox.setBounds(28, 440, 130, 28);
    scaleBox.setBounds(175, 440, 190, 28);
}
