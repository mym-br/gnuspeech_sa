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
// This file was copied from Gnuspeech and modified by Marcelo Y. Matuda.

#include "Controller.h"

#include <cstring>
#include <sstream>

#include "Exception.h"
#include "Tube.h"

#define VOICES_CONFIG_FILE_NAME "/voices.config"

#define TTS_INTONATION_NONE		0x00
#define TTS_INTONATION_MICRO		0x01
#define TTS_INTONATION_MACRO		0x02
#define TTS_INTONATION_DECLIN		0x04
#define TTS_INTONATION_CREAK		0x08
#define TTS_INTONATION_RANDOMIZE	0x10
#define TTS_INTONATION_ALL		0x1f

#define TTS_INTONATION_DEF		0x1f
//#define TTS_INTONATION_DEF		TTS_INTONATION_MICRO



namespace TRMControlModel {

Controller::Controller(const char* configDirPath, Model& model)
		: model_(model)
		, eventList_(configDirPath, model_)
		, stringParser_(configDirPath, model_, eventList_)
{
	initVoices(configDirPath);
}

Controller::~Controller()
{
}

void
Controller::initUtterance(const char* trmParamFile)
{
	//TODO: get values from file
	synthConfig_.intonation = TTS_INTONATION_DEF;
	synthConfig_.random = synthConfig_.intonation & TTS_INTONATION_RANDOMIZE;
	synthConfig_.voiceType = 0;
	synthConfig_.channels = 1;
	synthConfig_.volume = 60.0;
	synthConfig_.speed = 1.0;
	synthConfig_.balance = 0.0;
	synthConfig_.pitchOffset = 0.0;
	synthConfig_.vtlOffset = 0.0;
	synthConfig_.breathiness = 0.5;
	synthConfig_.samplingRate = 22050.0;

	if ((synthConfig_.samplingRate != 22050.0f) && (synthConfig_.samplingRate != 44100.0f)) {
		synthConfig_.samplingRate = 22050.0f;
	}
	if ((synthConfig_.vtlOffset + voices_[synthConfig_.voiceType].meanLength) < 15.9) {
		synthConfig_.samplingRate = 44100.0;
	}

#ifdef VERBOSE
	printf("Tube Length = %f\n", synthConfig_.vtlOffset + voices_[synthConfig_.voiceType].meanLength);
	printf("Voice: %d L: %f  tp: %f  tnMin: %f  tnMax: %f  glotPitch: %f\n", synthConfig_.voiceType,
		voices_[synthConfig_.voiceType].meanLength, voices_[synthConfig_.voiceType].tp, voices_[synthConfig_.voiceType].tnMin,
		voices_[synthConfig_.voiceType].tnMax, voices_[synthConfig_.voiceType].glotPitchMean);
	printf("sampling Rate: %f\n", synthConfig_.samplingRate);
#endif

	eventList_.setPitchMean(synthConfig_.pitchOffset + voices_[synthConfig_.voiceType].glotPitchMean);
	eventList_.setGlobalTempo(1.0 / synthConfig_.speed);
	setIntonation(synthConfig_.intonation);
	eventList_.setTgUseRandom(synthConfig_.random);

	//TODO: get values from file
	FILE* fp = fopen(trmParamFile, "w");
	if (!fp) {
		THROW_EXCEPTION(TRMControlModelException, "Could not open the file " << trmParamFile << '.');
	}
	fprintf(fp,"%f\n%f\n%f\n%d\n%f\n%d\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%d\n%f\n",
		synthConfig_.samplingRate,
		250.0,
		synthConfig_.volume,
		synthConfig_.channels,
		synthConfig_.balance,
		0, 		/* Waveform */
		voices_[synthConfig_.voiceType].tp    * 100.0,	/* tp */
		voices_[synthConfig_.voiceType].tnMin * 100.0,	/* tn Min */
		voices_[synthConfig_.voiceType].tnMax * 100.0,	/* tn Max */
		synthConfig_.breathiness,
		synthConfig_.vtlOffset + voices_[synthConfig_.voiceType].meanLength,
		32.0,	/* Temperature */
		0.8, 	/* Loss Factor */
		3.05,	/* Ap scaling */
		5000.0, 5000.0, 	/* Mouth and nose coef */
		1.35, 1.96, 1.91, 	/* n1, n2, n3 */
		1.3, 0.73,		/* n4, n5 */
		1500.0, 6.0,		/* Throat cutoff and volume */
		1, 48.0			/* Noise Modulation, mixOffset */
	);
	fclose(fp);
}

void
Controller::synthesizePhoneticString(const char* phoneticString, const char* trmParamFile, const char* outputFile)
{
	int chunks = calcChunks(phoneticString);

	initUtterance(trmParamFile);

	int index = 0;
	while (chunks > 0) {
#ifdef VERBOSE
		printf("Speaking \"%s\"\n", &phoneticString[index]);
#endif
		synthesizePhoneticStringChunk(&phoneticString[index], trmParamFile);

		index += nextChunk(&phoneticString[index + 2]) + 2;
		chunks--;
	}

	TRM::verbose = true;
	TRM::Tube trm;
	trm.synthesizeToFile(trmParamFile, outputFile);
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
	fp = fopen(path.str().c_str(), "r");
	if (fp == NULL) {
		THROW_EXCEPTION(IOException, "Could not open the file " << path.str().c_str() << '.');
	}
	while (fgets(line, 256, fp)) {
		if ((line[0] == '#') || (line[0] == ' ')) {
			// Skip.
		} else {
			if (!strncmp(line, "MinBlack", 8)) {
				minBlack_ = atof(&line[8]);
			} else if (!strncmp(line, "MinWhite", 8)) {
				minWhite_ = atof(&line[8]);
			} else if (!strncmp(line, "Male", 4)) {
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

#ifdef VERBOSE
	printf("===== Voices configuration:\n");
	printf("MinBlack = %f MinWhite = %f\n", minBlack_, minWhite_);
	for (int i = 0; i<MAX_VOICES; i++) {
		printf("L: %f  tp: %f  tnMin: %f  tnMax: %f  glotPitch: %f\n",
			voices_[i].meanLength, voices_[i].tp, voices_[i].tnMin,
			voices_[i].tnMax, voices_[i].glotPitchMean);
	}
#endif
}

int
Controller::validPhone(const char* token)
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
		return (model_.findPhone(token) != nullptr);
	}
}

void
Controller::setIntonation(int intonation)
{
	if (!intonation) {
		eventList_.setMicroIntonation(0);
		eventList_.setMacroIntonation(0);
		eventList_.setDrift(0);
		eventList_.setSmoothIntonation(0);
		return;
	}

	if (intonation & TTS_INTONATION_MICRO) {
		eventList_.setMicroIntonation(1);
	} else {
		eventList_.setMicroIntonation(0);
	}

	if (intonation & TTS_INTONATION_MACRO) {
		eventList_.setMacroIntonation(1);
		eventList_.setSmoothIntonation(1);
	} else {
		eventList_.setMacroIntonation(0);
		eventList_.setSmoothIntonation(0);
	}

	eventList_.setDrift(1);
}

void
Controller::synthesizePhoneticStringChunk(const char* phoneticStringChunk, const char* trmParamFile)
{
	stringParser_.parseString(eventList_, phoneticStringChunk);

	eventList_.generateEventList();

	eventList_.applyIntonation();
	eventList_.applyIntonationSmooth();

	eventList_.generateOutput(trmParamFile);
	eventList_.setUp();
}

} /* namespace TRMControlModel */
