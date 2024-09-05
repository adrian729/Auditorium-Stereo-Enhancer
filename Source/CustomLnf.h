#pragma once

#include <JuceHeader.h>

namespace Gui
{
	class CustomLnf : public juce::LookAndFeel_V4
	{
	public:
		//==============================================================================
		CustomLnf();
		~CustomLnf();

		//==============================================================================
		void drawRotarySlider(
			juce::Graphics&,
			int x, int y,
			int width, int height,
			float sliderPosProportional,
			float rotaryStartAngle, float rotaryEndAngle,
			juce::Slider&
		) override;

		void drawLinearSlider(
			juce::Graphics&,
			int x, int y,
			int width, int height,
			float sliderPos, float minSliderPos, float maxSliderPos,
			const juce::Slider::SliderStyle,
			juce::Slider&
		) override;

		juce::Label* createSliderTextBox(juce::Slider& slider) override; // TODO: why is the override not working?

		void drawLabel(juce::Graphics&, juce::Label&) override;

	private:
		//==============================================================================
		void drawPointerSlider(
			juce::Graphics&,
			int x, int y,
			int width, int height,
			float sliderPos, float minSliderPos, float maxSliderPos,
			const juce::Slider::SliderStyle,
			juce::Slider&
		);

		void drawBarSlider(
			juce::Graphics&,
			int x, int y,
			int width, int height,
			float sliderPos, float minSliderPos, float maxSliderPos,
			const juce::Slider::SliderStyle,
			juce::Slider&
		);

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLnf);
	};
}
