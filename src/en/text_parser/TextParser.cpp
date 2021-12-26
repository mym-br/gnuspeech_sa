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
*     parser_module.c
*
*     History:
*
*     July 7th, 1992          Completed.
*     December 12th, 1994     Added word begin /w and utterance
*                             boundary # markers.
*     January 5th, 1995       Fixed illegal_slash_code() so that it will
*                             recognize the new /w code when doing raw mode
*                             checking.  The # marker is a phone, so the new
*                             validPhone() function should return this as
*                             valid.  Also changed all closing of streams to
*                             use NX_FREEBUFFER instead of NX_TRUNCATEBUFFER,
*                             eliminating a potential memory leak.  The NeXT
*                             documentation is wrong, since it recommends
*                             using NX_TRUNCATEBUFFER, plus NXGetMemoryBuffer()
*                             and vm_deallocate() calls to free the internal
*                             stream buffer.
*     March 7th, 1995         Fixed bug when using medial punctuation (,;:)
*                             at the end of an utterance.
*
******************************************************************************/

#include "en/text_parser/TextParser.h"

#include <cmath>
#include <ctype.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "en/letter_to_sound/letter_to_sound.h"
#include "en/text_parser/abbreviations.h"
#include "en/text_parser/special_acronyms.h"
#include "Exception.h"
#include "Log.h"



/*  LOCAL DEFINES  ***********************************************************/
#define UNDEFINED_MODE        (-2)
#define NORMAL_MODE           (-1)
#define RAW_MODE              0
#define LETTER_MODE           1
#define EMPHASIS_MODE         2
#define TAGGING_MODE          3
#define SILENCE_MODE          4

#define RAW_MODE_BEGIN        (-1)
#define RAW_MODE_END          (-2)
#define LETTER_MODE_BEGIN     (-3)
#define LETTER_MODE_END       (-4)
#define EMPHASIS_MODE_BEGIN   (-5)
#define EMPHASIS_MODE_END     (-6)
#define TAGGING_MODE_BEGIN    (-7)
#define TAGGING_MODE_END      (-8)
#define SILENCE_MODE_BEGIN    (-9)
#define SILENCE_MODE_END      (-10)
#define DELETED               (-11)

#define BEGIN                 0
#define END                   1

#define WORD                  0
#define PUNCTUATION           1
#define PRONUNCIATION         1

#define AND                   "and"
#define PLUS                  "plus"
#define IS_LESS_THAN          "is less than"
#define IS_GREATER_THAN       "is greater than"
#define EQUALS                "equals"
#define MINUS                 "minus"
#define AT                    "at"

#define ABBREVIATION          0
#define EXPANSION             1

#define STATE_UNDEFINED       (-1)
#define STATE_BEGIN           0
#define STATE_WORD            1
#define STATE_MEDIAL_PUNC     2
#define STATE_FINAL_PUNC      3
#define STATE_END             4
#define STATE_SILENCE         5
#define STATE_TAGGING         6


#define CHUNK_BOUNDARY        "/c"
#define TONE_GROUP_BOUNDARY   "//"
#define FOOT_BEGIN            "/_"
#define TONIC_BEGIN           "/*"
#define SECONDARY_STRESS      "/\""
#define LAST_WORD             "/l"
#define TAG_BEGIN             "/t"
#define WORD_BEGIN            "/w"
#define UTTERANCE_BOUNDARY    "#"
#define MEDIAL_PAUSE          "^"
#define LONG_MEDIAL_PAUSE     "^ ^ ^"
#define SILENCE_PHONE         "^"

#define TG_UNDEFINED          "/x"
#define TG_STATEMENT          "/0"
#define TG_EXCLAMATION        "/1"
#define TG_QUESTION           "/2"
#define TG_CONTINUATION       "/3"
#define TG_HALF_PERIOD        "/4"

#define UNDEFINED_POSITION    (-1)

#define TTS_FALSE             0
#define TTS_TRUE              1
#define TTS_NO                0
#define TTS_YES               1

#define SYMBOL_LENGTH_MAX     12

#define WORD_LENGTH_MAX       1024
#define SILENCE_MAX           5.0
#define SILENCE_PHONE_LENGTH  0.1     /*  SILENCE PHONE IS 100ms  */

#define DEFAULT_END_PUNC      "."
#define MODE_NEST_MAX         100

#define NON_PHONEME           0
#define PHONEME               1
#define MAX_PHONES_PER_CHUNK  1500
#define MAX_FEET_PER_CHUNK    100

#define DEFAULT_ESCAPE_CHARACTER 27

/*  Dictionary Ordering Definitions  */
#define TTS_EMPTY                       0
#define TTS_NUMBER_PARSER               1
#define TTS_DICTIONARY_1                2
#define TTS_DICTIONARY_2                3
#define TTS_DICTIONARY_3                4
#define TTS_LETTER_TO_SOUND             5

#define TTS_PARSER_SUCCESS       (-1)
#define TTS_PARSER_FAILURE       0              /*  OR GREATER THAN 0 IF     */
						/*  POSITION OF ERROR KNOWN  */



namespace {

void print_stream(std::stringstream& stream, long stream_length);
void strip_punctuation(char* buffer, int length, std::stringstream& stream, long *stream_length);
int get_state(const char* buffer, long* i, long length, int* mode, int* next_mode,
		int* current_state, int* next_state, int* raw_mode_flag,
		char* word, std::stringstream& stream);
int set_tone_group(std::stringstream& stream, long tg_pos, const char* word);
float convert_silence(const char* buffer, std::stringstream& stream);
int another_word_follows(const char* buffer, long i, long length, int mode);
int shift_silence(const char* buffer, long i, long length, int mode, std::stringstream& stream);
void insert_tag(std::stringstream& stream, long insert_point, const char* word);
int expand_raw_mode(const char *buffer, long* j, long length, std::stringstream& stream);
int illegal_token(const char* token);
int illegal_slash_code(const char* code);
int expand_tag_number(const char* buffer, long* j, long length, std::stringstream& stream);
int is_mode(char c);
int is_isolated(char *buffer, int i, int len);
int part_of_number(char *buffer, int i, int len);
int number_follows(char *buffer, int i, int len);
void delete_ellipsis(char *buffer, int *i, int length);
int convert_dash(char *buffer, int *i, int length);
int is_telephone_number(char *buffer, int i, int length);
int is_punctuation(char c);
int word_follows(const char* buffer, int i, int length);
int expand_abbreviation(char* buffer, int i, int length, std::stringstream& stream);
void expand_letter_mode(const char* buffer, int* i, int length, std::stringstream& stream, int* status);
int is_all_upper_case(const char* word);
char *to_lower_case(char *word);
const char* is_special_acronym(const char* word);
int contains_primary_stress(const char *pronunciation);
int converted_stress(char *pronunciation);
int is_possessive(char* word);
void safety_check(std::stringstream& stream, long* stream_length);
void insert_chunk_marker(std::stringstream& stream, long insert_point, char tg_type);
void check_tonic(std::stringstream& stream, long start_pos, long end_pos);



/******************************************************************************
*
*       function:       print_stream
*
*       purpose:        Prints out the contents of a parser stream, inserting
*                       visible mode markers.
*
******************************************************************************/
void
print_stream(std::stringstream& stream, long stream_length)
{
	/*  REWIND STREAM TO BEGINNING  */
	stream.seekg(0);

	/*  PRINT LOOP  */
	printf("stream_length = %-ld\n<begin>", stream_length);
	for (long i = 0; i < stream_length; i++) {
		char c = stream.get();
		switch (c) {
		case RAW_MODE_BEGIN:
			printf("<raw mode begin>");
			break;
		case RAW_MODE_END:
			printf("<raw mode end>");
			break;
		case LETTER_MODE_BEGIN:
			printf("<letter mode begin>");
			break;
		case LETTER_MODE_END:
			printf("<letter mode end>");
			break;
		case EMPHASIS_MODE_BEGIN:
			printf("<emphasis mode begin>");
			break;
		case EMPHASIS_MODE_END:
			printf("<emphasis mode end>");
			break;
		case TAGGING_MODE_BEGIN:
			printf("<tagging mode begin>");
			break;
		case TAGGING_MODE_END:
			printf("<tagging mode end>");
			break;
		case SILENCE_MODE_BEGIN:
			printf("<silence mode begin>");
			break;
		case SILENCE_MODE_END:
			printf("<silence mode end>");
			break;
		case '\0':
			printf("\\0");
			break;
		default:
			printf("%c", c);
			break;
		}
	}
	printf("<end>\n");
}

/******************************************************************************
*
*       function:       strip_punctuation
*
*       purpose:        Deletes unnecessary punctuation, and converts some
*                       punctuation to another form.
*
******************************************************************************/
void
strip_punctuation(char* buffer, int length, std::stringstream& stream, long* stream_length)
{
	int i, mode = NORMAL_MODE, status;

	/*  DELETE OR CONVERT PUNCTUATION  */
	for (i = 0; i < length; i++) {
		switch(buffer[i]) {
		case RAW_MODE_BEGIN:      mode = RAW_MODE;      break;
		case LETTER_MODE_BEGIN:   mode = LETTER_MODE;   break;
		case EMPHASIS_MODE_BEGIN: mode = EMPHASIS_MODE; break;
		case TAGGING_MODE_BEGIN:  mode = TAGGING_MODE;  break;
		case SILENCE_MODE_BEGIN:  mode = SILENCE_MODE;  break;
		case RAW_MODE_END:
		case LETTER_MODE_END:
		case EMPHASIS_MODE_END:
		case TAGGING_MODE_END:
		case SILENCE_MODE_END:    mode = NORMAL_MODE;   break;
		default:
			if ((mode == NORMAL_MODE) || (mode == EMPHASIS_MODE)) {
				switch(buffer[i]) {
				case '[':
					buffer[i] = '(';
					break;
				case ']':
					buffer[i] = ')';
					break;
				case '-':
					if (!convert_dash(buffer, &i, length) &&
							!number_follows(buffer, i, length) &&
							!is_isolated(buffer, i, length)) {
						buffer[i] = DELETED;
					}
					break;
				case '+':
					if (!part_of_number(buffer, i, length) && !is_isolated(buffer, i, length)) {
						buffer[i] = DELETED;
					}
					break;
				case '\'':
					if (!(((i-1) >= 0) && isalpha(buffer[i-1]) && ((i+1) < length) && isalpha(buffer[i+1]))) {
						buffer[i] = DELETED;
					}
					break;
				case '.':
					delete_ellipsis(buffer, &i, length);
					break;
				case '/':
				case '$':
				case '%':
					if (!part_of_number(buffer, i, length)) {
						buffer[i] = DELETED;
					}
					break;
				case '<':
				case '>':
				case '&':
				case '=':
				case '@':
					if (!is_isolated(buffer, i, length)) {
						buffer[i] = DELETED;
					}
					break;
				case '"':
				case '`':
				case '#':
				case '*':
				case '\\':
				case '^':
				case '_':
				case '|':
				case '~':
				case '{':
				case '}':
					buffer[i] = DELETED;
					break;
				default:
					break;
				}
			}
			break;
		}
	}

	/*  SECOND PASS  */
	stream.str("");
	mode = NORMAL_MODE;  status = PUNCTUATION;
	for (i = 0; i < length; i++) {
		switch(buffer[i]) {
		case RAW_MODE_BEGIN:      mode = RAW_MODE;      stream << buffer[i]; break;
		case EMPHASIS_MODE_BEGIN: mode = EMPHASIS_MODE; stream << buffer[i]; break;
		case TAGGING_MODE_BEGIN:  mode = TAGGING_MODE;  stream << buffer[i]; break;
		case SILENCE_MODE_BEGIN:  mode = SILENCE_MODE;  stream << buffer[i]; break;
		case LETTER_MODE_BEGIN:   mode = LETTER_MODE;   /*  expand below  */    ; break;

		case RAW_MODE_END:
		case EMPHASIS_MODE_END:
		case TAGGING_MODE_END:
		case SILENCE_MODE_END:    mode = NORMAL_MODE;   stream << buffer[i]; break;
		case LETTER_MODE_END:     mode = NORMAL_MODE;   /*  expand below  */    ; break;

		case DELETED:
			/*  CONVERT ALL DELETED CHARACTERS TO BLANKS  */
			buffer[i] = ' ';
			stream << ' ';
			break;

		default:
			if ((mode == NORMAL_MODE) || (mode == EMPHASIS_MODE)) {
				switch(buffer[i]) {
				case '(':
					/*  CONVERT (?) AND (!) TO BLANKS  */
					if ( ((i+2) < length) && (buffer[i+2] == ')') &&
							((buffer[i+1] == '!') || (buffer[i+1] == '?')) ) {
						buffer[i] = buffer[i+1] = buffer[i+2] = ' ';
						stream << "   ";
						i += 2;
						continue;
					}
					/*  ALLOW TELEPHONE NUMBER WITH AREA CODE:  (403)274-3877  */
					if (is_telephone_number(buffer, i, length)) {
						int j;
						for (j = 0; j < 12; j++) {
							stream << buffer[i++];
						}
						status = WORD;
						continue;
					}
					/*  CONVERT TO COMMA IF PRECEDED BY WORD, FOLLOWED BY WORD  */
					if ((status == WORD) && word_follows(buffer, i, length)) {
						buffer[i] = ' ';
						stream << ", ";
						status = PUNCTUATION;
					} else {
						buffer[i] = ' ';
						stream << ' ';
					}
					break;
				case ')':
					/*  CONVERT TO COMMA IF PRECEDED BY WORD, FOLLOWED BY WORD  */
					if ((status == WORD) && word_follows(buffer, i, length)) {
						buffer[i] = ',';
						stream << ", ";
						status = PUNCTUATION;
					} else {
						buffer[i] = ' ';
						stream << ' ';
					}
					break;
				case '&':
					stream << AND;
					status = WORD;
					break;
				case '+':
					if (is_isolated(buffer, i, length)) {
						stream << PLUS;
					} else {
						stream << '+';
					}
					status = WORD;
					break;
				case '<':
					stream << IS_LESS_THAN;
					status = WORD;
					break;
				case '>':
					stream << IS_GREATER_THAN;
					status = WORD;
					break;
				case '=':
					stream << EQUALS;
					status = WORD;
					break;
				case '-':
					if (is_isolated(buffer, i, length)) {
						stream << MINUS;
					} else {
						stream << '-';
					}
					status = WORD;
					break;
				case '@':
					stream << AT;
					status = WORD;
					break;
				case '.':
					if (!expand_abbreviation(buffer, i, length, stream)) {
						stream << buffer[i];
						status = PUNCTUATION;
					}
					break;
				default:
					stream << buffer[i];
					if (is_punctuation(buffer[i])) {
						status = PUNCTUATION;
					} else if (isalnum(buffer[i])) {
						status = WORD;
					}
					break;
				}
			} else if (mode == LETTER_MODE) {
				/*  EXPAND LETTER MODE CONTENTS TO PLAIN WORDS OR SINGLE LETTERS  */

				expand_letter_mode(buffer, &i, length, stream, &status);
				continue;
			} else { /*  ELSE PASS CHARACTERS STRAIGHT THROUGH  */
				stream << buffer[i];
			}
			break;
		}
	}

	/*  SET STREAM LENGTH  */
	*stream_length = static_cast<long>(stream.tellp());
}

/******************************************************************************
*
*       function:       get_state
*
*       purpose:        Determines the current state and next state in buffer.
*                       A word or punctuation is put into word.  Raw mode
*                       contents are expanded and written to stream.
*
******************************************************************************/
int
get_state(const char* buffer, long* i, long length, int* mode, int* next_mode,
		int* current_state, int* next_state, int* raw_mode_flag,
		char* word, std::stringstream& stream)
{
	long j;
	int k, state = 0, current_mode;
	int *state_buffer[2];

	/*  PUT STATE POINTERS INTO ARRAY  */
	state_buffer[0] = current_state;
	state_buffer[1] = next_state;

	/*  GET 2 STATES  */
	for (j = *i, current_mode = *mode; j < length; j++) {
		/*  FILTER THROUGH EACH CHARACTER  */
		switch (buffer[j]) {
		case RAW_MODE_BEGIN:      current_mode = RAW_MODE;      break;
		case LETTER_MODE_BEGIN:   current_mode = LETTER_MODE;   break;
		case EMPHASIS_MODE_BEGIN: current_mode = EMPHASIS_MODE; break;
		case TAGGING_MODE_BEGIN:  current_mode = TAGGING_MODE;  break;
		case SILENCE_MODE_BEGIN:  current_mode = SILENCE_MODE;  break;

		case RAW_MODE_END:
		case LETTER_MODE_END:
		case EMPHASIS_MODE_END:
		case TAGGING_MODE_END:
		case SILENCE_MODE_END:    current_mode = NORMAL_MODE;   break;

		default:
			if ((current_mode == NORMAL_MODE) || (current_mode == EMPHASIS_MODE)) {
				/*  SKIP WHITE  */
				if (buffer[j] == ' ') {
					break;
				}

				/*  PUNCTUATION  */
				if (is_punctuation(buffer[j])) {
					if ((buffer[j] == '.') && ((j+1) < length) && isdigit(buffer[j+1])) {
						;  /*  DO NOTHING, HANDLE AS WORD BELOW  */
					} else {
						/*  SET STATE ACCORDING TO PUNCUATION TYPE  */
						switch (buffer[j]) {
						case '.':
						case '!':
						case '?':  *(state_buffer[state]) = STATE_FINAL_PUNC;  break;
						case ';':
						case ':':
						case ',':  *(state_buffer[state]) = STATE_MEDIAL_PUNC;  break;
						}

						/*  PUT PUNCTUATION INTO WORD BUFFER, SET OUTSIDE COUNTER, IN CURRENT STATE  */
						if (state == 0) {
							word[0] = buffer[j];
							word[1] = '\0';
							*i = j;
							/*  SET OUTSIDE MODE  */
							*mode = current_mode;
						} else { /*  SET NEXT MODE IF SECOND STATE  */
							*next_mode = current_mode;
						}

						/*  INCREMENT STATE  */
						state++;
						break;
					}
				}

				/*  WORD  */
				if (state == 0) {
					/*  PUT WORD INTO BUFFER  */
					k = 0;
					do {
						word[k++] = buffer[j++];
					} while ((j < length) && (buffer[j] != ' ') &&
						 !is_mode(buffer[j]) && (k < WORD_LENGTH_MAX));
					word[k] = '\0'; j--;

					/*  BACK UP IF WORD ENDS WITH PUNCTUATION  */
					while (k >= 1) {
						if (is_punctuation(word[k-1])) {
							word[--k] = '\0';
							j--;
						} else {
							break;
						}
					}

					/*  SET OUTSIDE COUNTER  */
					*i = j;

					/*  SET OUTSIDE MODE  */
					*mode = current_mode;
				} else {
					/*  SET NEXT MODE IF SECOND STATE  */
					*next_mode = current_mode;
				}

				/*  SET STATE TO WORD, INCREMENT STATE  */
				*(state_buffer[state++]) = STATE_WORD;
				break;
			} else if ((current_mode == SILENCE_MODE) && (state == 0)) {
				/*  PUT SILENCE LENGTH INTO WORD BUFFER IN CURRENT STATE ONLY  */
				k = 0;
				do {
					word[k++] = buffer[j++];
				} while ((j < length) && !is_mode(buffer[j]) && (k < WORD_LENGTH_MAX));
				word[k] = '\0';  j--;

				/*  SET OUTSIDE COUNTER  */
				*i = j;

				/*  SET OUTSIDE MODE  */
				*mode = current_mode;

				/*  SET STATE TO SILENCE, INCREMENT STATE  */
				*(state_buffer[state++]) = STATE_SILENCE;
			} else if ((current_mode == TAGGING_MODE) && (state == 0)) {
				/*  PUT TAG INTO WORD BUFFER IN CURRENT STATE ONLY  */
				k = 0;
				do {
					word[k++] = buffer[j++];
				} while ((j < length) && !is_mode(buffer[j]) && (k < WORD_LENGTH_MAX));
				word[k] = '\0';  j--;

				/*  SET OUTSIDE COUNTER  */
				*i = j;

				/*  SET OUTSIDE MODE  */
				*mode = current_mode;

				/*  SET STATE TO TAGGING, INCREMENT STATE  */
				*(state_buffer[state++]) = STATE_TAGGING;
			} else if ((current_mode == RAW_MODE) && (state == 0)) {
				/*  EXPAND RAW MODE IN CURRENT STATE ONLY  */
				if (expand_raw_mode(buffer, &j, length, stream) != TTS_PARSER_SUCCESS) {
					return(TTS_PARSER_FAILURE);
				}

				/*  SET RAW_MODE FLAG  */
				*raw_mode_flag = TTS_TRUE;

				/*  SET OUTSIDE COUNTER  */
				*i = j;
			}
			break;
		}

		/*  ONLY NEED TWO STATES  */
		if (state >= 2) {
			return TTS_PARSER_SUCCESS;
		}
	}

	/*  IF HERE, THEN END OF INPUT BUFFER, INDICATE END STATE  */
	if (state == 0) {
		/*  SET STATES  */
		*current_state = STATE_END;
		*next_state = STATE_UNDEFINED;
		/*  BLANK OUT WORD BUFFER  */
		word[0] = '\0';
		/*  SET OUTSIDE COUNTER  */
		*i = j;
		/*  SET OUTSIDE MODE  */
		*mode = current_mode;
	} else {
		*next_state = STATE_END;
	}

	/*  RETURN SUCCESS  */
	return TTS_PARSER_SUCCESS;
}

/******************************************************************************
*
*       function:       set_tone_group
*
*       purpose:        Set the tone group marker according to the punctuation
*                       passed in as "word".  The marker is inserted in the
*                       stream at position "tg_pos".
*
******************************************************************************/
int
set_tone_group(std::stringstream& stream, long tg_pos, const char* word)
{
	/*  RETURN IMMEDIATELY IF tg_pos NOT LEGAL  */
	if (tg_pos == UNDEFINED_POSITION) {
		return TTS_PARSER_FAILURE;
	}

	/*  GET CURRENT POSITION IN STREAM  */
	long current_pos = static_cast<long>(stream.tellp());

	/*  SEEK TO TONE GROUP MARKER POSITION  */
	stream.seekp(tg_pos);

	/*  WRITE APPROPRIATE TONE GROUP TYPE  */
	switch (word[0]) {
	case '.':
		stream << TG_STATEMENT;
		break;
	case '!':
		stream << TG_EXCLAMATION;
		break;
	case '?':
		stream << TG_QUESTION;
		break;
	case ',':
		stream << TG_CONTINUATION;
		break;
	case ';':
		stream << TG_HALF_PERIOD;
		break;
	case ':':
		stream << TG_CONTINUATION;
		break;
	default:
		return TTS_PARSER_FAILURE;
	}

	/*  SEEK TO ORIGINAL POSITION ON STREAM  */
	stream.seekp(current_pos);

	/*  RETURN SUCCESS */
	return TTS_PARSER_SUCCESS;
}

/******************************************************************************
*
*       function:       convert_silence
*
*       purpose:        Converts numeric quantity in "buffer" to appropriate
*                       number of silence phones, which are written onto the
*                       end of stream.  Rounding is performed.  Returns actual
*                       length of silence.
*
******************************************************************************/
float
convert_silence(const char* buffer, std::stringstream& stream)
{
	/*  CONVERT BUFFER TO DOUBLE  */
	double silence_length = strtod(buffer, NULL);

	/*  LIMIT SILENCE LENGTH TO MAXIMUM  */
	silence_length = (silence_length > SILENCE_MAX) ? SILENCE_MAX : silence_length;

	/*  FIND EQUIVALENT NUMBER OF SILENCE PHONES, PERFORMING ROUNDING  */
	int number_silence_phones = (int) rint(silence_length / SILENCE_PHONE_LENGTH);

	/*  PUT IN UTTERANCE BOUNDARY MARKER  */
	stream << UTTERANCE_BOUNDARY << ' ';

	/*  WRITE OUT SILENCE PHONES TO STREAMS  */
	for (int j = 0; j < number_silence_phones; j++) {
		stream << SILENCE_PHONE << ' ';
	}

	/*  RETURN ACTUAL LENGTH OF SILENCE  */
	return static_cast<float>(number_silence_phones * SILENCE_PHONE_LENGTH);
}

/******************************************************************************
*
*       function:       another_word_follows
*
*       purpose:        Returns 1 if another word follows in buffer, after
*                       position i.  Else, 0 is returned.
*
******************************************************************************/
int
another_word_follows(const char* buffer, long i, long length, int mode)
{
	for (long j = i+1; j < length; j++) {
		/*  FILTER THROUGH EACH CHARACTER  */
		switch(buffer[j]) {
		case RAW_MODE_BEGIN:      mode = RAW_MODE;      break;
		case LETTER_MODE_BEGIN:   mode = LETTER_MODE;   break;
		case EMPHASIS_MODE_BEGIN: mode = EMPHASIS_MODE; break;
		case TAGGING_MODE_BEGIN:  mode = TAGGING_MODE;  break;
		case SILENCE_MODE_BEGIN:  mode = SILENCE_MODE;  break;

		case RAW_MODE_END:
		case LETTER_MODE_END:
		case EMPHASIS_MODE_END:
		case TAGGING_MODE_END:
		case SILENCE_MODE_END:    mode = NORMAL_MODE;   break;

		default:
			if ((mode == NORMAL_MODE) || (mode == EMPHASIS_MODE)) {
				/*  WORD HAS BEEN FOUND  */
				if (!is_punctuation(buffer[j])) {
					return 1;
				}
			}
			break;
		}
	}

	/*  IF HERE, THEN NO WORD FOLLOWS  */
	return 0;
}

/******************************************************************************
*
*       function:       shift_silence
*
*       purpose:        Looks past punctuation to see if some silence occurs
*                       before the next word (or raw mode contents), and shifts
*                       the silence to the current point on the stream.  The
*                       the numeric quantity is converted to equivalent silence
*                       phones, and a 1 is returned.  0 is returned otherwise.
*
******************************************************************************/
int
shift_silence(const char* buffer, long i, long length, int mode, std::stringstream& stream)
{
	char word[WORD_LENGTH_MAX + 1];

	for (long j = i + 1; j < length; j++) {
		/*  FILTER THROUGH EACH CHARACTER  */
		switch (buffer[j]) {
		case RAW_MODE_BEGIN:      mode = RAW_MODE;      break;
		case LETTER_MODE_BEGIN:   mode = LETTER_MODE;   break;
		case EMPHASIS_MODE_BEGIN: mode = EMPHASIS_MODE; break;
		case TAGGING_MODE_BEGIN:  mode = TAGGING_MODE;  break;
		case SILENCE_MODE_BEGIN:  mode = SILENCE_MODE;  break;

		case RAW_MODE_END:
		case LETTER_MODE_END:
		case EMPHASIS_MODE_END:
		case TAGGING_MODE_END:
		case SILENCE_MODE_END:    mode = NORMAL_MODE;   break;

		default:
			if ((mode == NORMAL_MODE) || (mode == EMPHASIS_MODE)) {
				/*  SKIP WHITE SPACE  */
				if (buffer[j] == ' ') {
					continue;
				}
				/*  WORD HERE, SO RETURN WITHOUT SHIFTING  */
				if (!is_punctuation(buffer[j])) {
					return 0;
				}
			} else if (mode == RAW_MODE) {
				/*  ASSUME RAW MODE CONTAINS WORD OF SOME SORT  */
				return 0;
			} else if (mode == SILENCE_MODE) {
				/*  COLLECT SILENCE DIGITS INTO WORD BUFFER  */
				int k = 0;
				do {
					word[k++] = buffer[j++];
				} while ((j < length) && !is_mode(buffer[j]) && (k < WORD_LENGTH_MAX));
				word[k] = '\0';
				/*  CONVERT WORD TO SILENCE PHONES, APPENDING TO STREAM  */
				convert_silence(word, stream);
				/*  RETURN, INDICATING SILENCE SHIFTED BACKWARDS  */
				return 1;
			}
			break;
		}
	}

	/*  IF HERE, THEN SILENCE NOT SHIFTED  */
	return 0;
}

/******************************************************************************
*
*       function:       insert_tag
*
*       purpose:        Inserts the tag contained in word onto the stream at
*                       the insert_point.
*
******************************************************************************/
void
insert_tag(std::stringstream& stream, long insert_point, const char* word)
{
	/*  RETURN IMMEDIATELY IF NO INSERT POINT  */
	if (insert_point == UNDEFINED_POSITION) {
		return;
	}

	/*  FIND POSITION OF END OF STREAM  */
	long end_point = static_cast<long>(stream.tellp());

	/*  CALCULATE HOW MANY CHARACTERS TO SHIFT  */
	long length = end_point - insert_point;

	/*  IF LENGTH IS 0, THEN SIMPLY APPEND TAG TO STREAM  */
	if (length == 0) {
		stream << TAG_BEGIN << ' ' << word;
	} else {
		/*  ELSE, SAVE STREAM AFTER INSERT POINT  */
		std::string temp(length, '\0');
		stream.seekg(insert_point);
		for (long j = 0; j < length; j++) {
			char c;
			if (!stream.get(c)) {
				THROW_EXCEPTION(GS::EndOfBufferException, "Could not get a character from the stream.");
			}
			temp[j] = c;
		}

		/*  INSERT TAG; ADD TEMPORARY MATERIAL  */
		stream.seekp(insert_point);
		stream << TAG_BEGIN << ' ' << word << ' ' << temp;
	}
}

/******************************************************************************
*
*       function:       expand_raw_mode
*
*       purpose:        Writes raw mode contents to stream, checking phones
*                       and markers.
*
******************************************************************************/
int
expand_raw_mode(const char *buffer, long* j, long length, std::stringstream& stream)
{
	int k, super_raw_mode = TTS_FALSE, delimiter = TTS_FALSE, blank = TTS_TRUE;
	char token[SYMBOL_LENGTH_MAX+1];

	/*  EXPAND AND CHECK RAW MODE CONTENTS TILL END OF RAW MODE  */
	token[k = 0] = '\0';
	for ( ; (*j < length) && (buffer[*j] != RAW_MODE_END); (*j)++) {
		stream << buffer[*j];
		/*  CHECK IF ENTERING OR EXITING SUPER RAW MODE  */
		if (buffer[*j] == '%') {
			if (!super_raw_mode) {
				if (illegal_token(token)) {
					return TTS_PARSER_FAILURE;
				}
				super_raw_mode = TTS_TRUE;
				token[k = 0] = '\0';
				continue;
			} else {
				super_raw_mode = TTS_FALSE;
				token[k = 0] = '\0';
				delimiter = blank = TTS_FALSE;
				continue;
			}
		}
		/*  EXAMINE SLASH CODES, DELIMITERS, AND PHONES IN REGULAR RAW MODE  */
		if (!super_raw_mode) {
			switch (buffer[*j]) {
			case '/':
				/*  SLASH CODE  */
				/*  EVALUATE PENDING TOKEN  */
				if (illegal_token(token)) {
					return(TTS_PARSER_FAILURE);
				}
				/*  PUT SLASH CODE INTO TOKEN BUFFER  */
				token[0] = '/';
				if ((++(*j) < length) && (buffer[*j] != RAW_MODE_END)) {
					stream << buffer[*j];
					token[1] = buffer[*j];
					token[2] = '\0';
					/*  CHECK LEGALITY OF SLASH CODE  */
					if (illegal_slash_code(token)) {
						return TTS_PARSER_FAILURE;
					}
					/*  CHECK ANY TAG AND TAG NUMBER  */
					if (!strcmp(token,TAG_BEGIN)) {
						if (expand_tag_number(buffer, j, length, stream) == TTS_PARSER_FAILURE) {
							return TTS_PARSER_FAILURE;
						}
					}
					/*  RESET FLAGS  */
					token[k = 0] = '\0';
					delimiter = blank = TTS_FALSE;
				} else {
					return TTS_PARSER_FAILURE;
				}
				break;
			case '_':
			case '.':
				/*  SYLLABLE DELIMITERS  */
				/*  DON'T ALLOW REPEATED DELIMITERS, OR DELIMITERS AFTER BLANK  */
				if (delimiter || blank) {
					return TTS_PARSER_FAILURE;
				}
				delimiter++;
				blank = TTS_FALSE;
				/*  EVALUATE PENDING TOKEN  */
				if (illegal_token(token)) {
					return TTS_PARSER_FAILURE;
				}
				/*  RESET FLAGS  */
				token[k = 0] = '\0';
				break;
			case ' ':
				/*  WORD DELIMITER  */
				/*  DON'T ALLOW SYLLABLE DELIMITER BEFORE BLANK  */
				if (delimiter) {
					return TTS_PARSER_FAILURE;
				}
				/*  SET FLAGS  */
				blank++;
				delimiter = TTS_FALSE;
				/*  EVALUATE PENDING TOKEN  */
				if (illegal_token(token)) {
					return TTS_PARSER_FAILURE;
				}
				/*  RESET FLAGS  */
				token[k = 0] = '\0';
				break;
			default:
				/*  PHONE SYMBOL  */
				/*  RESET FLAGS  */
				delimiter = blank = TTS_FALSE;
				/*  ACCUMULATE PHONE SYMBOL IN TOKEN BUFFER  */
				token[k++] = buffer[*j];
				if (k <= SYMBOL_LENGTH_MAX) {
					token[k] = '\0';
				} else {
					return TTS_PARSER_FAILURE;
				}
				break;
			}
		}
	}

	/*  CHECK ANY REMAINING TOKENS  */
	if (illegal_token(token)) {
		return TTS_PARSER_FAILURE;
	}
	/*  CANNOT END WITH A DELIMITER  */
	if (delimiter) {
		return TTS_PARSER_FAILURE;
	}

	/*  PAD WITH SPACE, RESET EXTERNAL COUNTER  */
	stream << ' ';
	(*j)--;

	/*  RETURN SUCCESS  */
	return TTS_PARSER_SUCCESS;
}

/******************************************************************************
*
*       function:       illegal_token
*
*       purpose:        Returns 1 if token is not a valid DEGAS phone.
*                       Otherwise, 0 is returned.
*
******************************************************************************/
int
illegal_token(const char* token)
{
	/*  RETURN IMMEDIATELY IF ZERO LENGTH STRING  */
	if (strlen(token) == 0) {
		return 0;
	}

	/*  IF PHONE A VALID DEGAS PHONE, RETURN 0;  1 OTHERWISE  */
	if (1 /*validPhone(token)*/) { //TODO: implement
		return 0;
	} /*else {
		return 1;
	}*/
}

/******************************************************************************
*
*       function:       illegal_slash_code
*
*       purpose:        Returns 1 if code is illegal, 0 otherwise.
*
******************************************************************************/
int
illegal_slash_code(const char* code)
{
	int i = 0;
	 const char* legal_code[] = {
		CHUNK_BOUNDARY,TONE_GROUP_BOUNDARY,FOOT_BEGIN,
		TONIC_BEGIN,SECONDARY_STRESS,LAST_WORD,TAG_BEGIN,
		WORD_BEGIN,TG_STATEMENT,TG_EXCLAMATION,TG_QUESTION,
		TG_CONTINUATION,TG_HALF_PERIOD,NULL
	};

	/*  COMPARE CODE WITH LEGAL CODES, RETURN 0 IMMEDIATELY IF A MATCH  */
	while (legal_code[i] != NULL) {
		if (!strcmp(legal_code[i++], code)) {
			return 0;
		}
	}

	/*  IF HERE, THEN NO MATCH;  RETURN 1, INDICATING ILLEGAL CODE  */
	return 1;
}

/******************************************************************************
*
*       function:       expand_tag_number
*
*       purpose:        Expand tag number in buffer at position j and write to
*                       stream.  Perform error checking, returning error code
*                       if format of tag number is illegal.
*
******************************************************************************/
int
expand_tag_number(const char* buffer, long* j, long length, std::stringstream& stream)
{
	/*  SKIP WHITE  */
	while ((((*j)+1) < length) && (buffer[(*j)+1] == ' ')) {
		(*j)++;
		stream << buffer[*j];
	}

	/*  CHECK FORMAT OF TAG NUMBER  */
	int sign = 0;
	while ((((*j)+1) < length) && (buffer[(*j)+1] != ' ') &&
			(buffer[(*j)+1] != RAW_MODE_END) && (buffer[(*j)+1] != '%')) {
		stream << buffer[++(*j)];
		if ((buffer[*j] == '-') || (buffer[*j] == '+')) {
			if (sign) {
				return TTS_PARSER_FAILURE;
			}
			sign++;
		} else if (!isdigit(buffer[*j])) {
			return TTS_PARSER_FAILURE;
		}
	}

	/*  RETURN SUCCESS  */
	return TTS_PARSER_SUCCESS;
}

/******************************************************************************
*
*       function:       is_mode
*
*       purpose:        Returns 1 if character is a mode marker,
*                       0 otherwise.
*
******************************************************************************/
int
is_mode(char c)
{
  if ((c >= SILENCE_MODE_END) && (c <= RAW_MODE_BEGIN))
    return(1);
  else
    return(0);
}

/******************************************************************************
*
*       function:       is_isolated
*
*       purpose:        Returns 1 if character at position i is isolated,
*                       i.e. is surrounded by space or mode marker.  Returns
*                       0 otherwise.
*
******************************************************************************/
int
is_isolated(char *buffer, int i, int len)
{
  if ( ((i == 0) || (((i-1) >= 0) && (is_mode(buffer[i-1]) || (buffer[i-1] == ' ')))) &&
       ((i == (len-1)) || (((i+1) < len) && (is_mode(buffer[i+1]) || (buffer[i+1] == ' ')))))
    return(1);
  else
    return(0);
}

/******************************************************************************
*
*       function:       part_of_number
*
*       purpose:        Returns 1 if character at position i is part of
*                       a number (including mixtures with non-numeric
*                       characters).  Returns 0 otherwise.
*
******************************************************************************/
int
part_of_number(char *buffer, int i, int len)
{
  while( (--i >= 0) && (buffer[i] != ' ') && (buffer[i] != DELETED) && (!is_mode(buffer[i])) )
    if (isdigit(buffer[i]))
      return(1);

  while( (++i < len) && (buffer[i] != ' ') && (buffer[i] != DELETED) && (!is_mode(buffer[i])) )
    if (isdigit(buffer[i]))
      return(1);

  return(0);
}

/******************************************************************************
*
*       function:       number_follows
*
*       purpose:        Returns a 1 if at least one digit follows the character
*                       at position i, up to white space or mode marker.
*                       Returns 0 otherwise.
*
******************************************************************************/
int
number_follows(char *buffer, int i, int len)
{
  while( (++i < len) && (buffer[i] != ' ') &&
	 (buffer[i] != DELETED) && (!is_mode(buffer[i])) )
    if (isdigit(buffer[i]))
      return(1);

  return(0);
}

/******************************************************************************
*
*       function:       delete_ellipsis
*
*       purpose:        Deletes three dots in a row (disregarding white
*                       space).  If four dots, then the last three are
*                       deleted.
*
******************************************************************************/
void
delete_ellipsis(char *buffer, int *i, int length)
{
  /*  SET POSITION OF FIRST DOT  */
  int pos1 = *i, pos2, pos3;

  /*  IGNORE ANY WHITE SPACE  */
  while (((*i+1) < length) && (buffer[*i+1] == ' '))
    (*i)++;
  /*  CHECK FOR 2ND DOT  */
  if (((*i+1) < length) && (buffer[*i+1] == '.')) {
    pos2 = ++(*i);
    /*  IGNORE ANY WHITE SPACE  */
    while (((*i+1) < length) && (buffer[*i+1] == ' '))
      (*i)++;
    /*  CHECK FOR 3RD DOT  */
    if (((*i+1) < length) && (buffer[*i+1] == '.')) {
      pos3 = ++(*i);
      /*  IGNORE ANY WHITE SPACE  */
      while (((*i+1) < length) && (buffer[*i+1] == ' '))
	(*i)++;
      /*  CHECK FOR 4TH DOT  */
      if (((*i+1) < length) && (buffer[*i+1] == '.'))
	buffer[pos2] = buffer[pos3] = buffer[++(*i)] = DELETED;
      else
	buffer[pos1] = buffer[pos2] = buffer[pos3] = DELETED;
    }
  }
}

/******************************************************************************
*
*       function:       convert_dash
*
*       purpose:        Converts "--" to ", ", and "---" to ",  "
*                       Returns 1 if this is done, 0 otherwise.
*
******************************************************************************/
int
convert_dash(char *buffer, int *i, int length)
{
  /*  SET POSITION OF INITIAL DASH  */
  int pos1 = *i;

  /*  CHECK FOR 2ND DASH  */
  if (((*i+1) < length) && (buffer[*i+1] == '-')) {
    buffer[pos1] = ',';
    buffer[++(*i)] = DELETED;
    /*  CHECK FOR 3RD DASH  */
    if (((*i+1) < length) && (buffer[*i+1] == '-'))
      buffer[++(*i)] = DELETED;
    return(1);
  }

  /*  RETURN ZERO IF NOT CONVERTED  */
  return(0);
}

/******************************************************************************
*
*       function:       is_telephone_number
*
*       purpose:        Returns 1 if string at position i in buffer is of the
*                       form:  (ddd)ddd-dddd
*                       where each d is a digit.
*
******************************************************************************/
int
is_telephone_number(char *buffer, int i, int length)
{
  /*  CHECK FORMAT: (ddd)ddd-dddd  */
  if ( ((i+12) < length) &&
	isdigit(buffer[i+1]) && isdigit(buffer[i+2]) && isdigit(buffer[i+3]) &&
	(buffer[i+4] == ')') &&
	isdigit(buffer[i+5]) && isdigit(buffer[i+6]) && isdigit(buffer[i+7]) &&
	(buffer[i+8] == '-') &&
	isdigit(buffer[i+9]) && isdigit(buffer[i+10]) &&
	isdigit(buffer[i+11]) && isdigit(buffer[i+12]) ) {
    /*  MAKE SURE STRING ENDS WITH WHITE SPACE, MODE, OR PUNCTUATION  */
    if ( ((i+13) == length) ||
	 ( ((i+13) < length) &&
	   (
	     is_punctuation(buffer[i+13]) || is_mode(buffer[i+13]) ||
	     (buffer[i+13] == ' ') || (buffer[i+13] == DELETED)
	   )
	 )
       )
      /*  RETURN 1 IF ALL ABOVE CONDITIONS ARE MET  */
      return(1);
  }
  /*  IF HERE, THEN STRING IS NOT IN SPECIFIED FORMAT  */
  return(0);
}

/******************************************************************************
*
*       function:       is_punctuation
*
*       purpose:        Returns 1 if character is a .,;:?!
*                       Returns 0 otherwise.
*
******************************************************************************/
int
is_punctuation(char c)
{
  switch(c) {
    case '.':
    case ',':
    case ';':
    case ':':
    case '?':
    case '!':
      return(1);
    default:
      return(0);
  }
}

/******************************************************************************
*
*       function:       word_follows
*
*       purpose:        Returns a 1 if a word or speakable symbol (letter mode)
*                       follows the position i in buffer.  Raw, tagging, and
*                       silence mode contents are ignored.  Returns a 0 if any
*                       punctuation (except . as part of number) follows.
*
******************************************************************************/
int
word_follows(const char* buffer, int i, int length)
{
	int mode = NORMAL_MODE;

	for (int j = i + 1; j < length; j++) {
		switch(buffer[j]) {
		case RAW_MODE_BEGIN:      mode = RAW_MODE;      break;
		case LETTER_MODE_BEGIN:   mode = LETTER_MODE;   break;
		case EMPHASIS_MODE_BEGIN: mode = EMPHASIS_MODE; break;
		case TAGGING_MODE_BEGIN:  mode = TAGGING_MODE;  break;
		case SILENCE_MODE_BEGIN:  mode = SILENCE_MODE;  break;
		case RAW_MODE_END:
		case LETTER_MODE_END:
		case EMPHASIS_MODE_END:
		case TAGGING_MODE_END:
		case SILENCE_MODE_END:    mode = NORMAL_MODE;   break;
		default:
			switch(mode) {
			case NORMAL_MODE:
			case EMPHASIS_MODE:
				/*  IGNORE WHITE SPACE  */
				if ((buffer[j] == ' ') || (buffer[j] == DELETED)) {
					continue;
				} else if (is_punctuation(buffer[j])) {
					/*  PUNCTUATION MEANS NO WORD FOLLOWS (UNLESS PERIOD PART OF NUMBER)  */

					if ((buffer[j] == '.') && ((j+1) < length) && isdigit(buffer[j+1])) {
						return 1;
					} else {
						return 0;
					}
				} else { /*  ELSE, SOME WORD FOLLOWS  */
					return 1;
				}
			case LETTER_MODE:
				/*  IF LETTER MODE CONTAINS ANY SYMBOLS, THEN RETURN 1  */
				return 1;
			case RAW_MODE:
			case SILENCE_MODE:
			case TAGGING_MODE:
				/*  IGNORE CONTENTS OF RAW, SILENCE, AND TAGGING MODE  */
				continue;
			}
		}
	}

	/*  IF HERE, THEN A FOLLOWING WORD NOT FOUND  */
	return 0;
}

/******************************************************************************
*
*       function:       expand_abbreviation
*
*       purpose:        Expands listed abbreviations.  Two lists are used (see
*                       abbreviations.h):  one list expands unconditionally,
*                       the other only if the abbreviation is followed by a
*                       number.  The abbreviation p. is expanded to page.
*                       Single alphabetic characters have periods deleted, but
*                       no expansion is made.  They are also capitalized.
*                       Returns 1 if expansion made (i.e. period is deleted),
*                       0 otherwise.
*
******************************************************************************/
int
expand_abbreviation(char* buffer, int i, int length, std::stringstream& stream)
{
	int j, k, word_length = 0;
	char word[5];

	/*  DELETE PERIOD AFTER SINGLE CHARACTER (EXCEPT p.)  */
	if ( ((i-1) == 0) ||  ( ((i-2) >= 0) &&
				( (buffer[i-2] == ' ') || (buffer[i-2] == '.') || (is_mode(buffer[i-2])) )
				) ) {
		if (isalpha(buffer[i-1])) {
			if ((buffer[i-1] == 'p') && (((i-1) == 0) || (((i-2) >= 0) && (buffer[i-2] != '.')) ) ) {
				/*  EXPAND p. TO page  */
				stream.seekp(-1, std::ios_base::cur);
				stream << "page ";
			} else {
				/*  ELSE, CAPITALIZE CHARACTER IF NECESSARY, BLANK OUT PERIOD  */
				stream.seekp(-1, std::ios_base::cur);
				if (islower(buffer[i-1])) {
					buffer[i-1] = toupper(buffer[i-1]);
				}
				stream << buffer[i-1] << ' ';
			}
			/*  INDICATE ABBREVIATION EXPANDED  */
			return 1;
		}
	}

	/*  GET LENGTH OF PRECEDING ISOLATED STRING, UP TO 4 CHARACTERS  */
	for (j = 2; j <= 4; j++) {
		if (((i-j) == 0) ||
				(((i-(j+1)) >= 0) && ((buffer[i-(j+1)] == ' ') || (is_mode(buffer[i-(j+1)]))) ) ) {
			if (isalpha(buffer[i-j]) && isalpha(buffer[i-j+1])) {
				word_length = j;
				break;
			}
		}
	}

	/*  IS ABBREVIATION ONLY IF WORD LENGTH IS 2, 3, OR 4 CHARACTERS  */
	if ((word_length >= 2) && (word_length <= 4)) {
		/*  GET ABBREVIATION  */
		for (k = 0, j = i - word_length; k < word_length; k++) {
			word[k] = buffer[j++];
		}
		word[k] = '\0';

		/*  EXPAND THESE ABBREVIATIONS ONLY IF FOLLOWED BY NUMBER  */
		for (j = 0; abbr_with_number[j][ABBREVIATION] != NULL; j++) {
			if (!strcmp(abbr_with_number[j][ABBREVIATION],word)) {
				/*  IGNORE WHITE SPACE  */
				while (((i+1) < length) && ((buffer[i+1] == ' ') || (buffer[i+1] == DELETED))) {
					i++;
				}
				/*  EXPAND ONLY IF NUMBER FOLLOWS  */
				if (number_follows(buffer, i, length)) {
					stream.seekp(-word_length, std::ios_base::cur);
					stream << abbr_with_number[j][EXPANSION] << ' ';
					return 1;
				}
			}
		}

		/*  EXPAND THESE ABBREVIATIONS UNCONDITIONALLY  */
		for (j = 0; abbreviation[j][ABBREVIATION] != NULL; j++) {
			if (!strcmp(abbreviation[j][ABBREVIATION],word)) {
				stream.seekp(-word_length, std::ios_base::cur);
				stream << abbreviation[j][EXPANSION] << ' ';
				return 1;
			}
		}
	}

	/*  IF HERE, THEN NO EXPANSION MADE  */
	return 0;
}

/******************************************************************************
*
*       function:       expand_letter_mode
*
*       purpose:        Expands contents of letter mode string to word or
*                       words.  A comma is added after each expansion, except
*                       the last letter when it is followed by punctuation.
*
******************************************************************************/
void
expand_letter_mode(const char* buffer, int* i, int length, std::stringstream& stream, int* status)
{
	for ( ; ((*i) < length) && (buffer[*i] != LETTER_MODE_END); (*i)++) {
		/*  CONVERT LETTER TO WORD OR WORDS  */
		switch (buffer[*i]) {
		case ' ': stream << "blank";                break;
		case '!': stream << "exclamation point";    break;
		case '"': stream << "double quote";         break;
		case '#': stream << "number sign";          break;
		case '$': stream << "dollar";               break;
		case '%': stream << "percent";              break;
		case '&': stream << "ampersand";            break;
		case '\'':stream << "single quote";         break;
		case '(': stream << "open parenthesis";     break;
		case ')': stream << "close parenthesis";    break;
		case '*': stream << "asterisk";             break;
		case '+': stream << "plus sign";            break;
		case ',': stream << "comma";                break;
		case '-': stream << "hyphen";               break;
		case '.': stream << "period";               break;
		case '/': stream << "slash";                break;
		case '0': stream << "zero";                 break;
		case '1': stream << "one";                  break;
		case '2': stream << "two";                  break;
		case '3': stream << "three";                break;
		case '4': stream << "four";                 break;
		case '5': stream << "five";                 break;
		case '6': stream << "six";                  break;
		case '7': stream << "seven";                break;
		case '8': stream << "eight";                break;
		case '9': stream << "nine";                 break;
		case ':': stream << "colon";                break;
		case ';': stream << "semicolon";            break;
		case '<': stream << "open angle bracket";   break;
		case '=': stream << "equal sign";           break;
		case '>': stream << "close angle bracket";  break;
		case '?': stream << "question mark";        break;
		case '@': stream << "at sign";              break;
		case 'A':
		case 'a': stream << 'A';                    break;
		case 'B':
		case 'b': stream << 'B';                    break;
		case 'C':
		case 'c': stream << 'C';                    break;
		case 'D':
		case 'd': stream << 'D';                    break;
		case 'E':
		case 'e': stream << 'E';                    break;
		case 'F':
		case 'f': stream << 'F';                    break;
		case 'G':
		case 'g': stream << 'G';                    break;
		case 'H':
		case 'h': stream << 'H';                    break;
		case 'I':
		case 'i': stream << 'I';                    break;
		case 'J':
		case 'j': stream << 'J';                    break;
		case 'K':
		case 'k': stream << 'K';                    break;
		case 'L':
		case 'l': stream << 'L';                    break;
		case 'M':
		case 'm': stream << 'M';                    break;
		case 'N':
		case 'n': stream << 'N';                    break;
		case 'O':
		case 'o': stream << 'O';                    break;
		case 'P':
		case 'p': stream << 'P';                    break;
		case 'Q':
		case 'q': stream << 'Q';                    break;
		case 'R':
		case 'r': stream << 'R';                    break;
		case 'S':
		case 's': stream << 'S';                    break;
		case 'T':
		case 't': stream << 'T';                    break;
		case 'U':
		case 'u': stream << 'U';                    break;
		case 'V':
		case 'v': stream << 'V';                    break;
		case 'W':
		case 'w': stream << 'W';                    break;
		case 'X':
		case 'x': stream << 'X';                    break;
		case 'Y':
		case 'y': stream << 'Y';                    break;
		case 'Z':
		case 'z': stream << 'Z';                    break;
		case '[': stream << "open square bracket";  break;
		case '\\':stream << "back slash";           break;
		case ']': stream << "close square bracket"; break;
		case '^': stream << "caret";                break;
		case '_': stream << "under score";          break;
		case '`': stream << "grave accent";         break;
		case '{': stream << "open brace";           break;
		case '|': stream << "vertical bar";         break;
		case '}': stream << "close brace";          break;
		case '~': stream << "tilde";                break;
		default:  stream << "unknown";              break;
		}
		/*  APPEND COMMA, UNLESS PUNCTUATION FOLLOWS LAST LETTER  */
		if ( (((*i)+1) < length) &&
				(buffer[(*i)+1] == LETTER_MODE_END) &&
				!word_follows(buffer, (*i), length)) {
			stream << ' ';
			*status = WORD;
		} else {
			stream << ", ";
			*status = PUNCTUATION;
		}
	}
	/*  BE SURE TO SET INDEX BACK ONE, SO CALLING ROUTINE NOT FOULED UP  */
	(*i)--;
}

/******************************************************************************
*
*       function:       is_all_upper_case
*
*       purpose:        Returns 1 if all letters of the word are upper case,
*                       0 otherwise.
*
******************************************************************************/
int
is_all_upper_case(const char* word)
{
	while (*word) {
		if (!isupper(*word)) {
			return 0;
		}
		word++;
	}

	return 1;
}

/******************************************************************************
*
*       function:       to_lower_case
*
*       purpose:        Converts any upper case letter in word to lower case.
*
******************************************************************************/
char*
to_lower_case(char* word)
{
  char *ptr = word;

  while (*ptr) {
    if (isupper(*ptr))
      *ptr = tolower(*ptr);
    ptr++;
  }

  return(word);
}

/******************************************************************************
*
*       function:       is_special_acronym
*
*       purpose:        Returns a pointer to the pronunciation of a special
*                       acronym if it is defined in the list.  Otherwise,
*                       NULL is returned.
*
******************************************************************************/
const char*
is_special_acronym(const char* word)
{
	const char* acronym;

	/*  LOOP THROUGH LIST UNTIL MATCH FOUND, RETURN PRONUNCIATION  */
	for (int i = 0; (acronym = special_acronym[i][WORD]); i++) {
		if (!strcmp(word, acronym)) {
			return special_acronym[i][PRONUNCIATION];
		}
	}

	/*  IF HERE, NO SPECIAL ACRONYM FOUND, RETURN NULL  */
	return nullptr;
}

/******************************************************************************
*
*       function:       contains_primary_stress
*
*       purpose:        Returns 1 if the pronunciation contains ' (and ` for
*                       backwards compatibility).  Otherwise 0 is returned.
*
******************************************************************************/
int
contains_primary_stress(const char *pronunciation)
{
  for ( ; *pronunciation && (*pronunciation != '%'); pronunciation++)
    if ((*pronunciation == '\'') || (*pronunciation == '`'))
      return(TTS_YES);

  return(TTS_NO);
}

/******************************************************************************
*
*       function:       converted_stress
*
*       purpose:        Returns 1 if the first " is converted to a ',
*                       otherwise 0 is returned.
*
******************************************************************************/
int
converted_stress(char *pronunciation)
{
  /*  LOOP THRU PRONUNCIATION UNTIL " FOUND, REPLACE WITH '  */
  for ( ; *pronunciation && (*pronunciation != '%'); pronunciation++)
    if (*pronunciation == '"') {
      *pronunciation = '\'';
      return(TTS_YES);
    }

  /*  IF HERE, NO " FOUND  */
  return(TTS_NO);
}

/******************************************************************************
*
*       function:       is_possessive
*
*       purpose:        Returns 1 if 's is found at end of word, and removes
*                       the 's ending from the word.  Otherwise, 0 is returned.
*
******************************************************************************/
int
is_possessive(char* word)
{
	/*  LOOP UNTIL 's FOUND, REPLACE ' WITH NULL  */
	for ( ; *word; word++) {
		if ((*word == '\'') && *(word+1) && (*(word+1) == 's') && (*(word+2) == '\0')) {
			*word = '\0';
			return TTS_YES;
		}
	}

	/*  IF HERE, NO 's FOUND, RETURN FAILURE  */
	return TTS_NO;
}

/******************************************************************************
*
*       function:       safety_check
*
*       purpose:        Checks to make sure that there are not too many feet
*                       phones per chunk.  If there are, the input is split
*                       into two or mor chunks.
*
******************************************************************************/
void
safety_check(std::stringstream& stream, long* stream_length)
{
	int number_of_feet = 0, number_of_phones = 0, state = NON_PHONEME;
	long last_word_pos = UNDEFINED_POSITION, last_tg_pos = UNDEFINED_POSITION;
	char last_tg_type = '0';
	char c;

	/*  REWIND STREAM TO BEGINNING  */
	stream.seekg(0);

	/*  LOOP THROUGH STREAM, INSERTING NEW CHUNK MARKERS IF NECESSARY  */
	while (stream.get(c) && c != '\0') {
		switch (c) {
		case '%':
			/*  IGNORE SUPER RAW MODE CONTENTS  */
			while (stream.get(c) && c != '%') {
				if (c == '\0') {
					stream.unget();
					break;
				}
			}
			state = NON_PHONEME;
			break;
		case '/':
			/*  SLASH CODES  */
			if (!stream.get(c)) {
				THROW_EXCEPTION(GS::EndOfBufferException, "Could not get a character from the stream.");
			}
			switch (c) {
			case 'c':
				/*  CHUNK MARKER (/c)  */
				number_of_feet = number_of_phones = 0;
				break;
			case '_':
			case '*':
				/*  FOOT AND TONIC FOOT MARKERS  */
				if (++number_of_feet > MAX_FEET_PER_CHUNK) {
					/*  SPLIT STREAM INTO TWO CHUNKS  */
					insert_chunk_marker(stream, last_word_pos, last_tg_type);
					set_tone_group(stream, last_tg_pos, ",");
					check_tonic(stream, last_tg_pos, last_word_pos);
				}
				break;
			case 't':
				/*  IGNORE TAGGING MODE CONTENTS  */
				/*  SKIP WHITE  */
				while (stream.get(c) && c == ' ')
					;
				stream.unget();
				/*  SKIP OVER TAG NUMBER  */
				while (stream.get(c) && c != ' ') {
					if (c == '\0') {
						stream.unget();
						break;
					}
				}
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
				/*  REMEMBER TONE GROUP TYPE AND POSITION  */
				last_tg_type = c;
				last_tg_pos = static_cast<long>(stream.tellg()) - 2;
				break;
			default:
				/*  IGNORE ALL OTHER SLASH CODES  */
				break;
			}
			state = NON_PHONEME;
			break;
		case '.':
		case '_':
		case ' ':
			/*  END OF PHONE (AND WORD) DELIMITERS  */
			if (state == PHONEME) {
				if (++number_of_phones > MAX_PHONES_PER_CHUNK) {
					/*  SPLIT STREAM INTO TWO CHUNKS  */
					insert_chunk_marker(stream, last_word_pos, last_tg_type);
					set_tone_group(stream, last_tg_pos, ",");
					check_tonic(stream, last_tg_pos, last_word_pos);
					state = NON_PHONEME;
					break;
				}
				if (c == ' ') {
					last_word_pos = static_cast<long>(stream.tellg());
				}
			}
			state = NON_PHONEME;
			break;
		default:
			state = PHONEME;
			break;
		}
	}

	/*  BE SURE TO RESET LENGTH OF STREAM  */
	*stream_length = static_cast<long>(stream.tellg());
}

/******************************************************************************
*
*       function:       insert_chunk_marker
*
*       purpose:        Insert chunk markers and associated markers in the
*                       stream at the insert point.  Use the tone group type
*                       passed in as an argument.
*
******************************************************************************/
void
insert_chunk_marker(std::stringstream& stream, long insert_point, char tg_type)
{
	char c;
	std::stringstream temp_stream;

	/*  COPY STREAM FROM INSERT POINT TO END TO BUFFER TO ANOTHER STREAM  */
	stream.seekg(insert_point);
	while (stream.get(c) && c != '\0') {
		temp_stream << c;
	}
	temp_stream << '\0';

	/*  PUT IN MARKERS AT INSERT POINT  */
	stream.seekp(insert_point);
	stream << TONE_GROUP_BOUNDARY << ' ' << CHUNK_BOUNDARY << ' '
		<< TONE_GROUP_BOUNDARY << " /" << tg_type << ' ';
	long new_position = static_cast<long>(stream.tellp()) - 9; //TODO: check

	/*  APPEND CONTENTS OF TEMPORARY STREAM  */
	temp_stream.seekg(0);
	while (temp_stream.get(c) && c != '\0') {
		stream << c;
	}
	stream << '\0';

	/*  POSITION THE STREAM AT THE NEW /c MARKER  */
	stream.seekp(new_position);
}

/******************************************************************************
*
*       function:       check_tonic
*
*       purpose:        Checks to see if a tonic marker is present in the
*                       stream between the start and end positions.  If no
*                       tonic is present, then put one in at the last foot
*                       marker if it exists.
*
******************************************************************************/
void
check_tonic(std::stringstream& stream, long start_pos, long end_pos)
{
	long i, last_foot_pos = UNDEFINED_POSITION;

	/*  REMEMBER CURRENT POSITION IN STREAM  */
	long temp_pos = static_cast<long>(stream.tellp());

	/*  CALCULATE EXTENT OF STREAM TO LOOP THROUGH  */
	long extent = end_pos - start_pos;

	/*  REWIND STREAM TO START POSITION  */
	stream.seekg(start_pos);

	/*  LOOP THROUGH STREAM, DETERMINING LAST FOOT POSITION, AND PRESENCE OF TONIC  */
	char c;
	for (i = 0; i < extent; i++) {
		if (stream.get(c) && c == '/' && ++i < extent) {
			if (!stream.get(c)) {
				THROW_EXCEPTION(GS::EndOfBufferException, "Could not get a character from the stream.");
			}
			switch (c) {
			case '_':
				last_foot_pos = static_cast<long>(stream.tellg()) - 1;
				break;
			case '*':
				/*  GO TO ORIGINAL POSITION ON STREAM, AND RETURN IMMEDIATELY  */
				//NXSeek(stream, temp_pos, NX_FROMSTART);
				return;
			}
		}
	}

	/*  IF HERE, NO TONIC, SO INSERT TONIC MARKER  */
	if (last_foot_pos != UNDEFINED_POSITION) {
		stream.seekp(last_foot_pos);
		stream << '*';
	}

	/*  GO TO ORIGINAL POSITION ON STREAM  */
	stream.seekp(temp_pos);
}

} /* namespace */

//==============================================================================

namespace GS {
namespace En {

TextParser::TextParser(const char* configDirPath,
			const std::string& dictionary1Path,
			const std::string& dictionary2Path,
			const std::string& dictionary3Path)
		: escape_character_(DEFAULT_ESCAPE_CHARACTER)
{
	if (dictionary1Path != "none") {
		dict1_.reset(new DictionarySearch);
		std::ostringstream filePath;
		filePath << configDirPath << '/' << dictionary1Path;
		dict1_->load(filePath.str().c_str());
	}
	if (dictionary2Path != "none") {
		dict2_.reset(new DictionarySearch);
		std::ostringstream filePath;
		filePath << configDirPath << '/' << dictionary2Path;
		dict2_->load(filePath.str().c_str());
	}
	if (dictionary3Path != "none") {
		dict3_.reset(new DictionarySearch);
		std::ostringstream filePath;
		filePath << configDirPath << '/' << dictionary3Path;
		dict3_->load(filePath.str().c_str());
	}

	dictionaryOrder_[0] = TTS_NUMBER_PARSER;
	dictionaryOrder_[1] = TTS_DICTIONARY_1;
	dictionaryOrder_[2] = TTS_DICTIONARY_2;
	dictionaryOrder_[3] = TTS_DICTIONARY_3;
	dictionaryOrder_[4] = TTS_LETTER_TO_SOUND;
	dictionaryOrder_[5] = TTS_EMPTY;
}

TextParser::~TextParser()
{
}

/******************************************************************************
*
*       function:       init_parser_module
*
*       purpose:        Sets up parser module for subsequent use.  This must
*                       be called before parser() is ever used.
*
******************************************************************************/
void
TextParser::init_parser_module()
{
	auxStream_.str("");
}

/******************************************************************************
*
*       function:       set_escape_code
*
*       purpose:        Sets escape code for parsing.  Assumes Objective C
*                       client library checks validity of argument.
*
******************************************************************************/
int
TextParser::set_escape_code(char new_escape_code)
{
	/*  SET GLOBAL ESCAPE CHARACTER  */
	escape_character_ = new_escape_code;

	/*  RETURN SUCCESS  */
	return TTS_PARSER_SUCCESS;
}

/******************************************************************************
*
*       function:       parseText
*
*       purpose:        Takes plain english input, and produces phonetic
*                       output suitable for further processing in the TTS
*                       system.  If a parse error occurs, a value of 0 or
*                       above is returned.  Usually this will point to the
*                       position of the error in the input buffer, but in
*                       later stages of the parse only a 0 is returned since
*                       positional information is lost.  If no parser error,
*                       then TTS_PARSER_SUCCESS is returned.
*
******************************************************************************/
std::string
TextParser::parseText(const char* text)
{
	int error;
	int input_length, buffer1_length, buffer2_length;
	long stream1_length, auxStream_length;

	auxStream_.str("");

	/*  FIND LENGTH OF INPUT  */
	input_length = strlen(text);

	/*  ALLOCATE BUFFER1, BUFFER2  */
	std::vector<char> buffer1(input_length + 1);
	std::vector<char> buffer2(input_length + 1);

	if (Log::debugEnabled) {
		printf("text=%s\n", text);
	}

	/*  CONDITION INPUT:  CONVERT NON-PRINTABLE CHARS TO SPACES
	    (EXCEPT ESC CHAR), CONNECT WORDS HYPHENATED OVER A NEWLINE  */
	condition_input(text, &buffer1[0], input_length, &buffer1_length);

	if (Log::debugEnabled) {
		printf("buffer1=%s\n", &buffer1[0]);
	}

	/*  RATIONALIZE MODE MARKINGS, CHECKING FOR ERRORS  */
	if ((error = mark_modes(&buffer1[0], &buffer2[0], buffer1_length, &buffer2_length))
			!= TTS_PARSER_SUCCESS) {
		THROW_EXCEPTION(TextParserException, "Error in mark_modes();");
	}

	if (Log::debugEnabled) {
		printf("buffer2=%s\n", &buffer2[0]);
	}

	std::stringstream stream1;

	/*  STRIP OUT OR CONVERT UNESSENTIAL PUNCTUATION  */
	strip_punctuation(&buffer2[0], buffer2_length, stream1, &stream1_length);

	if (Log::debugEnabled) {
		/*  PRINT STREAM 1  */
		printf("\nSTREAM 1\n");
		print_stream(stream1, stream1_length);
	}

	// Clear the auxiliary stream.
	auxStream_.str("");

	/*  DO FINAL CONVERSION  */
	if ((error = final_conversion(stream1, stream1_length, auxStream_, &auxStream_length))
			!= TTS_PARSER_SUCCESS) {
		THROW_EXCEPTION(TextParserException, "Error in final_conversion();");
	}

	/*  DO SAFETY CHECK;  MAKE SURE NOT TOO MANY FEET OR PHONES PER CHUNK  */
	safety_check(auxStream_, &auxStream_length);

	if (Log::debugEnabled) {
		/*  PRINT OUT STREAM 2  */
		printf("STREAM 2\n");
		print_stream(auxStream_, auxStream_length);
	}

	/*  SET OUTPUT POINTER TO MEMORY STREAM BUFFER
	    THIS STREAM PERSISTS BETWEEN CALLS  */
	std::string phoneticString = auxStream_.str();
	return phoneticString.substr(0, phoneticString.size() - 1); // the last character is '\0'
}

/******************************************************************************
*
*       function:       lookup_word
*
*       purpose:        Returns the pronunciation of word, and sets dict to
*                       the dictionary in which it was found.  Relies on the
*                       global dictionaryOrder.
*
******************************************************************************/
const char*
TextParser::lookup_word(const char* word, short* dict)
{
	if (Log::debugEnabled) {
		printf("lookup_word word: %s\n", word);
	}

	/*  SEARCH DICTIONARIES IN USER ORDER TILL PRONUNCIATION FOUND  */
	for (int i = 0; i < DICTIONARY_ORDER_SIZE; i++) {
		switch(dictionaryOrder_[i]) {
		case TTS_EMPTY:
			break;
		case TTS_NUMBER_PARSER:
			{
				const char* pron = numberParser_.parseNumber(word, NumberParser::NORMAL);
				if (pron != nullptr) {
					*dict = TTS_NUMBER_PARSER;
					return pron;
				}
			}
			break;
		case TTS_DICTIONARY_1:
			if (dict1_) {
				const char* entry = dict1_->getEntry(word);
				if (entry != nullptr) {
					*dict = TTS_DICTIONARY_1;
					return entry;
				}
			}
			break;
		case TTS_DICTIONARY_2:
			if (dict2_) {
				const char* entry = dict2_->getEntry(word);
				if (entry != nullptr) {
					*dict = TTS_DICTIONARY_2;
					return entry;
				}
			}
			break;
		case TTS_DICTIONARY_3:
			if (dict3_) {
				const char* entry = dict3_->getEntry(word);
				if (entry != nullptr) {
					*dict = TTS_DICTIONARY_3;
					return entry;
				}
			}
			break;
		default:
			break;
		}
	}

	/*  IF HERE, THEN FIND WORD IN LETTER-TO-SOUND RULEBASE  */
	/*  THIS IS GUARANTEED TO FIND A PRONUNCIATION OF SOME SORT  */
	letter_to_sound(word, pronunciation_);
	if (!pronunciation_.empty()) {
		*dict = TTS_LETTER_TO_SOUND;
		return &pronunciation_[0];
	} else {
		*dict = TTS_LETTER_TO_SOUND;
		return numberParser_.degenerateString(word);
	}
}

/******************************************************************************
*
*       function:       condition_input
*
*       purpose:        Converts all non-printable characters (except escape
*                       character to blanks.  Also connects words hyphenated
*                       over a newline.
*
******************************************************************************/
void
TextParser::condition_input(const char* input, char* output, int length, int* output_length)
{
	int i, j = 0;

	for (i = 0; i < length; i++) {
		if ((input[i] == '-') && ((i-1) >= 0) && isalpha(input[i-1])) {
			/*  CONNECT HYPHENATED WORD OVER NEWLINE  */
			int ii = i;
			/*  IGNORE ANY WHITE SPACE UP TO NEWLINE  */
			while (((ii+1) < length) && (input[ii+1] != '\n') &&
					(input[ii+1] != escape_character_) && isspace(input[ii+1])) {
				ii++;
			}
			/*  IF NEWLINE, THEN CONCATENATE WORD  */
			if (((ii+1) < length) && input[ii+1] == '\n') {
				i = ++ii;
				/*  IGNORE ANY WHITE SPACE  */
				while (((i+1) < length) && (input[i+1] != escape_character_) && isspace(input[i+1])) {
					i++;
				}
			} else { /*  ELSE, OUTPUT HYPHEN  */
				output[j++] = input[i];
			}
		//} else if ( !isascii(input[i]) || (!isprint(input[i]) && input[i] != escape_character_) ) {
		//TODO: Complete UTF-8 support.
		// Temporary solution to allow UTF-8 characters.
		} else if (isascii(input[i]) && !isprint(input[i]) && input[i] != escape_character_) {
			/*  CONVERT NONPRINTABLE CHARACTERS TO SPACE  */
			output[j++] = ' ';
		} else {
			/*  PASS EVERYTHING ELSE THROUGH  */
			output[j++] = input[i];
		}
	}

	/*  BE SURE TO APPEND NULL TO STRING  */
	output[j] = '\0';
	*output_length = j;
}

/******************************************************************************
*
*       function:       mark_modes
*
*       purpose:        Parses input for modes, checking for errors, and
*                       marks output with mode start and end points.
*                       Tagging and silence mode arguments are checked.
*
******************************************************************************/
int
TextParser::mark_modes(const char* input, char* output, int length, int* output_length)
{
	int i, j = 0, pos, minus, period;
	int mode_stack[MODE_NEST_MAX], stack_ptr = 0, mode;
	int mode_marker[5][2] = {{RAW_MODE_BEGIN,      RAW_MODE_END},
				 {LETTER_MODE_BEGIN,   LETTER_MODE_END},
				 {EMPHASIS_MODE_BEGIN, EMPHASIS_MODE_END},
				 {TAGGING_MODE_BEGIN,  TAGGING_MODE_END},
				 {SILENCE_MODE_BEGIN,  SILENCE_MODE_END}};

	/*  INITIALIZE MODE STACK TO NORMAL MODE */
	mode_stack[stack_ptr] = NORMAL_MODE;

	/*  MARK THE MODES OF INPUT, CHECKING FOR ERRORS  */
	for (i = 0; i < length; i++) {
		/*  IF ESCAPE CODE, DO MODE PROCESSING  */
		if (input[i] == escape_character_) {
			/*  IF IN RAW MODE  */
			if (mode_stack[stack_ptr] == RAW_MODE) {
				/*  CHECK FOR RAW MODE END  */
				if ( ((i+2) < length) &&
						((input[i+1] == 'r') || (input[i+1] == 'R')) &&
						((input[i+2] == 'e') || (input[i+2] == 'E')) ) {
					/*  DECREMENT STACK POINTER, CHECKING FOR STACK UNDERFLOW  */
					if ((--stack_ptr) < 0) {
						return i;
					}
					/*  MARK END OF RAW MODE  */
					output[j++] = mode_marker[RAW_MODE][END];
					/*  INCREMENT INPUT INDEX  */
					i+=2;
					/*  MARK BEGINNING OF STACKED MODE, IF NOT NORMAL MODE  */
					if (mode_stack[stack_ptr] != NORMAL_MODE) {
						output[j++] = mode_marker[mode_stack[stack_ptr]][BEGIN];
					}
				} else { /*  IF NOT END OF RAW MODE, THEN PASS THROUGH ESC CHAR IF PRINTABLE  */
					if (isprint(escape_character_)) {
						output[j++] = escape_character_;
					}
				}
			} else { /*  ELSE, IF IN ANY OTHER MODE  */
				/*  CHECK FOR DOUBLE ESCAPE CHARACTER  */
				if ( ((i+1) < length) && (input[i+1] == escape_character_) ) {
					/*  OUTPUT SINGLE ESCAPE CHARACTER IF PRINTABLE  */
					if (isprint(escape_character_)) {
						output[j++] = escape_character_;
					}
					/*  INCREMENT INPUT INDEX  */
					i++;
				} else if ( ((i+2) < length) && ((input[i+2] == 'b') || (input[i+2] == 'B')) ) {
					/*  CHECK FOR BEGINNING OF MODE  */

					/*  CHECK FOR WHICH MODE  */
					switch(input[i+1]) {
					case 'r':
					case 'R': mode = RAW_MODE;       break;
					case 'l':
					case 'L': mode = LETTER_MODE;    break;
					case 'e':
					case 'E': mode = EMPHASIS_MODE;  break;
					case 't':
					case 'T': mode = TAGGING_MODE;   break;
					case 's':
					case 'S': mode = SILENCE_MODE;   break;
					default:  mode = UNDEFINED_MODE; break;
					}
					if (mode != UNDEFINED_MODE) {
						/*  IF CURRENT MODE NOT NORMAL, WRITE END OF CURRENT MODE  */
						if (mode_stack[stack_ptr] != NORMAL_MODE) {
							output[j++] = mode_marker[mode_stack[stack_ptr]][END];
						}
						/*  INCREMENT STACK POINTER, CHECKING FOR STACK OVERFLOW  */
						if ((++stack_ptr) >= MODE_NEST_MAX) {
							return i;
						}
						/*  STORE NEW MODE ON STACK  */
						mode_stack[stack_ptr] = mode;
						/*  MARK BEGINNING OF MODE  */
						output[j++] = mode_marker[mode][BEGIN];
						/*  INCREMENT INPUT INDEX  */
						i+=2;
						/*  ADD TAGGING MODE END, IF NOT GIVEN, GETTING RID OF BLANKS  */
						if (mode == TAGGING_MODE) {
							/*  IGNORE ANY WHITE SPACE  */
							while (((i+1) < length) && (input[i+1] == ' ')) {
								i++;
							}
							/*  COPY NUMBER, CHECKING VALIDITY  */
							pos = minus = 0;
							while (((i+1) < length) && (input[i+1] != ' ') && (input[i+1] != escape_character_)) {
								i++;
								/*  ALLOW ONLY MINUS OR PLUS SIGN AND DIGITS  */
								if (!isdigit(input[i]) && !((input[i] == '-') || (input[i] == '+'))) {
									return i;
								}
								/*  MINUS OR PLUS SIGN AT BEGINNING ONLY  */
								if ((pos > 0) && ((input[i] == '-') || (input[i] == '+'))) {
									return i;
								}
								/*  OUTPUT CHARACTER, KEEPING TRACK OF POSITION AND MINUS SIGN  */
								output[j++] = input[i];
								if ((input[i] == '-') || (input[i] == '+')) {
									minus++;
								}
								pos++;
							}
							/*  MAKE SURE MINUS OR PLUS SIGN HAS NUMBER FOLLOWING IT  */
							if (minus >= pos) {
								return i;
							}
							/*  IGNORE ANY WHITE SPACE  */
							while (((i+1) < length) && (input[i+1] == ' ')) {
								i++;
							}
							/*  IF NOT EXPLICIT TAG END, THEN INSERT ONE, POP STACK  */
							if (!(((i+3) < length) && (input[i+1] == escape_character_) &&
							      ((input[i+2] == 't') || (input[i+2] == 'T')) &&
							      ((input[i+3] == 'e') || (input[i+3] == 'E'))) ) {
								/*  MARK END OF MODE  */
								output[j++] = mode_marker[mode][END];
								/*  DECREMENT STACK POINTER, CHECKING FOR STACK UNDERFLOW  */
								if ((--stack_ptr) < 0) {
									return i;
								}
								/*  MARK BEGINNING OF STACKED MODE, IF NOT NORMAL MODE  */
								if (mode_stack[stack_ptr] != NORMAL_MODE) {
									output[j++] = mode_marker[mode_stack[stack_ptr]][BEGIN];
								}
							}
						} else if (mode == SILENCE_MODE) {
							/*  IGNORE ANY WHITE SPACE  */
							while (((i+1) < length) && (input[i+1] == ' ')) {
								i++;
							}
							/*  COPY NUMBER, CHECKING VALIDITY  */
							period = 0;
							while (((i+1) < length) && (input[i+1] != ' ') && (input[i+1] != escape_character_)) {
								i++;
								/*  ALLOW ONLY DIGITS AND PERIOD  */
								if (!isdigit(input[i]) && (input[i] != '.')) {
									return i;
								}
								/*  ALLOW ONLY ONE PERIOD  */
								if (period && (input[i] == '.')) {
									return i;
								}
								/*  OUTPUT CHARACTER, KEEPING TRACK OF # OF PERIODS  */
								output[j++] = input[i];
								if (input[i] == '.') {
									period++;
								}
							}
							/*  IGNORE ANY WHITE SPACE  */
							while (((i+1) < length) && (input[i+1] == ' ')) {
								i++;
							}
							/*  IF NOT EXPLICIT SILENCE END, THEN INSERT ONE, POP STACK  */
							if (!(((i+3) < length) && (input[i+1] == escape_character_) &&
							      ((input[i+2] == 's') || (input[i+2] == 'S')) &&
							      ((input[i+3] == 'e') || (input[i+3] == 'E'))) ) {
								/*  MARK END OF MODE  */
								output[j++] = mode_marker[mode][END];
								/*  DECREMENT STACK POINTER, CHECKING FOR STACK UNDERFLOW  */
								if ((--stack_ptr) < 0) {
									return i;
								}
								/*  MARK BEGINNING OF STACKED MODE, IF NOT NORMAL MODE  */
								if (mode_stack[stack_ptr] != NORMAL_MODE) {
									output[j++] = mode_marker[mode_stack[stack_ptr]][BEGIN];
								}
							}
						}
					} else {
						/*  ELSE, PASS ESC CHAR THROUGH IF PRINTABLE  */
						if (isprint(escape_character_)) {
							output[j++] = escape_character_;
						}
					}
				} else if ( ((i+2) < length) && ((input[i+2] == 'e') || (input[i+2] == 'E')) ) {
					/*  CHECK FOR END OF MODE  */

					/*  CHECK FOR WHICH MODE  */
					switch(input[i+1]) {
					case 'r':
					case 'R': mode = RAW_MODE;       break;
					case 'l':
					case 'L': mode = LETTER_MODE;    break;
					case 'e':
					case 'E': mode = EMPHASIS_MODE;  break;
					case 't':
					case 'T': mode = TAGGING_MODE;   break;
					case 's':
					case 'S': mode = SILENCE_MODE;   break;
					default:  mode = UNDEFINED_MODE; break;
					}
					if (mode != UNDEFINED_MODE) {
						/*  CHECK IF MATCHING MODE BEGIN  */
						if (mode_stack[stack_ptr] != mode) {
							return i;
						} else { /*  MATCHES WITH MODE BEGIN  */
							/*  DECREMENT STACK POINTER, CHECKING FOR STACK UNDERFLOW  */
							if ((--stack_ptr) < 0) {
								return i;
							}
							/*  MARK END OF MODE  */
							output[j++] = mode_marker[mode][END];
							/*  INCREMENT INPUT INDEX  */
							i += 2;
							/*  MARK BEGINNING OF STACKED MODE, IF NOT NORMAL MODE  */
							if (mode_stack[stack_ptr] != NORMAL_MODE) {
								output[j++] = mode_marker[mode_stack[stack_ptr]][BEGIN];
							}
						}
					} else {
						/*  ELSE, PASS ESC CHAR THROUGH IF PRINTABLE  */
						if (isprint(escape_character_)) {
							output[j++] = escape_character_;
						}
					}
				} else { /*  ELSE, PASS ESC CHAR THROUGH IF PRINTABLE  */
					if (isprint(escape_character_)) {
						output[j++] = escape_character_;
					}
				}
			}
		} else { /*  ELSE, SIMPLY COPY INPUT TO OUTPUT  */
			output[j++] = input[i];
		}
	}

	/*  BE SURE TO ADD A NULL TO END OF STRING  */
	output[j] = '\0';

	/*  SET LENGTH OF OUTPUT STRING  */
	*output_length = j;

	return TTS_PARSER_SUCCESS;
}

/******************************************************************************
*
*       function:       final_conversion
*
*       purpose:        Converts contents of stream1 to stream2.  Adds chunk,
*                       tone group, and associated markers;  expands words to
*                       pronunciations, and also expands other modes.
*
******************************************************************************/
int
TextParser::final_conversion(std::stringstream& stream1, long stream1_length,
				std::stringstream& stream2, long* stream2_length)
{
	long i, last_word_end = UNDEFINED_POSITION, tg_marker_pos = UNDEFINED_POSITION;
	int mode = NORMAL_MODE, next_mode = 0, prior_tonic = TTS_FALSE, raw_mode_flag = TTS_FALSE;
	int last_written_state = STATE_BEGIN, current_state, next_state;
	char word[WORD_LENGTH_MAX+1];
	//int length, max_length;

	/*  REWIND STREAM2 BACK TO BEGINNING  */
	stream2.str("");

	/*  GET MEMORY BUFFER ASSOCIATED WITH STREAM1  */
	std::string stream1String = stream1.str();
	const char* input = stream1String.data();

	/*  MAIN LOOP  */
	for (i = 0; i < stream1_length; i++) {
		switch (input[i]) {
		case RAW_MODE_BEGIN:      mode = RAW_MODE;      break;
		case LETTER_MODE_BEGIN:   mode = LETTER_MODE;   break;
		case EMPHASIS_MODE_BEGIN: mode = EMPHASIS_MODE; break;
		case TAGGING_MODE_BEGIN:  mode = TAGGING_MODE;  break;
		case SILENCE_MODE_BEGIN:  mode = SILENCE_MODE;  break;

		case RAW_MODE_END:
		case LETTER_MODE_END:
		case EMPHASIS_MODE_END:
		case TAGGING_MODE_END:
		case SILENCE_MODE_END:    mode = NORMAL_MODE;   break;

		default:
			/*  GET STATE INFORMATION  */
			if (get_state(input, &i, stream1_length, &mode, &next_mode, &current_state,
				      &next_state, &raw_mode_flag, word, stream2) != TTS_PARSER_SUCCESS) {
				return TTS_PARSER_FAILURE;
			}

#if 0
			printf("last_written_state = %-d current_state = %-d next_state = %-d ",
			       last_written_state,current_state,next_state);
			printf("mode = %-d next_mode = %-d word = %s\n",
			       mode,next_mode,word);
#endif

			/*  ACTION ACCORDING TO CURRENT STATE  */
			switch (current_state) {

			case STATE_WORD:
				/*  ADD BEGINNING MARKERS IF NECESSARY (SWITCH FALL-THRU DESIRED)  */
				switch(last_written_state) {
				case STATE_BEGIN:
					stream2 << CHUNK_BOUNDARY << ' ';
					[[fallthrough]];
				case STATE_FINAL_PUNC:
					stream2 << TONE_GROUP_BOUNDARY << ' ';
					prior_tonic = TTS_FALSE;
					[[fallthrough]];
				case STATE_MEDIAL_PUNC:
					stream2 << TG_UNDEFINED << ' ';
					tg_marker_pos = static_cast<long>(stream2.tellp()) - 3;
					[[fallthrough]];
				case STATE_SILENCE:
					stream2 << UTTERANCE_BOUNDARY << ' ';
				}

				if (mode == NORMAL_MODE) {
					/*  PUT IN WORD MARKER  */
					stream2 << WORD_BEGIN << ' ';
					/*  ADD LAST WORD MARKER AND TONICIZATION IF NECESSARY  */
					switch(next_state) {
					case STATE_MEDIAL_PUNC:
					case STATE_FINAL_PUNC:
					case STATE_END:
						/*  PUT IN LAST WORD MARKER  */
						stream2 << LAST_WORD << ' ';
						/*  WRITE WORD TO STREAM WITH TONIC IF NO PRIOR TONICIZATION  */
						expand_word(word, (!prior_tonic), stream2);
						break;
					default:
						/*  WRITE WORD TO STREAM WITHOUT TONIC  */
						expand_word(word, TTS_NO, stream2);
						break;
					}
				} else if (mode == EMPHASIS_MODE) {
					/*  START NEW TONE GROUP IF PRIOR TONIC ALREADY SET  */
					if (prior_tonic) {
						if (set_tone_group(stream2, tg_marker_pos, ",") == TTS_PARSER_FAILURE) {
							return TTS_PARSER_FAILURE;
						}
						stream2 << TONE_GROUP_BOUNDARY << ' ' << TG_UNDEFINED << ' ';
						tg_marker_pos = static_cast<long>(stream2.tellp()) - 3;
					}
					/*  PUT IN WORD MARKER  */
					stream2 << WORD_BEGIN << ' ';
					/*  MARK LAST WORD OF TONE GROUP, IF NECESSARY  */
					if ((next_state == STATE_MEDIAL_PUNC) ||
							(next_state == STATE_FINAL_PUNC) ||
							(next_state == STATE_END) ||
							((next_state == STATE_WORD) && (next_mode == EMPHASIS_MODE)) ) {
						stream2 << LAST_WORD << ' ';
					}
					/*  TONICIZE WORD  */
					expand_word(word, TTS_YES, stream2);
					prior_tonic = TTS_TRUE;
				}

				/*  SET LAST WRITTEN STATE, AND END POSITION AFTER THE WORD  */
				last_written_state = STATE_WORD;
				last_word_end = static_cast<long>(stream2.tellp());
				break;

			case STATE_MEDIAL_PUNC:
				/*  APPEND LAST WORD MARK, PAUSE, TONE GROUP MARK (FALL-THRU DESIRED)  */
				switch(last_written_state) {
				case STATE_WORD:
					if (shift_silence(input, i, stream1_length, mode, stream2)) {
						last_word_end = static_cast<long>(stream2.tellp());
					} else if ((next_state != STATE_END) &&
							another_word_follows(input, i, stream1_length, mode)) {
						if (!strcmp(word,",")) {
							stream2 << UTTERANCE_BOUNDARY << ' ' << MEDIAL_PAUSE << ' ';
						} else {
							stream2 << UTTERANCE_BOUNDARY << ' ' << LONG_MEDIAL_PAUSE << ' ';
						}
					} else if (next_state == STATE_END) {
						stream2 << UTTERANCE_BOUNDARY << ' ';
					}
					[[fallthrough]];
				case STATE_SILENCE:
					stream2 << TONE_GROUP_BOUNDARY << ' ';
					prior_tonic = TTS_FALSE;
					if (set_tone_group(stream2, tg_marker_pos, word) == TTS_PARSER_FAILURE) {
						return TTS_PARSER_FAILURE;
					}
					tg_marker_pos = UNDEFINED_POSITION;
					last_written_state = STATE_MEDIAL_PUNC;
				}
				break;

			case STATE_FINAL_PUNC:
				if (last_written_state == STATE_WORD) {
					if (shift_silence(input, i, stream1_length, mode, stream2)) {
						last_word_end = static_cast<long>(stream2.tellp());
						stream2 << TONE_GROUP_BOUNDARY << ' ';
						prior_tonic = TTS_FALSE;
						if (set_tone_group(stream2, tg_marker_pos, word) == TTS_PARSER_FAILURE) {
							return TTS_PARSER_FAILURE;
						}
						tg_marker_pos = UNDEFINED_POSITION;
						/*  IF SILENCE INSERTED, THEN CONVERT FINAL PUNCTUATION TO MEDIAL  */
						last_written_state = STATE_MEDIAL_PUNC;
					} else {
						stream2 << UTTERANCE_BOUNDARY << ' '
							<< TONE_GROUP_BOUNDARY << ' ' << CHUNK_BOUNDARY << ' ';
						prior_tonic = TTS_FALSE;
						if (set_tone_group(stream2, tg_marker_pos, word) == TTS_PARSER_FAILURE) {
							return TTS_PARSER_FAILURE;
						}
						tg_marker_pos = UNDEFINED_POSITION;
						last_written_state = STATE_FINAL_PUNC;
					}
				} else if (last_written_state == STATE_SILENCE) {
					stream2 << TONE_GROUP_BOUNDARY << ' ';
					prior_tonic = TTS_FALSE;
					if (set_tone_group(stream2, tg_marker_pos, word) == TTS_PARSER_FAILURE) {
						return TTS_PARSER_FAILURE;
					}
					tg_marker_pos = UNDEFINED_POSITION;
					/*  IF SILENCE INSERTED, THEN CONVERT FINAL PUNCTUATION TO MEDIAL  */
					last_written_state = STATE_MEDIAL_PUNC;
				}
				break;

			case STATE_SILENCE:
				if (last_written_state == STATE_BEGIN) {
					stream2 << CHUNK_BOUNDARY << ' ' << TONE_GROUP_BOUNDARY << ' ' << TG_UNDEFINED << ' ';
					prior_tonic = TTS_FALSE;
					tg_marker_pos = static_cast<long>(stream2.tellp()) - 3;
					if ((convert_silence(word, stream2) <= 0.0) && (next_state == STATE_END)) {
						return TTS_PARSER_FAILURE;
					}
					last_written_state = STATE_SILENCE;
					last_word_end = static_cast<long>(stream2.tellp());
				} else if (last_written_state == STATE_WORD) {
					convert_silence(word, stream2);
					last_written_state = STATE_SILENCE;
					last_word_end = static_cast<long>(stream2.tellp());
				}
				break;

			case STATE_TAGGING:
				insert_tag(stream2, last_word_end, word);
				last_word_end = UNDEFINED_POSITION;
				break;

			case STATE_END:
				break;
			}
			break;
		}
	}

	/*  FINAL STATE  */
	switch (last_written_state) {

	case STATE_MEDIAL_PUNC:
		stream2 << CHUNK_BOUNDARY;
		break;

	case STATE_WORD:
		stream2 << UTTERANCE_BOUNDARY << ' ';
		[[fallthrough]];
	case STATE_SILENCE:
		stream2 << TONE_GROUP_BOUNDARY << ' ' << CHUNK_BOUNDARY;
		prior_tonic = TTS_FALSE;
		if (set_tone_group(stream2, tg_marker_pos, DEFAULT_END_PUNC) == TTS_PARSER_FAILURE) {
			return TTS_PARSER_FAILURE;
		}
		tg_marker_pos = UNDEFINED_POSITION;
		break;

	case STATE_BEGIN:
		if (!raw_mode_flag)
			return(TTS_PARSER_FAILURE);
		break;
	}

	/*  BE SURE TO ADD NULL TO END OF STREAM  */
	stream2 << '\0';

	/*  SET STREAM2 LENGTH  */
	*stream2_length = static_cast<long>(stream2.tellp());

	/*  RETURN SUCCESS  */
	return TTS_PARSER_SUCCESS;
}

/******************************************************************************
*
*       function:       expand_word
*
*       purpose:        Write pronunciation of word to stream.  Deal with
*                       possessives if necessary.  Also, deal with single
*                       characters, and upper case words (including special
*                       acronyms) if necessary.  Add special marks if word
*                       is tonic.
*
******************************************************************************/
void
TextParser::expand_word(char* word, int is_tonic, std::stringstream& stream)
{
	short dictionary;
	const char *pronunciation, *ptr;
	long last_foot_begin;
	int possessive = TTS_NO;
	char last_phoneme[SYMBOL_LENGTH_MAX+1], *last_phoneme_ptr;

	/*  STRIP OF POSSESSIVE ENDING IF WORD ENDS WITH 's, SET FLAG  */
	possessive = is_possessive(word);

	/*  USE degenerate_string IF WORD IS A SINGLE CHARACTER
	    (EXCEPT SMALL, NON-POSSESSIVE A)  */
	if ((strlen(word) == 1) && isalpha(word[0])) {
		if (!strcmp(word,"a") && !possessive) {
			pronunciation = "uh";
		} else {
			pronunciation = numberParser_.degenerateString(word);
		}
		dictionary = TTS_LETTER_TO_SOUND;
	} else if (is_all_upper_case(word)) {
		/*  ALL UPPER CASE WORDS PRONOUNCED ONE LETTER AT A TIME,
		    EXCEPT SPECIAL ACRONYMS  */

		if (!(pronunciation = is_special_acronym(word))) {
			pronunciation = numberParser_.degenerateString(word);
		}

		dictionary = TTS_LETTER_TO_SOUND;
	} else { /*  ALL OTHER WORDS ARE LOOKED UP IN DICTIONARIES, AFTER CONVERTING TO LOWER CASE  */
		pronunciation = lookup_word((const char *)to_lower_case(word), &dictionary);
	}

	/*  ADD FOOT BEGIN MARKER TO FRONT OF WORD IF IT HAS NO PRIMARY STRESS AND IT IS
	    TO RECEIVE A TONIC;  IF ONLY A SECONDARY STRESS MARKER, CONVERT TO PRIMARY  */
	last_foot_begin = UNDEFINED_POSITION;
	if (is_tonic && !contains_primary_stress(pronunciation)) {
		if (!converted_stress((char *)pronunciation)) {
			stream << FOOT_BEGIN;
			last_foot_begin = static_cast<long>(stream.tellp()) - 2;
		}
	}

	/*  PRINT PRONUNCIATION TO STREAM, UP TO WORD TYPE MARKER (%)  */
	/*  KEEP TRACK OF LAST PHONEME  */
	ptr = pronunciation;
	last_phoneme[0] = '\0';
	last_phoneme_ptr = last_phoneme;
	while (*ptr && (*ptr != '%')) {
		switch(*ptr) {
		case '\'':
		case '`':
			stream << FOOT_BEGIN;
			last_foot_begin = static_cast<long>(stream.tellp()) - 2;
			last_phoneme[0] = '\0';
			last_phoneme_ptr = last_phoneme;
			break;
		case '"':
			stream << SECONDARY_STRESS;
			last_phoneme[0] = '\0';
			last_phoneme_ptr = last_phoneme;
			break;
		case '_':
		case '.':
			stream << *ptr;
			last_phoneme[0] = '\0';
			last_phoneme_ptr = last_phoneme;
			break;
		case ' ':
			/*  SUPPRESS UNNECESSARY BLANKS  */
			if (*(ptr+1) && (*(ptr+1) != ' ')) {
				stream << *ptr;
				last_phoneme[0] = '\0';
				last_phoneme_ptr = last_phoneme;
			}
			break;
		default:
			stream << *ptr;
			*last_phoneme_ptr++ = *ptr;
			*last_phoneme_ptr = '\0';
			break;
		}
		ptr++;
	}

	/*  ADD APPROPRIATE ENDING TO PRONUNCIATION IF POSSESSIVE  */
	if (possessive) {
		if (!strcmp(last_phoneme,"p") || !strcmp(last_phoneme,"t") ||
				!strcmp(last_phoneme,"k") || !strcmp(last_phoneme,"f") ||
				!strcmp(last_phoneme,"th")) {
			stream << "_s";
		} else if (!strcmp(last_phoneme,"s") || !strcmp(last_phoneme,"sh") ||
				!strcmp(last_phoneme,"z") || !strcmp(last_phoneme,"zh") ||
				!strcmp(last_phoneme,"j") || !strcmp(last_phoneme,"ch")) {
			stream << ".uh_z";
		} else {
			stream << "_z";
		}
	}

	/*  ADD SPACE AFTER WORD  */
	stream << ' ';

	/*  IF TONIC, CONVERT LAST FOOT MARKER TO TONIC MARKER  */
	if (is_tonic && (last_foot_begin != UNDEFINED_POSITION)) {
		long temporaryPosition = static_cast<long>(stream.tellp());
		stream.seekp(last_foot_begin);
		stream << TONIC_BEGIN;
		stream.seekp(temporaryPosition);
	}
}

} /* namespace En */
} /* namespace GS */
