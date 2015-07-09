/***************************************************************************
 *  Copyright 2014 Marcelo Y. Matuda                                       *
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
// 2014-12
// This file was created by Marcelo Y. Matuda, and code/information
// from Gnuspeech was added to it later.

#ifndef TRM_CONTROL_MODEL_SYMBOL_H_
#define TRM_CONTROL_MODEL_SYMBOL_H_

#include <memory>
#include <string>



namespace GS {
namespace TRMControlModel {

class Symbol {
public:
	Symbol(const std::string& name, float minimum, float maximum, float defaultValue, const std::string& comment)
		: name_(name)
		, minimum_(minimum)
		, maximum_(maximum)
		, defaultValue_(defaultValue)
		, comment_(comment)
	{}

	const std::string& name() const { return name_; }
	void setName(const std::string& name) { name_ = name; }

	float minimum() const { return minimum_; }
	void setMinimum(float minimum) { minimum_ = minimum; }

	float maximum() const { return maximum_; }
	void setMaximum(float maximum) { maximum_ = maximum; }

	float defaultValue() const { return defaultValue_; }
	void setDefaultValue(float defaultValue) { defaultValue_ = defaultValue; }

	const std::string& comment() const { return comment_; }
	void setComment(const std::string& comment) { comment_ = comment; }
private:
	std::string name_;
	float minimum_;
	float maximum_;
	float defaultValue_;
	std::string comment_;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_SYMBOL_H_ */
