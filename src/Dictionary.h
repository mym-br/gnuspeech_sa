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

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <string>
#include <unordered_map>

namespace GS {

class Dictionary {
public:
	Dictionary();
	~Dictionary();

	void load(const char* filePath);
	const char* getEntry(const char* word) const;
	const char* version() const;
private:
	Dictionary(const Dictionary&) = delete;
	Dictionary& operator=(const Dictionary&) = delete;

	std::unordered_map<std::string, std::string> map_;
	std::string version_;
};

} /* namespace GS */

#endif /* DICTIONARY_H_ */
