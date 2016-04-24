/***************************************************************************
 *  Copyright 2014 Marcelo Y. Matuda                                       *
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

#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "Controller.h"
#include "Exception.h"
#include "global.h"
#include "Log.h"
#include "Model.h"
#include "en/phonetic_string_parser/PhoneticStringParser.h"
#include "en/text_parser/TextParser.h"
#include "TRMControlModelConfiguration.h"



void
showUsage(const char* programName)
{
	std::cout << "\nGnuspeechSA " << PROGRAM_VERSION << "\n\n";
	std::cout << "Usage:\n\n";
	std::cout << programName << " --version\n";
	std::cout << "        Shows the program version.\n\n";
	std::cout << programName << " [-v] -c config_dir -p trm_param_file.txt -o output_file.wav \"Hello world.\"\n";
	std::cout << "        Synthesizes text from the command line.\n";
	std::cout << "        -v : verbose\n\n";
	std::cout << programName << " [-v] -c config_dir -i input_text.txt -p trm_param_file.txt -o output_file.wav\n";
	std::cout << "        Synthesizes text from a file.\n";
	std::cout << "        -v : verbose\n" << std::endl;
}

int
main(int argc, char* argv[])
{
	if (argc < 2) {
		showUsage(argv[0]);
		return 1;
	}

	const char* configDirPath = nullptr;
	const char* inputFile = nullptr;
	const char* outputFile = nullptr;
	const char* trmParamFile = nullptr;
	std::ostringstream inputTextStream;

	int i = 1;
	while (i < argc) {
		if (strcmp(argv[i], "-v") == 0) {
			++i;
			GS::Log::debugEnabled = true;
		} else if (strcmp(argv[i], "-c") == 0) {
			++i;
			if (i == argc) {
				showUsage(argv[0]);
				return 1;
			}
			configDirPath = argv[i];
			++i;
		} else if (strcmp(argv[i], "-i") == 0) {
			++i;
			if (i == argc) {
				showUsage(argv[0]);
				return 1;
			}
			inputFile = argv[i];
			++i;
		} else if (strcmp(argv[i], "-p") == 0) {
			++i;
			if (i == argc) {
				showUsage(argv[0]);
				return 1;
			}
			trmParamFile = argv[i];
			++i;
		} else if (strcmp(argv[i], "-o") == 0) {
			++i;
			if (i == argc) {
				showUsage(argv[0]);
				return 1;
			}
			outputFile = argv[i];
			++i;
		} else if (strcmp(argv[i], "--version") == 0) {
			++i;
			showUsage(argv[0]);
			return 0;
		} else {
			for ( ; i < argc; ++i) {
				inputTextStream << argv[i] << ' ';
			}
		}
	}

	if (configDirPath == nullptr || trmParamFile == nullptr || outputFile == nullptr) {
		showUsage(argv[0]);
		return 1;
	}

	if (inputFile != nullptr) {
		std::ifstream in(inputFile, std::ios_base::in | std::ios_base::binary);
		if (!in) {
			std::cerr << "Could not open the file " << inputFile << '.' << std::endl;
			return 1;
		}
		std::string line;
		while (std::getline(in, line)) {
			inputTextStream << line << ' ';
		}
	}
	std::string inputText = inputTextStream.str();
	if (inputText.empty()) {
		std::cerr << "Empty input text." << std::endl;
		return 1;
	}
	if (GS::Log::debugEnabled) {
		std::cout << "inputText=[" << inputText << ']' << std::endl;
	}

	try {
		std::unique_ptr<GS::TRMControlModel::Model> trmControlModel(new GS::TRMControlModel::Model());
		trmControlModel->load(configDirPath, TRM_CONTROL_MODEL_CONFIG_FILE);
		if (GS::Log::debugEnabled) {
			trmControlModel->printInfo();
		}

		std::unique_ptr<GS::TRMControlModel::Controller> trmController(new GS::TRMControlModel::Controller(configDirPath, *trmControlModel));
		const GS::TRMControlModel::Configuration& trmControlConfig = trmController->trmControlModelConfiguration();

		std::unique_ptr<GS::En::TextParser> textParser(new GS::En::TextParser(configDirPath,
											trmControlConfig.dictionary1File,
											trmControlConfig.dictionary2File,
											trmControlConfig.dictionary3File));
		std::unique_ptr<GS::En::PhoneticStringParser> phoneticStringParser(new GS::En::PhoneticStringParser(configDirPath, *trmController));

		std::string phoneticString = textParser->parseText(inputText.c_str());
		if (GS::Log::debugEnabled) {
			std::cout << "Phonetic string: [" << phoneticString << ']' << std::endl;
		}

		trmController->synthesizePhoneticString(*phoneticStringParser, phoneticString.c_str(), trmParamFile, outputFile);

	} catch (std::exception& e) {
		std::cerr << "Caught an exception: " << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Caught an unknown exception." << std::endl;
		return 1;
	}

	return 0;
}
