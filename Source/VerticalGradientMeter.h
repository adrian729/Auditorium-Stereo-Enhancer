#pragma once

#include <JuceHeader.h>

namespace Gui
{
	class VerticalGradientMeter :
		public juce::Component,
		public juce::Timer
	{
	public:
		//==============================================================================
		VerticalGradientMeter(std::function<float()>&& valueFunction);
		~VerticalGradientMeter() override;

		//==============================================================================
		void paint(juce::Graphics&) override;
		void resized() override;

		//==============================================================================
		void timerCallback() override;

		//==============================================================================
		void mouseDown(const juce::MouseEvent& event) override;

	private:
		//==============================================================================
		const int TIMER_HZ{ 24 };

		const float CLIP_LEVEL{ 0.f };
		const float WARN_LEVEL{ CLIP_LEVEL - 6.f };

		const float MAX_LEVEL{ 6.f };
		const float MIN_LEVEL{ -60.f };

		const float METER_HEIGHT_PROP{ 0.96f };
		const float CLIP_ALERT_HEIGHT_PROP{ 0.02f };
		const float MAX_LEVEL_VALUE_HEIGHT_PROP{ 0.015f };

		//==============================================================================
		std::function<float()> valueSupplier;

		juce::ColourGradient gradient;

		float maxLevelValue{ -100.f };
		bool isClipping{ false };

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalGradientMeter)
	};
}
