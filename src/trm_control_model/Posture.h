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

#include <list>
#include <string>
#include <vector>

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

	Posture(unsigned int numParameters) : parameterTargetList_(numParameters) {
		if (numParameters == 0) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid number of parameters: " << numParameters << '.');
		}
	}

	const std::string& name() const { return name_; }
	void setName(const std::string& name) { name_ = name; }

	std::list<Category>& categoryList() { return categoryList_; }
	const std::list<Category>& categoryList() const { return categoryList_; }

	std::vector<float>& parameterTargetList() { return parameterTargetList_; }
	float getParameterTarget(unsigned int parameterIndex) const {
		if (parameterIndex >= parameterTargetList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
		}

		return parameterTargetList_[parameterIndex];
	}
	void setParameterTarget(unsigned int parameterIndex, float target) {
		if (parameterIndex >= parameterTargetList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
		}

		parameterTargetList_[parameterIndex] = target;
	}

	Symbols& symbols() { return symbols_; }
	const Symbols& symbols() const { return symbols_; }

	bool isMemberOfCategory(unsigned int categoryCode) const;
	bool isMemberOfCategory(const std::string& categoryName, bool postureNameOnly) const;
	bool isMemberOfCategory(const Category& category) const;
	template<typename T> bool isMemberOfCategory(const std::string& categoryName, T match) const;
	const Category* findCategory(const std::string& name) const;

private:
	std::string name_;
	std::list<Category> categoryList_;
	std::vector<float> parameterTargetList_;
	Symbols symbols_;
};



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
	for (const auto& category : categoryList_) {
		if (category.name() == name) {
			return &category;
		}
	}
	return nullptr;
}

/*******************************************************************************
 *
 */
inline
bool
Posture::isMemberOfCategory(unsigned int categoryCode) const
{
	for (const auto& category : categoryList_) {
		if (category.code() == categoryCode) {
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
		for (const auto& category : categoryList_) {
			if (category.name() == categoryName) {
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
	if (category.code() != 0) {
		for (const auto& c : categoryList_) {
			if (c.code() == category.code()) {
				return true;
			}
		}
	} else if (name_ == category.name()) {
		return true;
	}
	return false;
}

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_POSTURE_H_ */
