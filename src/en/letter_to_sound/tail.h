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

#ifndef EN_TAIL_H_
#define EN_TAIL_H_



namespace {

/*  DATA TYPES  **************************************************************/
typedef struct {
	const char* tail;
	const char* type;
} tail_entry;

/*  GLOBAL VARIABLES (LOCAL TO THIS FILE)  ***********************************/
const tail_entry tail_list[] = {
	{"ly", "d"},
	{"er", "ca"},
	{"ish", "c"},
	{"ing", "cb"},
	{"se", "b"},
	{"ic", "c"},
	{"ify", "b"},
	{"ment", "a"},
	{"al", "c"},
	{"ed", "bc"},
	{"es", "ab"},
	{"ant", "ca"},
	{"ent", "ca"},
	{"ist", "a"},
	{"ism", "a"},
	{"gy", "a"},
	{"ness", "a"},
	{"ous", "c"},
	{"less", "c"},
	{"ful", "c"},
	{"ion", "a"},
	{"able", "c"},
	{"en", "c"},
	{"ry", "ac"},
	{"ey", "c"},
	{"or", "a"},
	{"y", "c"},
	{"us", "a"},
	{"s", "ab"},
	{nullptr, nullptr}
};

} /* namespace */

#endif /* EN_TAIL_H_ */
