/***************************************************************************
 *  Copyright 2014 Marcelo Y. Matuda                                       *
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
// This file was created by Marcelo Y. Matuda, and code/information
// from Gnuspeech was added to it later.

#include "FormulaSymbol.h"

#include "Log.h"



namespace GS {
namespace TRMControlModel {

/*******************************************************************************
 * Constructor.
 */
FormulaSymbol::FormulaSymbol() : codeMap() {
	LOG_DEBUG("Initializing formula symbol map...");

	codeMap["transition1"] = SYMB_TRANSITION1;
	codeMap["transition2"] = SYMB_TRANSITION2;
	codeMap["transition3"] = SYMB_TRANSITION3;
	codeMap["transition4"] = SYMB_TRANSITION4;
	codeMap["qssa1"]       = SYMB_QSSA1;
	codeMap["qssa2"]       = SYMB_QSSA2;
	codeMap["qssa3"]       = SYMB_QSSA3;
	codeMap["qssa4"]       = SYMB_QSSA4;
	codeMap["qssb1"]       = SYMB_QSSB1;
	codeMap["qssb2"]       = SYMB_QSSB2;
	codeMap["qssb3"]       = SYMB_QSSB3;
	codeMap["qssb4"]       = SYMB_QSSB4;
	codeMap["tempo1"]      = SYMB_TEMPO1;
	codeMap["tempo2"]      = SYMB_TEMPO2;
	codeMap["tempo3"]      = SYMB_TEMPO3;
	codeMap["tempo4"]      = SYMB_TEMPO4;
	codeMap["rd"]          = SYMB_RD;
	codeMap["beat"]        = SYMB_BEAT;
	codeMap["mark1"]       = SYMB_MARK1;
	codeMap["mark2"]       = SYMB_MARK2;
	codeMap["mark3"]       = SYMB_MARK3;
	//codeMap["null"]        = SYMB_NULL;
}

} /* namespace TRMControlModel */
} /* namespace GS */
