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

#ifndef EN_DICTIONARY_SEARCH_H_
#define EN_DICTIONARY_SEARCH_H_

#include <array>

#include "Dictionary.h"



namespace GS {
namespace En {

class DictionarySearch {
public:
	DictionarySearch();
	~DictionarySearch();

	void load(const char* dictionaryPath);

	// The returned string is invalidated if the dictionary is changed.
	const char* getEntry(const char* word);

	// The returned string is invalidated if the dictionary is changed.
	const char* version();
private:
	enum {
		WORD_TYPE_BUF_SIZE = 32,
		MAXLEN = 1024
	};

	DictionarySearch(const DictionarySearch&) = delete;
	DictionarySearch& operator=(const DictionarySearch&) = delete;

	void clearBuffers();
	const char* augmentedSearch(const char* orthography);

	Dictionary dict_;
	std::array<char, MAXLEN> buffer_;
	std::array<char, WORD_TYPE_BUF_SIZE> wordTypeBuffer_;
};

} /* namespace En */
} /* namespace GS */

#endif /* EN_DICTIONARY_SEARCH_H_ */
