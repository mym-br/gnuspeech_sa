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

#include "WAVEFileWriter.h"

#include "Exception.h"

#include <cmath> /* ceil, round */
#include <cstdint>



#define BITS_PER_SAMPLE 16



namespace GS {

WAVEFileWriter::WAVEFileWriter(const char* filePath, int channels, int numberSamples, float outputRate)
		: sampleScale_(INT16_MAX)
{
	stream_ = fopen(filePath, "wb"); // the b is for non-POSIX systems
	if (stream_ == NULL) {
		THROW_EXCEPTION(IOException, "Could not open the file " << filePath << " for writing.");
	}

	writeWaveFileHeader(channels, numberSamples, outputRate);
}

WAVEFileWriter::~WAVEFileWriter()
{
	fclose(stream_);
}

/******************************************************************************
*
*       function:       writeWaveFileHeader
*
*       purpose:        Writes the header in WAVE format to the output file.
*
******************************************************************************/
void
WAVEFileWriter::writeWaveFileHeader(int channels, int numberSamples, float outputRate)
{
	int dataChunkSize = channels * numberSamples * sizeof(std::int16_t);
	int formSize = 4 + 24 + (8 + dataChunkSize);
	int frameSize = static_cast<int>(std::ceil(channels * (BITS_PER_SAMPLE / 8.0)));
	int bytesPerSecond = static_cast<int>(std::ceil(outputRate * frameSize));

	/*  Form container identifier  */
	fputs("RIFF", stream_);

	/*  Form size  */
	writeUInt32LE(formSize);

	/*  Form container type  */
	fputs("WAVE", stream_);

	/*  Format chunk identifier (Note: space after 't' needed)  */
	fputs("fmt ", stream_);

	/*  Chunk size (fixed at 16 bytes)  */
	writeUInt32LE(16);

	/*  Compression code: 1 = PCM  */
	writeUInt16LE(1);

	/*  Number of channels  */
	writeUInt16LE(channels);

	/*  Output Sample Rate  */
	writeUInt32LE(static_cast<int>(std::round(outputRate)));

	/*  Bytes per second  */
	writeUInt32LE(bytesPerSecond);

	/*  Block alignment (frame size)  */
	writeUInt16LE(frameSize);

	/*  Bits per sample  */
	writeUInt16LE(BITS_PER_SAMPLE);

	/*  Sound Data chunk identifier  */
	fputs("data", stream_);

	/*  Chunk size  */
	writeUInt32LE(dataChunkSize);
}

/******************************************************************************
*
*       function:       writeSample
*
*       purpose:        Reads the double f.p. samples in the temporary file,
*                       scales them, rounds them to a short (16-bit) integer,
*                       and writes them to the output file in little-endian
*                       format.
*
*       sample: [-1.0, 1.0]
*
******************************************************************************/
void
WAVEFileWriter::writeSample(float sample)
{
	writeUInt16LE(static_cast<int>(std::round(sample * sampleScale_)));
}

/******************************************************************************
*
*       function:       writeStereoSamples
*
*       purpose:        Reads the double f.p. samples in the temporary file,
*                       does stereo scaling, rounds them to a short (16-bit)
*                       integer, and writes them to the output file in
*                       little-endian format.
*
*       leftSample, rightSample: [-1.0, 1.0]
*
******************************************************************************/
void
WAVEFileWriter::writeStereoSamples(float leftSample, float rightSample)
{
	writeUInt16LE(static_cast<int>(std::round(leftSample * sampleScale_)));
	writeUInt16LE(static_cast<int>(std::round(rightSample * sampleScale_)));
}

/******************************************************************************
*
*       function:       writeUInt32LE
*
*       purpose:        Writes a 4-byte integer to the file stream, starting
*                       with the least significant byte (i.e. writes the int
*                       in little-endian form).  This routine will work on both
*                       big-endian and little-endian architectures.
*
******************************************************************************/
void
WAVEFileWriter::writeUInt32LE(int data)
{
	unsigned char array[4];

	array[0] =  data        & 0xff;
	array[1] = (data >> 8)  & 0xff;
	array[2] = (data >> 16) & 0xff;
	array[3] = (data >> 24) & 0xff;

	fwrite(array, sizeof(unsigned char), 4, stream_);
}

/******************************************************************************
*
*       function:       writeUInt16LE
*
*       purpose:        Writes a 2-byte integer to the file stream, starting
*                       with the least significant byte (i.e. writes the int
*                       in little-endian form). This routine will work on both
*                       big-endian and little-endian architectures.
*
******************************************************************************/
void
WAVEFileWriter::writeUInt16LE(int data)
{
	unsigned char array[2];

	array[0] =  data       & 0xff;
	array[1] = (data >> 8) & 0xff;

	fwrite(array, sizeof(unsigned char), 2, stream_);
}

} /* namespace GS */

