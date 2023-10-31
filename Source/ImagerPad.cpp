/*
  ==============================================================================

	ImagerPad.cpp
	Created: 23 Oct 2023 11:35:08am
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include "ImagerPad.h"

namespace Gui {
	//==============================================================================
	ImagerPad::ImagerPad(
		juce::Slider* originalGainSlider,
		juce::Slider* auxiliarGainSlider,
		juce::Slider* widthSlider,
		juce::Slider* centerSlider
	) :
		centerThumb(
			thumbSize,
			Thumb::Shapes::square,
			juce::Colours::lightblue,
			Thumb::Direction::horizontal,
			0.5f
		),
		originalThumb(thumbSize, Thumb::Shapes::circle, juce::Colours::antiquewhite),
		auxiliarThumb(thumbSize, Thumb::Shapes::circle, juce::Colours::coral),
		sliders{
			originalGainSlider,
			auxiliarGainSlider,
			widthSlider,
			centerSlider
		}
	{
		setSlidersValueChange();
		setDragCallbacks();
		setDoubleClickCallbacks();

		addAndMakeVisible(centerThumb);
		addAndMakeVisible(originalThumb);
		addAndMakeVisible(auxiliarThumb);

	}

	ImagerPad::~ImagerPad()
	{
	}

	//==============================================================================
	void ImagerPad::paint(juce::Graphics& g)
	{
		const auto bounds = getLocalBounds().toFloat();

		g.setGradientFill(juce::ColourGradient{
				juce::Colours::black.brighter(0.2f), bounds.getTopLeft(),
				juce::Colours::black.brighter(0.1f), bounds.getBottomLeft(),
				false
			});
		g.fillRoundedRectangle(bounds, 10);

		paintThumb(originalThumb, g);
		paintThumb(auxiliarThumb, g, juce::Colours::plum, juce::Colours::pink);
	}

	//==============================================================================
	void ImagerPad::paintThumb(const Thumb& thumb, juce::Graphics& g, juce::Colour xColour, juce::Colour yColour)
	{
		const auto bounds = getLocalBounds().toFloat();

		g.setColour(juce::Colours::lightgreen.withAlpha(0.5f));
		g.drawLine(juce::Line<float> { {0.f, bounds.getHeight() * .5f}, { bounds.getWidth(), bounds.getHeight() * .5f } });

		const auto r = thumbSize / 2.f;
		const auto thumbX = thumb.getX() + r;
		const auto thumbY = thumb.getY() + r;

		g.setColour(xColour);
		g.drawLine(juce::Line<float> { {0.f, thumbY}, { bounds.getWidth(), thumbY } });

		g.setColour(yColour);
		g.drawLine(juce::Line<float> { {thumbX, 0.f}, { thumbX, bounds.getHeight() } });
	}

	void ImagerPad::resized()
	{
		const auto bounds = getLocalBounds();

		centerThumb.setBounds(bounds.withSizeKeepingCentre(thumbSize, thumbSize));
		originalThumb.setBounds(bounds.withSizeKeepingCentre(thumbSize, thumbSize));
		auxiliarThumb.setBounds(bounds.withSizeKeepingCentre(thumbSize, thumbSize));

		auto originalSlider = sliders[SliderID::originalGain];
		originalThumb.setTopLeftPosition(
			originalThumb.getX(),
			juce::jmap(
				originalSlider->getValue(),
				originalSlider->getMinimum(), originalSlider->getMaximum(),
				getLocalBounds().toDouble().getHeight() - thumbSize, 0.
			)
		);


		auto auxiliarSlider = sliders[SliderID::auxiliarGain];
		auxiliarThumb.setTopLeftPosition(
			auxiliarThumb.getX(),
			juce::jmap(
				auxiliarSlider->getValue(),
				auxiliarSlider->getMinimum(), auxiliarSlider->getMaximum(),
				getLocalBounds().toDouble().getHeight() - thumbSize, 0.
			)
		);

		auto centerSlider = sliders[SliderID::center];
		centerThumb.setTopLeftPosition(
			juce::jmap(
				centerSlider->getValue(),
				centerSlider->getMinimum(), centerSlider->getMaximum(),
				0.0, getLocalBounds().toDouble().getWidth() - thumbSize
			),
			centerThumb.getY()
		);
	}

	//==============================================================================
	void ImagerPad::setSlidersValueChange()
	{
		sliders[SliderID::originalGain]->onValueChange = [&]
			{
				auto slider = sliders[SliderID::originalGain];

				originalThumb.setTopLeftPosition(
					originalThumb.getX(),
					juce::jmap(
						slider->getValue(),
						slider->getMinimum(), slider->getMaximum(),
						getLocalBounds().toDouble().getHeight() - thumbSize, 0.
					)
				);
				repaint();
			};

		sliders[SliderID::auxiliarGain]->onValueChange = [&]
			{
				auto slider = sliders[SliderID::auxiliarGain];

				auxiliarThumb.setTopLeftPosition(
					auxiliarThumb.getX(),
					juce::jmap(
						slider->getValue(),
						slider->getMinimum(), slider->getMaximum(),
						getLocalBounds().toDouble().getHeight() - thumbSize, 0.
					)
				);
				repaint();
			};

		sliders[SliderID::center]->onValueChange = [&]
			{
				auto slider = sliders[SliderID::center];

				centerThumb.setTopLeftPosition(
					juce::jmap(
						slider->getValue(),
						slider->getMinimum(), slider->getMaximum(),
						0.0, getLocalBounds().toDouble().getWidth() - thumbSize
					),
					centerThumb.getY()
				);
				repaint();
			};
	}

	void ImagerPad::setDragCallbacks()
	{
		originalThumb.onDragCallback = [&](juce::Point<double> position)
			{
				const auto bounds = getLocalBounds().toDouble();
				const auto thumbSize_double = static_cast<double>(thumbSize);

				auto* originalGainSlider = sliders[SliderID::originalGain];
				originalGainSlider->setValue(juce::jmap(
					position.getY(),
					bounds.getHeight() - thumbSize_double, 0.0,
					originalGainSlider->getMinimum(), originalGainSlider->getMaximum()
				));

				auto* widthSlider = sliders[SliderID::width];
				widthSlider->setValue(juce::jmap(
					auxiliarThumb.getX() - position.getX(),
					thumbSize_double - bounds.getWidth(), bounds.getWidth() - thumbSize_double,
					widthSlider->getMinimum(), widthSlider->getMaximum()
				));
				repaint();
			};

		auxiliarThumb.onDragCallback = [&](juce::Point<double> position)
			{
				const auto bounds = getLocalBounds().toDouble();
				const auto thumbSize_double = static_cast<double>(thumbSize);

				auto* auxiliarGainSlider = sliders[SliderID::auxiliarGain];
				auxiliarGainSlider->setValue(juce::jmap(
					position.getY(),
					bounds.getHeight() - thumbSize_double, 0.0,
					auxiliarGainSlider->getMinimum(), auxiliarGainSlider->getMaximum()
				));

				auto* widthSlider = sliders[SliderID::width];
				widthSlider->setValue(juce::jmap(
					position.getX() - originalThumb.getX(),
					thumbSize_double - bounds.getWidth(), bounds.getWidth() - thumbSize_double,
					widthSlider->getMinimum(), widthSlider->getMaximum()
				));
				repaint();
			};

		centerThumb.onDragCallback = [&](juce::Point<double> position)
			{
				const auto bounds = getLocalBounds().toDouble();
				const auto thumbSize_double = static_cast<double>(thumbSize);

				auto* centerSlider = sliders[SliderID::center];
				centerSlider->setValue(juce::jmap(
					position.getX(),
					0., bounds.getWidth() - thumbSize_double,
					centerSlider->getMinimum(), centerSlider->getMaximum()
				));
				repaint();
			};
	}

	void ImagerPad::setDoubleClickCallbacks()
	{
		originalThumb.onDoubleClickCallback = [&]()
			{
				auto* originalGainSlider = sliders[SliderID::originalGain];
				originalGainSlider->setValue(originalGainSlider->getDoubleClickReturnValue());

				const auto bounds = getLocalBounds().toDouble();
				const auto thumbSize_double = static_cast<double>(thumbSize);

				// TODO: move first originalThumb X to the the middle of the pad
				auto* widthSlider = sliders[SliderID::width];
				widthSlider->setValue(juce::jmap(
					static_cast<double>(auxiliarThumb.getX() - originalThumb.getX()),
					thumbSize_double - bounds.getWidth(), bounds.getWidth() - thumbSize_double,
					widthSlider->getMinimum(), widthSlider->getMaximum()
				));
			};

		auxiliarThumb.onDoubleClickCallback = [&]()
			{
				auto* auxiliarGainSlider = sliders[SliderID::auxiliarGain];
				auxiliarGainSlider->setValue(auxiliarGainSlider->getDoubleClickReturnValue());
			};

		centerThumb.onDoubleClickCallback = [&]()
			{
				auto* centerSlider = sliders[SliderID::center];
				centerSlider->setValue(centerSlider->getDoubleClickReturnValue());
			};
	}
}; // -- namespace Gui