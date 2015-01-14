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

#include "SimpleXMLParser.h"

#include <cassert>
#include <fstream>
#include <iostream>

#include "Exception.h"



namespace {

const size_t BUFFER_SIZE = 4 * 1024;
const size_t INITIAL_SOURCE_SIZE = 256 * 1024;

const char* SPACE_CHARS = " \f\n\r\t\v";
const char* SPACE_DBL_QUOTE_EQUAL_CHARS = " \f\n\r\t\v\"=";

const std::string EMPTY_STRING;

} /* namespace */

//==============================================================================

namespace GS {

/*******************************************************************************
 * Constructor.
 */
SimpleXMLParser::SimpleXMLParser(const std::string& filePath)
		: closedTag_(false)
		, pos_(0)
{
	source_.reserve(INITIAL_SOURCE_SIZE);

	char buf[BUFFER_SIZE];

	std::ifstream inFile(filePath.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!inFile) {
		THROW_EXCEPTION(XMLException, "Could not open the file: " << filePath << '.');
	}

	while (inFile.read(buf, BUFFER_SIZE)) {
		source_.append(buf, inFile.gcount());
	}
	if (inFile.gcount() > 0) {
		source_.append(buf, inFile.gcount());
	}
	assert(source_.size() > 0);
}

/*******************************************************************************
 * Destructor.
 */
SimpleXMLParser::~SimpleXMLParser()
{
}



/*******************************************************************************
 *
 */
inline
bool
SimpleXMLParser::isInvalidPosition(std::string::size_type p) const
{
	return (p == std::string::npos || p >= source_.size());
}

/*******************************************************************************
 *
 */
inline
bool
SimpleXMLParser::finished() const
{
	return isInvalidPosition(pos_);
}

/*******************************************************************************
 * Searches for the char.
 */
inline
std::string::size_type
SimpleXMLParser::find(char c)
{
	assert(!finished());

	return source_.find(c, pos_);
}

/*******************************************************************************
 * Searches for the string.
 */
inline
std::string::size_type
SimpleXMLParser::find(const std::string& s)
{
	assert(!finished());

	return source_.find(s, pos_);
}

/*******************************************************************************
 *
 */
inline
void
SimpleXMLParser::skipSpaces()
{
	assert(!finished());

	pos_ = source_.find_first_not_of(SPACE_CHARS, pos_);
	if (finished()) {
		THROW_EXCEPTION(XMLException, "End of source reached.");
	}
}

/*******************************************************************************
 *
 */
char
SimpleXMLParser::nextChar()
{
	++pos_;
	if (finished()) {
		THROW_EXCEPTION(XMLException, "End of source reached.");
	}
	return source_[pos_];
}

/*******************************************************************************
 *
 */
void
SimpleXMLParser::fillAttributeMap()
{
	if (tagExtra_.empty()) return;

	std::string::size_type basePos = 0;
	while (basePos != std::string::npos) {

		// Skip spaces, '=' and '"'.
		basePos = tagExtra_.find_first_not_of(SPACE_DBL_QUOTE_EQUAL_CHARS, basePos);
		if (basePos == std::string::npos) {
			THROW_EXCEPTION(XMLException, "Attribute not found.");
		}

		std::string::size_type midPos = tagExtra_.find("=\"", basePos);
		if (midPos == std::string::npos) {
			THROW_EXCEPTION(XMLException, "Equal sign not found.");
		}

		std::string::size_type testPos = tagExtra_.find(SPACE_CHARS, basePos);
		if (testPos != std::string::npos && testPos < midPos) {
			THROW_EXCEPTION(XMLException, "Invalid attribute(s): " << tagExtra_ << '.');
		}

		std::string::size_type endPos = midPos + 2;
		if (endPos >= tagExtra_.size()) {
			THROW_EXCEPTION(XMLException, "End double quote not found (short string).");
		}
		endPos = tagExtra_.find('"', endPos);
		if (endPos == std::string::npos) {
			THROW_EXCEPTION(XMLException, "End double quote not found.");
		}

		attributeMap_[tagExtra_.substr(basePos, midPos - basePos)] = tagExtra_.substr(midPos + 2, endPos - midPos - 2);

		basePos = tagExtra_.find_first_of(SPACE_CHARS, endPos);
	}
}

/*******************************************************************************
 *
 */
void
SimpleXMLParser::closeTag()
{
	tag_.clear();
	tagExtra_.clear();
	attributeMap_.clear();
	closedTag_ = true;
}

/*******************************************************************************
 *
 */
void
SimpleXMLParser::closeParentTag()
{
	// Restore the previous tag.
	if (!tagStack_.empty()) {
		tag_ = tagStack_.top();
		tagStack_.pop();
		tagExtra_.clear();
	}

	attributeMap_.clear();
	closedTag_ = true;
}

/*******************************************************************************
 *
 */
void
SimpleXMLParser::getTagInfo(std::string::size_type size)
{
	std::string tagText_ = source_.substr(pos_, size);
	closedTag_ = (tagText_[tagText_.size() - 1] == '/');

	// Get tag and tagExtra (with attributes).
	std::string::size_type firstSpace = tagText_.find(' ');
	if (firstSpace == std::string::npos) {
		tag_ = closedTag_ ? tagText_.substr(0, tagText_.size() - 1) : tagText_;
		tagExtra_.clear();
	} else {
		tag_ = tagText_.substr(0, firstSpace);
		tagExtra_ = closedTag_ ?
				tagText_.substr(firstSpace, tagText_.size() - firstSpace - 1)
				: tagText_.substr(firstSpace);
	}
}

/*******************************************************************************
 * Gets the first child.
 *
 * Returns 0 if no child was found.
 * The returned pointer is valid only until the next non-const member
 *   function is executed.
 */
const std::string*
SimpleXMLParser::getFirstChild()
{
	if (closedTag_) {
		return 0;
	}

	for (;;) {
		pos_ = find('<');
		if (finished()) {
			return 0;
		}
		switch (nextChar()) {
		case '?':
		case '!':
			// Skip ignored elements.
			pos_ = find('>');
			nextChar();
			break;
		case '/':
			{
				nextChar();
				std::string::size_type endPos = find('>');
				if (isInvalidPosition(endPos)) {
					THROW_EXCEPTION(XMLException, "End of closing tag not found.");
				}
				std::string tagText_ = source_.substr(pos_, endPos - pos_);
				if (tagText_ != tag_) {
					THROW_EXCEPTION(XMLException, "Invalid closing tag (expected: "
									<< tag_ << " found: " << tagText_ << ").");
				}

				closeTag();

				// Go to the next char.
				pos_ = endPos; nextChar();

				return 0;
			}
		default:
			{
				// Save the current tag.
				if (!tag_.empty()) {
					tagStack_.push(tag_);
				}

				attributeMap_.clear();

				std::string::size_type endPos = find('>');
				if (isInvalidPosition(endPos)) {
					THROW_EXCEPTION(XMLException, "End of tag not found.");
				}
				getTagInfo(endPos - pos_);

				// Go to the next char.
				pos_ = endPos; nextChar();

				return &tag_;
			}
		}
	}
}

/*******************************************************************************
 * Gets the first child.
 *
 * Returns 0 if no child was found.
 * The returned pointer is valid only until the next non-const member
 *   function is executed.
 * If the names don't match, an exception will be thrown.
 */
const std::string*
SimpleXMLParser::getFirstChild(const std::string& name)
{
	if (getFirstChild() == 0) {
		return 0;
	}

	if (tag_ != name) {
		THROW_EXCEPTION(XMLException, "Unexpected tag: " << tag_ << " (expected: " << name << ").");
	}

	return &tag_;
}

/*******************************************************************************
 * Gets the next sibling.
 *
 * Returns 0 if no sibling was found.
 * The returned pointer is valid only until the next non-const member
 *   function is executed.
 */
const std::string*
SimpleXMLParser::getNextSibling()
{
	if (!closedTag_) {
		// Find the closing tag of the current element.

		bool found = false;
		while (!found) {
			pos_ = find('<');
			if (finished()) {
				THROW_EXCEPTION(XMLException, "Closing tag not found.");
			}
			switch (nextChar()) {
			case '!':
				// Skip comment.
				pos_ = find('>');
				nextChar();
				break;
			case '/':
				{
					nextChar();
					std::string::size_type endPos = find('>');
					if (isInvalidPosition(endPos)) {
						THROW_EXCEPTION(XMLException, "End of closing tag not found.");
					}
					std::string tagText_ = source_.substr(pos_, endPos - pos_);
					if (tagText_ != tag_) {
						THROW_EXCEPTION(XMLException, "Invalid closing tag (expected: "
										<< tag_ << " found: " << tagText_ << ").");
					}

					closeTag();

					// Go to the next char.
					pos_ = endPos; nextChar();

					found = true;
					break;
				}
			default:
				THROW_EXCEPTION(XMLException, "Closing tag not found.");
			}
		}
	}

	// Find the next sibling.
	for (;;) {
		pos_ = find('<');
		if (finished()) {
			THROW_EXCEPTION(XMLException, "Tag not found.");
		}
		switch (nextChar()) {
		case '!':
			// Skip comment.
			pos_ = find('>');
			nextChar();
			break;
		case '/':
			{
				closeParentTag();

				nextChar();
				std::string::size_type endPos = find('>');
				if (isInvalidPosition(endPos)) {
					THROW_EXCEPTION(XMLException, "End of closing tag not found.");
				}
				std::string tagText_ = source_.substr(pos_, endPos - pos_);
				if (tagText_ != tag_) {
					THROW_EXCEPTION(XMLException, "Invalid closing tag (expected: "
									<< tag_ << " found: " << tagText_ << ").");
				}

				// Go to the next char.
				pos_ = endPos; nextChar();

				return 0;
			}
		default:
			{
				attributeMap_.clear();

				std::string::size_type endPos = find('>');
				if (isInvalidPosition(endPos)) {
					THROW_EXCEPTION(XMLException, "End of tag not found.");
				}
				getTagInfo(endPos - pos_);

				// Go to the next char.
				pos_ = endPos; nextChar();

				return &tag_;
			}
		}
	}
}

/*******************************************************************************
 * Gets the next sibling.
 *
 * Returns 0 if no sibling was found.
 * The returned pointer is valid only until the next non-const member
 *   function is executed.
 * If the names don't match, an exception will be thrown.
 */
const std::string*
SimpleXMLParser::getNextSibling(const std::string& name)
{
	if (getNextSibling() == 0) {
		return 0;
	}

	if (tag_ != name) {
		THROW_EXCEPTION(XMLException, "Unexpected tag: " << tag_ << " (expected: " << name << ").");
	}

	return &tag_;
}

/*******************************************************************************
 * Gets the value of the attribute.
 *
 * The returned reference is valid only until the next non-const member
 *   function is executed.
 */
const std::string&
SimpleXMLParser::getAttribute(const std::string& name)
{
	if (attributeMap_.empty()) {
		fillAttributeMap();
	}
	AttributeMap::iterator iter = attributeMap_.find(name);
	if (iter == attributeMap_.end()) {
		return EMPTY_STRING;
		//THROW_EXCEPTION(XMLException, "Attribute not found: " << name << '.');
	}
	return iter->second;
}

/*******************************************************************************
 * Returns the text between the current tag and the next tag.
 *
 * The returned reference is valid only until the next non-const member
 *   function is executed.
 */
const std::string&
SimpleXMLParser::getText()
{
	std::string::size_type basePos = pos_;
	pos_ = find('<');
	if (finished()) {
		THROW_EXCEPTION(XMLException, "Next tag not found.");
	}

	text_ = source_.substr(basePos, pos_ - basePos);
	return text_;
}

} /* namespace GS */
