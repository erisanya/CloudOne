#pragma once

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/**
    Cloud One
    ---------
    A one-knob "brighter" EQ: a single high-shelf filter fixed at 8 kHz,
    boost only, 0-24 dB. No cut, no other bands, no extra controls.
*/
class CloudOneAudioProcessor : public juce::AudioProcessor
{
public:
    CloudOneAudioProcessor();
    ~CloudOneAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;

    static constexpr const char* brightnessParamID = "brightness";
    static constexpr float shelfFrequencyHz = 8000.0f;

    // 0.0 - 1.0, smoothed output level for the UI's reactive LED.
    float getOutputLevel() const noexcept { return outputLevel.load(); }

private:
    // Double precision internally: at high boost amounts, the tiny
    // rounding error in single-precision filter math gets amplified
    // along with the signal. Running the filter itself in double
    // keeps that error far below audibility even at +24 dB.
    using Filter       = juce::dsp::IIR::Filter<double>;
    using Coefficients = juce::dsp::IIR::Coefficients<double>;

    juce::dsp::ProcessorDuplicator<Filter, Coefficients> shelfFilter;
    juce::AudioBuffer<double> doubleBuffer;

    double lastSampleRate = 44100.0;
    float lastGainDb = -1000.0f;

    std::atomic<float> outputLevel { 0.0f };

    // LED meter envelope follower - same fast-attack/slower-release +
    // tanh gate mechanic as HYPERSCAPE's activity LED, just fed from
    // this plugin's own output signal, so the dot reacts identically.
    float meterEnvelope = 0.0f;
    float meterAttackCoeff = 0.0f;
    float meterReleaseCoeff = 0.0f;

    void updateFilter (float targetGainDb);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CloudOneAudioProcessor)
};
