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

#include <memory>
#include <string>
#include <vector>

#include "Category.h"
#include "Equation.h"
#include "Exception.h"
#include "FormulaSymbol.h"
#include "Parameter.h"
#include "Posture.h"
#include "PostureList.h"
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
	void setDefaultFormulaSymbols(Transition::Type transitionType);

	const std::vector<EquationGroup>& equationGroupList() const { return equationGroupList_; }
	std::vector<EquationGroup>& equationGroupList() { return equationGroupList_; }
	float evalEquationFormula(const Equation& equation) const;
	bool findEquationGroupName(const std::string& name) const;
	bool findEquationName(const std::string& name) const;
	bool findEquationIndex(const std::string& name, unsigned int& groupIndex, unsigned int& index) const;
	std::shared_ptr<Equation> findEquation(const std::string& name);

	const std::vector<Parameter>& parameterList() const { return parameterList_; }
	std::vector<Parameter>& parameterList() { return parameterList_; }
	unsigned int findParameterIndex(const std::string& name) const;
	float getParameterMinimum(unsigned int parameterIndex) const;
	float getParameterMaximum(unsigned int parameterIndex) const;
	const Parameter& getParameter(unsigned int parameterIndex) const;
	bool findParameterName(const std::string& name) const;

	const std::vector<Symbol>& symbolList() const { return symbolList_; }
	std::vector<Symbol>& symbolList() { return symbolList_; }
	bool findSymbolName(const std::string& name) const;

	const PostureList& postureList() const { return postureList_; }
	PostureList& postureList() { return postureList_; }

	const std::vector<TransitionGroup>& transitionGroupList() const { return transitionGroupList_; }
	std::vector<TransitionGroup>& transitionGroupList() { return transitionGroupList_; }
	const std::shared_ptr<Transition> findTransition(const std::string& name) const;
	bool findTransitionGroupName(const std::string& name) const;
	bool findTransitionName(const std::string& name) const;
	bool findTransitionIndex(const std::string& name, unsigned int& groupIndex, unsigned int& index) const;

	const std::vector<TransitionGroup>& specialTransitionGroupList() const { return specialTransitionGroupList_; }
	std::vector<TransitionGroup>& specialTransitionGroupList() { return specialTransitionGroupList_; }
	const std::shared_ptr<Transition> findSpecialTransition(const std::string& name) const;
	bool findSpecialTransitionGroupName(const std::string& name) const;
	bool findSpecialTransitionName(const std::string& name) const;
	bool findSpecialTransitionIndex(const std::string& name, unsigned int& groupIndex, unsigned int& index) const;

	const std::vector<std::unique_ptr<Rule>>& ruleList() const { return ruleList_; }
	std::vector<std::unique_ptr<Rule>>& ruleList() { return ruleList_; }
	const Rule* findFirstMatchingRule(const std::vector<const Posture*>& postureSequence, unsigned int& ruleIndex) const;

	const std::vector<std::shared_ptr<Category>>& categoryList() const { return categoryList_; }
	std::vector<std::shared_ptr<Category>>& categoryList() { return categoryList_; }
	const std::shared_ptr<Category> findCategory(const std::string& name) const;
	std::shared_ptr<Category> findCategory(const std::string& name);
	bool findCategoryName(const std::string& name) const;
private:
	std::vector<std::shared_ptr<Category>> categoryList_;
	std::vector<Parameter> parameterList_;
	std::vector<Symbol> symbolList_;
	PostureList postureList_;
	std::vector<std::unique_ptr<Rule>> ruleList_;
	std::vector<EquationGroup> equationGroupList_;
	std::vector<TransitionGroup> transitionGroupList_;
	std::vector<TransitionGroup> specialTransitionGroupList_;
	FormulaSymbolList formulaSymbolList_;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_MODEL_H_ */
