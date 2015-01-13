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

#include <memory>
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

	Posture(const std::string& name, unsigned int numParameters, unsigned int numSymbols)
			: name_(name)
			, parameterTargetList_(numParameters)
			, symbolTargetList_(numSymbols) {
		if (numParameters == 0) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid number of parameters: 0.");
		}
		if (numSymbols == 0) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid number of symbols: 0.");
		}

		std::shared_ptr<Category> newCategory(new Category(name));
		newCategory->setNative();
		categoryList_.push_back(newCategory);
	}

	const std::string& name() const { return name_; }

	const std::vector<std::shared_ptr<Category>>& categoryList() const { return categoryList_; }
	std::vector<std::shared_ptr<Category>>& categoryList() { return categoryList_; }

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

	float getSymbolTarget(unsigned int symbolIndex) const {
		if (symbolIndex >= symbolTargetList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid symbol index: " << symbolIndex << '.');
		}
		return symbolTargetList_[symbolIndex];
	}
	void setSymbolTarget(unsigned int symbolIndex, float target) {
		if (symbolIndex >= symbolTargetList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid symbol index: " << symbolIndex << '.');
		}
		symbolTargetList_[symbolIndex] = target;
	}

	const std::string& comment() const { return comment_; }
	void setComment(const std::string& comment) { comment_ = comment; }

	bool isMemberOfCategory(const Category& category) const;
	const std::shared_ptr<Category> findCategory(const std::string& name) const;

	std::unique_ptr<Posture> copy(const std::string& newName) const;
private:
	std::string name_; // must be immutable
	std::vector<std::shared_ptr<Category>> categoryList_;
	std::vector<float> parameterTargetList_;
	std::vector<float> symbolTargetList_;
	std::string comment_;
};



/*******************************************************************************
 *
 */
inline
const std::shared_ptr<Category>
Posture::findCategory(const std::string& name) const
{
	for (auto& category : categoryList_) {
		if (category->name() == name) {
			return category;
		}
	}
	return std::shared_ptr<Category>();
}

/*******************************************************************************
 *
 */
inline
bool
Posture::isMemberOfCategory(const Category& category) const
{
	for (const auto& postureCat : categoryList_) {
		if (postureCat.get() == &category) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
inline
std::unique_ptr<Posture>
Posture::copy(const std::string& newName) const
{
	std::unique_ptr<Posture> newPosture(new Posture(newName, parameterTargetList_.size(), symbolTargetList_.size()));

	for (const auto& category : categoryList_) {
		if (!category->native()) {
			newPosture->categoryList_.push_back(category);
		}
	}
	newPosture->parameterTargetList_ = parameterTargetList_;
	newPosture->symbolTargetList_ = symbolTargetList_;
	newPosture->comment_ = comment_;

	return newPosture;
}

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_POSTURE_H_ */
