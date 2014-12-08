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
#include "Posture.h"
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

	void clear();
	void load(const char* configDirPath, const char* configFileName);
	void printInfo() const;
	void clearFormulaSymbolList();
	void setFormulaSymbolValue(FormulaSymbol::Code symbol, float value);
	float getFormulaSymbolValue(FormulaSymbol::Code symbol) const;
	float evalEquationFormula(const std::string& equationName) const;
	unsigned int getNumParameters() const { return parameterList_.size(); }
	unsigned int findParameterIndex(const std::string& name) const;
	float getParameterMinimum(unsigned int parameterIndex) const;
	float getParameterMaximum(unsigned int parameterIndex) const;
	const Parameter& getParameter(unsigned int parameterIndex) const;
	const Posture* findPosture(const std::string& name) const;
	void addPosture(Posture& posture);
	const Transition* findTransition(const std::string& name) const;
	const Transition* findSpecialTransition(const std::string& name) const;
	const Rule* findFirstMatchingRule(const std::vector<const Posture*>& postureSequence, unsigned int& ruleIndex) const;
	const Category* findCategory(const std::string& name) const;
	unsigned int getCategoryCode(const std::string& name) const;

private:
	std::vector<Category> categoryList_;
	std::unordered_map<std::string, Category*> categoryMap_; // optimization
	std::vector<Parameter_ptr> parameterList_;
	std::list<Posture> postureList_;
	std::unordered_map<std::string, Posture*> postureMap_; // optimization
	RuleList ruleList_;
	const FormulaSymbol formulaSymbol_;
	EquationMap equationMap_;
	TransitionMap transitionMap_;
	TransitionMap specialTransitionMap_;
	FormulaSymbolList formulaSymbolList_;

	void prepareCategories();
	void preparePostures();
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
Model::getParameterMinimum(unsigned int parameterIndex) const
{
	if (parameterIndex >= parameterList_.size()) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
	}

	return parameterList_[parameterIndex]->minimum();
}

/*******************************************************************************
 *
 */
inline
float
Model::getParameterMaximum(unsigned int parameterIndex) const
{
	if (parameterIndex >= parameterList_.size()) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
	}

	return parameterList_[parameterIndex]->maximum();
}

/*******************************************************************************
 *
 */
inline
const Parameter&
Model::getParameter(unsigned int parameterIndex) const
{
	if (parameterIndex >= parameterList_.size()) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
	}

	return *parameterList_[parameterIndex];
}

/*******************************************************************************
 * Find a Posture with the given name.
 *
 * Returns a pointer to the Posture, or 0 (zero) if a Posture
 * was not found.
 */
inline
const Posture*
Model::findPosture(const std::string& name) const
{
	auto postureIter = postureMap_.find(name);
	if (postureIter == postureMap_.end()) {
		return nullptr;
	}
	return postureIter->second;
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
 * Finds the first Rule that matches the given sequence of Postures.
 */
inline
const Rule*
Model::findFirstMatchingRule(const std::vector<const Posture*>& postureSequence, unsigned int& ruleIndex) const
{
	for (unsigned int i = 0; i < ruleList_.size(); ++i) {
		const Rule& r = *ruleList_[i];
		if (r.numberOfExpressions() <= postureSequence.size()) {
			if (r.evalBooleanExpression(postureSequence)) {
				ruleIndex = i;
				return &r;
			}
		}
	}
	return ruleList_.back().get();
}

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_MODEL_H_ */
