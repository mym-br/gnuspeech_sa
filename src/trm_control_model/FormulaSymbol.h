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

#ifndef TRM_CONTROL_MODEL_FORMULA_SYMBOL_H_
#define TRM_CONTROL_MODEL_FORMULA_SYMBOL_H_

#include <string>
#include <array>
#include <unordered_map>



namespace GS {
namespace TRMControlModel {

struct FormulaSymbol {
	enum Code {
		SYMB_TRANSITION1,
		SYMB_TRANSITION2,
		SYMB_TRANSITION3,
		SYMB_TRANSITION4,
		SYMB_QSSA1,
		SYMB_QSSA2,
		SYMB_QSSA3,
		SYMB_QSSA4,
		SYMB_QSSB1,
		SYMB_QSSB2,
		SYMB_QSSB3,
		SYMB_QSSB4,
		SYMB_TEMPO1,
		SYMB_TEMPO2,
		SYMB_TEMPO3,
		SYMB_TEMPO4,
		SYMB_RD, // rule duration
		SYMB_BEAT,
		SYMB_MARK1,
		SYMB_MARK2,
		SYMB_MARK3,
		//SYMB_NULL,
		NUM_SYMBOLS
	};

	typedef std::unordered_map<std::string, Code> CodeMap;
	CodeMap codeMap; // symbol name -> symbol code

	FormulaSymbol();
};

typedef std::array<float, FormulaSymbol::NUM_SYMBOLS> FormulaSymbolList; // [symbol code] -> value

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_FORMULA_SYMBOL_H_ */
