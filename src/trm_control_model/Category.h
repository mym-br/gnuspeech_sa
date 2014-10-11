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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>



namespace GS {
namespace TRMControlModel {

struct Category;
typedef std::unordered_map<std::string, std::unique_ptr<Category>> CategoryMap;

struct Category {
	std::string name;
	int code;

	Category(const std::string& name, int code) : name(name), code(code) {}

	static int getCode(const CategoryMap& categoryMap, const std::string& name);
};

typedef std::unique_ptr<Category> Category_ptr;
typedef std::vector<Category_ptr> CategoryList;

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_CATEGORY_H_ */
