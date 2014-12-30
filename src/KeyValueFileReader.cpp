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

#include "KeyValueFileReader.h"

#include <cctype> /* isspace */
#include <fstream>



namespace {

const char SEPARATOR_CHAR = '=';
const char COMMENT_CHAR = '#';

void
throwException(const std::string& filePath, int lineNumber, const char* message)
{
	THROW_EXCEPTION(GS::ParsingException, "[KeyValueFileReader] Error in file " << filePath << " (line " << lineNumber << "): " << message << '.');
}

template<typename T>
void
throwException(const std::string& filePath, int lineNumber, const char* message, const T& complement)
{
	THROW_EXCEPTION(GS::ParsingException, "[KeyValueFileReader] Error in file " << filePath << " (line " << lineNumber << "): " << message << complement << '.');
}

} /* namespace */

//==============================================================================

namespace GS {

KeyValueFileReader::KeyValueFileReader(const std::string& filePath)
		: filePath_(filePath)
{
	typedef Map::iterator MI;
	typedef Map::value_type VT;

	std::ifstream in(filePath.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!in) THROW_EXCEPTION(IOException, "Could not open the file: " << filePath << '.');

	std::string line;
	int lineNum = 0;
	while (getline(in, line)) {
		++lineNum;

		if (line.empty()) continue;

		auto iter = line.begin();

		// Comment (must start at the beginning of the line).
		if (*iter == COMMENT_CHAR) {
			continue;
		}

		// Read key.
		if (std::isspace(*iter)) throwException(filePath, lineNum, "Space at the beginning of the line");
		while (iter != line.end() && !std::isspace(*iter) && *iter != SEPARATOR_CHAR) {
			++iter;
		}
		if (iter == line.end()) throwException(filePath, lineNum, "Key not found");
		std::string key(line.begin(), iter);
		if (key.empty()) throwException(filePath, lineNum, "Empty key");

		// Separator.
		while (iter != line.end() && std::isspace(*iter)) ++iter; // skip spaces
		if (iter == line.end() || *iter != SEPARATOR_CHAR) throwException(filePath, lineNum, "Missing separator");
		++iter;

		// Read value.
		while (iter != line.end() && std::isspace(*iter)) ++iter; // skip spaces
		if (iter == line.end()) throwException(filePath, lineNum, "Value not found");
		std::string value(iter, line.end());
		if (value.empty()) throwException(filePath, lineNum, "Empty value");

		std::pair<MI, bool> res = valueMap_.insert(VT(key, value));
		if (!res.second) {
			throwException(filePath, lineNum, "Duplicate key: ", key);
		}
	}
}

template<>
int
KeyValueFileReader::convertString<int>(const std::string& s)
{
	return std::stoi(s);
}

template<>
long
KeyValueFileReader::convertString<long>(const std::string& s)
{
	return std::stol(s);
}

template<>
float
KeyValueFileReader::convertString<float>(const std::string& s)
{
	return std::stof(s);
}

template<>
double
KeyValueFileReader::convertString<double>(const std::string& s)
{
	return std::stod(s);
}

template<>
std::string
KeyValueFileReader::convertString<std::string>(const std::string& s)
{
	return s;
}

} /* namespace GS */
