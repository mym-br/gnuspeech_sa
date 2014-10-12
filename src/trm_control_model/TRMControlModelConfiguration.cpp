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
		: intonation(0)
		, voiceType(0)
		, speed(0.0)
		, pitchOffset(0.0)
{
}

void
Configuration::load(const std::string& configFilePath)
{
	KeyValueFileReader reader(configFilePath);

	intonation = 0;
	if (reader.value<int>("micro_intonation") != 0) {
		intonation += INTONATION_MICRO;
	}
	if (reader.value<int>("macro_intonation") != 0) {
		intonation += INTONATION_MACRO;
	}
	if (reader.value<int>("declin_intonation") != 0) {
		intonation += INTONATION_DECLIN;
	}
	if (reader.value<int>("creak_intonation") != 0) {
		intonation += INTONATION_CREAK;
	}
	if (reader.value<int>("random_intonation") != 0) {
		intonation += INTONATION_RANDOMIZE;
	}

	voiceType = reader.value<int>("voice_type");
	speed = reader.value<double>("speed");
	pitchOffset = reader.value<double>("pitch_offset");
}

} /* namespace TRMControlModel */
} /* namespace GS */
