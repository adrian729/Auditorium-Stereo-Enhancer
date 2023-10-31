/*
  ==============================================================================

	Thumb.cpp
	Created: 23 Oct 2023 11:52:44am
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include "Thumb.h"

namespace Gui {
	//==============================================================================
	Thumb::Thumb(
		float size,
		Shapes shape,
		juce::Colour colour,
		Direction direction,
		float axisPosProportion
	) :
		size(size),
		shape(shape),
		colour(colour),
		direction(direction),
		axisPosProportion(axisPosProportion)
	{
		constrainer.setMinimumOnscreenAmounts(size, size, size, size);
	}

	Thumb::~Thumb()
	{
	}

	//==============================================================================
	void Thumb::paint(juce::Graphics& g)
	{
		g.setColour(colour);
		juce::Path path;
		switch (shape)
		{
		case Shapes::circle:
		{
			g.fillEllipse(getLocalBounds().toFloat());
			path.addEllipse(getLocalBounds().toFloat());
			break;
		}
		case Shapes::square:
		{
			g.fillRect(getLocalBounds().toFloat().withSizeKeepingCentre(size, size));
			path.addRectangle(getLocalBounds().toFloat().withSizeKeepingCentre(size, size));
			break;
		}
		}

		const juce::DropShadow dropShadow;
		dropShadow.drawForPath(g, path);
	}

	//==============================================================================
	void  Thumb::mouseDown(const juce::MouseEvent& event)
	{
		dragger.startDraggingComponent(this, event);
	}

	void  Thumb::mouseDrag(const juce::MouseEvent& event)
	{
		dragger.dragComponent(this, event, &constrainer);

		const auto comp = getParentComponent();
		if (direction == Direction::horizontal)
		{
			constrainer.applyBoundsToComponent(
				*this,
				getBounds().withY(static_cast<int>(axisPosProportion * comp->getHeight() - .5f * size))
			);
		}
		else if (direction == Direction::vertical)
		{
			constrainer.applyBoundsToComponent(
				*this,
				getBounds().withX(static_cast<int>(axisPosProportion * comp->getWidth() - .5f * size))
			);
		}

		if (onDragCallback)
		{
			onDragCallback(getPosition().toDouble());
		}
	}

	void Thumb::mouseDoubleClick(const juce::MouseEvent& event)
	{
		if (onDoubleClickCallback)
		{
			onDoubleClickCallback();
		}
	}
}; // -- namespace Gui