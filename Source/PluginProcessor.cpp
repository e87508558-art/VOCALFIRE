#include "PluginProcessor.h"
#include "PluginEditor.h"

VocalfireAudioProcessor::VocalfireAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "VOCALFIRE_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VocalfireAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add(std::make_unique<APF>("input", "Input", juce::NormalisableRange<float>(-24.f, 12.f, .1f), 0.f));
    p.add(std::make_unique<APF>("output", "Output", juce::NormalisableRange<float>(-24.f, 12.f, .1f), 0.f));
    p.add(std::make_unique<APF>("mix", "FX Mix", juce::NormalisableRange<float>(0.f, 100.f, 1.f), 20.f));
    p.add(std::make_unique<APF>("reverb", "Reverb", juce::NormalisableRange<float>(0.f, 100.f, 1.f), 12.f));
    p.add(std::make_unique<APF>("delay", "Delay", juce::NormalisableRange<float>(0.f, 100.f, 1.f), 0.f));
    p.add(std::make_unique<APF>("feedback", "Feedback", juce::NormalisableRange<float>(0.f, 90.f, 1.f), 25.f));
    p.add(std::make_unique<APF>("cutoff", "Low Cut", juce::NormalisableRange<float>(40.f, 250.f, 1.f), 85.f));
    p.add(std::make_unique<APF>("pitch", "Pitch Correction", juce::NormalisableRange<float>(0.f, 100.f, 1.f), 0.f));
    p.add(std::make_unique<APC>("key", "Key", juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 0));
    p.add(std::make_unique<APC>("scale", "Scale", juce::StringArray{"Chromatic","Major","Minor"}, 2));
    return p;
}

void VocalfireAudioProcessor::prepareToPlay(double sr, int)
{
    currentSampleRate = sr;
    juce::dsp::ProcessSpec spec { sr, 512, (juce::uint32)getTotalNumOutputChannels() };
    reverb.reset();
    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.55f; rp.damping = 0.45f; rp.wetLevel = 0.0f; rp.dryLevel = 1.0f; rp.width = 1.0f;
    reverb.setParameters(rp);
    delay.prepare(spec);

    delay.setDelay(juce::jlimit(1.0f, 191999.0f, (float)(sr * 0.375)));
    delay.reset();
    auto coeff = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, 85.0);
    *lowCutL.coefficients = *coeff;
    *lowCutR.coefficients = *coeff;
    lowCutL.reset(); lowCutR.reset();
    monoAnalysis.setSize(1, 2048);
}

void VocalfireAudioProcessor::releaseResources() {}

bool VocalfireAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

void VocalfireAudioProcessor::analysePitch(const juce::AudioBuffer<float>& b)
{
    // Lightweight autocorrelation pitch tracker for the alpha UI meter.
    // This estimates the fundamental; it does not yet shift/correct audio.
    const int n = juce::jmin(2048, b.getNumSamples());
    if (n < 512) return;
    auto* x = monoAnalysis.getWritePointer(0);
    for (int i = 0; i < n; ++i)
    {
        float s = b.getSample(0, i);
        if (b.getNumChannels() > 1) s = 0.5f * (s + b.getSample(1, i));
        x[i] = s;
    }
    const int minLag = juce::jmax(2, (int)(currentSampleRate / 1000.0));
    const int maxLag = juce::jmin(n / 2, (int)(currentSampleRate / 65.0));
    float best = 0.f; int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double corr = 0.0, e1 = 0.0, e2 = 0.0;
        for (int i = 0; i < n - lag; i += 2)
        {
            const float a = x[i], c = x[i + lag];
            corr += a * c; e1 += a * a; e2 += c * c;
        }
        const float score = (float)(corr / std::sqrt(e1 * e2 + 1.0e-12));
        if (score > best) { best = score; bestLag = lag; }
    }
    if (bestLag > 0 && best > 0.55f)
    {
        const float hz = (float)currentSampleRate / (float)bestLag;
        const float midi = 69.f + 12.f * std::log2(hz / 440.f);
        detectedHz.store(hz); detectedNote.store(midi);
    }
}

void VocalfireAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumOutputChannels();
    for (int ch = getTotalNumInputChannels(); ch < channels; ++ch) buffer.clear(ch, 0, buffer.getNumSamples());

    analysePitch(buffer);

    const float inGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("input")->load());
    const float outGain = juce::Decibels::decibelsToGain(parameters.getRawParameterValue("output")->load());
    const float rv = parameters.getRawParameterValue("reverb")->load() / 100.f;
    const float dl = parameters.getRawParameterValue("delay")->load() / 100.f;
    const float fb = parameters.getRawParameterValue("feedback")->load() / 100.f;
    const float cut = parameters.getRawParameterValue("cutoff")->load();

    auto coeff = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, cut);
    *lowCutL.coefficients = *coeff; *lowCutR.coefficients = *coeff;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        auto& filter = (ch == 0 ? lowCutL : lowCutR);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float dry = filter.processSample(data[i] * inGain);
            float d = delay.popSample(ch);
            delay.pushSample(ch, dry + d * fb);
            data[i] = (dry * (1.f - dl) + d * dl) * outGain;
        }
    }

    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.62f; rp.damping = 0.42f; rp.wetLevel = rv * 0.45f;
    rp.dryLevel = 1.0f - rv * 0.25f; rp.width = 1.0f;
    reverb.setParameters(rp);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

void VocalfireAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void VocalfireAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalfireAudioProcessor();
}
