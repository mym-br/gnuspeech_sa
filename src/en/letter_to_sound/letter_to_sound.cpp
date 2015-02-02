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

/******************************************************************************
*
*     Routines to return pronunciation of word based on letter-to-sound
*     rules.
*
******************************************************************************/

#include "en/letter_to_sound/letter_to_sound.h"

#include <string.h>
#include <stdio.h>

#include "en/letter_to_sound/word_to_patphone.h"
#include "en/letter_to_sound/isp_trans.h"
#include "en/letter_to_sound/syllabify.h"
#include "en/letter_to_sound/apply_stress.h"
#include "en/letter_to_sound/tail.h"



/*  LOCAL DEFINES  ***********************************************************/
#define WORD_TYPE_UNKNOWN          "j"
#define WORD_TYPE_DELIMITER        '%'
#define MAX_WORD_LENGTH            1024
#define MAX_PRONUNCIATION_LENGTH   8192
#define MAX(a,b)                   (a > b ? a : b)
#define WORDEND(word,string)       (!strcmp(MAX(word+strlen(word)-strlen(string),word),string))



namespace {

const char* word_type(const char* word);



/******************************************************************************
*
*	function:	word_type
*
*	purpose:	Returns the word type based on the word spelling.
*
*       arguments:      word
*
*	internal
*	functions:	WORDEND
*
*	library
*	functions:	(strlen, strcmp)
*
******************************************************************************/
const char*
word_type(const char* word)
{
	const tail_entry* list_ptr;

	/*  IF WORD END MATCHES LIST, RETURN CORRESPONDING TYPE  */
	for (list_ptr = tail_list; list_ptr->tail; list_ptr++) {
		if (WORDEND(word, list_ptr->tail)) {
			return list_ptr->type;
		}
	}

	/*  ELSE RETURN UNKNOWN WORD TYPE  */
	return WORD_TYPE_UNKNOWN;
}

} /* namespace */

//==============================================================================

namespace GS {
namespace En {

/******************************************************************************
*
*	function:	letter_to_sound
*
*	purpose:	Returns pronunciation of word based on letter-to-sound
*                       rules.  Returns NULL if any error (rare).
*
******************************************************************************/
void
letter_to_sound(const char* word, std::vector<char>& pronunciation)
{
	char buffer[MAX_WORD_LENGTH + 3];
	int number_of_syllables = 0;

	pronunciation.assign(MAX_PRONUNCIATION_LENGTH + 1, '\0');

	/*  FORMAT WORD  */
	sprintf(buffer, "#%s#", word);

	/*  CONVERT WORD TO PRONUNCIATION  */
	if (!word_to_patphone(buffer)) {
		isp_trans(buffer, &pronunciation[0]);
		/*  ATTEMPT TO MARK SYLL/STRESS  */
		number_of_syllables = syllabify(&pronunciation[0]);
		if (apply_stress(&pronunciation[0], word)) { // error
			pronunciation.clear();
			return;
		}
	} else {
		strcpy(&pronunciation[0], buffer);
	}

	/*  APPEND WORD_TYPE_DELIMITER  */
	pronunciation[strlen(&pronunciation[0]) - 1] = WORD_TYPE_DELIMITER;

	/*  GUESS TYPE OF WORD  */
	if (number_of_syllables != 1) {
		strcat(&pronunciation[0], word_type(word));
	} else {
		strcat(&pronunciation[0], WORD_TYPE_UNKNOWN);
	}

	/*  RETURN RESULTING PRONUNCIATION  */
	return;
}

} /* namespace En */
} /* namespace GS */
