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

#include "en/letter_to_sound/final_s.h"

#include "en/letter_to_sound/member.h"



namespace GS {
namespace En {

/******************************************************************************
*
*	function:	final_s
*
*	purpose:	Check for a final s, strip it if found and return s or
*                       z, or else return false.  Don't strip if it's the only
*                       character.
*
*       arguments:      in, eow
*                       
*	internal
*	functions:	member
*
*	library
*	functions:	none
*
******************************************************************************/
char
final_s(char * /* in */, char **eow)
{
    char *end = *eow;
    char retval = 0;

    /*  STRIP TRAILING S's  */
    if ((*(end - 1) == '\'') && (*(end - 2) == 's')) {
	*--end = '#';
	*eow = end;
    }

    /*  NOW LOOK FOR FINAL S  */
    if (*(end - 1) == 's') {
	*--end = '#';
	*eow = end;

	if (member(*(end - 1), "cfkpt"))
	    retval = 's';
	else
	    retval = 'z';

	/*  STRIP 'S  */
	if (*(end - 1) == '\'') {
	    *--end = '#';
	    *eow = end;
	}
    }
    return(retval);
}

} /* namespace En */
} /* namespace GS */
