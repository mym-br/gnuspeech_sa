/***************************************************************************
 *  Copyright 2014 Marcelo Y. Matuda                                       *
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

#ifndef TEXT_H_
#define TEXT_H_

#include <sstream>
#include <string>
#include <vector>

#include "Exception.h"



namespace GS {
namespace Text {

std::string trim(const std::string& s);



template<typename T>
std::string
convertToString(const T& t) {
	std::ostringstream out;
	out << t;
	return out.str();
}

template<typename T>
T
parseString(const std::string& s) {
	std::istringstream in(s);
	T res;
	in >> res;
	if (!in) {
		THROW_EXCEPTION(InvalidValueException, "Wrong format: " << s << '.');
	}
	if (!in.eof()) {
		THROW_EXCEPTION(InvalidValueException, "Invalid text at the end of: " << s << '.');
	}
	return res;
}

struct MatchAtBegin {
	bool operator()(const std::string& s, const std::string& prefix) const {
		std::string::size_type pos = s.find(prefix);
		return pos == 0;
	}
};

struct MatchAtEnd {
	bool operator()(const std::string& s, const std::string& suffix) const {
		std::string::size_type pos = s.rfind(suffix);
		return pos != std::string::npos && pos == s.size() - suffix.size();
	}
};

struct MatchWithOptionalCharSuffix {
	char charSuffix;
	MatchWithOptionalCharSuffix(char c) : charSuffix(c) {}
	bool operator()(const std::string& s, const std::string& other) const {
		if (s == other) return true;
		return ((s + charSuffix) == other);
	}
};

template<typename T>
void
parseValuesSeparatedByChar(const std::string& inString, char separator, std::vector<T>& outValueList) {
	std::string::size_type pos1 = 0, pos2;
	while ( (pos2 = inString.find(separator, pos1)) != std::string::npos ) {
		outValueList.push_back(parseString<T>(inString.substr(pos1, pos2 - pos1)));
		pos1 = pos2 + 1;
	}
	if (pos1 < inString.length()) {
		outValueList.push_back(parseString<T>(inString.substr(pos1)));
	}
}

} /* namespace Text */
} /* namespace GS */

#endif /* TEXT_H_ */
