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

#ifndef TRM_CONTROL_MODEL_CONTROLLER_H_
#define TRM_CONTROL_MODEL_CONTROLLER_H_

#include <cstdio>

#include "EventList.h"
#include "Model.h"
#include "StringParser.h"



namespace GS {
namespace TRMControlModel {

struct SynthConfig {
	int intonation;
	int random;
	int voiceType;
	int channels;
	float volume;
	float speed;
	float balance;
	float pitchOffset;
	float vtlOffset; // vocal tract length offset
	float breathiness;
	float samplingRate;
};

struct VoiceConfig {
	float meanLength;
	float tp;
	float tnMin;
	float tnMax;
	float glotPitchMean;
};

class Controller {
public:
	Controller(const char* configDirPath, Model& model);
	~Controller();

	void synthesizePhoneticString(const char* phoneticString, const char* trmParamFile, const char* outputFile);
private:
	enum {
		MAX_VOICES = 5
	};

	Controller(const Controller&);
	Controller& operator=(const Controller&);

	void initUtterance(const char* trmParamFile);
	int calcChunks(const char* string);
	int nextChunk(const char* string);
//	float* parseGroups(int number, FILE* fp);
//	void initToneGroups(const char* systemPath);
//	void printToneGroups();
	void printVowelTransitions();
	void initVoices(const char* configDirPath);

	int validPhone(const char* token);
	void setIntonation(int intonation);
	void synthesizePhoneticStringChunk(const char* phoneticStringChunk, const char* trmParamFile);

	Model& model_;
	SynthConfig synthConfig_;
	VoiceConfig voices_[MAX_VOICES];
	EventList eventList_;
	StringParser stringParser_;

//	float intonationRandom_;
//	float* tgParameters_[5];
//	int tgCount_[5];

	float minBlack_;//TODO: remove?
	float minWhite_;//TODO: remove?
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_CONTROLLER_H_ */
