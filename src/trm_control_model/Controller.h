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
#include <fstream>
#include <istream>
#include <vector>

#include "EventList.h"
#include "Log.h"
#include "Model.h"
#include "TRMConfiguration.h"
#include "TRMControlModelConfiguration.h"
#include "Tube.h"



namespace GS {
namespace TRMControlModel {

class Controller {
public:
	Controller(const char* configDirPath, Model& model);
	~Controller();

	template<typename T> void synthesizePhoneticString(T& phoneticStringParser, const char* phoneticString, const char* trmParamFile, const char* outputFile);

	Model& model() { return model_; }
	EventList& eventList() { return eventList_; }
	Configuration& trmControlModelConfiguration() { return trmControlModelConfig_; }
private:
	enum {
		MAX_VOICES = 5
	};

	Controller(const Controller&) = delete;
	Controller& operator=(const Controller&) = delete;

	void loadConfiguration(const char* configDirPath);
	void initUtterance(std::ostream& trmParamStream);
	int calcChunks(const char* string);
	int nextChunk(const char* string);
	void printVowelTransitions();

	int validPosture(const char* token);
	void setIntonation(int intonation);

	template<typename T> void synthesizePhoneticString(T& phoneticStringParser, const char* phoneticString, std::iostream& trmParamStream);
	template<typename T> void synthesizePhoneticStringChunk(T& phoneticStringParser, const char* phoneticStringChunk, std::ostream& trmParamStream);

	Model& model_;
	EventList eventList_;
	Configuration trmControlModelConfig_;
	TRM::Configuration trmConfig_;
};



template<typename T>
void
Controller::synthesizePhoneticString(T& phoneticStringParser, const char* phoneticString, const char* trmParamFile, const char* outputFile)
{
	std::fstream trmParamStream(trmParamFile, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	if (!trmParamStream) {
		THROW_EXCEPTION(IOException, "Could not open the file " << trmParamFile << '.');
	}

	synthesizePhoneticString(phoneticStringParser, phoneticString, trmParamStream);

	TRM::Tube trm;
	trm.synthesizeToFile(trmParamStream, outputFile);
}

template<typename T>
void
Controller::synthesizePhoneticString(T& phoneticStringParser, const char* phoneticString, std::iostream& trmParamStream)
{
	int chunks = calcChunks(phoneticString);

	initUtterance(trmParamStream);

	int index = 0;
	while (chunks > 0) {
		if (Log::debugEnabled) {
			printf("Speaking \"%s\"\n", &phoneticString[index]);
		}

		synthesizePhoneticStringChunk(phoneticStringParser, &phoneticString[index], trmParamStream);

		index += nextChunk(&phoneticString[index + 2]) + 2;
		chunks--;
	}

	trmParamStream.seekg(0);
}

template<typename T>
void
Controller::synthesizePhoneticStringChunk(T& phoneticStringParser, const char* phoneticStringChunk, std::ostream& trmParamStream)
{
	eventList_.setUp();

	phoneticStringParser.parseString(phoneticStringChunk);

	eventList_.generateEventList();

	eventList_.applyIntonation();
	eventList_.applyIntonationSmooth();

	eventList_.generateOutput(trmParamStream);
}

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_CONTROLLER_H_ */
