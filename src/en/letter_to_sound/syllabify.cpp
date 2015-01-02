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

#include "en/letter_to_sound/syllabify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "en/letter_to_sound/clusters.h"



/*  LOCAL DEFINES  ***********************************************************/
#define MAX_LEN    1024
#define isvowel(c) ((c)=='a' || (c)=='e' || (c)=='i' || (c)=='o' || (c)=='u' )
#define LEFT       begin_syllable
#define RIGHT      end_syllable



namespace {

/*  DATA TYPES  **************************************************************/
typedef char phone_type;

int syllable_break(const char* cluster);
void create_cv_signature(char *ptr, phone_type *arr);
char *add_1_phone(char *t);
void extract_consonant_cluster(char* ptr, phone_type* type, std::vector<char>& cluster);
int next_consonant_cluster(phone_type *pt);
int check_cluster(const char* p, const char** match_array);



/******************************************************************************
*
*	function:	syllable_break
*
*	purpose:	Returns -2 if could not break the cluster.
*
*
*       arguments:      cluster
*
*	internal
*	functions:	check_cluster
*
*	library
*	functions:	strlen, strcpy
*
******************************************************************************/
int
syllable_break(const char* cluster)
{
	const char* left_cluster;
	const char* right_cluster;
	char temp[MAX_LEN];
	int offset, length;

	/*  GET LENGTH OF CLUSTER  */
	length = strlen(cluster);

	/*  INITIALLY WE SHALL RETURN THE FIRST 'POSSIBLE' MATCH  */
	for (offset = -1; (offset <= length); offset++) {
		if (offset == -1 || offset == length || cluster[offset] == '_' || cluster[offset] == '.') {
			strcpy(temp, cluster);
			if (offset >= 0) {
				temp[offset] = 0;
			}
			left_cluster = (offset < 0 ? temp : offset == length ? temp + length : temp + (offset + 1));
			/*  POINTS TO BEGINNING OR NULL  */
			right_cluster = (offset >= 0 ? temp : temp + length);
			/*  NOW THEY POINT TO EITHER A LEFT/RIGHT HANDED CLUSTER OR A NULL STRING  */
			if (check_cluster(left_cluster, LEFT) && check_cluster(right_cluster, RIGHT)) {
				/*  IF THIS IS A POSSIBLE BREAK */
				/*  TEMPORARY:  WILL STORE LIST OF POSSIBLES AND PICK A 'BEST' ONE  */
				return offset;
			}
		}
	}

	/*  IF HERE, RETURN ERROR  */
	return -2;
}

/******************************************************************************
*
*	function:	create_cv_signature
*
*	purpose:
*
*
*       arguments:      ptr, arr
*
*	internal
*	functions:	(isvowel), add_1_phone
*
*	library
*	functions:	none
*
******************************************************************************/
void
create_cv_signature(char *ptr, phone_type *arr)
{
    phone_type         *arr_next;

    arr_next = arr;
    while (*ptr) {
	*arr_next++ = isvowel(*ptr) ? 'v' : 'c';
	ptr = add_1_phone(ptr);
    }
    *arr_next = 0;
}

/******************************************************************************
*
*	function:	add_1_phone
*
*	purpose:
*
*
*       arguments:      t
*
*	internal
*	functions:	none
*
*	library
*	functions:	none
*
******************************************************************************/
char*
add_1_phone(char *t)
{
    while (*t && *t != '_' && *t != '.')
	t++;

    while (*t == '_' || *t == '.')
	t++;

    return(t);
}

/******************************************************************************
*
*	function:	extract_consonant_cluster
*
******************************************************************************/
void
extract_consonant_cluster(char* ptr, phone_type* type, std::vector<char>& cluster)
{
	char* newptr = ptr;

	while (*type == 'c') {
		type++;
		newptr = add_1_phone(newptr);
	}

	cluster.assign(strlen(ptr) + 1, '\0');
	strcpy(&cluster[0], ptr);
	int offset = newptr - ptr - 1;

	if (offset >= 0) {
		cluster[offset] = '\0';
	} else {
		fprintf(stderr, "offset error\n");  // what's this??
	}
}

/******************************************************************************
*
*	function:	next_consonant_cluster
*
*	purpose:	Takes a pointer to phone_type and returns an integer
*                       offset from that point to the start of the next
*                       consonant cluster (or 0 if there are no vowels between
*                       the pointer and the end of the word, or if this is the
*                       second-last cluster and the word doesn't end with a
*                       vowel. Basically, 0 means to stop.)
*
*       arguments:      pt
*
*	internal
*	functions:	none
*
*	library
*	functions:	none
*
******************************************************************************/
int
next_consonant_cluster(phone_type *pt)
{
    phone_type         *pt_var, *pt_temp;

    pt_var = pt;
    while (*pt_var == 'c')
	pt_var++;

    while (*pt_var == 'v')
	pt_var++;

   /*  CHECK TO SEE IF WE ARE NOW ON THE FINAL CLUSTER OF THE WORD WHICH IS AT
       THE END OF THE WORD  */
    pt_temp = pt_var;

    while (*pt_temp == 'c')
	pt_temp++;

    return (*pt_var && *pt_temp ? pt_var - pt : 0);
}

/******************************************************************************
*
*	function:	check_cluster
*
*	purpose:	Returns 1 if it is a possible match, 0 otherwise.
*
*
*       arguments:      p, match_array
*
*	internal
*	functions:	none
*
*	library
*	functions:	strcmp
*
******************************************************************************/
int
check_cluster(const char *p, const char** match_array)
{
	const char** i;

	/*  EMPTY COUNTS AS A MATCH  */
	if (!*p)
		return 1;

	i = match_array;
	while (*i) {
		if (!strcmp(*i, p))
			return 1;
		i++;
	}
	return 0;
}

} /* namespace */

//==============================================================================

namespace GS {
namespace En {

/******************************************************************************
*
*	function:	syllabify
*
*	purpose:	Steps along until probable syllable beginning is found,
*                       taking the longest possible first; then continues
*			skipping vowels until a possible syllable end is found
*                       (again taking the longest possible.)  Changes '_' to
*                       '.' where it occurs between syllable end and start.
*
*       arguments:      word
*                       
*	internal
*	functions:	create_cv_signature, next_consonant_cluster,
*                       add_1_phone, extract_consonant_cluster, syllable_break
*
*	library
*	functions:	none
*
******************************************************************************/
int
syllabify(char* word)
{
	int        i, n, temp, number_of_syllables = 0;
	phone_type cv_signature[MAX_LEN], *current_type;
	char *ptr;
	std::vector<char> cluster;

	/*  INITIALIZE THIS ARRAY TO 'c' (CONSONANT), 'v' (VOWEL), 0 (END)  */
	ptr = word;
	create_cv_signature(ptr, cv_signature);
	current_type = cv_signature;

	/*  WHILE THERE IS ANOTHER CONSONANT CLUSTER (NOT THE LAST)  */
	while ( (temp = next_consonant_cluster(current_type)) ) {
		number_of_syllables++;

		/*  UPDATE CURRENT TYPE POINTER  */
		current_type += temp;

		/*  MOVE PTR TO POINT TO THAT CLUSTER  */
		for (i = 0; i < temp; i++) {
			ptr = add_1_phone(ptr);
		}

		/*  EXTRACT THE CLUSTER INTO A SEPARATE STRING  */
		extract_consonant_cluster(ptr, current_type, cluster);

		/*  DETERMINE WHERE THE PERIOD GOES (OFFSET FROM PTR, WHICH COULD BE -1)  */
		n = syllable_break(&cluster[0]);

		/*  MARK THE SYLLABLE IF POSSIBLE  */
		if (n != -2) {
			*(ptr + n) = '.';
		}
	}

	/*  RETURN NUMBER OF SYLLABLES  */
	return number_of_syllables ? number_of_syllables : 1;
}

} /* namespace En */
} /* namespace GS */
