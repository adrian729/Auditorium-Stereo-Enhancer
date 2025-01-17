#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginStateManager.h"
#include "CustomLnf.h"
#include "CustomSlider.h"
#include "XYPad.h"
#include "ImagerPad.h"
#include "VerticalGradientMeter.h"

class RasterComponent : public juce::Component
{
public:
	//==============================================================================
	RasterComponent(
		AuditoriumStereoEnhancerAudioProcessor&,
		std::shared_ptr<PluginStateManager>
	);
	~RasterComponent() override;

	//==============================================================================
	void paint(juce::Graphics&) override;
	void resized() override;

private:

	//==============================================================================
	const int LEFT_CHANNEL{ 0 };
	const int RIGHT_CHANNEL{ 1 };

	//==============================================================================
	using APVTS = juce::AudioProcessorValueTreeState;

	AuditoriumStereoEnhancerAudioProcessor& audioProcessor;
	std::shared_ptr<PluginStateManager> stateManager;
	APVTS& apvts;

	//==============================================================================
	Gui::CustomLnf customLnf;

	//==============================================================================
	using Attachment = APVTS::SliderAttachment;

	//struct SliderComponent
	//{
	//	SliderComponent(
	//		APVTS& apvts,
	//		juce::RangedAudioParameter* param,
	//		juce::Slider::SliderStyle style = juce::Slider::SliderStyle::LinearHorizontal
	//	) :
	//		slider(style, juce::Slider::TextBoxBelow),
	//		label(param->getParameterID(), param->getParameterID()),
	//		attachment(std::make_unique<Attachment>(apvts, param->getParameterID(), slider))
	//	{
	//		slider.setTextBoxIsEditable(true);
	//		//label.setJustificationType(juce::Justification::bottom);
	//		//label.attachToComponent(&slider, false);
	//	}

	//	juce::Slider slider;
	//	juce::Label label;
	//	std::unique_ptr<Attachment> attachment;
	//};

	struct SliderComponent
	{
		SliderComponent(
			APVTS& apvts,
			juce::RangedAudioParameter* param,
			juce::Slider::SliderStyle style = juce::Slider::SliderStyle::LinearHorizontal
		) :
			slider(param->getLabel()),
			label(param->getParameterID(), param->getParameterID()),
			attachment(std::make_unique<Attachment>(apvts, param->getParameterID(), slider))
		{
			slider.setTextBoxIsEditable(true);
			//label.setJustificationType(juce::Justification::bottom);
			//label.attachToComponent(&slider, false);
		}

		Gui::CustomSlider slider;
		juce::Label label;
		std::unique_ptr<Attachment> attachment;
	};

	enum SliderID
	{
		blend,
		preGain,
		imagerOriginalGain,
		imagerAuxiliarGain,
		imagerWidth,
		imagerCenter,
		imagerDelayTime,
		imagerCrossoverFreq,
		postGain,
		//==============================================================================
		countSliders
	};

	std::array<SliderComponent, SliderID::countSliders> sliders;

	std::array<juce::Slider, 1> testSliders;

	juce::ComboBox imageTypeComboBox;
	std::unique_ptr<APVTS::ComboBoxAttachment> imageTypeComboBoxAttachment;

	//==============================================================================
	Gui::ImagerPad imagerPad;

	// TODO: stereo meter (so that we can update both at the same time, f.e. when clicking it both meters reset clipping)
	Gui::VerticalGradientMeter inputMeter, meterL, meterR;
	//==============================================================================
	void addAndMakeVisibleComponents();


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasterComponent)
};
