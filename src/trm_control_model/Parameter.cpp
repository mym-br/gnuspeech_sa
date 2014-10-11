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

#include "Parameter.h"

#include <iostream>

#include "Log.h"



namespace GS {
namespace TRMControlModel {
namespace Parameter {

/*******************************************************************************
 * Constructor.
 */
CodeMap::CodeMap() : std::unordered_map<std::string, Code>()
{
	LOG_DEBUG("Initializing parameter code map...");

	(*this)["microInt" ] = PARAMETER_MICRO_INT;
	(*this)["glotVol"  ] = PARAMETER_GLOT_VOL;
	(*this)["aspVol"   ] = PARAMETER_ASP_VOL;
	(*this)["fricVol"  ] = PARAMETER_FRIC_VOL;
	(*this)["fricPos"  ] = PARAMETER_FRIC_POS;
	(*this)["fricCF"   ] = PARAMETER_FRIC_CF;
	(*this)["fricBW"   ] = PARAMETER_FRIC_BW;
	(*this)["r1"       ] = PARAMETER_R1;
	(*this)["r2"       ] = PARAMETER_R2;
	(*this)["r3"       ] = PARAMETER_R3;
	(*this)["r4"       ] = PARAMETER_R4;
	(*this)["r5"       ] = PARAMETER_R5;
	(*this)["r6"       ] = PARAMETER_R6;
	(*this)["r7"       ] = PARAMETER_R7;
	(*this)["r8"       ] = PARAMETER_R8;
	(*this)["velum"    ] = PARAMETER_VELUM;
}

void
setInfo(Info::Array& parameterInfoArray, const CodeMap& parameterCodeMap, const std::string& name,
	float min, float max, float defaultValue)
{
	CodeMap::const_iterator iter = parameterCodeMap.find(name);
	if (iter == parameterCodeMap.end()) {
		THROW_EXCEPTION(TRMControlModelException, "Parameter not found: " << name << '.');
	}

	parameterInfoArray[iter->second].minimum = min;
	parameterInfoArray[iter->second].maximum = max;
	parameterInfoArray[iter->second].defaultValue = defaultValue;
}

} /* namespace Parameter */
} /* namespace TRMControlModel */
} /* namespace GS */
