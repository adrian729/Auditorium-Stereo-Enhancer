/*
  ==============================================================================

	HorizontalMeter.h
	Created: 28 Oct 2023 1:00:04pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// TODO: check why on stop/close the meter doesn't go to -100.f (could just be a Reaper version thing, done exactly like the example it still doesn't do it)

namespace Gui {
	//==============================================================================
	class HorizontalMeter : public juce::Component
	{
	public:
		HorizontalMeter();
		~HorizontalMeter() override;

		//==============================================================================
		void paint(juce::Graphics&) override;
		void resized() override;

		//==============================================================================
		void setLevel(const float value);

	private:
		//==============================================================================
		float level{ -60.f };

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HorizontalMeter)
	};
}; // -- namespace Gui