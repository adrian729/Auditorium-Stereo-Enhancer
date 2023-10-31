#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include <math.h>
#include "Imager.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
Imager::Imager(
	std::shared_ptr<PluginStateManager> stateManager,
	ControlID bypassID,
	ControlID originalGainID,
	ControlID auxiliarGainID,
	ControlID widthID,
	ControlID centerID,
	ControlID delayTimeID,
	ControlID crossoverFreqID,
	ControlID imagerTypeID,
	bool monoToStereo // TODO: IMPLEMENT STEREO TO STEREO MODE, RIGHT NOW ONLY MONO TO STEREO
) :
	stateManager(stateManager),
	bypassID(bypassID),
	gainIDs{ originalGainID, auxiliarGainID },
	widthID(widthID),
	centerID(centerID),
	delayTimeID(delayTimeID),
	crossoverFreqID(crossoverFreqID),
	imagerTypeID(imagerTypeID),
	monoToStereo(monoToStereo)
{
	filters[FilterIDs::lowpass].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
	filters[FilterIDs::highpass].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
}

Imager::~Imager()
{
}

//==============================================================================
void Imager::prepare(const juce::dsp::ProcessSpec& spec)
{
	sampleRate = spec.sampleRate;

	bypass = stateManager->getFloatValue(bypassID);
	isBypassed = juce::approximatelyEqual(bypass, 1.f);

	for (int i{ 0 }; i < SignalID::countSignals; i++)
	{
		gains[i] = stateManager->getFloatValue(gainIDs[i]);
	}

	width = stateManager->getFloatValue(widthID);
	center = stateManager->getFloatValue(centerID);

	delayTime = stateManager->getFloatValue(delayTimeID);
	stereoImagerDelayLine.setDelay(getDelayTimeInSamples()); // -- delay time in samples (sampleRate * time in s)
	stereoImagerDelayLine.prepare(spec);

	crossoverFreq = stateManager->getFloatValue(crossoverFreqID);
	filters[FilterIDs::lowpass].setCutoffFrequency(crossoverFreq);
	filters[FilterIDs::highpass].setCutoffFrequency(crossoverFreq);
	filters[FilterIDs::lowpass].prepare(spec);
	filters[FilterIDs::highpass].prepare(spec);

	lowpassBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
	lowpassBuffer.clear();

	imagerType = intToEnum(stateManager->getChoiceIndex(imagerTypeID), ImagerTypes);
}

// TODO: check why in Haas mode it seems to be louder in the L channel than in the R channel (with same original and aux gains and center at 0, width > 0)
// TODO: stereo to stereo mode
// TODO: fix on load state, seems that the values of each control is not what showed until you move the slide/control

void Imager::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
	const auto& inputBlock = context.getInputBlock();
	auto& outputBlock = context.getOutputBlock();
	int numSamples = outputBlock.getNumSamples();

	const int numOutputChannels = context.getOutputBlock().getNumChannels();
	if (numOutputChannels < 2)
	{
		return;
	}

	preProcess();

	// -- Clear all channels except left and right
	for (int channel{ 2 }; channel < numOutputChannels; ++channel)
	{
		outputBlock.getSingleChannelBlock(channel).clear();
	}

	// -- If bypassed, copy mono input to left and right output
	if (isBypassed)
	{
		outputBlock.getSingleChannelBlock(LEFT_CHANNEL).copyFrom(inputBlock);
		outputBlock.getSingleChannelBlock(RIGHT_CHANNEL).copyFrom(inputBlock);
		return;
	}

	// -- Lowpass with original audio mono to stereo, and highpass with the imager processed signal
	lowpassBuffer.copyFrom(MONO_CHANNEL, 0, inputBlock.getChannelPointer(MONO_CHANNEL), numSamples); // TODO: buffer size is maxNumSamples, not numSamples. Check how we create audioBlocks with only numSamples
	lowpassBlock = juce::dsp::AudioBlock<float>(lowpassBuffer);
	lowpassBlock.multiplyBy(2.f);

	filters[FilterIDs::lowpass].process(juce::dsp::ProcessContextReplacing<float>(lowpassBlock));
	filters[FilterIDs::highpass].process(context);

	// -- Process stereo imager and generate final stereo signal
	const float* inSamples = inputBlock.getChannelPointer(MONO_CHANNEL);
	const float* lowpassSamples = lowpassBlock.getChannelPointer(MONO_CHANNEL);
	float* leftOutSamples = outputBlock.getChannelPointer(LEFT_CHANNEL);
	float* rightOutSamples = outputBlock.getChannelPointer(RIGHT_CHANNEL);

	switch (imagerType)
	{
	case Imager::haasMidSide:
	{
		processHaasMidSide(
			numSamples,
			inSamples,
			lowpassSamples,
			leftOutSamples,
			rightOutSamples
		);
		break;
	}
	case Imager::haasMono:
	{
		processHaasMono(
			numSamples,
			inSamples,
			lowpassSamples,
			leftOutSamples,
			rightOutSamples
		);
		break;
	}
	case Imager::haas:
	{
		processHaas(
			numSamples,
			inSamples,
			lowpassSamples,
			leftOutSamples,
			rightOutSamples
		);
		break;
	}
	}
}

void Imager::reset()
{
	stereoImagerDelayLine.reset();
}

//==============================================================================
float Imager::getLatency()
{
	// TODO: implement and check latency
	return 0.0f;
}

//==============================================================================
void Imager::setSampleRate(int newSampleRate)
{
	sampleRate = newSampleRate;
}

//==============================================================================
float Imager::getDelayTimeInSamples()
{
	return sampleRate * delayTime / 1000.f;
}

//==============================================================================
void Imager::preProcess()
{
	float newBypass = stateManager->getCurrentValue(bypassID);
	bool bypassChanged = !juce::approximatelyEqual(newBypass, bypass);
	bypass = newBypass;

	isBypassed = juce::approximatelyEqual(bypass, 1.0f);
	if (isBypassed)
	{
		return;
	}

	// TODO: check how to ramp gain to +0dB when bypassing changes
	for (int i{ 0 }; i < SignalID::countSignals; i++)
	{
		gains[i] = stateManager->getCurrentValue(gainIDs[i]);
	}

	width = stateManager->getCurrentValue(widthID);
	center = stateManager->getCurrentValue(centerID);

	float newDelayTime = stateManager->getCurrentValue(delayTimeID);
	if (!juce::approximatelyEqual(newDelayTime, delayTime) || bypassChanged)
	{
		delayTime = newDelayTime;
		stereoImagerDelayLine.setDelay((1.f - bypass) * getDelayTimeInSamples());
	}

	float newCrossoverFreq = stateManager->getCurrentValue(crossoverFreqID);
	if (!juce::approximatelyEqual(newCrossoverFreq, crossoverFreq))
	{
		crossoverFreq = newCrossoverFreq;
		filters[FilterIDs::lowpass].setCutoffFrequency(crossoverFreq);
		filters[FilterIDs::highpass].setCutoffFrequency(crossoverFreq);
	}

	// TODO: implement smoothing for imager type changes
	imagerType = intToEnum(stateManager->getChoiceIndex(imagerTypeID), ImagerTypes);
}

//==============================================================================
void Imager::processHaasMidSide(
	int numSamples,
	const float* inSamples,
	const float* lowpassSamples,
	float* leftOutSamples,
	float* rightOutSamples
)
{
	float lpan = center <= 0.f ? 1.f : std::sin((1.f - center) * M_PI_2);
	float rpan = center >= 0.f ? 1.f : std::sin((1.f + center) * M_PI_2);
	float coef_S = 1.5f * width;

	for (int i{ 0 }; i < numSamples; i++)
	{
		stereoImagerDelayLine.pushSample(MONO_CHANNEL, inSamples[i]);

		float originalSample = inSamples[i] * gains[SignalID::original];
		float auxSample = stereoImagerDelayLine.popSample(MONO_CHANNEL) * gains[SignalID::auxiliar];

		float mid = 2.f * originalSample;
		float side = auxSample * coef_S;
		leftOutSamples[i] = lpan * (mid - side);
		rightOutSamples[i] = rpan * (mid + side);

		// -- Add lowpass signal to both channels
		leftOutSamples[i] += lowpassSamples[i];
		rightOutSamples[i] += lowpassSamples[i];
	}
}

void Imager::processHaasMono(
	int numSamples,
	const float* inSamples,
	const float* lowpassSamples,
	float* leftOutSamples,
	float* rightOutSamples
)
{
	float leftCoef = center - 2.f * width;
	float rightCoef = center + 2.f * width;

	for (int i{ 0 }; i < numSamples; i++)
	{
		stereoImagerDelayLine.pushSample(MONO_CHANNEL, inSamples[i]);

		float originalSample = inSamples[i] * gains[SignalID::original];
		float auxSample = stereoImagerDelayLine.popSample(MONO_CHANNEL) * gains[SignalID::auxiliar];

		leftOutSamples[i] = 2.f * originalSample + leftCoef * auxSample;
		rightOutSamples[i] = 2.f * originalSample + rightCoef * auxSample;

		// -- Add lowpass signal to both channels
		leftOutSamples[i] += lowpassSamples[i];
		rightOutSamples[i] += lowpassSamples[i];
	}
}

void Imager::processHaas(
	int numSamples,
	const float* inSamples,
	const float* lowpassSamples,
	float* leftOutSamples,
	float* rightOutSamples
)
{
	// -- Panning distance between original/auxiliar and center. original panned to -width and auxiliar to +width
	// -- Panning center displacement
	// --  f.e. If center at 0 and width at 0.5, original signal will be panned to -0.5 and auxiliar to 0.5 (range L[-1, 1]R), or 0.25 and 0.75 (range L[0, 1]R)
	// -- f.e. If center at 0.2 and width 0.6, original signal will be panned at -0.4 and aux to 0.8 (range L[-1, 1]R), or 0.3 and 0.9 (range L[0, 1]R)

	// -- Panning coefficients -- range L[0, 1]R 0 full left, 1 full right, 0.5 center
	float origPanning = (juce::jlimit(-1.f, 1.f, center - width) + 1.f) * .5f;
	float auxPanning = (juce::jlimit(-1.f, 1.f, center + width) + 1.f) * .5f;
	float auxCompensation = 1.2f;
	float gainCompensation = 2.f;

	for (int i{ 0 }; i < numSamples; i++)
	{
		stereoImagerDelayLine.pushSample(MONO_CHANNEL, inSamples[i]);

		float originalSample = inSamples[i] * gains[SignalID::original];
		float auxSample = stereoImagerDelayLine.popSample(MONO_CHANNEL) * gains[SignalID::auxiliar];

		leftOutSamples[i] = gainCompensation * (1.f - origPanning) * originalSample + gainCompensation * auxCompensation * (1.f - auxPanning) * auxSample;
		rightOutSamples[i] = gainCompensation * origPanning * originalSample + gainCompensation * auxCompensation * auxPanning * auxSample;

		// -- Add lowpass signal to both channels
		leftOutSamples[i] += lowpassSamples[i];
		rightOutSamples[i] += lowpassSamples[i];
	}
}