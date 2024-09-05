#pragma once

#include "PluginEditor.h"

//==============================================================================
AuditoriumStereoEnhancerAudioProcessorEditor::AuditoriumStereoEnhancerAudioProcessorEditor(
	AuditoriumStereoEnhancerAudioProcessor& p,
	std::shared_ptr<PluginStateManager> stateManager
) :
	AudioProcessorEditor(&p),
	rasterComponent(p, stateManager)
{
	addAndMakeVisible(rasterComponent);

	//juce::PropertiesFile::Options options;
	//options.applicationName = ProjectInfo::projectName;
	//options.commonToAllUsers = true;
	//options.filenameSuffix = "settings";
	//options.osxLibrarySubFolder = "Application Support";
	//applicationProperties.setStorageParameters(options);


	if (auto* constrainer = getConstrainer())
	{
		constrainer->setFixedAspectRatio(static_cast<double> (originalWidth) / static_cast<double> (originalHeight));
		constrainer->setSizeLimits(
			originalWidth * minResizingTimes, originalHeight * minResizingTimes,
			originalWidth * maxResizingTimes, originalHeight * maxResizingTimes
		);
	}

	auto scaleFactor{ 1. };
	//if (auto* properties = applicationProperties.getCommonSettings(true))
	//{
	//	scaleFactor = properties->getDoubleValue("sizeRatio", scaleFactor);
	//}

	setResizable(true, true);
	setSize(originalWidth * scaleFactor, originalHeight * scaleFactor);
}

AuditoriumStereoEnhancerAudioProcessorEditor::~AuditoriumStereoEnhancerAudioProcessorEditor()
{
}

//==============================================================================
void AuditoriumStereoEnhancerAudioProcessorEditor::resized()
{
	const auto scaleFactor = static_cast<float> (getWidth()) / static_cast<float>(originalWidth);
	//if (auto* properties = applicationProperties.getCommonSettings(true))
	//{
	//	properties->setValue("sizeRatio", scaleFactor);
	//}

	rasterComponent.setTransform(juce::AffineTransform::scale(scaleFactor));
	rasterComponent.setBounds(0, 0, originalWidth, originalHeight);
}
