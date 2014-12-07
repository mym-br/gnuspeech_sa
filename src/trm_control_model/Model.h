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

#ifndef TRM_CONTROL_MODEL_MODEL_H_
#define TRM_CONTROL_MODEL_MODEL_H_

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Category.h"
#include "Equation.h"
#include "Exception.h"
#include "FormulaSymbol.h"
#include "Parameter.h"
#include "Phone.h"
#include "Rule.h"
#include "Transition.h"



namespace GS {
namespace TRMControlModel {

class ConfigFile;
class InputFile;

class Model {
public:
	Model();
	~Model();

	void load(const char* configDirPath, const char* configFileName);
	void printInfo() const;
	void clearFormulaSymbolList();
	void setFormulaSymbolValue(FormulaSymbol::Code symbol, float value);
	float getFormulaSymbolValue(FormulaSymbol::Code symbol) const;
	float evalEquationFormula(const std::string& equationName) const;
	float getParameterMinimum(Parameter::Code parameter) const;
	float getParameterMaximum(Parameter::Code parameter) const;
	const Parameter::Info& getParameterInfo(int parameter) const;
	const Phone* findPhone(const std::string& name) const;
	const Transition* findTransition(const std::string& name) const;
	const Transition* findSpecialTransition(const std::string& name) const;
	const Rule* findFirstMatchingRule(const PhoneSequence& phoneSequence, int& ruleIndex) const;
	const Category* findCategory(const std::string& name) const;
private:
	CategoryMap categoryMap_;
	Parameter::CodeMap parameterCodeMap_;
	Parameter::Info::Array parameterInfoArray_;
	PhoneMap phoneMap_;
	RuleList ruleList_;
	FormulaSymbol formulaSymbol_;
	EquationMap equationMap_;
	TransitionMap transitionMap_;
	TransitionMap specialTransitionMap_;
	FormulaSymbolList formulaSymbolList_;

	void preparePhones();
	void prepareEquations();
	void prepareRules();

	friend class XMLConfigFile;
};



/*******************************************************************************
 *
 */
inline
void
Model::clearFormulaSymbolList()
{
	formulaSymbolList_.fill(0.0);
}

/*******************************************************************************
 *
 */
inline
void
Model::setFormulaSymbolValue(FormulaSymbol::Code symbol, float value)
{
	formulaSymbolList_[symbol] = value;
}

/*******************************************************************************
 *
 */
inline
float
Model::getFormulaSymbolValue(FormulaSymbol::Code symbol) const
{
	return formulaSymbolList_[symbol];
}

/*******************************************************************************
 *
 */
inline
float
Model::evalEquationFormula(const std::string& equationName) const
{
	EquationMap::const_iterator iter = equationMap_.find(equationName);
	if (iter == equationMap_.end()) {
		THROW_EXCEPTION(TRMControlModelException, "Equation not found: " << equationName << '.');
	}

	return iter->second->evalFormula(formulaSymbolList_);
}

/*******************************************************************************
 *
 */
inline
float
Model::getParameterMinimum(Parameter::Code parameter) const
{
	return parameterInfoArray_[parameter].minimum;
}

/*******************************************************************************
 *
 */
inline
float
Model::getParameterMaximum(Parameter::Code parameter) const
{
	return parameterInfoArray_[parameter].maximum;
}

/*******************************************************************************
 *
 */
inline
const Parameter::Info&
Model::getParameterInfo(int parameter) const
{
	return parameterInfoArray_[parameter];
}

///*******************************************************************************
// *
// */
//inline
//void
//Model::clipParameterValue(float& value, Parameter::Code parameter) const
//{
//	if (Parameter::isVolumeParameter(parameter)) {
//		// Volumes are not in dB.
//		if (value < 0.0f) {
//			value = 0.0f;
//		} else if (value > 1.0f) {
//			value = 1.0f;
//		}
//	} else {
//		const Parameter::Info& info = parameterInfoArray_[parameter];
//		if (value < info.minimum) {
//			value = info.minimum;
//		} else {
//			const float max = info.minimum + info.span;
//			if (value > max) {
//				value = max;
//			}
//		}
//	}
//}

/*******************************************************************************
 * Find a Phone with the given name.
 *
 * Returns a pointer to the Phone, or 0 (zero) if a Phone
 * was not found.
 */
inline
const Phone*
Model::findPhone(const std::string& name) const
{
	PhoneMap::const_iterator itPost = phoneMap_.find(name);
	if (itPost == phoneMap_.end()) {
		return nullptr;
	}
	return itPost->second.get();
}

/*******************************************************************************
 * Find a Transition with the given name.
 *
 * Returns a pointer to the Transition, or 0 (zero) if a Transition
 * was not found.
 */
inline
const Transition*
Model::findTransition(const std::string& name) const
{
	TransitionMap::const_iterator itTrans = transitionMap_.find(name);
	if (itTrans == transitionMap_.end()) {
		return nullptr;
	}
	return itTrans->second.get();
}

/*******************************************************************************
 * Find a Special Transition with the given name.
 *
 * Returns a pointer to the Special Transition, or 0 (zero) if a Special
 * Transition was not found.
 */
inline
const Transition*
Model::findSpecialTransition(const std::string& name) const
{
	TransitionMap::const_iterator itTrans = specialTransitionMap_.find(name);
	if (itTrans == specialTransitionMap_.end()) {
		return nullptr;
	}
	return itTrans->second.get();
}

/*******************************************************************************
 * Finds the first Rule that matches the given sequence of Phones.
 */
inline
const Rule*
Model::findFirstMatchingRule(const PhoneSequence& phoneSequence, int& ruleIndex) const
{
	for (int i = 0; i < ruleList_.size(); ++i) {
		const Rule& r = *ruleList_[i];
		if (r.numberOfExpressions() <= phoneSequence.size()) {
			if (r.evalBooleanExpression(phoneSequence)) {
				ruleIndex = i;
				return &r;
			}
		}
	}
	return ruleList_.back().get();
}

/*******************************************************************************
 * Find the Category code.
 *
 * Returns nullptr if the Category was not found.
 */
inline
const Category*
Model::findCategory(const std::string& name) const
{
	auto iter = categoryMap_.find(name);
	if (iter != categoryMap_.end()) {
		return iter->second.get();
	}
	return nullptr;
}

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_MODEL_H_ */
