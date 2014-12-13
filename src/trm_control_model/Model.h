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
#include "Symbol.h"
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
	void save(const char* configDirPath, const char* configFileName);
	void printInfo() const;
	void clearFormulaSymbolList();
	void setFormulaSymbolValue(FormulaSymbol::Code symbol, float value);
	float getFormulaSymbolValue(FormulaSymbol::Code symbol) const;

	const std::list<Equation>& equationList() const { return equationList_; }
	std::list<Equation>& equationList() { return equationList_; }
	const std::vector<std::string>& equationGroupList() const { return equationGroupList_; }
	std::vector<std::string>& equationGroupList() { return equationGroupList_; }
	float evalEquationFormula(const std::string& equationName) const;

	const std::vector<Parameter>& parameterList() const { return parameterList_; }
	std::vector<Parameter>& parameterList() { return parameterList_; }
	unsigned int getNumParameters() const { return parameterList_.size(); }
	unsigned int findParameterIndex(const std::string& name) const;
	float getParameterMinimum(unsigned int parameterIndex) const;
	float getParameterMaximum(unsigned int parameterIndex) const;
	const Parameter& getParameter(unsigned int parameterIndex) const;

	const std::vector<Symbol>& symbolList() const { return symbolList_; }
	std::vector<Symbol>& symbolList() { return symbolList_; }

	const std::list<Posture>& postureList() const { return postureList_; }
	std::list<Posture>& postureList() { return postureList_; }
	const Posture* findPosture(const std::string& name) const;
	void addPosture(Posture& posture);

	const std::vector<std::string>& transitionGroupList() const { return transitionGroupList_; }
	std::vector<std::string>& transitionGroupList() { return transitionGroupList_; }
	const std::list<Transition>& transitionList() const { return transitionList_; }
	std::list<Transition>& transitionList() { return transitionList_; }
	const Transition* findTransition(const std::string& name) const;

	const std::vector<std::string>& specialTransitionGroupList() const { return specialTransitionGroupList_; }
	std::vector<std::string>& specialTransitionGroupList() { return specialTransitionGroupList_; }
	const std::list<Transition>& specialTransitionList() const { return specialTransitionList_; }
	std::list<Transition>& specialTransitionList() { return specialTransitionList_; }
	const Transition* findSpecialTransition(const std::string& name) const;

	const std::list<Rule>& ruleList() const { return ruleList_; }
	std::list<Rule>& ruleList() { return ruleList_; }
	const Rule* findFirstMatchingRule(const std::vector<const Posture*>& postureSequence, unsigned int& ruleIndex) const;

	const std::vector<Category>& categoryList() const { return categoryList_; }
	std::vector<Category>& categoryList() { return categoryList_; }
	const Category* findCategory(const std::string& name) const;
	unsigned int getCategoryCode(const std::string& name) const;

private:
	std::vector<Category> categoryList_;
	std::unordered_map<std::string, Category*> categoryMap_; // optimization

	std::vector<Parameter> parameterList_;

	std::vector<Symbol> symbolList_;

	std::list<Posture> postureList_;
	std::unordered_map<std::string, Posture*> postureMap_; // optimization

	std::list<Rule> ruleList_;

	std::vector<std::string> equationGroupList_;
	std::list<Equation> equationList_;
	std::unordered_map<std::string, Equation*> equationMap_; // optimization

	std::vector<std::string> transitionGroupList_;
	std::list<Transition> transitionList_;
	std::unordered_map<std::string, Transition*> transitionMap_; // optimization

	std::vector<std::string> specialTransitionGroupList_;
	std::list<Transition> specialTransitionList_;
	std::unordered_map<std::string, Transition*> specialTransitionMap_; // optimization

	FormulaSymbolList formulaSymbolList_;
	const FormulaSymbol formulaSymbol_;

	void prepareCategories();
	void preparePostures();
	void prepareEquations();
	void prepareTransitions();
	void prepareRules();
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_MODEL_H_ */
