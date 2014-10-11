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

#include "en/letter_to_sound/vowel_before.h"

#include "en/letter_to_sound/member.h"



namespace GS {
namespace En {

/******************************************************************************
*
*       function:     vowel_before
*
*       purpose:      Return the position of a vowel prior to 'position'.
*                     If no vowel prior return 0.
*
*       arguments:    start, position
*
*       internal
*       functions:    member
*
*       library
*       functions:    none
*
******************************************************************************/
const char*
vowel_before(const char* start, const char* position)
{
	position--;
	while (position >= start) {
		if (member(*position, "aeiouyAEIOUY"))
			return position;
		position--;
	}
	return nullptr;
}

} /* namespace En */
} /* namespace GS */
