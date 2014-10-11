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

#ifndef TRM_CONTROL_MODEL_PARAMETER_H_
#define TRM_CONTROL_MODEL_PARAMETER_H_

#include <string>
#include <array>
#include <unordered_map>

#include "Exception.h"



namespace GS {
namespace TRMControlModel {
namespace Parameter {

enum Code {
	PARAMETER_MICRO_INT,
	PARAMETER_GLOT_VOL,
	PARAMETER_ASP_VOL,
	PARAMETER_FRIC_VOL,
	PARAMETER_FRIC_POS,
	PARAMETER_FRIC_CF,
	PARAMETER_FRIC_BW,
	PARAMETER_R1,
	PARAMETER_R2,
	PARAMETER_R3,
	PARAMETER_R4,
	PARAMETER_R5,
	PARAMETER_R6,
	PARAMETER_R7,
	PARAMETER_R8,
	PARAMETER_VELUM,
	NUM_PARAMETERS
};

struct CodeMap : public std::unordered_map<std::string, Code> {
	CodeMap();
};

struct Info {
	// Note: For volume parameters, the values are in dB.
	float minimum;
	float maximum;
	float defaultValue;

	typedef std::array<Info, NUM_PARAMETERS> Array;
};

//inline
//bool
//isVolumeParameter(Code parameter)
//{
//	switch (parameter) {
//	case PARAMETER_GLOT_VOL:
//	case PARAMETER_ASP_VOL:
//	case PARAMETER_FRIC_VOL:
//		return true;
//	default:
//		return false;
//	}
//}

void setInfo(Info::Array& parameterInfoArray, const CodeMap& parameterCodeMap, const std::string& name,
		float min, float max, float defaultValue);

template<typename T>
void
setValue(const CodeMap& parameterCodeMap, T& t, const std::string& name, float value) {

	CodeMap::const_iterator iter = parameterCodeMap.find(name);
	if (iter == parameterCodeMap.end()) {
		THROW_EXCEPTION(TRMControlModelException, "Parameter not found: " << name << '.');
	}
	t[iter->second] = value;
}

template<typename T>
void
setValue(const CodeMap& parameterCodeMap, T& t, const std::string& name, const std::string& value) {

	CodeMap::const_iterator iter = parameterCodeMap.find(name);
	if (iter == parameterCodeMap.end()) {
		THROW_EXCEPTION(TRMControlModelException, "Parameter not found: " << name << '.');
	}
	t[iter->second] = value;
}

} /* namespace Parameter */
} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_PARAMETER_H_ */
