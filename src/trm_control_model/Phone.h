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
// 2014-09
// This file was created by Marcelo Y. Matuda, and code/information
// from Gnuspeech was added to it later.

#ifndef TRM_CONTROL_MODEL_PHONE_H_
#define TRM_CONTROL_MODEL_PHONE_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "Category.h"
#include "Exception.h"
#include "Parameter.h"



namespace GS {
namespace TRMControlModel {

class Phone {
public:
	struct Symbols {
		float duration;
		float transition;
		float qssa;
		float qssb;

		Symbols() : duration(0.0), transition(0.0), qssa(0.0), qssb(0.0) {}
	};

	Phone(const std::string& name);

	bool isMemberOfCategory(int code) const;
	bool isMemberOfCategory(const std::string& categoryName, bool phoneNameOnly) const;
	bool isMemberOfCategory(const Category& category) const;
	template<typename T> bool isMemberOfCategory(const std::string& categoryName, T match) const;
	const Category* findCategory(const std::string& name) const;

	const std::string& name() const {
		return name_;
	}
	std::vector<float>& parameterTargetList() { return parameterTargetList_; }

	float getParameterTarget(unsigned int parameterIndex) const {
		if (parameterIndex >= parameterTargetList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
		}

		return parameterTargetList_[parameterIndex];
	}
	const Symbols& symbols() const {
		return symbols_;
	}
	const CategoryList& categoryList() const {
		return categoryList_;
	}
private:
	Phone(const Phone&);
	Phone& operator=(const Phone&);

	std::string name_;
	std::vector<float> parameterTargetList_;
	CategoryList categoryList_;
	Symbols symbols_;

	friend class XMLConfigFile;
};

typedef std::unique_ptr<Phone> Phone_ptr;
typedef std::unordered_map<std::string, Phone_ptr> PhoneMap; // type of container that manages the Phone instances
typedef std::vector<const Phone*> PhoneSequence;

/*******************************************************************************
 * Obs.: Considers only the name of the phone.
 */
template<typename T>
bool
Phone::isMemberOfCategory(const std::string& categoryName, T match) const
{
	if (match(categoryName, name_)) {
		return true;
	}

	return false;
}

/*******************************************************************************
 *
 */
inline
void
insert(PhoneMap& map, const std::string& key, Phone_ptr value)
{
	typedef PhoneMap::iterator MI;
	typedef PhoneMap::value_type VT;

	std::pair<MI, bool> res = map.insert(VT(key, std::move(value)));
	if (!res.second) {
		THROW_EXCEPTION(TRMControlModelException, "Duplicate phone: " << key << '.');
	}
}

/*******************************************************************************
 *
 */
inline
const Category*
Phone::findCategory(const std::string& name) const
{
	for (auto& category : categoryList_) {
		if (category->name == name) {
			return category.get();
		}
	}
	return nullptr;
}

/*******************************************************************************
 *
 */
inline
bool
Phone::isMemberOfCategory(int code) const
{
	for (CategoryList::size_type size = categoryList_.size(), i = 0; i < size; ++i) {
		if (categoryList_[i]->code == code) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 * If phoneNameOnly = true, considers only the name of the phone.
 */
inline
bool
Phone::isMemberOfCategory(const std::string& categoryName, bool phoneNameOnly) const
{
	if (name_ == categoryName) return true;

	if (!phoneNameOnly) {
		for (CategoryList::size_type size = categoryList_.size(), i = 0; i < size; ++i) {
			if (categoryList_[i]->name == categoryName) {
				return true;
			}
		}
	}

	return false;
}

/*******************************************************************************
 *
 */
inline
bool
Phone::isMemberOfCategory(const Category& category) const
{
	if (category.code != 0) {
		for (const auto& c : categoryList_) {
			if (c->code == category.code) {
				return true;
			}
		}
	} else if (name_ == category.name) {
		return true;
	}
	return false;
}

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_PHONE_H_ */
