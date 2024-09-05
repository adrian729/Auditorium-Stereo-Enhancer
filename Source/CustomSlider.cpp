#pragma once

#include "CustomSlider.h"

namespace Gui {
	//==============================================================================
	CustomSlider::CustomSlider() :
		juce::Slider(
			juce::Slider::SliderStyle::RotaryVerticalDrag,
			juce::Slider::TextEntryBoxPosition::NoTextBox
		),
		suffix(""),
		fontSize(15.f)
	{
		setLookAndFeel(&lnf);
	}

	CustomSlider::CustomSlider(juce::String suffix) :
		juce::Slider(
			juce::Slider::SliderStyle::RotaryVerticalDrag,
			juce::Slider::TextEntryBoxPosition::TextBoxBelow
		),
		suffix(suffix),
		fontSize(15.f)
	{
		setLookAndFeel(&lnf);
	}

	CustomSlider::~CustomSlider()
	{
		setLookAndFeel(nullptr);
	}

	//==============================================================================
	//void CustomSlider::paint(juce::Graphics& g)
	//{
	//	g.setColour(juce::Colours::red);
	//	g.drawRect(getLocalBounds());

	//	const auto startAngle = juce::degreesToRadians(180.f + 45.f);
	//	const auto endAngle = juce::degreesToRadians(180.f - 45.f) + juce::MathConstants<float>::twoPi;

	//	auto range = getRange();

	//	const auto sliderBounds = getSliderBounds();

	//	getLookAndFeel().drawRotarySlider(
	//		g,
	//		sliderBounds.getX(), sliderBounds.getY(),
	//		sliderBounds.getWidth(), sliderBounds.getHeight(),
	//		juce::jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
	//		startAngle,
	//		endAngle,
	//		*this
	//	);
	//}

	//==============================================================================
	juce::String CustomSlider::getDisplayText() const
	{
		// TODO: check how to deal with choice parameters
		//if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
		//	return choiceParam->getCurrentChoiceName();

		float val = getValue();

		bool addK = false;

		juce::String displayText = juce::String(val, val < 1000.f ? 2 : 0);

		if (suffix.isNotEmpty())
		{
			displayText << suffix;
		}

		return displayText;
	}

	juce::String CustomSlider::getMaxWidthText() const
	{
		// -- get the max sized text between the max integer value and the decimals + suffix to center the label around the decimal point
		juce::String frontText = " 99999";
		juce::String backText = "99 ";
		if (suffix.isNotEmpty())
		{
			backText << suffix << " ";
		}
		return frontText.length() > backText.length() ? frontText + frontText : backText + backText;
	}

	juce::Array<CustomSlider::MarkObject> CustomSlider::getMarks() const
	{
		return marks;
	}

	//==============================================================================
	float CustomSlider::getLabelHeight() const
	{
		return fontSize * 1.5f;
	}

	juce::Rectangle<float> CustomSlider::getSliderBounds() const
	{
		const auto bounds = getLocalBounds().toFloat().reduced(fontSize * 1.5f);
		const auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

		juce::Rectangle<float> sliderBounds;
		sliderBounds.setSize(size, size);
		sliderBounds.setCentre(bounds.getCentre());

		return sliderBounds;
	}

	juce::Rectangle<int> CustomSlider::getLabelBounds() const
	{
		const auto bounds = getScreenBounds();

		juce::Rectangle<int> labelBounds;
		labelBounds.setSize(bounds.getWidth(), fontSize * 1.5f);
		labelBounds.setCentre(bounds.getCentre());
		return bounds;
	}

	float CustomSlider::getFontSize() const
	{
		return fontSize;
	}
} // namespace Gui