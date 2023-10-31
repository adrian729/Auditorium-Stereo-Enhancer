/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include "parameterTypes.h"
#include "PluginStateManager.h"
#include "Imager.h"

//==============================================================================
/**
*/
class AuditoriumStereoEnhancerAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	AuditoriumStereoEnhancerAudioProcessor();
	~AuditoriumStereoEnhancerAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	//==============================================================================
	void reset() override;

	//==============================================================================
	float getInputRmsValue() const;
	float getRmsValue(const int channel) const;

private:
	//==============================================================================
	// --- Object parameters management and information
	const juce::String APVTS_ID = "ParametersAPVTS";
	std::shared_ptr<PluginStateManager> stateManager;

	// -- General parameters
	ControlID bypassID{ ControlID::bypass };
	ControlID blendID{ ControlID::blend };
	ControlID preGainID{ ControlID::preGain };
	ControlID postGainID{ ControlID::postGain };

	//==============================================================================
	// --- Object member variables
	const int MONO_CHANNEL{ 0 };
	const int LEFT_CHANNEL{ 0 };
	const int RIGHT_CHANNEL{ 1 };

	// --- stage 0: General -- Bypass ALL // Blend (dry/wet)
	float bypass{ 0.f }; // -- using a float to smooth the bypass transition
	float blend{ 0.f };
	juce::dsp::DryWetMixer<float> blendMixer;
	juce::AudioBuffer<float> blendMixerBuffer; // -- buffer to replicate mono signal to all channels for the blend mixer

	// -- Mono Stages

	// -- Pre Gain
	float preGainGain{ 0.f };
	juce::dsp::Gain<float> preGain;

	// -- Multi channel stages
	// -- Imager
	Imager imager;

	// -- Post Gain
	float postGainGain{ 0.f };
	juce::dsp::Gain<float> postGain;

	//==============================================================================
	// --- Metering
	juce::LinearSmoothedValue<float> inputRmsLevel, rmsLevelLeft, rmsLevelRight;

	//==============================================================================
	void initBlendMixer(double sampleRate, int samplesPerBlock);

	//==============================================================================
	void preProcessBlock();
	void postProcessBlock(juce::AudioBuffer<float>&);

	void postUpdatePluginParameters();
	//==============================================================================
	float getLatency();

	//==============================================================================
	juce::dsp::AudioBlock<float> createDryBlock(juce::AudioBuffer<float>& buffer, int numChannels, int numSamples);

	//==============================================================================
	void updateRmsLevel(juce::AudioBuffer<float>& buffer, const int channel, juce::LinearSmoothedValue<float>& rmsLevel);

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuditoriumStereoEnhancerAudioProcessor)
};
