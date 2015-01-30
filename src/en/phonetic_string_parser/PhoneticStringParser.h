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

#ifndef EN_PHONETIC_STRING_PARSER_H_
#define EN_PHONETIC_STRING_PARSER_H_

#include <memory>

#include "Controller.h"



namespace GS {
namespace En {

class PhoneticStringParser {
public:
	PhoneticStringParser(const char* configDirPath, TRMControlModel::Controller& controller);
	~PhoneticStringParser();

	int parseString(const char* string);
private:
	PhoneticStringParser(const PhoneticStringParser&) = delete;
	PhoneticStringParser& operator=(const PhoneticStringParser&) = delete;

	struct RewriterData {
		int currentState;
		const TRMControlModel::Posture* lastPosture;
		RewriterData() : currentState(0), lastPosture(nullptr) {}
	};

	void initVowelTransitions(const char* configDirPath);
	void printVowelTransitions();
	const TRMControlModel::Posture* rewrite(const TRMControlModel::Posture& nextPosture, int wordMarker, RewriterData& data);
	const TRMControlModel::Posture* calcVowelTransition(const TRMControlModel::Posture& nextPosture, RewriterData& data);
	std::shared_ptr<TRMControlModel::Category> getCategory(const char* name);
	const TRMControlModel::Posture* getPosture(const char* name);

	const TRMControlModel::Model& model_;
	TRMControlModel::EventList& eventList_;
	std::shared_ptr<const TRMControlModel::Category> category_[18];
	const TRMControlModel::Posture* returnPhone_[7];
	int vowelTransitions_[13][13];
};

} /* namespace En */
} /* namespace GS */

#endif /* EN_PHONETIC_STRING_PARSER_H_ */
