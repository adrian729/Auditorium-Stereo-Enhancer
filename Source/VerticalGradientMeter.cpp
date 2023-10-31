#pragma once

#include "VerticalGradientMeter.h"

namespace Gui
{
	//==============================================================================
	VerticalGradientMeter::VerticalGradientMeter(std::function<float()>&& valueSupplier) :
		valueSupplier(std::move(valueSupplier))
	{
		startTimerHz(TIMER_HZ);
	}

	VerticalGradientMeter::~VerticalGradientMeter()
	{
	}

	//==============================================================================
	void VerticalGradientMeter::paint(juce::Graphics& g)
	{
		const float level = juce::jlimit(MIN_LEVEL, MAX_LEVEL, valueSupplier());
		if (level >= CLIP_LEVEL)
		{
			isClipping = true;
		}
		if (level > maxLevelValue)
		{
			maxLevelValue = level;
		}

		auto bounds = getLocalBounds().toFloat();
		const float h = bounds.getHeight();
		const float w = bounds.getWidth();

		g.setColour(isClipping ? juce::Colours::red : juce::Colours::black);
		g.fillRect(bounds.removeFromTop(h * CLIP_ALERT_HEIGHT_PROP));

		auto meterBounds = bounds.removeFromBottom(h * METER_HEIGHT_PROP);
		const float meterHeight = meterBounds.getHeight();
		const float meterX = meterBounds.getX();
		const float meterY = meterBounds.getY();

		g.setColour(juce::Colours::black);
		g.fillRect(meterBounds);

		g.setGradientFill(gradient);

		const float scaledMaxValue = juce::jmap(
			maxLevelValue,
			MIN_LEVEL, MAX_LEVEL,
			meterHeight, 0.f
		);

		const float scaledY = juce::jmap(
			level,
			MIN_LEVEL, MAX_LEVEL,
			0.f, meterHeight
		);
		g.fillRect(meterBounds.removeFromBottom(scaledY));

		g.setColour(juce::Colours::white);

		g.fillRect(juce::Rectangle<float>(
			meterX, meterY + scaledMaxValue, w, MAX_LEVEL_VALUE_HEIGHT_PROP * h
		));
	}

	void VerticalGradientMeter::resized()
	{
		auto bounds = getLocalBounds().toFloat();
		bounds.removeFromTop(8);

		const float clipPointStart = juce::jmap(
			CLIP_LEVEL,
			MIN_LEVEL, MAX_LEVEL,
			bounds.getHeight(), 0.f
		);

		gradient = juce::ColourGradient{
			juce::Colours::green, bounds.getBottomLeft(),
			juce::Colours::darkred, bounds.getTopLeft(),
			false
		};

		const float warningZoneStart = juce::jmap(
			WARN_LEVEL,
			MIN_LEVEL, MAX_LEVEL,
			0.f, 1.f
		);
		gradient.addColour(warningZoneStart, juce::Colours::yellow);

		const float dangerZoneStart = juce::jmap(
			CLIP_LEVEL,
			MIN_LEVEL, MAX_LEVEL,
			0.f, 1.f
		);
		gradient.addColour(dangerZoneStart, juce::Colours::red);
	}

	//==============================================================================
	void VerticalGradientMeter::timerCallback()
	{
		repaint();
	}

	//==============================================================================
	void VerticalGradientMeter::mouseDown(const juce::MouseEvent& event)
	{
		maxLevelValue = -100.f;
		isClipping = false;
		repaint();
	}
}
