/*
  ==============================================================================

	parameterTypes.h
	Created: 30 Aug 2023 12:22:56pm
	Author:  Brutus729

  ==============================================================================
*/

#pragma once

//==============================================================================
// --- MACROS enum
//------------------------------------------------------------------------------
#define enumToInt(ENUM) static_cast<int>(ENUM)
#define intToEnum(INT, ENUM) static_cast<ENUM>(INT)
#define compareEnumToInt(ENUM, INT) (enumToInt(ENUM) == INT)
#define compareIntToEnum(INT, ENUM) (INT == enumToInt(ENUM))

enum class SmoothingType
{
	NoSmoothing,
	Linear,
	Multiplicative
};

//==============================================================================
// --- VERSIONS
//==============================================================================
enum VersionIDs
{
	NoVersion,
	// --- current version
	V1_0_0,
};

//==============================================================================
// --- CONTROL IDs -- for param definitions array access
//==============================================================================
enum ControlID
{
	bypass,
	blend,
	// -- Pre Gain
	preGain,

	// -- Imager
	imagerBypass,
	imagerOriginalGain,
	imagerAuxiliarGain,
	imagerWidth,
	imagerCenter,
	imagerDelayTime,
	imagerCrossoverFreq,
	imagerType,
	// -- Post Gain
	postGain,
	//==============================================================================
	countParams // value to keep track of the total number of parameters
};