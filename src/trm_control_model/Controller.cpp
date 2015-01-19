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

#define TRM_CONTROL_MODEL_CONFIG_FILE_NAME "/trm_control_model.config"
#define TRM_CONFIG_FILE_NAME "/trm.config"
#define VOICES_CONFIG_FILE_NAME "/voices.config"



namespace GS {
namespace TRMControlModel {

Controller::Controller(const char* configDirPath, Model& model)
		: model_(model)
		, eventList_(configDirPath, model_)
{
	loadTRMControlModelConfig(configDirPath);
	loadTRMConfig(configDirPath);
	initVoices(configDirPath);
}

Controller::~Controller()
{
}

void
Controller::loadTRMControlModelConfig(const char* configDirPath)
{
	std::ostringstream trmControlModelConfigFilePath;
	trmControlModelConfigFilePath << configDirPath << TRM_CONTROL_MODEL_CONFIG_FILE_NAME;
	trmControlModelConfig_.load(trmControlModelConfigFilePath.str());
}

void
Controller::loadTRMConfig(const char* configDirPath)
{
	std::ostringstream trmConfigFilePath;
	trmConfigFilePath << configDirPath << TRM_CONFIG_FILE_NAME;
	trmConfig_.load(trmConfigFilePath.str());
}

/*******************************************************************************
 * This function synthesizes speech from data contained in the event list.
 */
void
Controller::synthesizeFromEventList(const char* trmParamFile, const char* outputFile)
{
	initUtterance(trmParamFile);

	eventList_.generateOutput(trmParamFile);

	TRM::Tube trm;
	trm.synthesizeToFile(trmParamFile, outputFile);
}

void
Controller::initUtterance(const char* trmParamFile)
{
	if (trmControlModelConfig_.voiceType < 0 || trmControlModelConfig_.voiceType >= MAX_VOICES) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid voice type: " << trmControlModelConfig_.voiceType << '.');
	}

	if ((trmConfig_.outputRate != 22050.0) && (trmConfig_.outputRate != 44100.0)) {
		trmConfig_.outputRate = 44100.0;
	}
	if ((trmConfig_.vtlOffset + voices_[trmControlModelConfig_.voiceType].meanLength) < 15.9) {
		trmConfig_.outputRate = 44100.0;
	}

	if (Log::debugEnabled) {
		printf("Tube Length = %f\n", trmConfig_.vtlOffset + voices_[trmControlModelConfig_.voiceType].meanLength);
		printf("Voice: %d L: %f  tp: %f  tnMin: %f  tnMax: %f  glotPitch: %f\n", trmControlModelConfig_.voiceType,
			voices_[trmControlModelConfig_.voiceType].meanLength, voices_[trmControlModelConfig_.voiceType].tp, voices_[trmControlModelConfig_.voiceType].tnMin,
			voices_[trmControlModelConfig_.voiceType].tnMax, voices_[trmControlModelConfig_.voiceType].glotPitchMean);
		printf("sampling Rate: %f\n", trmConfig_.outputRate);
	}

	eventList_.setPitchMean(trmControlModelConfig_.pitchOffset + voices_[trmControlModelConfig_.voiceType].glotPitchMean);
	eventList_.setGlobalTempo(trmControlModelConfig_.tempo);
	setIntonation(trmControlModelConfig_.intonation);
	eventList_.setUpDriftGenerator(trmControlModelConfig_.driftDeviation, trmControlModelConfig_.controlRate, trmControlModelConfig_.driftLowpassCutoff);

	FILE* fp = fopen(trmParamFile, "wb");
	if (!fp) {
		THROW_EXCEPTION(TRMControlModelException, "Could not open the file " << trmParamFile << '.');
	}
	fprintf(fp, "%f\n%f\n%f\n%d\n%f\n%d\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%d\n%f\n",
		trmConfig_.outputRate,
		trmControlModelConfig_.controlRate,
		trmConfig_.volume,
		trmConfig_.channels,
		trmConfig_.balance,
		trmConfig_.waveform,
		trmConfig_.tp,
		trmConfig_.tnMin,
		trmConfig_.tnMax,
		trmConfig_.breathiness,
		trmConfig_.vtlOffset + voices_[trmControlModelConfig_.voiceType].meanLength, // tube length
		trmConfig_.temperature,
		trmConfig_.lossFactor,
		trmConfig_.apScale,
		trmConfig_.mouthCoef,
		trmConfig_.noseCoef,
		trmConfig_.noseRadius[1],
		trmConfig_.noseRadius[2],
		trmConfig_.noseRadius[3],
		trmConfig_.noseRadius[4],
		trmConfig_.noseRadius[5],
		trmConfig_.throatCutoff,
		trmConfig_.throatVol,
		trmConfig_.modulation,
		trmConfig_.mixOffset
	);
	fclose(fp);
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

void
Controller::initVoices(const char* configDirPath)
{
	FILE* fp;
	char line[256];
	int currentVoice = 0;

	memset(voices_, 0, sizeof(VoiceConfig) * MAX_VOICES);
	std::ostringstream path;
	path << configDirPath << VOICES_CONFIG_FILE_NAME;
	fp = fopen(path.str().c_str(), "rb");
	if (fp == NULL) {
		THROW_EXCEPTION(IOException, "Could not open the file " << path.str().c_str() << '.');
	}
	while (fgets(line, 256, fp)) {
		if ((line[0] == '#') || (line[0] == ' ')) {
			// Skip.
		} else {
			if (!strncmp(line, "Male", 4)) {
				currentVoice = 0;
			} else if (!strncmp(line, "Female", 6)) {
				currentVoice = 1;
			} else if (!strncmp(line, "LgChild", 7)) {
				currentVoice = 2;
			} else if (!strncmp(line, "SmChild", 7)) {
				currentVoice = 3;
			} else if (!strncmp(line, "Baby", 4)) {
				currentVoice = 4;
			} else if (!strncmp(line, "length", 6)) {
				voices_[currentVoice].meanLength = atof(&line[6]);
			} else if (!strncmp(line, "tp", 2)) {
				voices_[currentVoice].tp = atof(&line[2]);
			} else if (!strncmp(line, "tnMin", 5)) {
				voices_[currentVoice].tnMin = atof(&line[5]);
			} else if (!strncmp(line, "tnMax", 5)) {
				voices_[currentVoice].tnMax = atof(&line[5]);
			} else if (!strncmp(line, "glotPitch", 9)) {
				voices_[currentVoice].glotPitchMean = atof(&line[9]);
			}
		}
	}
	fclose(fp);

	if (Log::debugEnabled) {
		printf("===== Voices configuration:\n");
		for (int i = 0; i < MAX_VOICES; i++) {
			printf("L: %f  tp: %f  tnMin: %f  tnMax: %f  glotPitch: %f\n",
				voices_[i].meanLength, voices_[i].tp, voices_[i].tnMin,
				voices_[i].tnMax, voices_[i].glotPitchMean);
		}
	}
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
