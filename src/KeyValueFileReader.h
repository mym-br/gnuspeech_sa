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

#ifndef KEY_VALUE_FILE_READER_H_
#define KEY_VALUE_FILE_READER_H_

#include <string>
#include <unordered_map>

#include "Exception.h"



namespace GS {

// Format: "key = value"
class KeyValueFileReader {
public:
	KeyValueFileReader(const std::string& filePath);

	template<typename T> T value(const std::string& key) const;
	template<typename T> T value(const std::string& key, T minValue, T maxValue) const;
private:
	typedef std::unordered_map<std::string, std::string> Map;

	template<typename T> static T convertString(const std::string& s);

	std::string filePath_;
	Map valueMap_;
};



template<typename T>
T
KeyValueFileReader::value(const std::string& key) const
{
	auto iter = valueMap_.find(key);
	if (iter == valueMap_.end()) {
		THROW_EXCEPTION(InvalidParameterException, "Key '" << key << "' not found in file " << filePath_ << '.');
	}

	T value;
	try {
		value = convertString<T>(iter->second);
	} catch (const std::invalid_argument& e) {
		THROW_EXCEPTION(InvalidValueException, "No value conversion could be performed for key '" << key << "' in file " << filePath_ << ": " << e.what() << '.');
	} catch (const std::out_of_range& e) {
		THROW_EXCEPTION(InvalidValueException, "The converted value would fall out of the range for key '" << key << "' in file " << filePath_ << ": " << e.what() << '.');
	}
	return value;
}

template<typename T>
T
KeyValueFileReader::value(const std::string& key, T minValue, T maxValue) const
{
	T v = value<T>(key);

	if (v > maxValue) {
		THROW_EXCEPTION(InvalidValueException, "The value for key '" << key << "' must be <= " << maxValue << " in file " << filePath_ << '.');
	} else if (v < minValue) {
		THROW_EXCEPTION(InvalidValueException, "The value for key '" << key << "' must be >= " << minValue << " in file " << filePath_ << '.');
	}

	return v;
}

} /* namespace GS */

#endif /* KEY_VALUE_FILE_READER_H_ */
