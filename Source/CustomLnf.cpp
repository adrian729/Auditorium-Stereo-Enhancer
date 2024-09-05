#pragma once

#include "CustomLnf.h"
#include "CustomSlider.h"

namespace Gui
{
	//==============================================================================
	CustomLnf::CustomLnf()
	{
	}
	CustomLnf::~CustomLnf()
	{
	}

	//==============================================================================
	void CustomLnf::drawRotarySlider(
		juce::Graphics& g,
		int x, int y,
		int width, int height,
		float sliderPosProportional,
		float rotaryStartAngle, float rotaryEndAngle,
		juce::Slider& slider
	)
	{
		auto bounds = juce::Rectangle<float>(x, y, width, height);
		g.setColour(juce::Colours::green);
		g.drawRect(bounds);
		if (auto* customSlider_ptr = dynamic_cast<CustomSlider*>(&slider))
		{
			const auto labelHeight = customSlider_ptr->getLabelHeight();
			bounds = bounds.reduced(labelHeight);
		}
		const auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
		const auto centre = bounds.getCentre();
		bounds.setSize(size, size);
		bounds.setCentre(centre);
		g.setColour(juce::Colours::red);
		g.drawRect(bounds);

		// TODO: fit the knob to the bounds
		const auto radius = bounds.getWidth() / 2.0f;
		//const auto centre = bounds.getCentre();
		const float centreX = centre.getX();
		const float centreY = centre.getY();
		const float rx = centreX - radius;
		const float ry = centreY - radius;
		const float rw = radius * 2.f;
		jassert(rotaryStartAngle < rotaryEndAngle);
		const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

		// -- shadow
		const float shadowRadiusDiff = 0.f;
		juce::Path backgroundShadowPath;
		backgroundShadowPath.addArc(
			rx - shadowRadiusDiff,
			ry - shadowRadiusDiff,
			rw + 2 * shadowRadiusDiff,
			rw + 2 * shadowRadiusDiff,
			rotaryStartAngle,
			rotaryEndAngle,
			true
		);
		const juce::DropShadow backgroundShadow{
			juce::Colours::black,
			static_cast<int>(radius),
			{}
		};
		backgroundShadow.drawForPath(g, backgroundShadowPath);

		// -- meter

		// -- meter background
		const float meterBackgroundThickness = 8.f;
		const float meterRadiusDiff = -meterBackgroundThickness / 2.f;
		g.setColour(juce::Colours::black.withAlpha(0.5f));
		juce::Path meterBackground;
		meterBackground.addArc(
			rx - meterRadiusDiff,
			ry - meterRadiusDiff,
			rw + 2 * meterRadiusDiff,
			rw + 2 * meterRadiusDiff,
			rotaryStartAngle,
			rotaryEndAngle,
			true
		);
		g.strokePath(
			meterBackground,
			juce::PathStrokeType(
				meterBackgroundThickness,
				juce::PathStrokeType::curved,
				juce::PathStrokeType::rounded
			)
		);

		// -- meter ammount
		const float meterAmmountThickness = 2.5f;
		g.setColour(juce::Colours::lightgreen.brighter(.2f).withAlpha(.8f));
		juce::Path meterAmmount;
		meterAmmount.addArc(
			rx - meterRadiusDiff,
			ry - meterRadiusDiff,
			rw + 2 * meterRadiusDiff,
			rw + 2 * meterRadiusDiff,
			rotaryStartAngle,
			angle,
			true
		);
		g.strokePath(
			meterAmmount,
			juce::PathStrokeType(
				meterAmmountThickness,
				juce::PathStrokeType::curved,
				juce::PathStrokeType::rounded
			)
		);

		// -- fill
		const float fillRadiusDiff = -8.f;
		juce::ColourGradient fillGradient = juce::ColourGradient{
			juce::Colours::white, bounds.getTopLeft(),
			juce::Colours::white.darker(.6f), bounds.getBottomRight(),
			true
		};
		fillGradient.addColour(0.45f, juce::Colours::white);
		g.setGradientFill(fillGradient);
		g.fillEllipse(
			rx - fillRadiusDiff, ry - fillRadiusDiff,
			rw + 2 * fillRadiusDiff, rw + 2 * fillRadiusDiff
		);

		// -- pointer
		const float distanceFromEdge = 3.f - fillRadiusDiff;
		const float pointerRadius = radius * .2f;

		// -- pointer fill
		g.setColour(juce::Colours::black.withAlpha(.7f));
		juce::Path pointerPath;
		const juce::Rectangle<float> pointerBounds(
			-pointerRadius, -radius + distanceFromEdge,
			pointerRadius * 2, pointerRadius * 2
		);
		pointerPath.addEllipse(
			pointerBounds
		);
		pointerPath.applyTransform(
			juce::AffineTransform::rotation(angle)
			.translated(centreX, centreY)
		);
		g.fillPath(pointerPath);

		// -- pointer shadow
		const juce::DropShadow pointerShadow{
			juce::Colours::white.darker(.07f),
			2,
			{}
		};
		pointerShadow.drawForPath(g, pointerPath);

		// -- pointer outline
		g.setColour(juce::Colours::black.withAlpha(.08f));
		const float pointerOutlineThickness = 1.f;
		g.strokePath(
			pointerPath,
			juce::PathStrokeType(
				pointerOutlineThickness,
				juce::PathStrokeType::curved,
				juce::PathStrokeType::rounded
			)
		);

		//if (auto* customSlider = dynamic_cast<CustomSlider*>(&slider))
		//{
		//	const auto labelBounds = customSlider->getLabelBounds();

		//	const auto textMaxWidth = g.getCurrentFont().getStringWidth(customSlider->getMaxSizeText());
		//	const juce::String displayText = customSlider->getDisplayText();

		//	juce::Rectangle<int> textBounds;
		//	textBounds.setSize(textMaxWidth, labelBounds.getHeight());
		//	textBounds.setCentre(labelBounds.getCentre());

		//	// -- display text
		//	g.setColour(juce::Colours::black);
		//	g.drawFittedText((angle - rotaryStartAngle) / (rotaryEndAngle - rotaryStartAngle) < 0.5f ? customSlider->getMaxSizeText() : displayText, textBounds, juce::Justification::centred, 1);

		//	// -- label outline
		//	g.setColour(juce::Colours::black.withAlpha(0.8f));
		//	g.drawRect(textBounds);
		//}
	}

	void CustomLnf::drawLinearSlider(
		juce::Graphics& g,
		int x, int y,
		int width, int height,
		float sliderPos, float minSliderPos, float maxSliderPos,
		const juce::Slider::SliderStyle style,
		juce::Slider& slider
	)
	{
		if (slider.isBar())
		{
			drawBarSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
		}
		else
		{
			drawPointerSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
		}
	}

	// TODO: override this to draw the slider label
	juce::Label* CustomLnf::createSliderTextBox(juce::Slider& slider)
	{
		if (auto* customSlider = dynamic_cast<CustomSlider*>(&slider))
		{
			auto* label = new CustomSlider::CustomLabel(customSlider);
			return label;
		}

		return nullptr;
	}

	void CustomLnf::drawLabel(juce::Graphics& g, juce::Label& label)
	{
		auto* customLabel = dynamic_cast<CustomSlider::CustomLabel*>(&label);
		if (customLabel == nullptr)
		{
			return;
		}

		auto* customSlider = customLabel->getOwnerSlider();
		if (customSlider == nullptr)
		{
			return;
		}

		//const auto alpha = label.isEnabled() ? 1.f : .5f;
		//const float height = customSlider->getLabelHeight();
		//const float width = g.getCurrentFont().getStringWidth(customSlider->getMaxWidthText());

		//const auto sliderBounds = customSlider->getLocalBounds(); // TODO: why is this returning something random?
		//const int x = sliderBounds.getCentreX() - width / 2.f;
		//const int y = sliderBounds.getY() + sliderBounds.getHeight() - height;

		//juce::Rectangle<int> bounds(
		//	x, y,
		//	width, height
		//);

		g.setColour(juce::Colours::pink);
		customLabel->setBounds(customSlider->getLocalBounds());
		g.drawRect(customSlider->getLocalBounds());

		//const auto text = label.getText();

		//g.setColour(juce::Colours::black.withAlpha(alpha));
		//g.setFont(15.f);
		//g.drawFittedText(label.getText(), sliderBounds, juce::Justification::centred, 1);
	}

	//==============================================================================
	void CustomLnf::drawPointerSlider(
		juce::Graphics& g,
		int x, int y,
		int width, int height,
		float sliderPos, float minSliderPos, float maxSliderPos,
		const juce::Slider::SliderStyle style,
		juce::Slider& slider
	)
	{

	}

	void CustomLnf::drawBarSlider(
		juce::Graphics& g,
		int x, int y,
		int width, int height,
		float sliderPos, float minSliderPos, float maxSliderPos,
		const juce::Slider::SliderStyle style,
		juce::Slider& slider
	)
	{
	}
} // namespace Gui
