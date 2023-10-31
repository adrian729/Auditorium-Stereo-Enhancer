/*
  ==============================================================================

	PluginStateManager.cpp
	Created: 1 Sep 2023 1:42:27pm
	Author:  Brutus729

  ==============================================================================
*/

#include "PluginStateManager.h"

//==============================================================================
// -- CONSTRUCTORS
//==============================================================================
PluginStateManager::PluginStateManager(
	juce::AudioProcessor& processorToConnectTo,
	juce::UndoManager* undoManagerToUse,
	const juce::Identifier& valueTreeType
) :
	apvts_ptr(
		new juce::AudioProcessorValueTreeState(
			processorToConnectTo,
			undoManagerToUse,
			valueTreeType,
			initParamsAndCreateLayout()
		)
	)
{
	attachParameters(*apvts_ptr);
	syncInBoundVariables();
}

PluginStateManager::~PluginStateManager()
{
	for (auto& param : parameters)
	{
		param = nullptr;
	}
	delete apvts_ptr;
}

//==============================================================================
// TODO: implement copy/move constructors and copy/move assignment operators
PluginStateManager::PluginStateManager(const PluginStateManager& other) : // copy constructor
	apvts_ptr(other.apvts_ptr),
	parameterIDs(other.parameterIDs),
	parameterTypes(other.parameterTypes),
	parameters(other.parameters),
	smoothingTypes(other.smoothingTypes),
	smoothingValueIndexes(other.smoothingValueIndexes),
	linearSmoothedValues(other.linearSmoothedValues),
	multiplicativeSmoothedValues(other.multiplicativeSmoothedValues)
{
}

// TODO: check how to implement move constructor correctly for array variables
PluginStateManager::PluginStateManager(PluginStateManager&& other) noexcept : // move constructor
	apvts_ptr(std::exchange(other.apvts_ptr, nullptr)),
	parameterIDs(other.parameterIDs),
	parameterTypes(other.parameterTypes),
	parameters(other.parameters),
	smoothingTypes(other.smoothingTypes),
	smoothingValueIndexes(other.smoothingValueIndexes),
	linearSmoothedValues(other.linearSmoothedValues),
	multiplicativeSmoothedValues(other.multiplicativeSmoothedValues)
{
}

PluginStateManager& PluginStateManager::operator=(const PluginStateManager& other) // copy assignment
{
	if (this == &other)
	{
		return *this;
	}
	apvts_ptr = other.apvts_ptr;
	parameterIDs = other.parameterIDs;
	parameterTypes = other.parameterTypes;
	parameters = other.parameters;
	smoothingTypes = other.smoothingTypes;
	smoothingValueIndexes = other.smoothingValueIndexes;
	linearSmoothedValues = other.linearSmoothedValues;
	multiplicativeSmoothedValues = other.multiplicativeSmoothedValues;

	return *this;
}

PluginStateManager& PluginStateManager::operator=(PluginStateManager&& other) noexcept // move assignment
{
	if (this == &other)
	{
		return *this;
	}
	std::swap(apvts_ptr, other.apvts_ptr);
	std::swap(parameterIDs, other.parameterIDs);
	std::swap(parameterTypes, other.parameterTypes);
	std::swap(parameters, other.parameters);
	std::swap(smoothingTypes, other.smoothingTypes);
	std::swap(smoothingValueIndexes, other.smoothingValueIndexes);
	std::swap(linearSmoothedValues, other.linearSmoothedValues);
	std::swap(multiplicativeSmoothedValues, other.multiplicativeSmoothedValues);

	return *this;
}

//==============================================================================
// // --- APVTS
juce::AudioProcessorValueTreeState* PluginStateManager::getAPVTS()
{
	return apvts_ptr;
}

// -- Parameters
void PluginStateManager::attachParameters(const juce::AudioProcessorValueTreeState& apvts)
{
	for (int i{ 0 }; i < ControlID::countParams; i++)
	{
		parameters[i] = std::make_shared<ParameterObject>(
			ParameterObject(
				apvts.getParameter(parameterIDs[i]),
				parameterTypes[i]
			)
		);
	}
}

std::shared_ptr<ParameterObject> PluginStateManager::getParameterObject(ControlID controlID)
{
	return parameters[controlID];
}

juce::RangedAudioParameter* PluginStateManager::getParameter(ControlID controlID)
{
	return parameters[controlID]->getParameter();
}

// -- Inbound Values
bool PluginStateManager::getBoolValue(ControlID controlID)
{
	return parameters[controlID]->getBoolValue();
}

int PluginStateManager::getChoiceIndex(ControlID controlID)
{
	return parameters[controlID]->getChoiceIndex();
}

template <typename ChoiceType>
ChoiceType PluginStateManager::getChoiceValue(ControlID controlID)
{
	return parameters[controlID]->getChoiceValue<ChoiceType>();
}

float PluginStateManager::getFloatValue(ControlID controlID)
{
	return parameters[controlID]->getFloatValue();
}

int PluginStateManager::getIntValue(ControlID controlID)
{
	return parameters[controlID]->getIntValue();
}

// -- Smoothing
void PluginStateManager::initSmoothedValues(double sampleRate)
{
	resetSmoothedValues(sampleRate);
	for (int i{ 0 }; i < ControlID::countParams; i++)
	{
		setCurrentAndTargetValue(intToEnum(i, ControlID), parameters[i]->getFloatValue());
	}
}

void PluginStateManager::resetSmoothedValues(double sampleRate)
{
	for (auto& smoothedValue : linearSmoothedValues)
	{
		smoothedValue.smoothedValue.reset(sampleRate, smoothedValue.rampLengthInSeconds);
	}

	for (auto& smoothedValue : multiplicativeSmoothedValues)
	{
		smoothedValue.smoothedValue.reset(sampleRate, smoothedValue.rampLengthInSeconds);
	}
}

void PluginStateManager::reset(ControlID controlID, double sampleRate, double rampLengthInSeconds)
{
	SmoothingType smoothingType = smoothingTypes[controlID];
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return;
	}

	int idx = smoothingValueIndexes[controlID];
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		linearSmoothedValues[idx].rampLengthInSeconds = rampLengthInSeconds;
		linearSmoothedValues[idx].smoothedValue.reset(sampleRate, rampLengthInSeconds);
		break;
	case SmoothingType::Multiplicative:
		multiplicativeSmoothedValues[idx].rampLengthInSeconds = rampLengthInSeconds;
		multiplicativeSmoothedValues[idx].smoothedValue.reset(sampleRate, rampLengthInSeconds);
		break;
	}
}


float PluginStateManager::getCurrentValue(ControlID controlID)
{
	SmoothingType smoothingType = smoothingTypes[controlID];
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return parameters[controlID]->getFloatValue();
	}

	int idx = smoothingValueIndexes[controlID];
	float normalizedValue = 0.f;
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		normalizedValue = linearSmoothedValues[idx].smoothedValue.getCurrentValue();
		break;
	case SmoothingType::Multiplicative:
		normalizedValue = multiplicativeSmoothedValues[idx].smoothedValue.getCurrentValue();
	}
	// Denormalize value to the selected range
	return parameters[controlID]->getParameterRange().convertFrom0to1(safeDenormalizableValue(normalizedValue));
}

float PluginStateManager::getNextValue(ControlID controlID)
{
	SmoothingType smoothingType = smoothingTypes[controlID];
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return parameters[controlID]->getFloatValue();
	}

	int idx = smoothingValueIndexes[controlID];
	float normalizedValue = 0.f;
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		normalizedValue = linearSmoothedValues[idx].smoothedValue.getNextValue();
		break;
	case SmoothingType::Multiplicative:
		normalizedValue = multiplicativeSmoothedValues[idx].smoothedValue.getNextValue();
	}
	// Denormalize value to the selected range
	return parameters[controlID]->getParameterRange().convertFrom0to1(safeDenormalizableValue(normalizedValue));
}

void PluginStateManager::setCurrentAndTargetValue(ControlID controlID, float newValue)
{
	SmoothingType smoothingType = smoothingTypes[controlID];
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return;
	}

	int idx = smoothingValueIndexes[controlID];
	// Need to normalize the inBound variable to the range [0, 1] first
	float normalizedNewValue = parameters[controlID]->getParameterRange().convertTo0to1(newValue);
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		linearSmoothedValues[idx].smoothedValue.setCurrentAndTargetValue(normalizedNewValue);
		break;
	case SmoothingType::Multiplicative:
		multiplicativeSmoothedValues[idx].smoothedValue.setCurrentAndTargetValue(safeMultiplicativeValue(normalizedNewValue));
		break;
	}
}

void PluginStateManager::setTargetValue(ControlID controlID, float newValue)
{
	SmoothingType smoothingType = smoothingTypes[controlID];
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return;
	}

	int idx = smoothingValueIndexes[controlID];
	// Need to normalize the inBound variable to the range [0, 1] first
	float normalizedNewValue = parameters[controlID]->getParameterRange().convertTo0to1(newValue);
	switch (smoothingType)
	{
	case SmoothingType::Linear:
		linearSmoothedValues[idx].smoothedValue.setTargetValue(normalizedNewValue);
		break;
	case SmoothingType::Multiplicative:
		multiplicativeSmoothedValues[idx].smoothedValue.setTargetValue(safeMultiplicativeValue(normalizedNewValue));
		break;
	}
}

//==============================================================================
// -- PARAMETERS CREATION
//==============================================================================
// -- BOOL
inline void PluginStateManager::addParam(
	juce::AudioProcessorValueTreeState::ParameterLayout& layout,
	ControlID controlID,
	juce::String id,
	int versionHint,
	const juce::String& name,
	bool defaultValue,
	const juce::String& suffixLabel,
	SmoothingType smoothingType,
	double rampLengthInSeconds
)
{
	layout.add(
		std::make_unique<juce::AudioParameterBool>(
			juce::ParameterID{ id, versionHint },
			name,
			defaultValue,
			juce::AudioParameterBoolAttributes().withLabel(suffixLabel)
		)
	);
	parameterIDs[controlID] = id;
	parameterTypes[controlID] = ParameterType::Bool;
	addSmoothedValue(controlID, smoothingType, defaultValue, rampLengthInSeconds);
}
// -- CHOICE CONSTRUCTOR
inline void PluginStateManager::addParam(
	juce::AudioProcessorValueTreeState::ParameterLayout& layout,
	ControlID controlID,
	juce::String id,
	int versionHint,
	const juce::String& name,
	const juce::StringArray& choices,
	int defaultItemIndex,
	const juce::String& suffixLabel,
	SmoothingType smoothingType,
	double rampLengthInSeconds
)
{
	layout.add(
		std::make_unique<juce::AudioParameterChoice>(
			juce::ParameterID{ id, versionHint },
			name,
			choices,
			defaultItemIndex,
			juce::AudioParameterChoiceAttributes().withLabel(suffixLabel)
		)
	);
	parameterIDs[controlID] = id;
	parameterTypes[controlID] = ParameterType::Choice;
	addSmoothedValue(controlID, smoothingType, defaultItemIndex, rampLengthInSeconds);
}
// -- FLOAT CONSTRUCTOR
inline void PluginStateManager::addParam(
	juce::AudioProcessorValueTreeState::ParameterLayout& layout,
	ControlID controlID,
	juce::String id,
	int versionHint,
	const juce::String& name,
	const juce::NormalisableRange<float>& floatRange,
	float defaultValue,
	const juce::String& suffixLabel,
	SmoothingType smoothingType,
	double rampLengthInSeconds
)
{
	layout.add(
		std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ id, versionHint },
			name,
			floatRange,
			defaultValue,
			juce::AudioParameterFloatAttributes().withLabel(suffixLabel)
		)
	);
	parameterIDs[controlID] = id;
	parameterTypes[controlID] = ParameterType::Float;
	addSmoothedValue(controlID, smoothingType, defaultValue, rampLengthInSeconds);
}
// -- INT CONSTRUCTOR
inline void PluginStateManager::addParam(
	juce::AudioProcessorValueTreeState::ParameterLayout& layout,
	ControlID controlID,
	juce::String id,
	int versionHint,
	const juce::String& name,
	int minValue,
	int maxValue,
	int defaultValue,
	const juce::String& suffixLabel,
	SmoothingType smoothingType,
	double rampLengthInSeconds
)
{
	layout.add(
		std::make_unique<juce::AudioParameterInt>(
			juce::ParameterID{ id, versionHint },
			name,
			minValue,
			maxValue,
			defaultValue,
			juce::AudioParameterIntAttributes().withLabel(suffixLabel)
		)
	);
	parameterIDs[controlID] = id;
	parameterTypes[controlID] = ParameterType::Int;
	addSmoothedValue(controlID, smoothingType, defaultValue, rampLengthInSeconds);
}

//==============================================================================
void PluginStateManager::syncInBoundVariables()
{
	for (int i{ 0 }; i < ControlID::countParams; i++)
	{
		float lastValue = parameters[i]->getInBoundVariable();
		float newValue = parameters[i]->updateInBoundVariable();
		updateSmoothingValue(intToEnum(i, ControlID), lastValue, newValue);
	}
}

void PluginStateManager::updateSmoothingValue(ControlID controlID, float lastValue, float newValue)
{
	SmoothingType smoothingType = smoothingTypes[controlID];
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		return;
	}

	int idx = smoothingValueIndexes[controlID];
	bool valueChanged = !juce::approximatelyEqual(lastValue, newValue);
	// Need to normalize the inBound variable to the range [0, 1] first
	float normalizedNewValue = valueChanged ? parameters[controlID]->getParameterRange().convertTo0to1(newValue) : 0.f;
	switch (smoothingType)
	{
	case SmoothingType::Linear:
	{
		auto& smoothedValue = linearSmoothedValues[idx].smoothedValue;
		if (valueChanged)
		{
			smoothedValue.setTargetValue(normalizedNewValue);
		}
		smoothedValue.getNextValue();
		break;
	}
	case SmoothingType::Multiplicative:
	{
		auto& smoothedValue = multiplicativeSmoothedValues[idx].smoothedValue;
		if (valueChanged)
		{
			smoothedValue.setTargetValue(safeMultiplicativeValue(normalizedNewValue));
		}
		smoothedValue.getNextValue();
		break;
	}
	}
}

//==============================================================================
template<typename T>
inline void PluginStateManager::addSmoothedValue(
	ControlID controlID,
	SmoothingType smoothingType,
	T initialValue,
	double rampLengthInSeconds
)
{
	smoothingTypes[controlID] = smoothingType;
	if (smoothingType == SmoothingType::NoSmoothing)
	{
		smoothingValueIndexes[controlID] = -1;
		return;
	}

	float initialValAsFloat = static_cast<float>(initialValue);
	int idx;
	switch (smoothingType)
	{
	case SmoothingType::Linear:
	{
		idx = linearSmoothedValues.size();
		SmoothedValue<juce::LinearSmoothedValue<float>> linearSmoothedValue(
			controlID,
			juce::LinearSmoothedValue<float>(initialValAsFloat),
			rampLengthInSeconds
		);
		linearSmoothedValues.push_back(linearSmoothedValue);
		break;
	}
	case SmoothingType::Multiplicative:
	{
		int idx = multiplicativeSmoothedValues.size();
		SmoothedValue<juce::SmoothedValue<float, Multiplicative>> multiplicativeSmoothedValue(
			controlID,
			juce::SmoothedValue<float, Multiplicative>(safeMultiplicativeValue(initialValAsFloat)),
			rampLengthInSeconds
		);
		multiplicativeSmoothedValues.push_back(multiplicativeSmoothedValue);
		break;
	}
	}
	smoothingValueIndexes[controlID] = idx;
}

//==============================================================================
// -- PRIVATE METHODS
//==============================================================================
inline float PluginStateManager::safeMultiplicativeValue(float value, float smallestValue)
{
	return juce::approximatelyEqual(value, 0.f) ? smallestValue : value;
}

inline float PluginStateManager::safeDenormalizableValue(float value)
{
	return juce::jlimit(0.f, 1.f, value);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginStateManager::initParamsAndCreateLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	//==============================================================================
	// -- ranges
	auto gainRange = juce::NormalisableRange<float>(-24.f, 24.f, .01f);
	auto freqRange = juce::NormalisableRange<float>(20.f, 20000.f, 1.f, .198893842f);
	auto filterQRange = juce::NormalisableRange<float>(.1f, 10.f, .05f, 1.f);
	auto thresholdRange = juce::NormalisableRange<float>(-60.f, 24.f, 1.f, 1.f);
	auto attackReleaseRange = juce::NormalisableRange<float>(5.f, 500.f, 1.f, 1.f);
	auto ratioRange = juce::NormalisableRange<float>(1.f, 100.f, .1f, .4f);

	//==============================================================================
	// -- choices
	juce::StringArray freqPassSlopeChoices;
	for (int i{ 0 }; i < 4; ++i)
	{
		juce::String str;
		str << (12 + i * 12);
		str << " db/Oct";
		freqPassSlopeChoices.add(str);
	}

	//==============================================================================
	// -- Bypass ALL
	addParam(
		layout,
		ControlID::bypass,
		"bypass",
		V1_0_0,
		"bypass",
		false,
		"",
		SmoothingType::Linear,
		.01f
	);
	// -- Blend (dry/wet)
	addParam(
		layout,
		ControlID::blend,
		"blend",
		V1_0_0,
		"blend",
		juce::NormalisableRange<float>(0.f, 1.f, .01f),
		1.f,
		"",
		SmoothingType::Linear
	);
	// -- Pre Gain
	addParam(
		layout,
		ControlID::preGain,
		"preGain",
		V1_0_0,
		"pre-gain",
		gainRange,
		0.f,
		"dB",
		SmoothingType::NoSmoothing // -- Gain DSP module already has smoothing
	);

	//==============================================================================
	// -- Imager
	addParam(
		layout,
		ControlID::imagerBypass,
		"imagerBypass",
		V1_0_0,
		"imager bypass",
		false,
		"",
		SmoothingType::Linear,
		.01f
	);

	addParam(
		layout,
		ControlID::imagerOriginalGain,
		"imagerOriginalGain",
		V1_0_0,
		"imager original gain",
		juce::NormalisableRange<float>(0.f, 2.f, .001f),
		1.f,
		"",
		SmoothingType::Linear
	);

	addParam(
		layout,
		ControlID::imagerAuxiliarGain,
		"imagerAuxiliarGain",
		V1_0_0,
		"imager aux gain",
		juce::NormalisableRange<float>(0.f, 2.f, .001f),
		1.f,
		"",
		SmoothingType::Linear
	);

	addParam(
		layout,
		ControlID::imagerWidth,
		"imagerWidth",
		V1_0_0,
		"imager width",
		juce::NormalisableRange<float>(-1.f, 1.f, .001f),
		0.f,
		"",
		SmoothingType::Linear
	);

	addParam(
		layout,
		ControlID::imagerCenter,
		"imagerCenter",
		V1_0_0,
		"imager center",
		juce::NormalisableRange<float>(-1.f, 1.f, .001f),
		0.f,
		"",
		SmoothingType::Linear
	);

	addParam(
		layout,
		ControlID::imagerDelayTime,
		"imagerDelayTime",
		V1_0_0,
		"imager delay time",
		juce::NormalisableRange<float>(0.f, 50.f, .001f, .7f),
		4.f,
		"ms",
		SmoothingType::Linear
	);

	addParam(
		layout,
		ControlID::imagerCrossoverFreq,
		"imagerCrossoverFreq",
		V1_0_0,
		"imager crossover freq",
		freqRange,
		300.f,
		"Hz",
		SmoothingType::Linear
	);

	juce::StringArray imagerTypeChoices{ "Haas - mid/side processing", "Haas - mono cancelation", "Haas" };
	addParam(
		layout,
		ControlID::imagerType,
		"imagerType",
		V1_0_0,
		"imager type",
		imagerTypeChoices
	);

	// -- Post Gain
	addParam(
		layout,
		ControlID::postGain,
		"postGain",
		V1_0_0,
		"post-gain",
		gainRange,
		0.f,
		"dB",
		SmoothingType::NoSmoothing // -- Gain DSP module already has smoothing
	);

	return layout;
}
