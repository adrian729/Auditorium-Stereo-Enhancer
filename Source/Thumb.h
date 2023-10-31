/*
  ==============================================================================

	Thumb.h
	Created: 23 Oct 2023 11:52:44am
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Gui {
	//==============================================================================
	class Thumb : public juce::Component
	{
	public:
		//==============================================================================
		enum class Shapes
		{
			circle,
			square
		};

		//==============================================================================
		enum class Direction
		{
			any,
			horizontal,
			vertical
		};

		//==============================================================================
		// -- CONSTRUCTORS
		//==============================================================================
		Thumb(
			float size,
			Shapes shape,
			juce::Colour colour = juce::Colours::white,
			Direction direction = Direction::any,
			float axisPosProportion = .5f
		);
		~Thumb();

		//==============================================================================
		void paint(juce::Graphics& g) override;

		//==============================================================================
		void mouseDown(const juce::MouseEvent& event) override;
		void mouseDrag(const juce::MouseEvent& event) override;
		void mouseDoubleClick(const juce::MouseEvent& event) override;

		//==============================================================================
		std::function<void(juce::Point<double>)> onDragCallback;
		std::function<void()> onDoubleClickCallback;

	private:
		//==============================================================================
		float size{ 0.f };
		Shapes shape;
		juce::Colour colour;
		Direction direction;
		float axisPosProportion{ .5f };

		//==============================================================================
		juce::ComponentDragger dragger;
		juce::ComponentBoundsConstrainer constrainer;

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Thumb);
	};
}; // -- namespace Gui