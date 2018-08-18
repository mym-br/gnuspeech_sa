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
// 2014-09
// This file was copied from Gnuspeech and modified by Marcelo Y. Matuda.

#ifndef TRM_TUBE_H_
#define TRM_TUBE_H_

#include <algorithm> /* max, min */
#include <istream>
#include <memory>
#include <vector>

#include "BandpassFilter.h"
#include "NoiseFilter.h"
#include "NoiseSource.h"
#include "RadiationFilter.h"
#include "ReflectionFilter.h"
#include "SampleRateConverter.h"
#include "Throat.h"
#include "VocalTractModelParameterValue.h"
#include "WavetableGlottalSource.h"

#define GS_TRM_TUBE_MIN_RADIUS (0.001)



namespace GS {
namespace TRM {

class Tube {
public:
	enum { /*  OROPHARYNX REGIONS  */
		R1 = 0, /*  S1  */
		R2 = 1, /*  S2  */
		R3 = 2, /*  S3  */
		R4 = 3, /*  S4 & S5  */
		R5 = 4, /*  S6 & S7  */
		R6 = 5, /*  S8  */
		R7 = 6, /*  S9  */
		R8 = 7, /*  S10  */
		TOTAL_REGIONS = 8
	};
	enum { /*  NASAL TRACT SECTIONS  */
		N1 = 0,
		N2 = 1,
		N3 = 2,
		N4 = 3,
		N5 = 4,
		N6 = 5,
		TOTAL_NASAL_SECTIONS = 6
	};

	Tube();
	~Tube();

	void synthesizeToFile(std::istream& inputStream, const char* outputFile);
private:
	enum {
		VELUM = N1
	};
	enum { /*  OROPHARYNX SCATTERING JUNCTION COEFFICIENTS (BETWEEN EACH REGION)  */
		C1 = R1, /*  R1-R2 (S1-S2)  */
		C2 = R2, /*  R2-R3 (S2-S3)  */
		C3 = R3, /*  R3-R4 (S3-S4)  */
		C4 = R4, /*  R4-R5 (S5-S6)  */
		C5 = R5, /*  R5-R6 (S7-S8)  */
		C6 = R6, /*  R6-R7 (S8-S9)  */
		C7 = R7, /*  R7-R8 (S9-S10)  */
		C8 = R8, /*  R8-AIR (S10-AIR)  */
		TOTAL_COEFFICIENTS = TOTAL_REGIONS
	};
	enum { /*  OROPHARYNX SECTIONS  */
		S1  = 0, /*  R1  */
		S2  = 1, /*  R2  */
		S3  = 2, /*  R3  */
		S4  = 3, /*  R4  */
		S5  = 4, /*  R4  */
		S6  = 5, /*  R5  */
		S7  = 6, /*  R5  */
		S8  = 7, /*  R6  */
		S9  = 8, /*  R7  */
		S10 = 9, /*  R8  */
		TOTAL_SECTIONS = 10
	};
	enum { /*  NASAL TRACT COEFFICIENTS  */
		NC1 = N1, /*  N1-N2  */
		NC2 = N2, /*  N2-N3  */
		NC3 = N3, /*  N3-N4  */
		NC4 = N4, /*  N4-N5  */
		NC5 = N5, /*  N5-N6  */
		NC6 = N6, /*  N6-AIR  */
		TOTAL_NASAL_COEFFICIENTS = TOTAL_NASAL_SECTIONS
	};
	enum { /*  THREE-WAY JUNCTION ALPHA COEFFICIENTS  */
		LEFT  = 0,
		RIGHT = 1,
		UPPER = 2,
		TOTAL_ALPHA_COEFFICIENTS = 3
	};
	enum { /*  FRICATION INJECTION COEFFICIENTS  */
		FC1 = 0, /*  S3  */
		FC2 = 1, /*  S4  */
		FC3 = 2, /*  S5  */
		FC4 = 3, /*  S6  */
		FC5 = 4, /*  S7  */
		FC6 = 5, /*  S8  */
		FC7 = 6, /*  S9  */
		FC8 = 7, /*  S10  */
		TOTAL_FRIC_COEFFICIENTS = 8
	};

	struct InputData {
		double glotPitch;
		double glotVol;
		double aspVol;
		double fricVol;
		double fricPos;
		double fricCF;
		double fricBW;
		double radius[TOTAL_REGIONS];
		double velum;
	};

	/*  VARIABLES FOR INTERPOLATION  */
	struct CurrentData {
		double glotPitch;
		double glotPitchDelta;
		double glotVol;
		double glotVolDelta;
		double aspVol;
		double aspVolDelta;
		double fricVol;
		double fricVolDelta;
		double fricPos;
		double fricPosDelta;
		double fricCF;
		double fricCFDelta;
		double fricBW;
		double fricBWDelta;
		double radius[TOTAL_REGIONS];
		double radiusDelta[TOTAL_REGIONS];
		double velum;
		double velumDelta;
	};

	Tube(const Tube&) = delete;
	Tube& operator=(const Tube&) = delete;

	void initializeSynthesizer();
	void synthesizeForInputSequence();
	void reset();
	void calculateTubeCoefficients();
	void initializeNasalCavity();
	void printInfo(const char* inputFile);
	void parseInputStream(std::istream& in);
	void sampleRateInterpolation();
	void setControlRateParameters(int pos);
	void setFricationTaps();
	double vocalTract(double input, double frication);
	void writeOutputToFile(const char* outputFile);
	void synthesize();
	float calculateMonoScale();
	void calculateStereoScale(float& leftScale, float& rightScale);

	static double amplitude(double decibelLevel);
	static double frequency(double pitch);
	static double speedOfSound(double temperature);

	float  outputRate_;                  /*  output sample rate (22.05, 44.1)  */
	float  controlRate_;                 /*  1.0-1000.0 input tables/second (Hz)  */

	double volume_;                      /*  master volume (0 - 60 dB)  */
	int    channels_;                    /*  # of sound output channels (1, 2)  */
	double balance_;                     /*  stereo balance (-1 to +1)  */

	int    waveform_;                    /*  GS waveform type (0=PULSE, 1=SINE  */
	double tp_;                          /*  % glottal pulse rise time  */
	double tnMin_;                       /*  % glottal pulse fall time minimum  */
	double tnMax_;                       /*  % glottal pulse fall time maximum  */
	double breathiness_;                 /*  % glottal source breathiness  */

	double length_;                      /*  nominal tube length (10 - 20 cm)  */
	double temperature_;                 /*  tube temperature (25 - 40 C)  */
	double lossFactor_;                  /*  junction loss factor in (0 - 5 %)  */

	double apertureRadius_;              /*  aperture scl. radius (3.05 - 12 cm)  */
	double mouthCoef_;                   /*  mouth aperture coefficient  */
	double noseCoef_;                    /*  nose aperture coefficient  */

	double noseRadius_[TOTAL_NASAL_SECTIONS]; /*  fixed nose radii (0 - 3 cm)  */

	double throatCutoff_;                /*  throat lp cutoff (50 - nyquist Hz)  */
	double throatVol_;                   /*  throat volume (0 - 48 dB) */

	int    modulation_;                  /*  pulse mod. of noise (0=OFF, 1=ON)  */
	double mixOffset_;                   /*  noise crossmix offset (30 - 60 dB)  */

	/*  DERIVED VALUES  */
	int    controlPeriod_;
	int    sampleRate_;
	double actualTubeLength_;            /*  actual length in cm  */

	/*  MEMORY FOR TUBE AND TUBE COEFFICIENTS  */
	double oropharynx_[TOTAL_SECTIONS][2][2];
	double oropharynxCoeff_[TOTAL_COEFFICIENTS];

	double nasal_[TOTAL_NASAL_SECTIONS][2][2];
	double nasalCoeff_[TOTAL_NASAL_COEFFICIENTS];

	double alpha_[TOTAL_ALPHA_COEFFICIENTS];
	int currentPtr_;
	int prevPtr_;

	/*  MEMORY FOR FRICATION TAPS  */
	double fricationTap_[TOTAL_FRIC_COEFFICIENTS];

	double dampingFactor_;               /*  calculated damping factor  */
	double crossmixFactor_;              /*  calculated crossmix factor  */
	double breathinessFactor_;

	double prevGlotAmplitude_;

	std::vector<std::unique_ptr<InputData>> inputData_;
	CurrentData currentData_;
	std::size_t outputDataPos_;
	std::vector<float> outputData_;
	std::unique_ptr<SampleRateConverter> srConv_;
	std::unique_ptr<RadiationFilter> mouthRadiationFilter_;
	std::unique_ptr<ReflectionFilter> mouthReflectionFilter_;
	std::unique_ptr<RadiationFilter> nasalRadiationFilter_;
	std::unique_ptr<ReflectionFilter> nasalReflectionFilter_;
	std::unique_ptr<Throat> throat_;
	std::unique_ptr<WavetableGlottalSource> glottalSource_;
	std::unique_ptr<BandpassFilter> bandpassFilter_;
	std::unique_ptr<NoiseFilter> noiseFilter_;
	std::unique_ptr<NoiseSource> noiseSource_;
};

} /* namespace TRM */
} /* namespace GS */

#endif /* TRM_TUBE_H_ */
