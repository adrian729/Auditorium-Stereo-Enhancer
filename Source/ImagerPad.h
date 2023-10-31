/*
  ==============================================================================

	ImagerPad.h
	Created: 23 Oct 2023 11:35:08am
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include "Thumb.h"

namespace Gui {
	//==============================================================================
	class ImagerPad :
		public juce::Component
	{
	public:
		//==============================================================================
		ImagerPad(
			juce::Slider* originalGainSlider,
			juce::Slider* auxiliarGainSlider,
			juce::Slider* widthSlider,
			juce::Slider* centerSlider
		);
		~ImagerPad() override;

		//==============================================================================
		void paint(juce::Graphics&) override;
		void resized() override;

	private:
		//==============================================================================
		static constexpr int thumbSize = 20;
		Thumb centerThumb, originalThumb, auxiliarThumb;

		//==============================================================================
		enum SliderID
		{
			originalGain,
			auxiliarGain,
			width,
			center,
			//==============================================================================
			countSliders
		};

		//==============================================================================
		std::array<juce::Slider*, SliderID::countSliders> sliders;

		//==============================================================================
		void paintThumb(
			const Thumb& thumb,
			juce::Graphics& g,
			juce::Colour xColour = juce::Colours::violet,
			juce::Colour yColour = juce::Colours::cyan
		);

		//==============================================================================
		void setSlidersValueChange();
		void setDragCallbacks();
		void setDoubleClickCallbacks();

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImagerPad)
	};
}; // -- namespace Gui