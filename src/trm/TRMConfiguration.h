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

#ifndef TRM_CONFIGURATION_H_
#define TRM_CONFIGURATION_H_

#include "Tube.h"

#include <string>



namespace GS {
namespace TRM {

struct Configuration {
	Configuration();

	void load(const std::string& configFilePath, const std::string& voiceFilePath);

	double outputRate;                   /*  output sample rate (22.05, 44.1)  */

	double volume;                       /*  master volume (0 - 60 dB)  */
	int    channels;                     /*  # of sound output channels (1, 2)  */
	double balance;                      /*  stereo balance (-1 to +1)  */

	int    waveform;                     /*  GS waveform type (0=PULSE, 1=SINE  */

	double vtlOffset;                    /*  tube length offset  */
	double temperature;                  /*  tube temperature (25 - 40 C)  */
	double lossFactor;                   /*  junction loss factor in (0 - 5 %)  */

	double mouthCoef;                    /*  mouth aperture coefficient  */
	double noseCoef;                     /*  nose aperture coefficient  */

	double throatCutoff;                 /*  throat lp cutoff (50 - nyquist Hz)  */
	double throatVol;                    /*  throat volume (0 - 48 dB) */

	int    modulation;                   /*  pulse mod. of noise (0=OFF, 1=ON)  */
	double mixOffset;                    /*  noise crossmix offset (30 - 60 dB)  */

	// Parameters that depend on the voice.
	double glottalPulseTp;               /*  % glottal pulse rise time  */
	double glottalPulseTnMin;            /*  % glottal pulse fall time minimum  */
	double glottalPulseTnMax;            /*  % glottal pulse fall time maximum  */
	double breathiness;                  /*  % glottal source breathiness  */
	double vocalTractLength;
	double referenceGlottalPitch;
	double apertureRadius;               /*  aperture scl. radius (3.05 - 12 cm)  */
	double noseRadius[Tube::TOTAL_NASAL_SECTIONS];   /*  fixed nose radii (0 - 3 cm)  */
	double radiusCoef[Tube::TOTAL_REGIONS];
};

} /* namespace TRM */
} /* namespace GS */

#endif /* TRM_CONFIGURATION_H_ */
