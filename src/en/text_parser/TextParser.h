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

#ifndef EN_TEXT_PARSER_H_
#define EN_TEXT_PARSER_H_

#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "en/dictionary/DictionarySearch.h"
#include "en/text_parser/NumberParser.h"



namespace GS {
namespace En {

class TextParser {
public:
	TextParser(const char* configDirPath,
			const std::string& dictionary1Path,
			const std::string& dictionary2Path,
			const std::string& dictionary3Path);
	~TextParser();

	std::string parseText(const char* text);

private:
	enum {
		DICTIONARY_ORDER_SIZE = 6
	};

	TextParser(const TextParser&) = delete;
	TextParser& operator=(const TextParser&) = delete;

	void init_parser_module();
	int set_escape_code(char new_escape_code);
	const char* lookup_word(const char* word, short* dict);
	void condition_input(const char* input, char* output, int length, int* output_length);
	int mark_modes(const char* input, char *output, int length, int *output_length);
	void expand_word(char* word, int is_tonic, std::stringstream& stream);
	int final_conversion(std::stringstream& stream1, long stream1_length,
				std::stringstream& stream2, long* stream2_length);

	std::unique_ptr<DictionarySearch> dict1_;
	std::unique_ptr<DictionarySearch> dict2_;
	std::unique_ptr<DictionarySearch> dict3_;

	char escape_character_;
	short dictionaryOrder_[DICTIONARY_ORDER_SIZE];

	std::stringstream auxStream_;
	std::vector<char> pronunciation_;
	NumberParser numberParser_;
};

} /* namespace En */
} /* namespace GS */

#endif /* EN_TEXT_PARSER_H_ */
