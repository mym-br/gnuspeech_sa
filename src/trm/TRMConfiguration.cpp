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
		, tp(0.0)
		, tnMin(0.0)
		, tnMax(0.0)
		, breathiness(0.0)
		, vtlOffset(0.0)
		, temperature(0.0)
		, lossFactor(0.0)
		, apScale(0.0)
		, mouthCoef(0.0)
		, noseCoef(0.0)
		, throatCutoff(0.0)
		, throatVol(0.0)
		, modulation(0)
		, mixOffset(0.0)
{
	for (int i = 0; i < Tube::TOTAL_NASAL_SECTIONS; ++i) {
		noseRadius[i] = 0.0;
	}
}

void
Configuration::load(const std::string& configFilePath)
{
	KeyValueFileReader reader(configFilePath);

	outputRate    = reader.value<double>("output_rate");
	volume        = reader.value<double>("volume");
	channels      = reader.value<int>("channels");
	balance       = reader.value<double>("balance");
	waveform      = reader.value<int>("waveform");
	tp            = reader.value<double>("tp");
	tnMin         = reader.value<double>("tn_min");
	tnMax         = reader.value<double>("tn_max");
	breathiness   = reader.value<double>("breathiness");
	vtlOffset     = reader.value<double>("vocal_tract_length_offset");
	temperature   = reader.value<double>("temperature");
	lossFactor    = reader.value<double>("loss_factor");
	apScale       = reader.value<double>("aperture_scale");
	mouthCoef     = reader.value<double>("mouth_coefficient");
	noseCoef      = reader.value<double>("nose_coefficient");

	noseRadius[0] = 0.0;
	noseRadius[1] = reader.value<double>("nose_radius_1");
	noseRadius[2] = reader.value<double>("nose_radius_2");
	noseRadius[3] = reader.value<double>("nose_radius_3");
	noseRadius[4] = reader.value<double>("nose_radius_4");
	noseRadius[5] = reader.value<double>("nose_radius_5");

	throatCutoff  = reader.value<double>("throat_cutoff");
	throatVol     = reader.value<double>("throat_volume");
	modulation    = reader.value<int>("noise_modulation");
	mixOffset     = reader.value<double>("mix_offset");
}

} /* namespace TRM */
} /* namespace GS */
