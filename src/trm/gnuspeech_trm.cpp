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

#include <cstring>
#include <fstream>
#include <iostream>

#include "global.h"
#include "Log.h"
#include "Tube.h"



int
main(int argc, char* argv[])
{
	using namespace GS;

	const char* inputFile = nullptr;
	const char* outputFile = nullptr;

	/*  PARSE THE COMMAND LINE  */
	if (argc == 3) {
		inputFile = argv[1];
		outputFile = argv[2];
	} else if ((argc == 4) && (strcmp("-v", argv[1]) == 0)) {
		Log::debugEnabled = true;
		inputFile = argv[2];
		outputFile = argv[3];
	} else {
		std::cout << "\nGnuspeechSA TRM " << PROGRAM_VERSION << "\n\n";
		std::cerr << "Usage: " << argv[0] << " [-v] trm_param_file.txt output_file.wav\n";
		std::cout << "         -v : verbose\n" << std::endl;
		return 1;
	}

	std::ifstream inputStream(inputFile, std::ios_base::in | std::ios_base::binary);
	if (!inputStream) {
		std::cerr << "Could not open the file " << inputFile << '.' << std::endl;
		return 1;
	}

	TRM::Tube trm;
	trm.synthesizeToFile(inputStream, outputFile);

	LOG_DEBUG("\nWrote scaled samples to file: " << outputFile);

	return 0;
}
