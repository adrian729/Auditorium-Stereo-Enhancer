#pragma once

#include <JuceHeader.h>
#include <vector>
#include <mutex>
#include "Thumb.h"

namespace Gui {
	//==============================================================================
	class XYPad :
		public juce::Component,
		public juce::Slider::Listener
	{
	public:
		enum class Axis {
			X,
			Y
		};

		//==============================================================================
		// -- CONSTRUCTORS
		//==============================================================================
		XYPad();
		~XYPad();

		//==============================================================================
		void paint(juce::Graphics& g) override;
		void resized() override;

		//==============================================================================
		void registerSlider(juce::Slider* slider, Axis axis);
		void deregisterSlider(juce::Slider* slider);

	private:
		//==============================================================================
		static constexpr int thumbSize = 20;
		Thumb thumb;

		//==============================================================================
		std::vector<juce::Slider*> xSliders, ySliders;

		//==============================================================================
		std::mutex vectorMutex;

		//==============================================================================
		void sliderValueChanged(juce::Slider* slider) override;

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPad);
	};
}; // -- namespace Gui