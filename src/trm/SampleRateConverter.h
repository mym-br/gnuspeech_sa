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
// This file was copied from Gnuspeech and modified by Marcelo Y. Matuda.

#ifndef TRM_SAMPLE_RATE_CONVERTER_H_
#define TRM_SAMPLE_RATE_CONVERTER_H_

#include <vector>



namespace TRM {

class SampleRateConverter {
public:
	SampleRateConverter(int sampleRate, float outputRate, std::vector<double>& outputData);
	~SampleRateConverter();

	void dataFill(double data);
	void dataEmpty();
	void flushBuffer();

	double maximumSampleValue() const { return maximumSampleValue_; }
	long numberSamples() const { return numberSamples_; }
private:
	SampleRateConverter(const SampleRateConverter&);
	SampleRateConverter& operator=(const SampleRateConverter&);

	void initializeConversion(int sampleRate, float outputRate);
	void initializeBuffer();
	void initializeFilter();

	static double Izero(double x);
	static void srIncrement(int *pointer, int modulus);
	static void srDecrement(int *pointer, int modulus);

	double sampleRateRatio_;
	int fillPtr_;
	int emptyPtr_;
	int padSize_;
	int fillSize_;
	unsigned int timeRegisterIncrement_;
	unsigned int filterIncrement_;
	unsigned int phaseIncrement_;
	unsigned int timeRegister_;
	int fillCounter_;

	double maximumSampleValue_;
	long numberSamples_;

	std::vector<double> h_;
	std::vector<double> deltaH_;
	std::vector<double> buffer_;
	std::vector<double>& outputData_;
};

} /* namespace TRM */

#endif /* TRM_SAMPLE_RATE_CONVERTER_H_ */
