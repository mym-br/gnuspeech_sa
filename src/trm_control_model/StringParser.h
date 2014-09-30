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

#ifndef TRM_CONTROL_MODEL_STRING_PARSER_H_
#define TRM_CONTROL_MODEL_STRING_PARSER_H_

#include "EventList.h"
#include "Model.h"



namespace TRMControlModel {

class StringParser {
public:
	StringParser(const char* configDirPath, const Model& model, EventList& eventList);
	~StringParser();

	int parseString(const char* string);
private:
	StringParser(const StringParser&);
	StringParser& operator=(const StringParser&);

	struct RewriterData {
		int currentState;
		const Phone* lastPhone;
		RewriterData() : currentState(0), lastPhone(nullptr) {}
	};

	void initVowelTransitions(const char* configDirPath);
	void printVowelTransitions();
	const Phone* rewrite(const Phone& nextPhone, int wordMarker, RewriterData& data);
	const Phone* calcVowelTransition(const Phone& nextPhone, RewriterData& data);

	const Model& model_;
	EventList& eventList_;
	const Category* category_[18];
	const Phone* returnPhone_[7];
	int vowelTransitions_[13][13];
};

} /* namespace TRMControlModel */

#endif /* TRM_CONTROL_MODEL_STRING_PARSER_H_ */
