#pragma once

#include "XYPad.h"

namespace Gui {
	//==============================================================================
	// -- CONSTRUCTORS
	//==============================================================================
	XYPad::XYPad() :
		thumb(thumbSize, Thumb::Shapes::circle)
	{
		addAndMakeVisible(thumb);

		thumb.onDragCallback = [&](juce::Point<double> position)
			{
				const std::lock_guard<std::mutex> lock(vectorMutex);

				const auto bounds = getLocalBounds().toDouble();
				const auto thumbSize_double = static_cast<double>(thumbSize);

				for (auto* slider : xSliders)
				{
					slider->setValue(juce::jmap(
						position.getX(),
						0.0, bounds.getWidth() - thumbSize_double,
						slider->getMinimum(), slider->getMaximum()
					));
				}
				for (auto* slider : ySliders)
				{
					slider->setValue(juce::jmap(
						position.getY(),
						bounds.getHeight() - thumbSize_double, 0.0,
						slider->getMinimum(), slider->getMaximum()
					));
				}
				repaint();
			};
	}

	XYPad::~XYPad()
	{
	}

	//==============================================================================
	void XYPad::paint(juce::Graphics& g)
	{
		using namespace juce;
		const auto bounds = getLocalBounds().toFloat();

		g.setGradientFill(ColourGradient{ Colours::black.brighter(0.2f), bounds.getTopLeft(), Colours::black.brighter(0.1f), bounds.getBottomLeft(), false });
		g.fillRoundedRectangle(bounds, 10);

		const auto r = thumbSize / 2.f;
		const auto thumbX = thumb.getX() + r;
		const auto thumbY = thumb.getY() + r;

		g.setColour(Colours::violet);
		g.drawLine(Line<float> { {0.f, thumbY}, { bounds.getWidth(), thumbY } });

		g.setColour(Colours::cyan);
		g.drawLine(Line<float> { {thumbX, 0.f}, { thumbX, bounds.getHeight() } });
	}

	void XYPad::resized()
	{
		// -- Thumb
		thumb.setBounds(getLocalBounds().withSizeKeepingCentre(thumbSize, thumbSize));
		// -- Set thumb initial position if sliders are already set
		if (!xSliders.empty())
		{
			sliderValueChanged(xSliders.front());
		}
		if (!ySliders.empty())
		{
			sliderValueChanged(ySliders.front());
		}
	}

	//==============================================================================
	void XYPad::registerSlider(juce::Slider* slider, Axis axis)
	{
		slider->addListener(this);

		const std::lock_guard<std::mutex> lock(vectorMutex);

		if (axis == Axis::X)
		{
			xSliders.push_back(slider);
		}
		else if (axis == Axis::Y)
		{
			ySliders.push_back(slider);
		}
	}
	void XYPad::deregisterSlider(juce::Slider* slider)
	{
		slider->removeListener(this);

		const std::lock_guard<std::mutex> lock(vectorMutex);

		xSliders.erase(std::remove(xSliders.begin(), xSliders.end(), slider), xSliders.end());
		ySliders.erase(std::remove(ySliders.begin(), ySliders.end(), slider), ySliders.end());
	}

	//==============================================================================
	void XYPad::sliderValueChanged(juce::Slider* slider)
	{
		// -- Avoid loopback
		if (thumb.isMouseOverOrDragging(false))
		{
			return;
		}

		const auto isXAxisSlider = std::find(xSliders.begin(), xSliders.end(), slider) != xSliders.end();
		const auto isYAxisSlider = std::find(ySliders.begin(), ySliders.end(), slider) != ySliders.end();
		const auto bounds = getLocalBounds().toDouble();
		const auto thumbSize_double = static_cast<double>(thumbSize);

		if (isXAxisSlider)
		{
			thumb.setTopLeftPosition(
				juce::jmap(
					slider->getValue(),
					slider->getMinimum(), slider->getMaximum(),
					0.0, bounds.getWidth() - thumbSize_double
				),
				thumb.getY()
			);
		}
		if (isYAxisSlider)
		{
			thumb.setTopLeftPosition(
				thumb.getX(),
				juce::jmap(
					slider->getValue(),
					slider->getMinimum(), slider->getMaximum(),
					bounds.getHeight() - thumbSize_double, 0.0
				)
			);
		}

		if (isXAxisSlider || isYAxisSlider)
		{
			repaint();
		}
	}
}; // -- namespace Gui