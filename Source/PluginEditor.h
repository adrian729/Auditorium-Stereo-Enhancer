#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginStateManager.h"
#include "RasterComponent.h"

//==============================================================================
class AuditoriumStereoEnhancerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
	//==============================================================================
	AuditoriumStereoEnhancerAudioProcessorEditor(
		AuditoriumStereoEnhancerAudioProcessor&,
		std::shared_ptr<PluginStateManager>
	);
	~AuditoriumStereoEnhancerAudioProcessorEditor() override;

	//==============================================================================
	void resized() override;

private:
	//==============================================================================
	static constexpr double ratio{ 2. / 3. };
	static constexpr int originalWidth{ 1000 };
	static constexpr int originalHeight{ static_cast<int>(originalWidth * ratio) };

	//==============================================================================
	static constexpr float minResizingTimes{ 1.f / 2.f };
	static constexpr float maxResizingTimes{ 2.f };

	//==============================================================================
	RasterComponent rasterComponent;
	juce::ApplicationProperties applicationProperties;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuditoriumStereoEnhancerAudioProcessorEditor)
};
