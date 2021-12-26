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

#include "en/letter_to_sound/medial_silent_e.h"

#include "en/letter_to_sound/member.h"
#include "en/letter_to_sound/insert_mark.h"



namespace GS {
namespace En {

/******************************************************************************
*
*	function:	medial_silent_e
*
*	purpose:	
*                       
*       arguments:      in, eow
*                       
*	internal
*	functions:	member, insert_mark
*
*	library
*	functions:	none
*
******************************************************************************/
void
medial_silent_e(char *in, char **eow)
{
    char *end = *eow;
    char *position;
    int index;

    for (position = in + 2; position < end - 5; position++) {
	if (!member(position[0], "bcdfgmnprst"))
	    continue;		/* c */
	if (!member(position[1], "bdfgkpt"))
	    continue;		/* k */
	if ((position[2] != 'l') || (position[3] != 'e'))
	    continue;		/* le */
	if (member(position[4] | 040, "aeiouy"))
	    continue;		/* s */
	if (position[4] == '|')
	    continue;

	index = 5;
	while (!member(position[index] | 040, "aeiouy|")) {	/* he */
	    index++;
	    if (&position[index] >= end) {
		index = 0;
		break;
	    }
	}

	if (!index)
	    continue;
	if (position[index] == '|')
	    continue;
	if ((position[index] == 'e') && (position[index + 1] == '|'))
	    continue;
	insert_mark(&end, &position[3]);
	break;
    }

    for (position = in; position < end - 5; position++) {
	if ((member(position[0], "aeiou#")))
	    continue;
	if (!member(position[1], "aiouy"))
	    continue;
	if (member(position[2] | 040, "aehiouwxy"))
	    continue;
	if (position[3] != 'e')
	    continue;
	if (member(position[4] | 040, "aeiouynr"))
	    continue;

	index = 5;
	if ((position[index] == '|') ||
	    ((position[index] == 'e') && (position[++index] == '|')))
	    continue;
	index++;
	if (!member(position[index] | 040, "aeiouy"))
	    continue;
	insert_mark(&end, &position[3]);
	position[1] &= 0xdf;
	break;
    }

    for (position = in + 1; position < end - 5; position++) {
	if (position[0] != 'o')
	    continue;
	if (!member(position[1], "aiouyU"))
	    continue;
	if (member(position[2] | 040, "aehiouwxy"))
	    continue;
	if (position[3] != 'e')
	    continue;
	if (member(position[4] | 040, "aeiouynr"))
	    continue;
	index = 5;
	if ((position[index] == '|') ||
	    ((position[index] == 'e') && (position[++index] == '|')))
	    continue;
	index++;
	if (!member(position[index] | 040, "aeiouy"))
	    continue;
	insert_mark(&end, &position[3]);
	break;
    }
    *eow = end;
}

} /* namespace En */
} /* namespace GS */
