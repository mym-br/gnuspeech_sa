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

#ifndef SIMPLE_XML_PARSER_H_
#define SIMPLE_XML_PARSER_H_

#include <stack>
#include <string>
#include <unordered_map>



namespace GS {

/*******************************************************************************
 * This class is not XML-compliant.
 *
 * Notes:
 *   - The elements must be accessed in the same order they appear in the XML.
 *   - The XML is supposed to contain only 8-bit characters.
 */
class SimpleXMLParser {
public:
	SimpleXMLParser(const std::string& filePath);
	~SimpleXMLParser();

	const std::string* getFirstChild();
	const std::string* getFirstChild(const std::string& name);

	const std::string* getNextSibling();
	const std::string* getNextSibling(const std::string& name);

	const std::string& getAttribute(const std::string& name);

	const std::string& getText();
private:
	typedef std::unordered_map<std::string, std::string> AttributeMap;

	bool closedTag_;
	std::string::size_type pos_;
	std::string source_;
	std::string text_;
	std::string tag_;
	std::string tagExtra_;
	std::stack<std::string> tagStack_;
	AttributeMap attributeMap_;

	std::string::size_type find(char c);
	std::string::size_type find(const std::string& s);
	void skipSpaces();
	bool finished() const;
	char nextChar();
	bool isInvalidPosition(std::string::size_type p) const;
	void getTagInfo(std::string::size_type size);
	void closePreviousSiblingTag();
	void closeParentTag();
	void fillAttributeMap();
};

} /* namespace GS */

#endif /*SIMPLE_XML_PARSER_H_*/
