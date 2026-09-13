#include "PluginProcessor.h"
#include "PluginEditor.h"

CloudOneAudioProcessor::CloudOneAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS",
      {
          std::make_unique<juce::AudioParameterFloat> (
              juce::ParameterID { brightnessParamID, 1 },
              "Brightness",
              juce::NormalisableRange<float> (0.0f, 24.0f, 0.1f),
              0.0f,
              juce::AudioParameterFloatAttributes().withLabel ("dB"))
      })
{
}

CloudOneAudioProcessor::~CloudOneAudioProcessor() = default;

void CloudOneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) getTotalNumOutputChannels();

    shelfFilter.prepare (spec);
    doubleBuffer.setSize ((int) spec.numChannels, samplesPerBlock, false, false, true);

    lastGainDb = -1000.0f; // force the first updateFilter call to actually apply
    updateFilter (*parameters.getRawParameterValue (brightnessParamID));
}

void CloudOneAudioProcessor::releaseResources() {}

bool CloudOneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainOut == layouts.getMainInputChannelSet();
}

void CloudOneAudioProcessor::updateFilter (float targetGainDb)
{
    if (juce::approximatelyEqual (targetGainDb, lastGainDb))
        return;

    lastGainDb = targetGainDb;

    // Q of 0.707 (Butterworth) keeps the shelf transition smooth with no
    // resonant peak at the knee. Coefficients are recomputed the instant
    // the knob moves — no ramping, matching a standard parametric EQ.
    const double gainLinear = juce::Decibels::decibelsToGain ((double) targetGainDb);
    *shelfFilter.state = *Coefficients::makeHighShelf (lastSampleRate, (double) shelfFrequencyHz, 0.707, gainLinear);
}

void CloudOneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateFilter (*parameters.getRawParameterValue (brightnessParamID));

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    if (doubleBuffer.getNumChannels() < numChannels || doubleBuffer.getNumSamples() < numSamples)
        doubleBuffer.setSize (numChannels, numSamples, false, false, true);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* src = buffer.getReadPointer (ch);
        auto* dst = doubleBuffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
            dst[i] = (double) src[i];
    }

    juce::dsp::AudioBlock<double> block (doubleBuffer.getArrayOfWritePointers(),
                                          (size_t) numChannels, (size_t) numSamples);
    juce::dsp::ProcessContextReplacing<double> context (block);
    shelfFilter.process (context);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* src = doubleBuffer.getReadPointer (ch);
        auto* dst = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
            dst[i] = (float) src[i];
    }
}

juce::AudioProcessorEditor* CloudOneAudioProcessor::createEditor()
{
    return new CloudOneAudioProcessorEditor (*this);
}

bool CloudOneAudioProcessor::hasEditor() const { return true; }

const juce::String CloudOneAudioProcessor::getName() const { return JucePlugin_Name; }

bool CloudOneAudioProcessor::acceptsMidi() const  { return false; }
bool CloudOneAudioProcessor::producesMidi() const { return false; }
bool CloudOneAudioProcessor::isMidiEffect() const { return false; }
double CloudOneAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int CloudOneAudioProcessor::getNumPrograms() { return 1; }
int CloudOneAudioProcessor::getCurrentProgram() { return 0; }
void CloudOneAudioProcessor::setCurrentProgram (int) {}
const juce::String CloudOneAudioProcessor::getProgramName (int) { return {}; }
void CloudOneAudioProcessor::changeProgramName (int, const juce::String&) {}

void CloudOneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = parameters.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void CloudOneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

// This creates the instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CloudOneAudioProcessor();
}
