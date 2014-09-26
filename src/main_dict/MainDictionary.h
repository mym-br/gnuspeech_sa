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

#ifndef MAIN_DICTIONARY_H_
#define MAIN_DICTIONARY_H_

#include "Dictionary.h"

class MainDictionary {
public:
	MainDictionary();
	~MainDictionary();

	void load(const char* configDirPath);

	// The returned string is invalidated if the dictionary is changed.
	const char* getEntry(const char* word);

	// The returned string is invalidated if the dictionary is changed.
	const char* version();
private:
	MainDictionary(const MainDictionary&);
	MainDictionary& operator=(const MainDictionary&);

	const char* augmentedSearch(const char* orthography);

	Dictionary dict_;
};

#endif /* MAIN_DICTIONARY_H_ */
