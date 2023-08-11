/***************************************************************************
 *  Copyright 1991, 1992, 1993, 1994, 1995, 1996, 2001, 2002               *
 *    David R. Hill, Leonard Manzara, Craig Schock                         *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
// 2014-09
// This file was copied from Gnuspeech and modified by Marcelo Y. Matuda.

#include "Controller.h"

#include <cstring>
#include <sstream>

#include "Exception.h"
#include "Tube.h"

#define TRM_CONTROL_MODEL_CONFIG_FILE_NAME "/trm_control_model.txt"
#define TRM_CONFIG_FILE_NAME "/trm.txt"
#define VOICE_FILE_PREFIX "/voice_"



namespace GS {
namespace TRMControlModel {

Controller::Controller(const char* configDirPath, Model& model)
		: model_(model)
		, eventList_(configDirPath, model_)
{
	loadConfiguration(configDirPath);
}

Controller::~Controller()
{
}

void
Controller::loadConfiguration(const char* configDirPath)
{
	// Load TRMControlModel::Configuration.

	std::ostringstream trmControlModelConfigFilePath;
	trmControlModelConfigFilePath << configDirPath << TRM_CONTROL_MODEL_CONFIG_FILE_NAME;
	trmControlModelConfig_.load(trmControlModelConfigFilePath.str());

	// Load TRM::Configuration.

	std::ostringstream trmConfigFilePath;
	trmConfigFilePath << configDirPath << TRM_CONFIG_FILE_NAME;

	std::ostringstream voiceFilePath;
	voiceFilePath << configDirPath << VOICE_FILE_PREFIX << trmControlModelConfig_.voiceName << ".txt";

	trmConfig_.load(trmConfigFilePath.str(), voiceFilePath.str());
}

void
Controller::initUtterance(std::ostream& trmParamStream)
{
	if ((trmConfig_.outputRate != 22050.0) && (trmConfig_.outputRate != 44100.0)) {
		trmConfig_.outputRate = 44100.0;
	}
	if ((trmConfig_.vtlOffset + trmConfig_.vocalTractLength) < 15.9) {
		trmConfig_.outputRate = 44100.0;
	}

	if (Log::debugEnabled) {
		printf("Tube Length = %f\n", trmConfig_.vtlOffset + trmConfig_.vocalTractLength);
		printf("Voice: %s L: %f  tp: %f  tnMin: %f  tnMax: %f  glotPitch: %f\n", trmControlModelConfig_.voiceName.c_str(),
			trmConfig_.vocalTractLength, trmConfig_.glottalPulseTp, trmConfig_.glottalPulseTnMin,
			trmConfig_.glottalPulseTnMax, trmConfig_.referenceGlottalPitch);
		printf("sampling Rate: %f\n", trmConfig_.outputRate);
	}

	eventList_.setPitchMean(trmControlModelConfig_.pitchOffset + trmConfig_.referenceGlottalPitch);
	eventList_.setGlobalTempo(trmControlModelConfig_.tempo);
	setIntonation(trmControlModelConfig_.intonation);
	eventList_.setUpDriftGenerator(trmControlModelConfig_.driftDeviation, trmControlModelConfig_.controlRate, trmControlModelConfig_.driftLowpassCutoff);
	eventList_.setRadiusCoef(trmConfig_.radiusCoef);

	trmParamStream <<
		trmConfig_.outputRate              << '\n' <<
		trmControlModelConfig_.controlRate << '\n' <<
		trmConfig_.volume                  << '\n' <<
		trmConfig_.channels                << '\n' <<
		trmConfig_.balance                 << '\n' <<
		trmConfig_.waveform                << '\n' <<
		trmConfig_.glottalPulseTp          << '\n' <<
		trmConfig_.glottalPulseTnMin       << '\n' <<
		trmConfig_.glottalPulseTnMax       << '\n' <<
		trmConfig_.breathiness             << '\n' <<
		trmConfig_.vtlOffset + trmConfig_.vocalTractLength << '\n' << // tube length
		trmConfig_.temperature             << '\n' <<
		trmConfig_.lossFactor              << '\n' <<
		trmConfig_.apertureRadius          << '\n' <<
		trmConfig_.mouthCoef               << '\n' <<
		trmConfig_.noseCoef                << '\n' <<
		trmConfig_.noseRadius[1]           << '\n' <<
		trmConfig_.noseRadius[2]           << '\n' <<
		trmConfig_.noseRadius[3]           << '\n' <<
		trmConfig_.noseRadius[4]           << '\n' <<
		trmConfig_.noseRadius[5]           << '\n' <<
		trmConfig_.throatCutoff            << '\n' <<
		trmConfig_.throatVol               << '\n' <<
		trmConfig_.modulation              << '\n' <<
		trmConfig_.mixOffset               << '\n';
}

// Chunks are separated by /c.
// There is always one /c at the begin and another at the end of the string.
int
Controller::calcChunks(const char* string)
{
	int temp = 0, index = 0;
	while (string[index] != '\0') {
		if ((string[index] == '/') && (string[index + 1] == 'c')) {
			temp++;
			index += 2;
		} else {
			index++;
		}
	}
	temp--;
	if (temp < 0) temp = 0;
	return temp;
}

// Returns the position of the next /c (the position of the /).
int
Controller::nextChunk(const char* string)
{
	int index = 0;
	while (string[index] != '\0') {
		if ((string[index] == '/') && (string[index + 1] == 'c')) {
			return index;
		} else {
			index++;
		}
	}
	return 0;
}

int
Controller::validPosture(const char* token)
{
	switch(token[0]) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return 1;
	default:
		return (model_.postureList().find(token) != nullptr);
	}
}

void
Controller::setIntonation(int intonation)
{
	if (intonation & Configuration::INTONATION_MICRO) {
		eventList_.setMicroIntonation(1);
	} else {
		eventList_.setMicroIntonation(0);
	}

	if (intonation & Configuration::INTONATION_MACRO) {
		eventList_.setMacroIntonation(1);
		eventList_.setSmoothIntonation(1); // Macro and not smooth is not working.
	} else {
		eventList_.setMacroIntonation(0);
		eventList_.setSmoothIntonation(0); // Macro and not smooth is not working.
	}

	// Macro and not smooth is not working.
//	if (intonation & Configuration::INTONATION_SMOOTH) {
//		eventList_.setSmoothIntonation(1);
//	} else {
//		eventList_.setSmoothIntonation(0);
//	}

	if (intonation & Configuration::INTONATION_DRIFT) {
		eventList_.setDrift(1);
	} else {
		eventList_.setDrift(0);
	}

	if (intonation & Configuration::INTONATION_RANDOMIZE) {
		eventList_.setTgUseRandom(true);
	} else {
		eventList_.setTgUseRandom(false);
	}
}

} /* namespace TRMControlModel */
} /* namespace GS */
