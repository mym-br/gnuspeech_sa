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

#include "DictionarySearch.h"

#include <cstring>

#include "en/dictionary/suffix_list.h"



namespace {

/**************************************************************************
*
*       function:   word_has_suffix
*
*       purpose:    Returns position of suffix if word has suffix which
*                   matches, else returns NULL.
*
**************************************************************************/
const char*
word_has_suffix(const char* word, const char* suffix)
{
	int word_length, suffix_length;
	const char* suffix_position;

	/*  GET LENGTH OF WORD AND SUFFIX  */
	word_length = strlen(word);
	suffix_length = strlen(suffix);

	/*  DON'T ALLOW SUFFIX TO BE LONGER THAN THE WORD, OR THE WHOLE WORD  */
	if (suffix_length >= word_length) {
		return nullptr;
	}

	/*  FIND POSITION OF SUFFIX IN WORD  */
	suffix_position = word + word_length - suffix_length;

	/*  RETURN SUFFIX POSITION IF THE SUFFIX MATCHES, ELSE RETURN NULL  */
	if (!strcmp(suffix_position, suffix)) {
		return suffix_position;
	} else {
		return nullptr;
	}
}

} /* namespace */

//==============================================================================

namespace GS {
namespace En {

void
DictionarySearch::clearBuffers()
{
	buffer_.fill('\0');
	wordTypeBuffer_.fill('\0');
}

DictionarySearch::DictionarySearch()
{
	clearBuffers();
}

DictionarySearch::~DictionarySearch()
{
}

void
DictionarySearch::load(const char* dictionaryPath)
{
	dict_.load(dictionaryPath);
}

const char*
DictionarySearch::getEntry(const char* word)
{
	return augmentedSearch(word);
}

const char*
DictionarySearch::version()
{
	return dict_.version();
}

/**************************************************************************
*
*       function:   augmented search
*
*       purpose:    First looks in main dictionary to see if word is there.
*                   If not, it tries the main dictionary without suffixes,
*                   and if found, tacks on the appropriate ending.
*
*                   NOTE:  some forms will have to be put in the main
*                   dictionary.  For example, "houses" is NOT pronounced as
*                   "house" + "s", or even "house" + "ez".
*
**************************************************************************/
const char*
DictionarySearch::augmentedSearch(const char* orthography)
{
	const char* word;
	const char* pt;
	char* word_type_pos;
	const suffix_list_t* list_ptr;

	clearBuffers();

	/*  RETURN IMMEDIATELY IF WORD FOUND IN DICTIONARY  */
	if ( (word = dict_.getEntry(orthography)) ) {
		return word;
	}

	/*  LOOP THROUGH SUFFIX LIST  */
	for (list_ptr = suffix_list; list_ptr->suffix; list_ptr++) {
		if ( (pt = word_has_suffix(orthography, list_ptr->suffix)) ) {
			/*  TACK ON REPLACEMENT ENDING  */
			strcpy(&buffer_[0], orthography);
			*(&buffer_[0] + (pt - orthography)) = '\0';
			strcat(&buffer_[0], list_ptr->replacement);

			/*  IF WORD FOUND WITH REPLACEMENT ENDING  */
			if ( (word = dict_.getEntry(&buffer_[0])) ) {
				/*  PUT THE FOUND PRONUNCIATION IN THE BUFFER  */
				strcpy(&buffer_[0], word);

				/*  FIND THE WORD-TYPE INFO  */
				for (word_type_pos = &buffer_[0]; *word_type_pos && (*word_type_pos != '%'); word_type_pos++)
					;

				/*  SAVE IT INTO WORD TYPE BUFFER  */
				strcpy(&wordTypeBuffer_[0], word_type_pos);

				/*  APPEND SUFFIX PRONUNCIATION TO WORD  */
				*word_type_pos = '\0';
				strcat(&buffer_[0], list_ptr->pronunciation);

				/*  AND PUT BACK THE WORD TYPE  */
				strncat(&buffer_[0], &wordTypeBuffer_[0], WORD_TYPE_BUF_SIZE);

				/*  RETURN WORD WITH SUFFIX AND ORIGINAL WORD TYPE  */
				return &buffer_[0];
			}
		}
	}

	/*  WORD NOT FOUND, EVEN WITH SUFFIX STRIPPED  */
	return nullptr;
}

} /* namespace En */
} /* namespace GS */
