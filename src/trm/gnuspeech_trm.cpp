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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "Tube.h"



int
main(int argc, char* argv[])
{
	const char* inputFile = nullptr;
	const char* outputFile = nullptr;

	/*  PARSE THE COMMAND LINE  */
	if (argc == 3) {
		inputFile = argv[1];
		outputFile = argv[2];
	} else if ((argc == 4) && (strcmp("-v", argv[1]) == 0)) {
		TRM::verbose = true;
		inputFile = argv[2];
		outputFile = argv[3];
	} else {
		fprintf(stderr, "Usage:  %s [-v] inputFile outputFile\n", argv[0]);
		return -1;
	}

	TRM::Tube trm;
	trm.synthesizeToFile(inputFile, outputFile);

	if (TRM::verbose) printf("\nWrote scaled samples to file: %s\n", outputFile);

	return 0;
}
