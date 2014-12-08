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

#ifndef TRM_CONTROL_MODEL_CATEGORY_H_
#define TRM_CONTROL_MODEL_CATEGORY_H_

#include <string>



namespace GS {
namespace TRMControlModel {

class Category {
public:
	Category(const std::string& name, unsigned int code) : name_(name), code_(code) {}

	std::string& name() { return name_; }
	const std::string& name() const { return name_; }

	unsigned int code() const { return code_; }
	void setCode(unsigned int code) { code_ = code; }
private:
	std::string name_;
	unsigned int code_; // optimization
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_CATEGORY_H_ */
