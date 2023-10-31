#pragma once

#include "RasterComponent.h"

//==============================================================================
RasterComponent::RasterComponent(
	AuditoriumStereoEnhancerAudioProcessor& p,
	std::shared_ptr<PluginStateManager> stateManager
) :
	audioProcessor(p),
	stateManager(stateManager),
	apvts(*stateManager->getAPVTS()),
	sliders{
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::blend),
			juce::Slider::SliderStyle::Rotary
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::preGain),
			juce::Slider::SliderStyle::Rotary
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::imagerOriginalGain),
			juce::Slider::SliderStyle::LinearVertical
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::imagerAuxiliarGain),
			juce::Slider::SliderStyle::LinearVertical
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::imagerWidth)
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::imagerCenter)
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::imagerDelayTime)
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::imagerCrossoverFreq)
		),
		SliderComponent(
			apvts,
			stateManager->getParameter(ControlID::postGain),
			juce::Slider::SliderStyle::Rotary
		)
	},
	imagerPad(
		&sliders[SliderID::imagerOriginalGain].slider,
		&sliders[SliderID::imagerAuxiliarGain].slider,
		&sliders[SliderID::imagerWidth].slider,
		&sliders[SliderID::imagerCenter].slider
	),
	inputMeter([&]() { return audioProcessor.getInputRmsValue(); }),
	meterL([&]() { return audioProcessor.getRmsValue(LEFT_CHANNEL); }),
	meterR([&]() { return audioProcessor.getRmsValue(RIGHT_CHANNEL); })
{
	addAndMakeVisibleComponents();
}

RasterComponent::~RasterComponent()
{
}

//==============================================================================
void RasterComponent::paint(juce::Graphics& g)
{
	g.setGradientFill(juce::ColourGradient{ juce::Colours::darkgrey.brighter(0.2f), getLocalBounds().toFloat().getCentre(), juce::Colours::darkgrey.darker(0.8f), {}, true });
	g.fillRect(getLocalBounds());

	const juce::DropShadow dropShadow{ juce::Colours::black.withAlpha(0.5f), 10, {} };
	dropShadow.drawForRectangle(g, imagerPad.getBoundsInParent());
}

void RasterComponent::resized()
{
	const auto container = getLocalBounds();
	auto bounds = container.toFloat().reduced(20);

	const auto w = container.getWidth();
	const auto h = container.getHeight();

	// -- Pad section -----------------------------------------------------------
	const auto padWidth = h;
	const auto padHeight = h / 2.f;
	const auto padBounds = bounds.removeFromTop(padHeight);

	// --- Pad
	const int padInitialX = static_cast<int>(padBounds.getCentreX() - padWidth / 2.f);
	const int padInitialY = 13;

	imagerPad.setBounds(
		padInitialX, padInitialY,
		static_cast<int>(padWidth), static_cast<int>(padHeight)
	);

	// --- gain sliders
	const float gainSliderProportion = 1.f / 5.f;
	const float gainSliderWidth = gainSliderProportion * padWidth;
	const int gainSliderHeightOffset = 24;
	const float gainSliderHeight = padHeight + gainSliderHeightOffset;

	const int originalSliderInitialX = static_cast<int>(padBounds.getCentreX() - padWidth / 2.f - gainSliderWidth - 10);
	const int auxiliarSliderInitialX = static_cast<int>(padBounds.getCentreX() + padWidth / 2.f + 10);

	const int gainSliderInitialY = 10;

	sliders[SliderID::imagerOriginalGain].slider.setBounds(
		originalSliderInitialX, gainSliderInitialY,
		static_cast<int>(gainSliderWidth), static_cast<int>(gainSliderHeight)
	);
	sliders[SliderID::imagerAuxiliarGain].slider.setBounds(
		auxiliarSliderInitialX, gainSliderInitialY,
		static_cast<int>(gainSliderWidth), static_cast<int>(gainSliderHeight)
	);

	// -- Menu section -----------------------------------------------------------
	bounds.removeFromTop(20);
	const int menuInitialY = static_cast<int>(bounds.getY());
	const float menuHeight = bounds.getHeight();
	const float menuColumnWidth = bounds.getWidth() / 9.f;

	const float sideMenuWidth = 2 * menuColumnWidth;

	const int rotarySliderSize = static_cast<int>(sideMenuWidth / 2);
	const int bottomSliderInitialY = static_cast<int>(menuInitialY + menuHeight - rotarySliderSize);

	const int meterWidth = static_cast<int>((sideMenuWidth - rotarySliderSize) / 5.f);

	// --- Left menu
	const auto leftMenuBounds = bounds.removeFromLeft(sideMenuWidth);
	const int leftMenuInitialX = static_cast<int>(leftMenuBounds.getX());
	const int leftMenuSliderInitialX = leftMenuInitialX + sideMenuWidth - rotarySliderSize;

	inputMeter.setBounds(
		leftMenuInitialX, menuInitialY,
		meterWidth, menuHeight
	);

	sliders[SliderID::blend].slider.setBounds(
		leftMenuSliderInitialX, menuInitialY,
		rotarySliderSize, rotarySliderSize
	);

	const int preGainInitialY = static_cast<int>(menuInitialY + menuHeight - rotarySliderSize);
	sliders[SliderID::preGain].slider.setBounds(
		leftMenuSliderInitialX, bottomSliderInitialY,
		rotarySliderSize, rotarySliderSize
	);

	// --- Middle menu
	const auto middleMenuBounds = bounds.removeFromLeft(5 * menuColumnWidth).reduced(10, 0);
	const int middleMenuWidth = static_cast<int>(middleMenuBounds.getWidth());

	const int middleMenuInitialX = static_cast<int>(middleMenuBounds.getX());
	const int middleItemHeight = static_cast<int>(menuHeight / 5.f);

	imageTypeComboBox.setBounds(
		middleMenuInitialX, menuInitialY,
		middleMenuWidth, middleItemHeight
	);

	sliders[SliderID::imagerWidth].slider.setBounds(
		middleMenuInitialX, menuInitialY + middleItemHeight,
		middleMenuWidth, middleItemHeight
	);

	sliders[SliderID::imagerCenter].slider.setBounds(
		middleMenuInitialX, menuInitialY + 2 * middleItemHeight,
		middleMenuWidth, middleItemHeight
	);

	sliders[SliderID::imagerDelayTime].slider.setBounds(
		middleMenuInitialX, menuInitialY + 3 * middleItemHeight,
		middleMenuWidth, middleItemHeight
	);

	sliders[SliderID::imagerCrossoverFreq].slider.setBounds(
		middleMenuInitialX, menuInitialY + 4 * middleItemHeight,
		middleMenuWidth, middleItemHeight
	);

	// --- Right menu
	const auto rightMenuBounds = bounds;
	const int rightMenuInitialX = static_cast<int>(rightMenuBounds.getX());
	const int meterRInitialX = rightMenuInitialX + sideMenuWidth - meterWidth;
	const int meterLInitialX = meterRInitialX - meterWidth - 5;

	sliders[SliderID::postGain].slider.setBounds(
		rightMenuInitialX, bottomSliderInitialY,
		rotarySliderSize, rotarySliderSize
	);

	meterL.setBounds(
		meterLInitialX, menuInitialY,
		meterWidth, menuHeight
	);

	meterR.setBounds(
		meterRInitialX, menuInitialY,
		meterWidth, menuHeight
	);
}

//==============================================================================
void RasterComponent::addAndMakeVisibleComponents()
{
	// --- Sliders
	for (auto& comp : sliders)
	{
		addAndMakeVisible(comp.slider);
	}

	// --- Image Types ComboBox

	// -- Add items to ComboBox
	auto* parameter = stateManager->getParameter(ControlID::imagerType);
	juce::String imagerTypeParamID = parameter->getParameterID();
	imageTypeComboBox.addItemList(parameter->getAllValueStrings(), 1);
	imageTypeComboBoxAttachment = std::make_unique<APVTS::ComboBoxAttachment>(apvts, imagerTypeParamID, imageTypeComboBox);

	addAndMakeVisible(imageTypeComboBox);

	// --- Imager Pad
	addAndMakeVisible(imagerPad);

	// --- Level meters
	addAndMakeVisible(inputMeter);
	addAndMakeVisible(meterL);
	addAndMakeVisible(meterR);
}