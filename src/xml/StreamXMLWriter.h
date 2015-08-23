/***************************************************************************
 *  Copyright 2015 Marcelo Y. Matuda                                       *
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

#ifndef STREAM_XML_WRITER_H_
#define STREAM_XML_WRITER_H_

#include <ostream>
#include <string>



namespace GS {

class StreamXMLWriter {
public:
	StreamXMLWriter(std::ostream& out, int indentSize)
		: out_(out)
		, indentSize_(indentSize)
		, indentLevel_(0)
	{}

	void writeDeclaration() {
		out_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	}
	void writeComment(const std::string& text) {
		out_ << "<!-- " << text << " -->\n";
	}

	void openElement(const char* name) {
		indent();
		++indentLevel_;
		out_ << '<' << name << ">\n";
	}
	void openInlineElement(const char* name) {
		indent();
		++indentLevel_;
		out_ << '<' << name << '>';
	}
	void openElementWithAttributes(const char* name) {
		indent();
		++indentLevel_;
		out_ << '<' << name;
	}

	void addAttribute(const char* name, int value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}
	void addAttribute(const char* name, unsigned int value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}
	void addAttribute(const char* name, const std::string& value) {
		out_ << ' ' << name << "=\"";
		for (char c : value) {
			switch (c) {
			case '&':
				out_ << "&amp;";
				break;
			case '"':
				out_ << "&quot;";
				break;
			default:
				out_ << c;
			}
		}
		out_ << '"';
	}
	void addAttribute(const char* name, float value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}
	void addAttribute(const char* name, double value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}

	void endAttributes() {
		out_ << ">\n";
	}
	void endAttributesAndCloseElement() {
		--indentLevel_;
		out_ << "/>\n";
	}

	void addText(const std::string& text) {
		for (char c : text) {
			switch (c) {
			case '&':
				out_ << "&amp;";
				break;
			case '<':
				out_ << "&lt;";
				break;
			default:
				out_ << c;
			}
		}
	}

	void closeElement(const char* name) {
		--indentLevel_;
		indent();
		out_ << "</" << name << ">\n";
	}
	void closeInlineElement(const char* name) {
		--indentLevel_;
		out_ << "</" << name << ">\n";
	}

	void indent() {
		if (indentLevel_ < 0) return;
		for (int i = 0, size = indentLevel_ * indentSize_; i < size; ++i) {
			out_ << ' ';
		}
	}
private:
	std::ostream& out_;
	int indentSize_;
	int indentLevel_;
};

} /* namespace GS */

#endif /*STREAM_XML_WRITER_H_*/
