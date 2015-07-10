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
// 2014-10
// This file was copied from Gnuspeech and modified by Marcelo Y. Matuda.

#ifndef TRM_CONTROL_MODEL_CONFIGURATION_H_
#define TRM_CONTROL_MODEL_CONFIGURATION_H_

#include <string>



namespace GS {
namespace TRMControlModel {

struct Configuration {
	enum Intonation {
		INTONATION_NONE      = 0x00,
		INTONATION_MICRO     = 0x01,
		INTONATION_MACRO     = 0x02,
		INTONATION_SMOOTH    = 0x04,
		INTONATION_DRIFT     = 0x08,
		INTONATION_RANDOMIZE = 0x10
	};

	Configuration();

	void load(const std::string& configFilePath);

	double controlRate;                 /*  1.0-1000.0 input tables/second (Hz)  */
	double tempo;
	double pitchOffset;
	double driftDeviation;
	double driftLowpassCutoff;
	int    intonation;

	// Intonation parameters.
	double notionalPitch;
	double pretonicRange;
	double pretonicLift;
	double tonicRange;
	double tonicMovement;

	std::string voiceName;
	std::string dictionary1File;
	std::string dictionary2File;
	std::string dictionary3File;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_CONFIGURATION_H_ */
