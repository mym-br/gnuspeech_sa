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

#ifndef TRM_CONTROL_MODEL_POSTURE_H_
#define TRM_CONTROL_MODEL_POSTURE_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "Category.h"
#include "Exception.h"
#include "Parameter.h"



namespace GS {
namespace TRMControlModel {

class Posture {
public:
	struct Symbols {
		float duration;
		float transition;
		float qssa;
		float qssb;

		Symbols() : duration(0.0), transition(0.0), qssa(0.0), qssb(0.0) {}
	};

	Posture(const std::string& name) : name_(name) {}

	bool isMemberOfCategory(int code) const;
	bool isMemberOfCategory(const std::string& categoryName, bool postureNameOnly) const;
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

	Symbols& symbols() { return symbols_; }
	const Symbols& symbols() const { return symbols_; }

	CategoryList& categoryList() { return categoryList_; }
	const CategoryList& categoryList() const { return categoryList_; }

private:
	Posture(const Posture&);
	Posture& operator=(const Posture&);

	std::string name_;
	std::vector<float> parameterTargetList_;
	CategoryList categoryList_;
	Symbols symbols_;
};

typedef std::unique_ptr<Posture> Posture_ptr;
typedef std::unordered_map<std::string, Posture_ptr> PostureMap; // type of container that manages the Posture instances
typedef std::vector<const Posture*> PostureSequence;

/*******************************************************************************
 *
 */
inline
void
insert(PostureMap& map, const std::string& key, Posture_ptr value)
{
	typedef PostureMap::iterator MI;
	typedef PostureMap::value_type VT;

	std::pair<MI, bool> res = map.insert(VT(key, std::move(value)));
	if (!res.second) {
		THROW_EXCEPTION(TRMControlModelException, "Duplicate posture: " << key << '.');
	}
}

/*******************************************************************************
 * Obs.: Considers only the name of the posture.
 */
template<typename T>
bool
Posture::isMemberOfCategory(const std::string& categoryName, T match) const
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
const Category*
Posture::findCategory(const std::string& name) const
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
Posture::isMemberOfCategory(int code) const
{
	for (CategoryList::size_type size = categoryList_.size(), i = 0; i < size; ++i) {
		if (categoryList_[i]->code == code) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 * If postureNameOnly = true, considers only the name of the posture.
 */
inline
bool
Posture::isMemberOfCategory(const std::string& categoryName, bool postureNameOnly) const
{
	if (name_ == categoryName) return true;

	if (!postureNameOnly) {
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
Posture::isMemberOfCategory(const Category& category) const
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

#endif /* TRM_CONTROL_MODEL_POSTURE_H_ */
