/***************************************************************************
 *  Copyright 1991, 1992, 1993, 1994, 1995, 1996, 2001, 2002               *
 *    David R. Hill, Leonard Manzara, Craig Schock                         *
 *  Copyright 2004 Steve Nygard                                            *
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
// This code was copied from the GNUstep port of Gnuspeech and modified
// by Marcelo Y. Matuda.

#ifndef WAVE_FILE_WRITER_H_
#define WAVE_FILE_WRITER_H_

#include <cstdio>



namespace GS {

// Note: Almost no error checking.

class WAVEFileWriter {
public:
	WAVEFileWriter(const char* filePath, int channels, int numberSamples, float outputRate);
	~WAVEFileWriter();

	void writeSample(float sample);
	void writeStereoSamples(float leftSample, float rightSample);
private:
	WAVEFileWriter(const WAVEFileWriter&) = delete;
	WAVEFileWriter& operator=(const WAVEFileWriter&) = delete;

	void writeWaveFileHeader(int channels, int numberSamples, float outputRate);
	void writeUInt32LE(int data);
	void writeUInt16LE(int data);

	FILE* stream_;
	float sampleScale_;
};

} /* namespace GS */

#endif /* WAVE_FILE_WRITER_H_ */
