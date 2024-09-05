#pragma once

#include <JuceHeader.h>
#include "CustomLnf.h"

namespace Gui {
	//==============================================================================
	class CustomSlider : public juce::Slider
	{
	public:
		//==============================================================================
		struct MarkObject
		{
			float value;
			juce::String label;
		};

		// TODO: implement in separate class -- nicer code
		class CustomLabel : public juce::Label
		{
		public:
			CustomLabel(CustomSlider* slider) :
				juce::Label({}, {}),
				ownerSlider(slider)
			{
				setEditable(false, true, false);
			}
			~CustomLabel()
			{
			}

			//==============================================================================
			void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override {}

			void mouseDoubleClick(const juce::MouseEvent& e) override
			{
				if (isEditableOnDoubleClick()
					&& isEnabled()
					&& !e.mods.isPopupMenu())
				{
					showCustomEditor();
				}
				//getCurrentTextEditor()
			}

			void showCustomEditor()
			{
				//showEditor();
				auto* editor = getCurrentTextEditor();
				editor->setSize(50, 100);
				//if (editor == nullptr)
				//{
				//	editor.reset(createEditorComponent());
				//	editor->setSize(10, 10);
				//	addAndMakeVisible(editor.get());
				//	editor->setText(getText(), false);
				//	editor->setKeyboardType(keyboardType);
				//	editor->addListener(this);
				//	editor->grabKeyboardFocus();

				//	if (editor == nullptr) // may be deleted by a callback
				//		return;

				//	editor->setHighlightedRegion(Range<int>(0, textValue.toString().length()));

				//	resized();
				//	repaint();

				//	editorShown(editor.get());

				//	enterModalState(false);
				//	editor->grabKeyboardFocus();
				//}
			}

			//==============================================================================
			std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
			{
				return createIgnoredAccessibilityHandler(*this);
			}

			//==============================================================================
			CustomSlider* getOwnerSlider() const noexcept { return ownerSlider; }

		private:
			//==============================================================================
			CustomSlider* ownerSlider;

			//==============================================================================
			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLabel);
		};
		// TODO: implement to be able to set labels etc
		// Add custom slider types here to be able to do linear centred etc

		//==============================================================================
		CustomSlider();
		CustomSlider(juce::String suffix);
		~CustomSlider();

		//==============================================================================
		//void paint(juce::Graphics& g) override;

		//==============================================================================
		juce::String getDisplayText() const;
		juce::String getMaxWidthText() const;
		juce::Array<MarkObject> getMarks() const;

		//==============================================================================
		float getLabelHeight() const;
		juce::Rectangle<float> getSliderBounds() const;
		juce::Rectangle<int> getLabelBounds() const;
		float getFontSize() const;

	private:
		//==============================================================================
		float fontSize;
		juce::Array<MarkObject> marks;

		//==============================================================================
		juce::String suffix;

		//==============================================================================
		CustomLnf lnf;

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomSlider);
	};
} // namespace Gui