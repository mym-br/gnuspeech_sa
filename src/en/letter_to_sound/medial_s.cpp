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

#include "en/letter_to_sound/medial_s.h"

#include "en/letter_to_sound/member.h"



namespace GS {
namespace En {

/******************************************************************************
*
*	function:	medial_s
*
*	purpose:	
*                       
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
void
medial_s(char *in, char **eow)
{
    char *end = *eow;

    while (in < end - 1) {
	if ((member(*in | 040, "aeiouy")) && (in[1] == 's')
	    && (member(in[2], "AEIOUYaeiouym")))
	    in[1] &= 0xdf;
	in++;
    }
}

} /* namespace En */
} /* namespace GS */
