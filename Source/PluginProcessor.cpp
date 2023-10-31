/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/
#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AuditoriumStereoEnhancerAudioProcessor::AuditoriumStereoEnhancerAudioProcessor()
	: AudioProcessor(BusesProperties()
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
	),
	stateManager(std::make_shared<PluginStateManager>(
		PluginStateManager(*this, nullptr, juce::Identifier(APVTS_ID))
	)),
	imager(
		stateManager,
		ControlID::imagerBypass,
		ControlID::imagerOriginalGain,
		ControlID::imagerAuxiliarGain,
		ControlID::imagerWidth,
		ControlID::imagerCenter,
		ControlID::imagerDelayTime,
		ControlID::imagerCrossoverFreq,
		ControlID::imagerType
	)
{
}

AuditoriumStereoEnhancerAudioProcessor::~AuditoriumStereoEnhancerAudioProcessor()
{
}

//==============================================================================
const juce::String AuditoriumStereoEnhancerAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool AuditoriumStereoEnhancerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool AuditoriumStereoEnhancerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool AuditoriumStereoEnhancerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double AuditoriumStereoEnhancerAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int AuditoriumStereoEnhancerAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int AuditoriumStereoEnhancerAudioProcessor::getCurrentProgram()
{
	return 0;
}

void AuditoriumStereoEnhancerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String AuditoriumStereoEnhancerAudioProcessor::getProgramName(int index)
{
	return {};
}

void AuditoriumStereoEnhancerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void AuditoriumStereoEnhancerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	auto busesLayout = getBusesLayout();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();

	// -- Multi channel Chain
	juce::dsp::ProcessSpec spec{};
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = totalNumOutputChannels;

	// -- Bypass
	bypass = stateManager->getFloatValue(bypassID);

	// -- Blend
	initBlendMixer(sampleRate, samplesPerBlock);

	// -- pre gain
	preGainGain = stateManager->getFloatValue(preGainID);
	preGain.setRampDurationSeconds(0.01f);
	preGain.setGainDecibels(preGainGain);
	preGain.prepare(spec);

	// -- Stereo Imager
	imager.prepare(spec);

	// -- post gain
	postGainGain = stateManager->getFloatValue(postGainID);
	postGain.setRampDurationSeconds(0.01f);
	postGain.setGainDecibels(postGainGain);
	postGain.prepare(spec);

	// -- Setup smoothing
	stateManager->initSmoothedValues(sampleRate);

	// -- Meters
	inputRmsLevel.reset(sampleRate, 0.5f);
	rmsLevelLeft.reset(sampleRate, 0.5f);
	rmsLevelRight.reset(sampleRate, 0.5f);

	inputRmsLevel.setCurrentAndTargetValue(-100.f);
	rmsLevelLeft.setCurrentAndTargetValue(-100.f);
	rmsLevelRight.setCurrentAndTargetValue(-100.f);
}

void AuditoriumStereoEnhancerAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

bool AuditoriumStereoEnhancerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled()
		|| layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
	{
		return false;
	}

	// -- mono/stereo and stereo/stereo supported
	if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono() || layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
	{
		return false;
	}

	return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AuditoriumStereoEnhancerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
	juce::ScopedNoDenormals noDenormals;

	auto busesLayout = getBusesLayout();
	auto totalNumInputChannels = busesLayout.getMainInputChannels();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();
	auto numSamples = buffer.getNumSamples();

	// -- clear output channels not in use
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
	{
		buffer.clear(i, 0, numSamples);
	}

	preProcessBlock();

	if (!juce::approximatelyEqual(bypass, 1.f))
	{
		// -- create the audio blocks and context
		juce::dsp::AudioBlock<float> audioBlock(buffer);
		juce::dsp::ProcessContextReplacing<float> context(audioBlock);

		//const auto& inputBlock = context.getInputBlock();
		auto& outputBlock = context.getOutputBlock();

		// -- prepare dry wet mixer -- dryWetMixer needs an AudioBlock with same number of input and output channels
		blendMixer.setWetLatency(getLatency());
		blendMixer.pushDrySamples(createDryBlock(buffer, totalNumOutputChannels, numSamples));

		preGain.process(context);

		updateRmsLevel(buffer, MONO_CHANNEL, inputRmsLevel);

		imager.process(context);

		postGain.process(context);

		// -- mix dry wet
		blendMixer.mixWetSamples(outputBlock);
	}

	postProcessBlock(buffer);
}

//==============================================================================
bool AuditoriumStereoEnhancerAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AuditoriumStereoEnhancerAudioProcessor::createEditor()
{
	return new AuditoriumStereoEnhancerAudioProcessorEditor(*this, stateManager);
	//return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void AuditoriumStereoEnhancerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = stateManager->getAPVTS()->copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void AuditoriumStereoEnhancerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr && xmlState->hasTagName(stateManager->getAPVTS()->state.getType()))
	{
		stateManager->getAPVTS()->replaceState(juce::ValueTree::fromXml(*xmlState));
	}
}

//==============================================================================
void AuditoriumStereoEnhancerAudioProcessor::reset()
{
	blendMixer.reset();
	preGain.reset();
	imager.reset();
	postGain.reset();
}

//==============================================================================
float AuditoriumStereoEnhancerAudioProcessor::getInputRmsValue() const
{
	return inputRmsLevel.getCurrentValue();
}

float AuditoriumStereoEnhancerAudioProcessor::getRmsValue(const int channel) const
{
	jassert(channel == 0 || channel == 1);
	if (channel == 0)
	{
		return rmsLevelLeft.getCurrentValue();
	}
	else if (channel == 1)
	{
		return rmsLevelRight.getCurrentValue();
	}

	return -100.f;
}

//==============================================================================
void  AuditoriumStereoEnhancerAudioProcessor::initBlendMixer(double sampleRate, int samplesPerBlock)
{
	auto busesLayout = getBusesLayout();
	auto totalNumOutputChannels = busesLayout.getMainOutputChannels();

	juce::dsp::ProcessSpec mixerSpec{};
	mixerSpec.sampleRate = sampleRate;
	mixerSpec.maximumBlockSize = samplesPerBlock;
	mixerSpec.numChannels = totalNumOutputChannels;

	blend = stateManager->getFloatValue(blendID);

	blendMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
	blendMixer.setWetMixProportion(blend);
	blendMixer.prepare(mixerSpec);

	blendMixerBuffer.setSize(totalNumOutputChannels, samplesPerBlock); // -- allocate space
	blendMixerBuffer.clear();
}


//==============================================================================
void AuditoriumStereoEnhancerAudioProcessor::preProcessBlock()
{
	stateManager->syncInBoundVariables();
	postUpdatePluginParameters();
}

void AuditoriumStereoEnhancerAudioProcessor::postProcessBlock(juce::AudioBuffer<float>& buffer)
{
	updateRmsLevel(buffer, LEFT_CHANNEL, rmsLevelLeft);
	updateRmsLevel(buffer, RIGHT_CHANNEL, rmsLevelRight);
}

void AuditoriumStereoEnhancerAudioProcessor::postUpdatePluginParameters()
{
	float newBypass = stateManager->getCurrentValue(bypassID);
	float newBlend = stateManager->getCurrentValue(blendID);

	if (!juce::approximatelyEqual(newBypass, bypass) || !juce::approximatelyEqual(newBlend, blend))
	{
		bypass = newBypass;
		blend = newBlend;
		blendMixer.setWetMixProportion(std::max(0.f, blend * (1.f - bypass))); // -- if bypassing, all dry. bypass should smooth from 0.f to 1.f when selected.
	}

	float newPreGainGain = stateManager->getCurrentValue(preGainID);
	if (!juce::approximatelyEqual(newPreGainGain, preGainGain))
	{
		preGain.setGainDecibels(stateManager->getCurrentValue(preGainID));
	}

	float newPostGainGain = stateManager->getCurrentValue(postGainID);
	if (!juce::approximatelyEqual(newPostGainGain, postGainGain))
	{
		postGain.setGainDecibels(stateManager->getCurrentValue(postGainID));
	}
}

float AuditoriumStereoEnhancerAudioProcessor::getLatency()
{
	return 0.f; // TODO: update this
}

//==============================================================================
juce::dsp::AudioBlock<float> AuditoriumStereoEnhancerAudioProcessor::createDryBlock(juce::AudioBuffer<float>& buffer, int numChannels, int numSamples)
{
	auto* inputDataPointer = buffer.getReadPointer(0);
	blendMixerBuffer.clear();
	for (int i{ 0 }; i < numChannels; i++)
	{
		blendMixerBuffer.copyFrom(i, 0, inputDataPointer, numSamples);
	}
	return juce::dsp::AudioBlock<float>(blendMixerBuffer);
}

//==============================================================================
void AuditoriumStereoEnhancerAudioProcessor::updateRmsLevel(juce::AudioBuffer<float>& buffer, const int channel, juce::LinearSmoothedValue<float>& rmsLevel)
{
	rmsLevel.skip(buffer.getNumSamples());

	const auto value = juce::Decibels::gainToDecibels(buffer.getRMSLevel(channel, 0, buffer.getNumSamples()));
	if (value < rmsLevel.getCurrentValue())
	{
		rmsLevel.setTargetValue(value);
	}
	else
	{
		rmsLevel.setCurrentAndTargetValue(value);
	}
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new AuditoriumStereoEnhancerAudioProcessor();
}
