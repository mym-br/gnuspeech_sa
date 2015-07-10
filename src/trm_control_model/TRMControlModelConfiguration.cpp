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

#include "TRMControlModelConfiguration.h"

#include "KeyValueFileReader.h"



namespace GS {
namespace TRMControlModel {

Configuration::Configuration()
		: controlRate(0.0)
		, tempo(0.0)
		, pitchOffset(0.0)
		, driftDeviation(0.0)
		, driftLowpassCutoff(0.0)
		, intonation(0)
		, notionalPitch(0.0)
		, pretonicRange(0.0)
		, pretonicLift(0.0)
		, tonicRange(0.0)
		, tonicMovement(0.0)
{
}

void
Configuration::load(const std::string& configFilePath)
{
	KeyValueFileReader reader(configFilePath);

	controlRate        = reader.value<double>("control_rate");
	tempo              = reader.value<double>("tempo");
	pitchOffset        = reader.value<double>("pitch_offset");
	driftDeviation     = reader.value<double>("drift_deviation");
	driftLowpassCutoff = reader.value<double>("drift_lowpass_cutoff");

	intonation = 0;
	if (reader.value<int>("micro_intonation") != 0) {
		intonation += INTONATION_MICRO;
	}
	if (reader.value<int>("macro_intonation") != 0) {
		intonation += INTONATION_MACRO;
	}
//	if (reader.value<int>("smooth_intonation") != 0) {
//		intonation += INTONATION_SMOOTH;
//	}
	if (reader.value<int>("intonation_drift") != 0) {
		intonation += INTONATION_DRIFT;
	}
	if (reader.value<int>("random_intonation") != 0) {
		intonation += INTONATION_RANDOMIZE;
	}

	notionalPitch = reader.value<double>("notional_pitch");
	pretonicRange = reader.value<double>("pretonic_range");
	pretonicLift  = reader.value<double>("pretonic_lift");
	tonicRange    = reader.value<double>("tonic_range");
	tonicMovement = reader.value<double>("tonic_movement");

	voiceName = reader.value<std::string>("voice_name");
	dictionary1File = reader.value<std::string>("dictionary_1_file");
	dictionary2File = reader.value<std::string>("dictionary_2_file");
	dictionary3File = reader.value<std::string>("dictionary_3_file");
}

} /* namespace TRMControlModel */
} /* namespace GS */
