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
#include "Log.h"
#include "Model.h"
#include "TRMConfiguration.h"
#include "TRMControlModelConfiguration.h"



namespace GS {
namespace TRMControlModel {

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

	template<typename T> void synthesizePhoneticString(T& phoneticStringParser, const char* phoneticString, const char* trmParamFile, const char* outputFile);

	Model& model() { return model_; }
	EventList& eventList() { return eventList_; }
private:
	enum {
		MAX_VOICES = 5
	};

	Controller(const Controller&);
	Controller& operator=(const Controller&);

	void initUtterance(const char* trmParamFile);
	int calcChunks(const char* string);
	int nextChunk(const char* string);
	void printVowelTransitions();
	void initVoices(const char* configDirPath);

	int validPhone(const char* token);
	void setIntonation(int intonation);

	template<typename T> void synthesizePhoneticStringChunk(T& phoneticStringParser, const char* phoneticStringChunk, const char* trmParamFile);

	Model& model_;
	VoiceConfig voices_[MAX_VOICES];
	EventList eventList_;
	Configuration trmControlModelConfig_;
	TRM::Configuration trmConfig_;
};



template<typename T>
void
Controller::synthesizePhoneticString(T& phoneticStringParser, const char* phoneticString, const char* trmParamFile, const char* outputFile)
{
	int chunks = calcChunks(phoneticString);

	initUtterance(trmParamFile);

	int index = 0;
	while (chunks > 0) {
		if (Log::debugEnabled) {
			printf("Speaking \"%s\"\n", &phoneticString[index]);
		}

		synthesizePhoneticStringChunk(phoneticStringParser, &phoneticString[index], trmParamFile);

		index += nextChunk(&phoneticString[index + 2]) + 2;
		chunks--;
	}

	TRM::Tube trm;
	trm.synthesizeToFile(trmParamFile, outputFile);
}

template<typename T>
void
Controller::synthesizePhoneticStringChunk(T& phoneticStringParser, const char* phoneticStringChunk, const char* trmParamFile)
{
	phoneticStringParser.parseString(phoneticStringChunk);

	eventList_.generateEventList();

	eventList_.applyIntonation();
	eventList_.applyIntonationSmooth();

	eventList_.generateOutput(trmParamFile);
	eventList_.setUp();
}

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_CONTROLLER_H_ */
