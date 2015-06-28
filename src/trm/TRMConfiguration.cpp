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

#include <sstream>

#include "TRMConfiguration.h"

#include "KeyValueFileReader.h"



namespace GS {
namespace TRM {

Configuration::Configuration()
		: outputRate(0.0)
		, volume(0.0)
		, channels(0)
		, balance(0.0)
		, waveform(0)
		, vtlOffset(0.0)
		, temperature(0.0)
		, lossFactor(0.0)
		, mouthCoef(0.0)
		, noseCoef(0.0)
		, throatCutoff(0.0)
		, throatVol(0.0)
		, modulation(0)
		, mixOffset(0.0)
		, glottalPulseTp(0.0)
		, glottalPulseTnMin(0.0)
		, glottalPulseTnMax(0.0)
		, breathiness(0.0)
		, vocalTractLength(0.0)
		, referenceGlottalPitch(0.0)
		, apertureRadius(0.0)
{
	for (int i = 0; i < Tube::TOTAL_NASAL_SECTIONS; ++i) {
		noseRadius[i] = 0.0;
	}
	for (int i = 0; i < Tube::TOTAL_REGIONS; ++i) {
		radiusCoef[i] = 0.0;
	}
}

void
Configuration::load(const std::string& configFilePath, const std::string& voiceFilePath)
{
	KeyValueFileReader reader(configFilePath);
	KeyValueFileReader voiceReader(voiceFilePath);

	outputRate    = reader.value<double>("output_rate");
	volume        = reader.value<double>("volume");
	channels      = reader.value<int>("channels");
	balance       = reader.value<double>("balance");
	waveform      = reader.value<int>("waveform");

	vtlOffset     = reader.value<double>("vocal_tract_length_offset");
	temperature   = reader.value<double>("temperature");
	lossFactor    = reader.value<double>("loss_factor");
	mouthCoef     = reader.value<double>("mouth_coefficient");
	noseCoef      = reader.value<double>("nose_coefficient");

	throatCutoff  = reader.value<double>("throat_cutoff");
	throatVol     = reader.value<double>("throat_volume");
	modulation    = reader.value<int>("noise_modulation");
	mixOffset     = reader.value<double>("mix_offset");

	const double globalRadiusCoef     = voiceReader.value<double>("global_radius_coef");
	const double globalNoseRadiusCoef = voiceReader.value<double>("global_nose_radius_coef");
	glottalPulseTp        = voiceReader.value<double>("glottal_pulse_tp");
	glottalPulseTnMin     = voiceReader.value<double>("glottal_pulse_tn_min");
	glottalPulseTnMax     = voiceReader.value<double>("glottal_pulse_tn_max");
	breathiness           = voiceReader.value<double>("breathiness");
	vocalTractLength      = voiceReader.value<double>("vocal_tract_length");
	referenceGlottalPitch = voiceReader.value<double>("reference_glottal_pitch");
	apertureRadius        = voiceReader.value<double>("aperture_radius") * globalRadiusCoef;
	noseRadius[0] = 0.0;
	noseRadius[1] = voiceReader.value<double>("nose_radius_1") * globalNoseRadiusCoef;
	noseRadius[2] = voiceReader.value<double>("nose_radius_2") * globalNoseRadiusCoef;
	noseRadius[3] = voiceReader.value<double>("nose_radius_3") * globalNoseRadiusCoef;
	noseRadius[4] = voiceReader.value<double>("nose_radius_4") * globalNoseRadiusCoef;
	noseRadius[5] = voiceReader.value<double>("nose_radius_5") * globalNoseRadiusCoef;
	radiusCoef[0] = voiceReader.value<double>("radius_1_coef") * globalRadiusCoef;
	radiusCoef[1] = voiceReader.value<double>("radius_2_coef") * globalRadiusCoef;
	radiusCoef[2] = voiceReader.value<double>("radius_3_coef") * globalRadiusCoef;
	radiusCoef[3] = voiceReader.value<double>("radius_4_coef") * globalRadiusCoef;
	radiusCoef[4] = voiceReader.value<double>("radius_5_coef") * globalRadiusCoef;
	radiusCoef[5] = voiceReader.value<double>("radius_6_coef") * globalRadiusCoef;
	radiusCoef[6] = voiceReader.value<double>("radius_7_coef") * globalRadiusCoef;
	radiusCoef[7] = voiceReader.value<double>("radius_8_coef") * globalRadiusCoef;
}

} /* namespace TRM */
} /* namespace GS */
