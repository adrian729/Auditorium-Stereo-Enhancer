/*
  ==============================================================================

	HorizontalMeter.cpp
	Created: 28 Oct 2023 1:00:04pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include "HorizontalMeter.h"

namespace Gui {
	//==============================================================================
	HorizontalMeter::HorizontalMeter()
	{
	}

	HorizontalMeter::~HorizontalMeter()
	{
	}

	//==============================================================================
	void HorizontalMeter::paint(juce::Graphics& g)
	{
		auto bounds = getLocalBounds().toFloat();

		g.setColour(juce::Colours::white.withBrightness(0.4f));
		g.fillRoundedRectangle(bounds, 5.f);

		g.setColour(juce::Colours::white);
		const auto scaledX = juce::jmap(level, -60.f, 6.f, 0.f, bounds.getWidth());
		g.fillRoundedRectangle(bounds.removeFromLeft(scaledX), 5.f);
	}

	void HorizontalMeter::resized()
	{
	}

	//==============================================================================
	void HorizontalMeter::setLevel(const float value)
	{
		level = value;
	}
}; // -- namespace Gui
