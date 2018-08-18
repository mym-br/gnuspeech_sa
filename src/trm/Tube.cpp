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

/******************************************************************************
*
*     Program:       tube
*
*     Description:   Software (non-real-time) implementation of the Tube
*                    Resonance Model for speech production.
*
*     Author:        Leonard Manzara
*
*     Date:          July 5th, 1994
*
******************************************************************************/

#include "Tube.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <utility> /* move */

#include "Exception.h"
#include "Log.h"
#include "Text.h"
#include "WAVEFileWriter.h"

/*  COMPILE SO THAT INTERPOLATION NOT DONE FOR SOME CONTROL RATE PARAMETERS  */
//#define MATCH_DSP                 1

#define INPUT_VECTOR_RESERVE 128
#define OUTPUT_VECTOR_RESERVE 1024

#define GLOTTAL_SOURCE_PULSE 0
#define GLOTTAL_SOURCE_SINE 1

/*  PITCH VARIABLES  */
#define PITCH_BASE                220.0
#define PITCH_OFFSET              3           /*  MIDDLE C = 0  */

/*  RANGE OF ALL VOLUME CONTROLS  */
#define VOL_MAX                   60

/*  SCALING CONSTANT FOR INPUT TO VOCAL TRACT & THROAT (MATCHES DSP)  */
//#define VT_SCALE                  0.03125     /*  2^(-5)  */
// this is a temporary fix only, to try to match dsp synthesizer
#define VT_SCALE                  0.125     /*  2^(-3)  */

/*  FINAL OUTPUT SCALING, SO THAT .SND FILES APPROX. MATCH DSP OUTPUT  */
#define OUTPUT_SCALE              0.95

/*  BI-DIRECTIONAL TRANSMISSION LINE POINTERS  */
#define TOP                       0
#define BOTTOM                    1



namespace GS {
namespace TRM {

Tube::Tube()
{
	reset();

	inputData_.reserve(INPUT_VECTOR_RESERVE);
	outputData_.reserve(OUTPUT_VECTOR_RESERVE);
}

Tube::~Tube()
{
}

void
Tube::reset()
{
	outputRate_  = 0.0;
	controlRate_ = 0.0;
	volume_      = 0.0;
	channels_    = 0;
	balance_     = 0.0;
	waveform_    = 0;
	tp_          = 0.0;
	tnMin_       = 0.0;
	tnMax_       = 0.0;
	breathiness_ = 0.0;
	length_      = 0.0;
	temperature_ = 0.0;
	lossFactor_  = 0.0;
	apertureRadius_ = 0.0;
	mouthCoef_   = 0.0;
	noseCoef_    = 0.0;
	memset(noseRadius_, 0, sizeof(double) * TOTAL_NASAL_SECTIONS);
	throatCutoff_     = 0.0;
	throatVol_        = 0.0;
	modulation_       = 0;
	mixOffset_        = 0.0;
	controlPeriod_    = 0;
	sampleRate_       = 0;
	actualTubeLength_ = 0.0;
	memset(&oropharynx_[0][0][0], 0, sizeof(double) * TOTAL_SECTIONS * 2 * 2);
	memset(oropharynxCoeff_,      0, sizeof(double) * TOTAL_COEFFICIENTS);
	memset(&nasal_[0][0][0],      0, sizeof(double) * TOTAL_NASAL_SECTIONS * 2 * 2);
	memset(nasalCoeff_,           0, sizeof(double) * TOTAL_NASAL_COEFFICIENTS);
	memset(alpha_,                0, sizeof(double) * TOTAL_ALPHA_COEFFICIENTS);
	currentPtr_ = 1;
	prevPtr_    = 0;
	memset(fricationTap_, 0, sizeof(double) * TOTAL_FRIC_COEFFICIENTS);
	dampingFactor_     = 0.0;
	crossmixFactor_    = 0.0;
	breathinessFactor_ = 0.0;
	prevGlotAmplitude_ = -1.0;
	inputData_.resize(0);
	memset(&currentData_, 0, sizeof(CurrentData));
	outputDataPos_ = 0;
	outputData_.resize(0);

	if (srConv_) srConv_->reset();
	if (mouthRadiationFilter_) mouthRadiationFilter_->reset();
	if (mouthReflectionFilter_) mouthReflectionFilter_->reset();
	if (nasalRadiationFilter_) nasalRadiationFilter_->reset();
	if (nasalReflectionFilter_) nasalReflectionFilter_->reset();
	if (throat_) throat_->reset();
	if (glottalSource_) glottalSource_->reset();
	if (bandpassFilter_) bandpassFilter_->reset();
	if (noiseFilter_) noiseFilter_->reset();
	if (noiseSource_) noiseSource_->reset();
}

void
Tube::synthesizeToFile(std::istream& inputStream, const char* outputFile)
{
	if (!outputData_.empty()) {
		reset();
	}
	parseInputStream(inputStream);
	initializeSynthesizer();
#if 0
	if (Log::debugEnabled) {
		printInfo(inputFile);
	}
#endif
	synthesizeForInputSequence();
	writeOutputToFile(outputFile);
}

/******************************************************************************
*
*  function:  printInfo
*
*  purpose:   Prints pertinent variables to standard output.
*
******************************************************************************/
void
Tube::printInfo(const char* inputFile)
{
	/*  PRINT INPUT FILE NAME  */
	printf("input file:\t\t%s\n\n", inputFile);

	/*  ECHO INPUT PARAMETERS  */
	printf("outputRate:\t\t%.1f Hz\n", outputRate_);
	printf("controlRate:\t\t%.2f Hz\n\n", controlRate_);

	printf("volume:\t\t\t%.2f dB\n", volume_);
	printf("channels:\t\t%-d\n", channels_);
	printf("balance:\t\t%+1.2f\n\n", balance_);

	printf("waveform:\t\t");
	if (waveform_ == GLOTTAL_SOURCE_PULSE) {
		printf("pulse\n");
	} else if (waveform_ == GLOTTAL_SOURCE_SINE) {
		printf("sine\n");
	}
	printf("tp:\t\t\t%.2f%%\n", tp_);
	printf("tnMin:\t\t\t%.2f%%\n", tnMin_);
	printf("tnMax:\t\t\t%.2f%%\n", tnMax_);
	printf("breathiness:\t\t%.2f%%\n\n", breathiness_);

	printf("nominal tube length:\t%.2f cm\n", length_);
	printf("temperature:\t\t%.2f degrees C\n", temperature_);
	printf("lossFactor:\t\t%.2f%%\n\n", lossFactor_);

	printf("apertureRadius:\t\t%.2f cm\n", apertureRadius_);
	printf("mouthCoef:\t\t%.1f Hz\n", mouthCoef_);
	printf("noseCoef:\t\t%.1f Hz\n\n", noseCoef_);

	for (int i = 1; i < TOTAL_NASAL_SECTIONS; i++) {
		printf("n%-d:\t\t\t%.2f cm\n", i, noseRadius_[i]);
	}

	printf("\nthroatCutoff:\t\t%.1f Hz\n", throatCutoff_);
	printf("throatVol:\t\t%.2f dB\n\n", throatVol_);

	printf("modulation:\t\t");
	if (modulation_) {
		printf("on\n");
	} else {
		printf("off\n");
	}
	printf("mixOffset:\t\t%.2f dB\n\n", mixOffset_);

	/*  PRINT OUT DERIVED VALUES  */
	printf("\nactual tube length:\t%.4f cm\n", actualTubeLength_);
	printf("internal sample rate:\t%-d Hz\n", sampleRate_);
	printf("control period:\t\t%-d samples (%.4f seconds)\n\n",
		controlPeriod_, (float) controlPeriod_ / (float) sampleRate_);

#if 0
	/*  PRINT OUT WAVE TABLE VALUES  */
	printf("\n");
	for (int i = 0; i < TABLE_LENGTH; i++)
		printf("table[%-d] = %.4f\n", i, wavetable[i]);
#endif

	/*  ECHO TABLE VALUES  */
	printf("\n%-ld control rate input tables:\n\n", inputData_.size() - 1);

	/*  HEADER  */
	printf("glPitch");
	printf("\tglotVol");
	printf("\taspVol");
	printf("\tfricVol");
	printf("\tfricPos");
	printf("\tfricCF");
	printf("\tfricBW");
	for (int i = 1; i <= TOTAL_REGIONS; i++) {
		printf("\tr%-d", i);
	}
	printf("\tvelum\n");

	/*  ACTUAL VALUES  */
	for (int i = 0; i < static_cast<int>(inputData_.size()) - 1; ++i) {
		printf("%.2f"  , inputData_[i]->glotPitch);
		printf("\t%.2f", inputData_[i]->glotVol);
		printf("\t%.2f", inputData_[i]->aspVol);
		printf("\t%.2f", inputData_[i]->fricVol);
		printf("\t%.2f", inputData_[i]->fricPos);
		printf("\t%.2f", inputData_[i]->fricCF);
		printf("\t%.2f", inputData_[i]->fricBW);
		for (int j = 0; j < TOTAL_REGIONS; ++j) {
			printf("\t%.2f", inputData_[i]->radius[j]);
		}
		printf("\t%.2f\n", inputData_[i]->velum);
	}
	printf("\n");
}

/******************************************************************************
*
*  function:  parseInputStream
*
*  purpose:   Parses the input stream and assigns values to global
*             variables.
*
******************************************************************************/
void
Tube::parseInputStream(std::istream& in)
{
	std::string line;

	/*  GET THE OUTPUT SAMPLE RATE  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read output sample rate.");
	} else {
		outputRate_ = Text::parseString<float>(line);
	}

	/*  GET THE INPUT CONTROL RATE  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read input control rate.");
	} else {
		controlRate_ = Text::parseString<float>(line);
	}

	/*  GET THE MASTER VOLUME  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read master volume.");
	} else {
		volume_ = Text::parseString<double>(line);
	}

	/*  GET THE NUMBER OF SOUND OUTPUT CHANNELS  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read number of sound output channels.");
	} else {
		channels_ = Text::parseString<int>(line);
	}

	/*  GET THE STEREO BALANCE  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read stereo balance.");
	} else {
		balance_ = Text::parseString<double>(line);
	}

	/*  GET THE GLOTTAL SOURCE WAVEFORM TYPE  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read glottal source waveform type.");
	} else {
		waveform_ = Text::parseString<int>(line);
	}

	/*  GET THE GLOTTAL PULSE RISE TIME (tp)  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read glottal pulse rise time (tp).");
	} else {
		tp_ = Text::parseString<double>(line);
	}

	/*  GET THE GLOTTAL PULSE FALL TIME MINIMUM (tnMin)  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read glottal pulse fall time minimum (tnMin).");
	} else {
		tnMin_ = Text::parseString<double>(line);
	}

	/*  GET THE GLOTTAL PULSE FALL TIME MAXIMUM (tnMax)  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read glottal pulse fall time maximum (tnMax).");
	} else {
		tnMax_ = Text::parseString<double>(line);
	}

	/*  GET THE GLOTTAL SOURCE BREATHINESS  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read glottal source breathiness.");
	} else {
		breathiness_ = Text::parseString<double>(line);
	}

	/*  GET THE NOMINAL TUBE LENGTH  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read nominal tube length.");
	} else {
		length_ = Text::parseString<double>(line);
	}

	/*  GET THE TUBE TEMPERATURE  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read tube temperature.");
	} else {
		temperature_ = Text::parseString<double>(line);
	}

	/*  GET THE JUNCTION LOSS FACTOR  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read junction loss factor.");
	} else {
		lossFactor_ = Text::parseString<double>(line);
	}

	/*  GET THE APERTURE SCALING RADIUS  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read aperture scaling radius.");
	} else {
		apertureRadius_ = Text::parseString<double>(line);
	}

	/*  GET THE MOUTH APERTURE COEFFICIENT  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read mouth aperture coefficient.");
	} else {
		mouthCoef_ = Text::parseString<double>(line);
	}

	/*  GET THE NOSE APERTURE COEFFICIENT  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read nose aperture coefficient.");
	} else {
		noseCoef_ = Text::parseString<double>(line);
	}

	/*  GET THE NOSE RADII  */
	noseRadius_[0] = 0.0;
	for (int i = 1; i < TOTAL_NASAL_SECTIONS; i++) {
		if (!std::getline(in, line)) {
			THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read nose radius " << i << '.');
		} else {
			noseRadius_[i] = std::max(Text::parseString<double>(line), GS_TRM_TUBE_MIN_RADIUS);
		}
	}

	/*  GET THE THROAT LOWPASS FREQUENCY CUTOFF  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read throat lowpass filter cutoff.");
	} else {
		throatCutoff_ = Text::parseString<double>(line);
	}

	/*  GET THE THROAT VOLUME  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read throat volume.");
	} else {
		throatVol_ = Text::parseString<double>(line);
	}

	/*  GET THE PULSE MODULATION OF NOISE FLAG  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read pulse modulation of noise flag.");
	} else {
		modulation_ = Text::parseString<int>(line);
	}

	/*  GET THE NOISE CROSSMIX OFFSET  */
	if (!std::getline(in, line)) {
		THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read noise crossmix offset.");
	} else {
		mixOffset_ = Text::parseString<double>(line);
	}

	/*  GET THE INPUT TABLE VALUES  */
	unsigned int paramNumber = 0;
	while (std::getline(in, line)) {
		std::istringstream lineStream(line);
		std::unique_ptr<InputData> data(new InputData());

		/*  GET EACH PARAMETER  */
		lineStream >>
			data->glotPitch >>
			data->glotVol >>
			data->aspVol >>
			data->fricVol >>
			data->fricPos >>
			data->fricCF >>
			data->fricBW;
		for (int i = 0; i < TOTAL_REGIONS; i++) {
			double radius;
			lineStream >> radius;
			data->radius[i] = std::max(radius, GS_TRM_TUBE_MIN_RADIUS);
		}
		lineStream >> data->velum;

		if (!lineStream) {
			THROW_EXCEPTION(TRMException, "Error in tube input parsing: Could not read parameters (number " << paramNumber << ").");
		}

		inputData_.push_back(std::move(data));
		++paramNumber;
	}

	/*  DOUBLE UP THE LAST INPUT TABLE, TO HELP INTERPOLATION CALCULATIONS  */
	if (!inputData_.empty()) {
		std::unique_ptr<InputData> lastData(new InputData());
		*lastData = *inputData_.back();
		inputData_.push_back(std::move(lastData));
	}
}

/******************************************************************************
*
*  function:  speedOfSound
*
*  purpose:   Returns the speed of sound according to the value of
*             the temperature (in Celsius degrees).
*
******************************************************************************/
double
Tube::speedOfSound(double temperature)
{
	return 331.4 + (0.6 * temperature);
}

/******************************************************************************
*
*  function:  initializeSynthesizer
*
*  purpose:   Initializes all variables so that the synthesis can
*             be run.
*
******************************************************************************/
void
Tube::initializeSynthesizer()
{
	double nyquist;

	/*  CALCULATE THE SAMPLE RATE, BASED ON NOMINAL TUBE LENGTH AND SPEED OF SOUND  */
	if (length_ > 0.0) {
		double c = speedOfSound(temperature_);
		controlPeriod_ = static_cast<int>(rint((c * TOTAL_SECTIONS * 100.0) / (length_ * controlRate_)));
		sampleRate_ = static_cast<int>(controlRate_ * controlPeriod_);
		actualTubeLength_ = (c * TOTAL_SECTIONS * 100.0) / sampleRate_;
		nyquist = sampleRate_ / 2.0;
	} else {
		THROW_EXCEPTION(TRMException, "Illegal tube length.\n");
	}

	/*  CALCULATE THE BREATHINESS FACTOR  */
	breathinessFactor_ = breathiness_ / 100.0;

	/*  CALCULATE CROSSMIX FACTOR  */
	crossmixFactor_ = 1.0 / amplitude(mixOffset_);

	/*  CALCULATE THE DAMPING FACTOR  */
	dampingFactor_ = (1.0 - (lossFactor_ / 100.0));

	/*  INITIALIZE THE WAVE TABLE  */
	glottalSource_.reset(new WavetableGlottalSource(
				waveform_ == GLOTTAL_SOURCE_PULSE ?
					WavetableGlottalSource::TYPE_PULSE :
					WavetableGlottalSource::TYPE_SINE,
				sampleRate_,
				tp_, tnMin_, tnMax_));

	/*  INITIALIZE REFLECTION AND RADIATION FILTER COEFFICIENTS FOR MOUTH  */
	double mouthApertureCoeff = (nyquist - mouthCoef_) / nyquist;
	mouthRadiationFilter_.reset(new RadiationFilter(mouthApertureCoeff));
	mouthReflectionFilter_.reset(new ReflectionFilter(mouthApertureCoeff));

	/*  INITIALIZE REFLECTION AND RADIATION FILTER COEFFICIENTS FOR NOSE  */
	double nasalApertureCoeff = (nyquist - noseCoef_) / nyquist;
	nasalRadiationFilter_.reset(new RadiationFilter(nasalApertureCoeff));
	nasalReflectionFilter_.reset(new ReflectionFilter(nasalApertureCoeff));

	/*  INITIALIZE NASAL CAVITY FIXED SCATTERING COEFFICIENTS  */
	initializeNasalCavity();

	/*  INITIALIZE THE THROAT LOWPASS FILTER  */
	throat_.reset(new Throat(sampleRate_, throatCutoff_, amplitude(throatVol_)));

	/*  INITIALIZE THE SAMPLE RATE CONVERSION ROUTINES  */
	srConv_.reset(new SampleRateConverter(sampleRate_, outputRate_, outputData_));

	/*  INITIALIZE THE OUTPUT VECTOR  */
	outputData_.clear();

	bandpassFilter_.reset(new BandpassFilter());
	noiseFilter_.reset(new NoiseFilter());
	noiseSource_.reset(new NoiseSource());
}

/******************************************************************************
*
*  function:  synthesize
*
*  purpose:   Performs the actual synthesis of sound samples.
*
******************************************************************************/
void
Tube::synthesizeForInputSequence()
{
	/*  CONTROL RATE LOOP  */
	for (int i = 1, size = inputData_.size(); i < size; i++) {
		/*  SET CONTROL RATE PARAMETERS FROM INPUT TABLES  */
		setControlRateParameters(i);

		/*  SAMPLE RATE LOOP  */
		for (int j = 0; j < controlPeriod_; j++) {
			synthesize();

			/*  DO SAMPLE RATE INTERPOLATION OF CONTROL PARAMETERS  */
			sampleRateInterpolation();
		}
	}
}

void
Tube::synthesize()
{
	/*  CONVERT PARAMETERS HERE  */
	double f0 = frequency(currentData_.glotPitch);
	double ax = amplitude(currentData_.glotVol);
	double ah1 = amplitude(currentData_.aspVol);
	calculateTubeCoefficients();
	setFricationTaps();
	bandpassFilter_->update(sampleRate_, currentData_.fricBW, currentData_.fricCF);

	/*  DO SYNTHESIS HERE  */
	/*  CREATE LOW-PASS FILTERED NOISE  */
	double lpNoise = noiseFilter_->filter(noiseSource_->getSample());

	/*  UPDATE THE SHAPE OF THE GLOTTAL PULSE, IF NECESSARY  */
	if (waveform_ == GLOTTAL_SOURCE_PULSE) {
		if (ax != prevGlotAmplitude_) {
			glottalSource_->updateWavetable(ax);
		}
	}

	/*  CREATE GLOTTAL PULSE (OR SINE TONE)  */
	double pulse = glottalSource_->getSample(f0);

	/*  CREATE PULSED NOISE  */
	double pulsedNoise = lpNoise * pulse;

	/*  CREATE NOISY GLOTTAL PULSE  */
	pulse = ax * ((pulse * (1.0 - breathinessFactor_)) +
			(pulsedNoise * breathinessFactor_));

	double signal;
	/*  CROSS-MIX PURE NOISE WITH PULSED NOISE  */
	if (modulation_) {
		double crossmix = ax * crossmixFactor_;
		crossmix = (crossmix < 1.0) ? crossmix : 1.0;
		signal = (pulsedNoise * crossmix) +
				(lpNoise * (1.0 - crossmix));
	} else {
		signal = lpNoise;
	}

	/*  PUT SIGNAL THROUGH VOCAL TRACT  */
	signal = vocalTract(((pulse + (ah1 * signal)) * VT_SCALE),
				bandpassFilter_->filter(signal));

	/*  PUT PULSE THROUGH THROAT  */
	signal += throat_->process(pulse * VT_SCALE);

	/*  OUTPUT SAMPLE HERE  */
	srConv_->dataFill(signal);

	prevGlotAmplitude_ = ax;
}

/******************************************************************************
*
*  function:  setControlRateParameters
*
*  purpose:   Calculates the current table values, and their
*             associated sample-to-sample delta values.
*
******************************************************************************/
void
Tube::setControlRateParameters(int pos)
{
	double controlFreq = 1.0 / controlPeriod_;

	/*  GLOTTAL PITCH  */
	currentData_.glotPitch = inputData_[pos - 1]->glotPitch;
	currentData_.glotPitchDelta = (inputData_[pos]->glotPitch - currentData_.glotPitch) * controlFreq;

	/*  GLOTTAL VOLUME  */
	currentData_.glotVol = inputData_[pos - 1]->glotVol;
	currentData_.glotVolDelta = (inputData_[pos]->glotVol - currentData_.glotVol) * controlFreq;

	/*  ASPIRATION VOLUME  */
	currentData_.aspVol = inputData_[pos - 1]->aspVol;
#if MATCH_DSP
	currentData_.aspVolDelta = 0.0;
#else
	currentData_.aspVolDelta = (inputData_[pos]->aspVol - currentData_.aspVol) * controlFreq;
#endif

	/*  FRICATION VOLUME  */
	currentData_.fricVol = inputData_[pos - 1]->fricVol;
#if MATCH_DSP
	currentData_.fricVolDelta = 0.0;
#else
	currentData_.fricVolDelta = (inputData_[pos]->fricVol - currentData_.fricVol) * controlFreq;
#endif

	/*  FRICATION POSITION  */
	currentData_.fricPos = inputData_[pos - 1]->fricPos;
#if MATCH_DSP
	currentData_.fricPosDelta = 0.0;
#else
	currentData_.fricPosDelta = (inputData_[pos]->fricPos - currentData_.fricPos) * controlFreq;
#endif

	/*  FRICATION CENTER FREQUENCY  */
	currentData_.fricCF = inputData_[pos - 1]->fricCF;
#if MATCH_DSP
	currentData_.fricCFDelta = 0.0;
#else
	currentData_.fricCFDelta = (inputData_[pos]->fricCF - currentData_.fricCF) * controlFreq;
#endif

	/*  FRICATION BANDWIDTH  */
	currentData_.fricBW = inputData_[pos - 1]->fricBW;
#if MATCH_DSP
	currentData_.fricBWDelta = 0.0;
#else
	currentData_.fricBWDelta = (inputData_[pos]->fricBW - currentData_.fricBW) * controlFreq;
#endif

	/*  TUBE REGION RADII  */
	for (int i = 0; i < TOTAL_REGIONS; i++) {
		currentData_.radius[i] = inputData_[pos - 1]->radius[i];
		currentData_.radiusDelta[i] = (inputData_[pos]->radius[i] - currentData_.radius[i]) * controlFreq;
	}

	/*  VELUM RADIUS  */
	currentData_.velum = inputData_[pos - 1]->velum;
	currentData_.velumDelta = (inputData_[pos]->velum - currentData_.velum) * controlFreq;
}

/******************************************************************************
*
*  function:  sampleRateInterpolation
*
*  purpose:   Interpolates table values at the sample rate.
*
******************************************************************************/
void
Tube::sampleRateInterpolation()
{
	currentData_.glotPitch += currentData_.glotPitchDelta;
	currentData_.glotVol   += currentData_.glotVolDelta;
	currentData_.aspVol    += currentData_.aspVolDelta;
	currentData_.fricVol   += currentData_.fricVolDelta;
	currentData_.fricPos   += currentData_.fricPosDelta;
	currentData_.fricCF    += currentData_.fricCFDelta;
	currentData_.fricBW    += currentData_.fricBWDelta;
	for (int i = 0; i < TOTAL_REGIONS; i++) {
		currentData_.radius[i] += currentData_.radiusDelta[i];
	}
	currentData_.velum     += currentData_.velumDelta;
}

/******************************************************************************
*
*  function:  initializeNasalCavity
*
*  purpose:   Calculates the scattering coefficients for the fixed
*             sections of the nasal cavity.
*
******************************************************************************/
void
Tube::initializeNasalCavity()
{
	double radA2, radB2;

	/*  CALCULATE COEFFICIENTS FOR INTERNAL FIXED SECTIONS OF NASAL CAVITY  */
	for (int i = N2, j = NC2; i < N6; i++, j++) {
		radA2 = noseRadius_[i]     * noseRadius_[i];
		radB2 = noseRadius_[i + 1] * noseRadius_[i + 1];
		nasalCoeff_[j] = (radA2 - radB2) / (radA2 + radB2);
	}

	/*  CALCULATE THE FIXED COEFFICIENT FOR THE NOSE APERTURE  */
	radA2 = noseRadius_[N6] * noseRadius_[N6];
	radB2 = apertureRadius_ * apertureRadius_;
	nasalCoeff_[NC6] = (radA2 - radB2) / (radA2 + radB2);
}


/******************************************************************************
*
*  function:  calculateTubeCoefficients
*
*  purpose:   Calculates the scattering coefficients for the vocal
*             ract according to the current radii.  Also calculates
*             the coefficients for the reflection/radiation filter
*             pair for the mouth and nose.
*
******************************************************************************/
void
Tube::calculateTubeCoefficients()
{
	double radA2, radB2, r0_2, r1_2, r2_2, sum;

	/*  CALCULATE COEFFICIENTS FOR THE OROPHARYNX  */
	for (int i = 0; i < (TOTAL_REGIONS - 1); i++) {
		radA2 = currentData_.radius[i]     * currentData_.radius[i];
		radB2 = currentData_.radius[i + 1] * currentData_.radius[i + 1];
		oropharynxCoeff_[i] = (radA2 - radB2) / (radA2 + radB2);
	}

	/*  CALCULATE THE COEFFICIENT FOR THE MOUTH APERTURE  */
	radA2 = currentData_.radius[R8] * currentData_.radius[R8];
	radB2 = apertureRadius_ * apertureRadius_;
	oropharynxCoeff_[C8] = (radA2 - radB2) / (radA2 + radB2);

	/*  CALCULATE ALPHA COEFFICIENTS FOR 3-WAY JUNCTION  */
	/*  NOTE:  SINCE JUNCTION IS IN MIDDLE OF REGION 4, r0_2 = r1_2  */
	r0_2 = r1_2 = currentData_.radius[R4] * currentData_.radius[R4];
	r2_2 = currentData_.velum * currentData_.velum;
	sum = 2.0 / (r0_2 + r1_2 + r2_2);
	alpha_[LEFT]  = sum * r0_2;
	alpha_[RIGHT] = sum * r1_2;
	alpha_[UPPER] = sum * r2_2;

	/*  AND 1ST NASAL PASSAGE COEFFICIENT  */
	radA2 = currentData_.velum * currentData_.velum;
	radB2 = noseRadius_[N2] * noseRadius_[N2];
	nasalCoeff_[NC1] = (radA2 - radB2) / (radA2 + radB2);
}

/******************************************************************************
*
*  function:  setFricationTaps
*
*  purpose:   Sets the frication taps according to the current
*             position and amplitude of frication.
*
******************************************************************************/
void
Tube::setFricationTaps()
{
	int integerPart;
	double complement, remainder;
	double fricationAmplitude = amplitude(currentData_.fricVol);

	/*  CALCULATE POSITION REMAINDER AND COMPLEMENT  */
	integerPart = (int) currentData_.fricPos;
	complement = currentData_.fricPos - (double) integerPart;
	remainder = 1.0 - complement;

	/*  SET THE FRICATION TAPS  */
	for (int i = FC1; i < TOTAL_FRIC_COEFFICIENTS; i++) {
		if (i == integerPart) {
			fricationTap_[i] = remainder * fricationAmplitude;
			if ((i + 1) < TOTAL_FRIC_COEFFICIENTS) {
				fricationTap_[++i] = complement * fricationAmplitude;
			}
		} else {
			fricationTap_[i] = 0.0;
		}
	}

#if 0
	/*  PRINT OUT  */
	printf("fricationTaps:  ");
	for (i = FC1; i < TOTAL_FRIC_COEFFICIENTS; i++)
		printf("%.6f  ", fricationTap[i]);
	printf("\n");
#endif
}

/******************************************************************************
*
*  function:  vocalTract
*
*  purpose:   Updates the pressure wave throughout the vocal tract,
*             and returns the summed output of the oral and nasal
*             cavities.  Also injects frication appropriately.
*
******************************************************************************/
double
Tube::vocalTract(double input, double frication)
{
	int i, j, k;
	double delta, output, junctionPressure;

	/*  INCREMENT CURRENT AND PREVIOUS POINTERS  */
	if (++currentPtr_ > 1) {
		currentPtr_ = 0;
	}
	if (++prevPtr_ > 1) {
		prevPtr_ = 0;
	}

	/*  UPDATE OROPHARYNX  */
	/*  INPUT TO TOP OF TUBE  */
	oropharynx_[S1][TOP][currentPtr_] =
			(oropharynx_[S1][BOTTOM][prevPtr_] * dampingFactor_) + input;

	/*  CALCULATE THE SCATTERING JUNCTIONS FOR S1-S2  */
	delta = oropharynxCoeff_[C1] *
			(oropharynx_[S1][TOP][prevPtr_] - oropharynx_[S2][BOTTOM][prevPtr_]);
	oropharynx_[S2][TOP][currentPtr_] =
			(oropharynx_[S1][TOP][prevPtr_] + delta) * dampingFactor_;
	oropharynx_[S1][BOTTOM][currentPtr_] =
			(oropharynx_[S2][BOTTOM][prevPtr_] + delta) * dampingFactor_;

	/*  CALCULATE THE SCATTERING JUNCTIONS FOR S2-S3 AND S3-S4  */
	for (i = S2, j = C2, k = FC1; i < S4; i++, j++, k++) {
		delta = oropharynxCoeff_[j] *
				(oropharynx_[i][TOP][prevPtr_] - oropharynx_[i + 1][BOTTOM][prevPtr_]);
		oropharynx_[i + 1][TOP][currentPtr_] =
				((oropharynx_[i][TOP][prevPtr_] + delta) * dampingFactor_) +
				(fricationTap_[k] * frication);
		oropharynx_[i][BOTTOM][currentPtr_] =
				(oropharynx_[i + 1][BOTTOM][prevPtr_] + delta) * dampingFactor_;
	}

	/*  UPDATE 3-WAY JUNCTION BETWEEN THE MIDDLE OF R4 AND NASAL CAVITY  */
	junctionPressure = (alpha_[LEFT] * oropharynx_[S4][TOP][prevPtr_])+
			(alpha_[RIGHT] * oropharynx_[S5][BOTTOM][prevPtr_]) +
			(alpha_[UPPER] * nasal_[VELUM][BOTTOM][prevPtr_]);
	oropharynx_[S4][BOTTOM][currentPtr_] =
			(junctionPressure - oropharynx_[S4][TOP][prevPtr_]) * dampingFactor_;
	oropharynx_[S5][TOP][currentPtr_] =
			((junctionPressure - oropharynx_[S5][BOTTOM][prevPtr_]) * dampingFactor_)
			+ (fricationTap_[FC3] * frication);
	nasal_[VELUM][TOP][currentPtr_] =
			(junctionPressure - nasal_[VELUM][BOTTOM][prevPtr_]) * dampingFactor_;

	/*  CALCULATE JUNCTION BETWEEN R4 AND R5 (S5-S6)  */
	delta = oropharynxCoeff_[C4] *
			(oropharynx_[S5][TOP][prevPtr_] - oropharynx_[S6][BOTTOM][prevPtr_]);
	oropharynx_[S6][TOP][currentPtr_] =
			((oropharynx_[S5][TOP][prevPtr_] + delta) * dampingFactor_) +
			(fricationTap_[FC4] * frication);
	oropharynx_[S5][BOTTOM][currentPtr_] =
			(oropharynx_[S6][BOTTOM][prevPtr_] + delta) * dampingFactor_;

	/*  CALCULATE JUNCTION INSIDE R5 (S6-S7) (PURE DELAY WITH DAMPING)  */
	oropharynx_[S7][TOP][currentPtr_] =
			(oropharynx_[S6][TOP][prevPtr_] * dampingFactor_) +
			(fricationTap_[FC5] * frication);
	oropharynx_[S6][BOTTOM][currentPtr_] =
			oropharynx_[S7][BOTTOM][prevPtr_] * dampingFactor_;

	/*  CALCULATE LAST 3 INTERNAL JUNCTIONS (S7-S8, S8-S9, S9-S10)  */
	for (i = S7, j = C5, k = FC6; i < S10; i++, j++, k++) {
		delta = oropharynxCoeff_[j] *
				(oropharynx_[i][TOP][prevPtr_] - oropharynx_[i + 1][BOTTOM][prevPtr_]);
		oropharynx_[i + 1][TOP][currentPtr_] =
				((oropharynx_[i][TOP][prevPtr_] + delta) * dampingFactor_) +
				(fricationTap_[k] * frication);
		oropharynx_[i][BOTTOM][currentPtr_] =
				(oropharynx_[i + 1][BOTTOM][prevPtr_] + delta) * dampingFactor_;
	}

	/*  REFLECTED SIGNAL AT MOUTH GOES THROUGH A LOWPASS FILTER  */
	oropharynx_[S10][BOTTOM][currentPtr_] =  dampingFactor_ *
			mouthReflectionFilter_->filter(oropharynxCoeff_[C8] *
							oropharynx_[S10][TOP][prevPtr_]);

	/*  OUTPUT FROM MOUTH GOES THROUGH A HIGHPASS FILTER  */
	output = mouthRadiationFilter_->filter((1.0 + oropharynxCoeff_[C8]) *
						oropharynx_[S10][TOP][prevPtr_]);

	/*  UPDATE NASAL CAVITY  */
	for (i = VELUM, j = NC1; i < N6; i++, j++) {
		delta = nasalCoeff_[j] *
				(nasal_[i][TOP][prevPtr_] - nasal_[i + 1][BOTTOM][prevPtr_]);
		nasal_[i+1][TOP][currentPtr_] =
				(nasal_[i][TOP][prevPtr_] + delta) * dampingFactor_;
		nasal_[i][BOTTOM][currentPtr_] =
				(nasal_[i + 1][BOTTOM][prevPtr_] + delta) * dampingFactor_;
	}

	/*  REFLECTED SIGNAL AT NOSE GOES THROUGH A LOWPASS FILTER  */
	nasal_[N6][BOTTOM][currentPtr_] = dampingFactor_ *
			nasalReflectionFilter_->filter(nasalCoeff_[NC6] * nasal_[N6][TOP][prevPtr_]);

	/*  OUTPUT FROM NOSE GOES THROUGH A HIGHPASS FILTER  */
	output += nasalRadiationFilter_->filter((1.0 + nasalCoeff_[NC6]) *
						nasal_[N6][TOP][prevPtr_]);
	/*  RETURN SUMMED OUTPUT FROM MOUTH AND NOSE  */
	return output;
}

/******************************************************************************
*
*  function:  writeOutputToFile
*
*  purpose:   Scales the samples stored in the temporary file, and
*             writes them to the output file, with the appropriate
*             header. Also does master volume scaling, and stereo
*             balance scaling, if 2 channels of output.
*
******************************************************************************/
void
Tube::writeOutputToFile(const char* outputFile)
{
	/*  BE SURE TO FLUSH SRC BUFFER  */
	srConv_->flushBuffer();

	LOG_DEBUG("\nNumber of samples: " << srConv_->numberSamples() <<
			"\nMaximum sample value: " << srConv_->maximumSampleValue());

	WAVEFileWriter fileWriter(outputFile, channels_, srConv_->numberSamples(), outputRate_);

	if (channels_ == 1) {
		float scale = calculateMonoScale();
		for (unsigned int i = 0, end = srConv_->numberSamples(); i < end; ++i) {
			fileWriter.writeSample(outputData_[i] * scale);
		}
	} else {
		float leftScale, rightScale;
		calculateStereoScale(leftScale, rightScale);
		for (unsigned int i = 0, end = srConv_->numberSamples(); i < end; ++i) {
			fileWriter.writeStereoSamples(outputData_[i] * leftScale, outputData_[i] * rightScale);
		}
	}
}

float
Tube::calculateMonoScale()
{
	float scale = static_cast<float>((OUTPUT_SCALE / srConv_->maximumSampleValue()) * amplitude(volume_));
	LOG_DEBUG("\nScale: " << scale << '\n');
	return scale;
}

void
Tube::calculateStereoScale(float& leftScale, float& rightScale)
{
	leftScale = static_cast<float>(-((balance_ / 2.0) - 0.5));
	rightScale = static_cast<float>(((balance_ / 2.0) + 0.5));
	float newMax = static_cast<float>(srConv_->maximumSampleValue() * (balance_ > 0.0 ? rightScale : leftScale));
	float scale = static_cast<float>((OUTPUT_SCALE / newMax) * amplitude(volume_));
	leftScale  *= scale;
	rightScale *= scale;
	LOG_DEBUG("\nLeft scale: " << leftScale << " Right scale: " << rightScale << '\n');
}

/******************************************************************************
*
*  function:  amplitude
*
*  purpose:   Converts dB value to amplitude value.
*
******************************************************************************/
double
Tube::amplitude(double decibelLevel)
{
	/*  CONVERT 0-60 RANGE TO -60-0 RANGE  */
	decibelLevel -= VOL_MAX;

	/*  IF -60 OR LESS, RETURN AMPLITUDE OF 0  */
	if (decibelLevel <= (-VOL_MAX)) {
		return 0.0;
	}

	/*  IF 0 OR GREATER, RETURN AMPLITUDE OF 1  */
	if (decibelLevel >= 0.0) {
		return 1.0;
	}

	/*  ELSE RETURN INVERSE LOG VALUE  */
	return pow(10.0, decibelLevel / 20.0);
}

/******************************************************************************
*
*  function:  frequency
*
*  purpose:   Converts a given pitch (0 = middle C) to the
*             corresponding frequency.
*
******************************************************************************/
double
Tube::frequency(double pitch)
{
	return PITCH_BASE * pow(2.0, (pitch + PITCH_OFFSET) / 12.0);
}

} /* namespace TRM */
} /* namespace GS */
