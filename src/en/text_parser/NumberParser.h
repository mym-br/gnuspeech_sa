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

#ifndef EN_NUMBER_PARSER_H_
#define EN_NUMBER_PARSER_H_

#include <array>



namespace GS {
namespace En {

/********************************************************************
number_parser() RETURNS A POINTER TO A NULL TERMINATED CHARACTER
STRING, WHICH CONTAINS THE CORRESPONDING PRONUNCIATION FOR THE
NUMBER TO BE PARSED.  number_parser() TAKES TWO ARGUMENTS:
 1)  word:  a pointer to the NULL terminated string to be parsed.
 2)  mode:  one of the above flags.


TYPICAL USAGE:
  char word[124], *ptr;
  int mode;

  strcat(word,"45,023.34");
  mode = NP_NORMAL;

  if ((ptr = number_parser(word,mode)) == NULL)
      printf("The word contains no numbers.\n");
  else
      printf("%s\n",ptr);



degenerate_string() RETURNS A CHARACTER-BY-CHARACTER PRONUNCIATION
OF A NUMBER STRING.  degenerate_string() TAKES ONE ARGUMENT:
 1) word:  a pointer to the NULL terminated string to be parsed.


TYPICAL USAGE:
  char word[124], *ptr;

  strcat(word,"%^@3*5");

  ptr = degenerate_string(word)
  printf("%s\n",ptr);

********************************************************************/

class NumberParser {
public:
	/*  FLAGS FOR ARGUMENT mode WHEN CALLING number_parser()  */
	enum Mode {
		NORMAL         = 0,
		OVERRIDE_YEARS = 1,
		FORCE_SPELL    = 2
	};
	enum {
		CLOCK_MAX             = 2,    /*  MAX # OF COLONS IN CLOCK TIMES  */
		NEGATIVE_MAX          = 3,    /*  MAX # OF NEGATIVE SIGNS (-)     */
		COMMAS_MAX            = 33,   /*  MAX # OF COMMAS                 */
		INTEGER_DIGITS_MAX    = 100,  /*  MAX # OF INTEGER DIGITS         */
		OUTPUT_MAX            = 8192  /*  OUTPUT BUFFER SIZE IN CHARS     */
	};
	enum {
		FRACTIONAL_DIGITS_MAX = 100   /*  MAX # OF FRACTIONAL DIGITS      */
	};

	NumberParser();
	~NumberParser();

	const char* parseNumber(const char* word, Mode mode);
	const char* degenerateString(const char* word);
private:
	NumberParser(const NumberParser&) = delete;
	NumberParser& operator=(const NumberParser&) = delete;

	int errorCheck(Mode mode);
	void initialParse();
	char* processWord(Mode mode);

	/*  INPUT AND OUTPUT VARIABLES  */
	const char* word_;
	std::array<char, OUTPUT_MAX> output_;        /*  STORAGE FOR OUTPUT  */

	/*  PARSING STATISTIC VARIABLES  */
	int wordLength_;
	int degenerate_;
	int integerDigits_;
	int fractionalDigits_;
	int commas_;
	int decimal_;
	int dollar_;
	int percent_;
	int negative_;
	int positive_;
	int ordinal_;
	int clock_;
	int slash_;
	int leftParen_;
	int rightParen_;
	int blank_;
	int dollarPlural_;
	int dollarNonzero_;
	int centsPlural_;
	int centsNonzero_;
	int telephone_;
	int leftZeroPad_;
	int rightZeroPad_;
	int ordinalPlural_;
	int fracLeftZeroPad_;
	int fracRightZeroPad_;
	int fracOrdinalTriad_;

	std::array<int, COMMAS_MAX> commasPos_;
	int decimalPos_;
	int dollarPos_;
	int percentPos_;
	std::array<int, NEGATIVE_MAX> negativePos_;
	int positivePos_;
	std::array<int, INTEGER_DIGITS_MAX> integerDigitsPos_;
	std::array<int, FRACTIONAL_DIGITS_MAX> fractionalDigitsPos_;
	std::array<int, 2> ordinalPos_;
	std::array<int, CLOCK_MAX> clockPos_;
	int slashPos_;
	int leftParenPos_;
	int rightParenPos_;
	int blankPos_;

	std::array<char, 3> triad_;

	/*  ORDINAL VARIABLES  */
	std::array<char, 3> ordinalBuffer_;
	int ordinalTriad_;

	/*  CLOCK VARIABLES  */
	std::array<char, 4> hour_;
	std::array<char, 4> minute_;
	std::array<char, 4> second_;
	int military_;
	int seconds_;
};

} /* namespace En */
} /* namespace GS */

#endif /* EN_NUMBER_PARSER_H_ */
