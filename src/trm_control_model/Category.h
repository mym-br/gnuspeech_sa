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
// 2015-01
// This file was created by Marcelo Y. Matuda, and code/information
// from Gnuspeech was added to it later.

#ifndef TRM_CONTROL_MODEL_CATEGORY_H_
#define TRM_CONTROL_MODEL_CATEGORY_H_

#include <string>



namespace GS {
namespace TRMControlModel {

class Category {
public:
	Category(const std::string& name) : name_(name), native_(false) {}

	const std::string& name() const { return name_; }
	void setName(const std::string& name) { name_ = name; }

	const std::string& comment() const { return comment_; }
	void setComment(const std::string& comment) { comment_ = comment; }

	bool native() const { return native_; }
	void setNative() { native_ = true; }
private:
	std::string name_;
	std::string comment_;
	bool native_;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_CATEGORY_H_ */
