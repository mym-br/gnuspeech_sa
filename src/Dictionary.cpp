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

#include "Dictionary.h"

#include <fstream>
#include <iostream>

#include "Exception.h"
#include "Log.h"



namespace GS {

Dictionary::Dictionary()
{
}

Dictionary::~Dictionary()
{
}

void
Dictionary::load(const char* filePath)
{
	map_.clear();

	std::ifstream in(filePath);
	if (!in) {
		THROW_EXCEPTION(IOException, "Could not open the file " << filePath << '.');
	}

	if (!std::getline(in, version_)) {
		THROW_EXCEPTION(IOException, "Could not read the dictionary version.");
	}
	LOG_DEBUG("Dictionary version: " << version_);

	std::string line;
	while (std::getline(in, line)) {
		auto pos = line.find_first_of(' ');
		if (pos == std::string::npos) {
			THROW_EXCEPTION(IOException, "Could not find a space in the line: [" << line << ']');
		}

		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1, std::string::npos);
		//std::cout << "key[" << key << "] value[" << value << ']' << std::endl;

		auto iter = map_.find(key);
		if (iter == map_.end()) {
			map_[key] = value;
		} else {
			std::cerr << "Duplicate word: [" << key << ']' << std::endl;
			//THROW_EXCEPTION(IOException, "Duplicate word: [" << key << ']');
		}
	}
}

const char*
Dictionary::getEntry(const char* word) const
{
	if (map_.empty()) {
		return nullptr;
	}

	std::string key(word);
	auto iter = map_.find(key);
	if (iter == map_.end()) {
		return nullptr;
	} else {
		return iter->second.c_str();
	}
}

const char*
Dictionary::version() const
{
	if (map_.empty()) {
		return "None";
	}

	return version_.c_str();
}

} /* namespace GS */
