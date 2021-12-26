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

#include "en/letter_to_sound/word_to_patphone.h"

#include <array>
#include <string.h>

#include "en/letter_to_sound/vowel_before.h"
#include "en/letter_to_sound/check_word_list.h"
#include "en/letter_to_sound/final_s.h"
#include "en/letter_to_sound/ie_to_y.h"
#include "en/letter_to_sound/mark_final_e.h"
#include "en/letter_to_sound/long_medial_vowels.h"
#include "en/letter_to_sound/medial_silent_e.h"
#include "en/letter_to_sound/medial_s.h"
#include "en/number_pronunciations.h"



/*  LOCAL DEFINES  ***********************************************************/
#define SPELL_STRING_LEN   8192



namespace {

int spell_it(char* word);
int all_caps(char* in);



const char* letters[] = {
  BLANK, EXCLAMATION_POINT, DOUBLE_QUOTE, NUMBER_SIGN, DOLLAR_SIGN,
  PERCENT_SIGN, AMPERSAND, SINGLE_QUOTE, OPEN_PARENTHESIS, CLOSE_PARENTHESIS,
  ASTERISK, PLUS_SIGN, COMMA, HYPHEN, PERIOD, SLASH,
  ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
  COLON, SEMICOLON, OPEN_ANGLE_BRACKET, EQUAL_SIGN, CLOSE_ANGLE_BRACKET,
  QUESTION_MARK, AT_SIGN,
  A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
  OPEN_SQUARE_BRACKET, BACKSLASH, CLOSE_SQUARE_BRACKET, CARET, UNDERSCORE,
  GRAVE_ACCENT,
  A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
  OPEN_BRACE, VERTICAL_BAR, CLOSE_BRACE, TILDE, UNKNOWN
};



/******************************************************************************
*
*	function:	spell_it
*
*	purpose:
*
*
*       arguments:      word
*
*	internal
*	functions:	none
*
*	library
*	functions:	strcpy
*
******************************************************************************/
int
spell_it(char* word)
{
	std::array<char, SPELL_STRING_LEN> spell_string;
	spell_string.fill(0);

	char* s = &spell_string[0];
	const char* t;
	char* hold = word;

	/*  EAT THE '#'  */
	word++;

	do {
		if (*word < ' ') {
			if (*word == '\t') {
				t = "'t_aa_b";
			} else {
				t = "'u_p_s";	/* (OOPS!) */
			}
		} else {
			t = letters[*word - ' '];
		}
		word++;
		while (*t) {
			*s++ = *t++;
		}
	} while (*word != '#');

	*s = 0;

	strcpy(hold, &spell_string[0]);
	return 2;
}

/******************************************************************************
*
*	function:	all_caps
*
*	purpose:
*
*
*       arguments:      in
*
*	internal
*	functions:	none
*
*	library
*	functions:	none
*
******************************************************************************/
int
all_caps(char *in)
{
    int                 all_up = 1;
    int                 force_up = 0;

    in++;
    if (*in == '#')
	force_up = 1;

    while (*in != '#') {
	if ((*in <= 'z') && (*in >= 'a'))
	    all_up = 0;
	else if ((*in <= 'Z') && (*in >= 'A'))
	    *in |= 0x20;
	else if (*in != '\'')
	    force_up = 1;
	in++;
    }
    return (all_up || force_up);
}

} /* namespace */

//==============================================================================

namespace GS {
namespace En {

/******************************************************************************
*
*	function:	word_to_patphone
*
*	purpose:	
*                       
*			
*       arguments:      word
*                       
*	internal
*	functions:	all_caps, spell_it, vowel_before, check_word_list,
*                       final_s, ie_to_y, mark_final_e, long_medial_vowels,
*                       medial_silent_e, medial_s
*
*	library
*	functions:	none
*
******************************************************************************/
int
word_to_patphone(char *word)
{
    char *end_of_word;
    char replace_s = 0;

    /*  FIND END OF WORD  */
    end_of_word = word + 1;
    while (*end_of_word != '#')
	end_of_word++;

    /*  IF NO LITTLE LETTERS SPELL THE WORD  */
    if (all_caps(word))
	return(spell_it(word));

    /*  IF SINGLE LETTER, SPELL IT  */
    if (end_of_word == (word + 2))
	return(spell_it(word));

    /*  IF NO VOWELS SPELL THE WORD  */
    if (!vowel_before(word, end_of_word))
	return(spell_it(word));

    /*  SEE IF IT IS IN THE EXCEPTION LIST  */
    if (check_word_list(word, &end_of_word)) {
	*++end_of_word = 0;
	return(1);
    }

    /*  KILL ANY TRAILING S  */
    replace_s = final_s(word, &end_of_word);

    /*  FLIP IE TO Y, IF ANY CHANGES RECHECK WORD LIST  */
    if (ie_to_y(word, &end_of_word) || replace_s)
        /*  IN WORD LIST NOW ALL DONE  */
	if (check_word_list(word, &end_of_word)) {   /* Will eliminate this as well */
	    if (replace_s) {
		*++end_of_word = replace_s;            /* & 0x5f [source of problems] */
		*++end_of_word = '/';
	    }
	    *++end_of_word = 0;
	    return(1);
	}

    mark_final_e(word, &end_of_word);
    long_medial_vowels(word, &end_of_word);
    medial_silent_e(word, &end_of_word);
    medial_s(word, &end_of_word);

    if (replace_s) {
	*end_of_word++ = replace_s;
	*end_of_word = '#';
    }
    *++end_of_word = 0;
    return(0);
}

} /* namespace En */
} /* namespace GS */
