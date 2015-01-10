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

#include "Model.h"

#include <algorithm> /* sort */
#include <iostream>
#include <utility> /* make_pair */

#include "Log.h"
#include "XMLConfigFileReader.h"
#include "XMLConfigFileWriter.h"



namespace GS {
namespace TRMControlModel {

/*******************************************************************************
 * Constructor.
 */
Model::Model()
{
}

/*******************************************************************************
 * Destructor.
 */
Model::~Model()
{
}

/*******************************************************************************
 *
 */
void
Model::clear()
{
	categoryList_.clear();

	parameterList_.clear();

	postureList_.clear();
	postureMap_.clear();

	ruleList_.clear();

	equationGroupList_.clear();
	equationMap_.clear();

	transitionGroupList_.clear();
	transitionMap_.clear();

	specialTransitionGroupList_.clear();
	specialTransitionMap_.clear();

	formulaSymbolList_.fill(0.0f);
}

/*******************************************************************************
 *
 */
void
Model::load(const char* configDirPath, const char* configFileName)
{
	clear();

	try {
		std::string filePath = std::string(configDirPath) + configFileName;

		// Load the configuration file.
		LOG_DEBUG("Loading xml configuration: " << filePath);
		XMLConfigFileReader cfg(*this, filePath);
		cfg.loadModel();

		preparePostures();
		prepareEquations();
		prepareTransitions();
		prepareRules();
	} catch (...) {
		clear();
		throw;
	}
}

/*******************************************************************************
 *
 */
void
Model::save(const char* configDirPath, const char* configFileName)
{
	std::string filePath = std::string(configDirPath) + configFileName;

	// Save the configuration file.
	LOG_DEBUG("Saving xml configuration: " << filePath);
	XMLConfigFileWriter cfg(*this, filePath);
	cfg.saveModel();
}

/*******************************************************************************
 *
 * Needed by Model::findPosture,
 *           Model::findFirstMatchingRule,
 *             Rule::evalBooleanExpression.
 */
void
Model::preparePostures()
{
	LOG_DEBUG("Preparing postures...");

	postureMap_.clear();
	for (auto& posture : postureList_) {
		auto res = postureMap_.insert(std::make_pair(posture->name(), posture.get()));
		if (!res.second) {
			THROW_EXCEPTION(TRMControlModelException, "Duplicate posture name: " << posture->name() << '.');
		}
	}
}

/*******************************************************************************
 *
 * Needed by Rule::evaluateExpressionSymbols,
 *           Model::evalEquationFormula.
 */
void
Model::prepareEquations()
{
	LOG_DEBUG("Preparing equations...");

	equationMap_.clear();
	for (auto& group : equationGroupList_) {
		for (auto& equation : group.equationList) {
			auto res = equationMap_.insert(std::make_pair(equation.name, &equation));
			if (!res.second) {
				THROW_EXCEPTION(TRMControlModelException, "Duplicate equation: " << equation.name << '.');
			}

			// Convert the formula expression to a tree.
			equation.parseFormula(formulaSymbol_);
		}
	}
}

/*******************************************************************************
 *
 * Needed by Model::findTransition,
 *           Model::findSpecialTransition.
 */
void
Model::prepareTransitions()
{
	LOG_DEBUG("Preparing transitions...");

	transitionMap_.clear();
	for (auto& group : transitionGroupList_) {
		for (auto& transition : group.transitionList) {
			auto res = transitionMap_.insert(std::make_pair(transition.name(), &transition));
			if (!res.second) {
				//THROW_EXCEPTION(TRMControlModelException, "Duplicate transition: " << transition.name() << '.');
				LOG_ERROR("Duplicate transition: " << transition.name() << " (ignored).");
			}
		}
	}

	specialTransitionMap_.clear();
	for (auto& group : specialTransitionGroupList_) {
		for (auto& transition : group.transitionList) {
			auto res = specialTransitionMap_.insert(std::make_pair(transition.name(), &transition));
			if (!res.second) {
				//THROW_EXCEPTION(TRMControlModelException, "Duplicate transition: " << transition.name() << '.');
				LOG_ERROR("Duplicate special transition: " << transition.name() << " (ignored).");
			}
		}
	}
}

/*******************************************************************************
 *
 * Needed by Model::findFirstMatchingRule,
 *             Rule::evalBooleanExpression.
 */
void
Model::prepareRules()
{
	LOG_DEBUG("Preparing rules...");

	// Convert the boolean expression string of each Rule to a tree.
	for (auto& rule : ruleList_) {
		rule->parseBooleanExpressions(*this);
	}
}

/*******************************************************************************
 *
 */
void
Model::printInfo() const
{
	//---------------------------------------------------------
	// Categories.
	std::cout << std::string(40, '-') << "\nCategories:\n" << std::endl;
	for (const auto& category : categoryList_) {
		std::cout << "category: " << category->name() << std::endl;
	}

	//---------------------------------------------------------
	// Postures.
	std::cout << std::string(40, '-') << "\nPostures:\n" << std::endl;
	for (const auto& posture : postureList()) {
		std::cout << "posture symbol: " << posture->name() << std::endl;
		for (const auto& category : posture->categoryList()) {
			std::cout << "  categ: " << category->name() << std::endl;
		}
		std::cout << "  parameter targets:" << std::endl;
		for (unsigned int i = 0, size = parameterList_.size(); i < size; ++i) {
			const Parameter& param = parameterList_[i];
			std::cout << "    " << param.name() << ": " << posture->getParameterTarget(i) << std::endl;
		}
		std::cout << "  symbol targets:" << std::endl;
		for (unsigned int i = 0, size = symbolList_.size(); i < size; ++i) {
			const Symbol& symbol = symbolList_[i];
			std::cout << "    " << symbol.name() << ": " << posture->getSymbolTarget(i) << std::endl;
		}
	}

	//---------------------------------------------------------
	// Equations.
	std::cout << std::string(40, '-') << "\nEquations:\n" << std::endl;
	FormulaSymbolList symbolList;
	symbolList[FormulaSymbol::SYMB_TRANSITION1] = 100.1f;
	symbolList[FormulaSymbol::SYMB_TRANSITION2] = 100.2f;
	symbolList[FormulaSymbol::SYMB_TRANSITION3] = 100.3f;
	symbolList[FormulaSymbol::SYMB_TRANSITION4] = 100.4f;
	symbolList[FormulaSymbol::SYMB_QSSA1] = 20.5f;
	symbolList[FormulaSymbol::SYMB_QSSA2] = 20.6f;
	symbolList[FormulaSymbol::SYMB_QSSA3] = 20.7f;
	symbolList[FormulaSymbol::SYMB_QSSA4] = 20.8f;
	symbolList[FormulaSymbol::SYMB_QSSB1] = 20.9f;
	symbolList[FormulaSymbol::SYMB_QSSB2] = 20.0f;
	symbolList[FormulaSymbol::SYMB_QSSB3] = 20.1f;
	symbolList[FormulaSymbol::SYMB_QSSB4] = 20.2f;
	symbolList[FormulaSymbol::SYMB_TEMPO1] = 1.1f;
	symbolList[FormulaSymbol::SYMB_TEMPO2] = 1.2f;
	symbolList[FormulaSymbol::SYMB_TEMPO3] = 1.3f;
	symbolList[FormulaSymbol::SYMB_TEMPO4] = 1.4f;
	symbolList[FormulaSymbol::SYMB_RD] = 150.4f;
	symbolList[FormulaSymbol::SYMB_MARK1] = 150.5f;
	symbolList[FormulaSymbol::SYMB_MARK2] = 150.6f;
	//symbolList[FormulaSymbol::SYMB_NULL] = 1.0f;
	for (const auto& group : equationGroupList_) {
		std::cout << "=== Equation group: " << group.name << std::endl;
		for (const auto& equation : group.equationList) {
			std::cout << "=== Equation: [" << equation.name << "]" << std::endl;
			std::cout << "    [" << equation.formula << "]" << std::endl;
			std::cout << *equation.formulaRoot << std::endl;
			std::cout << "*** EVAL=" << equation.formulaRoot->eval(symbolList) << std::endl;
		}
	}

	//---------------------------------------------------------
	// Transitions.
	std::cout << std::string(40, '-') << "\nTransitions:" << std::endl << std::endl;
	for (const auto& group : transitionGroupList_) {
		std::cout << "=== Transition group: " << group.name << std::endl;
		for (const auto& transition : group.transitionList) {
			std::cout << "### Transition: [" << transition.name() << "]" << std::endl;
			std::cout << "    type=" << transition.type() << " special=" << transition.special() << std::endl;

			for (auto& pointOrSlope : transition.pointOrSlopeList()) {
				if (!pointOrSlope->isSlopeRatio()) {
					const auto& point = dynamic_cast<const Transition::Point&>(*pointOrSlope);
					std::cout << "       point: type=" << point.type << " value=" << point.value
						<< " freeTime=" << point.freeTime << " timeExpression=" << point.timeExpression
						<< " isPhantom=" << point.isPhantom << std::endl;
				} else {
					const auto& slopeRatio = dynamic_cast<const Transition::SlopeRatio&>(*pointOrSlope);
					for (auto& point : slopeRatio.pointList) {
						std::cout << "         point: type=" << point->type << " value=" << point->value
							<< " freeTime=" << point->freeTime << " timeExpression=" << point->timeExpression
							<< " isPhantom=" << point->isPhantom << std::endl;
					}
					for (auto& slope : slopeRatio.slopeList) {
						std::cout << "         slope: slope=" << slope->slope << " displayTime=" << slope->displayTime << std::endl;
					}
				}
			}
		}
	}

	//---------------------------------------------------------
	// Special transitions.
	std::cout << std::string(40, '-') << "\nSpecial transitions:" << std::endl << std::endl;
	for (const auto& group : specialTransitionGroupList_) {
		std::cout << "=== Special transition group: " << group.name << std::endl;
		for (const auto& transition : group.transitionList) {
			std::cout << "### Transition: [" << transition.name() << "]" << std::endl;
			std::cout << "    type=" << transition.type() << " special=" << transition.special() << std::endl;

			for (const auto& pointOrSlope : transition.pointOrSlopeList()) {
				const auto& point = dynamic_cast<const Transition::Point&>(*pointOrSlope);

				std::cout << "       point: type=" << point.type << " value=" << point.value
					<< " freeTime=" << point.freeTime << " timeExpression=" << point.timeExpression
					<< " isPhantom=" << point.isPhantom << std::endl;
			}
		}
	}

	//---------------------------------------------------------
	// Rules.

	// Print boolean expression tree for each Rule.
	std::cout << std::string(40, '-') << "\nRules:\n" << std::endl;
	unsigned int ruleNumber = 0;
	for (auto& r : ruleList_) {
		std::cout << "--------------------------------------" << std::endl;
		std::cout << "Rule number: " << ++ruleNumber << '\n' << std::endl;
		r->printBooleanNodeTree();
	}
	std::cout << "--------------------------------------" << std::endl;
	std::vector<const Posture*> postSeq;
	const Posture* pp = findPosture("m");
	if (pp) postSeq.push_back(pp);
	pp = findPosture("ah");
	if (pp) postSeq.push_back(pp);
	pp = findPosture("s");
	if (pp) postSeq.push_back(pp);
	ruleNumber = 0;
	for (auto& r : ruleList_) {
		std::cout << "---" << std::endl;
		std::cout << "Rule number: " << ++ruleNumber << std::endl;
		std::cout << "bool=" << r->evalBooleanExpression(postSeq) << std::endl;
	}
	std::cout << "--------------------------------------" << std::endl;
	ruleNumber = 0;
	for (auto& r : ruleList_) {
		std::cout << "---" << std::endl;
		std::cout << "Rule number: " << ++ruleNumber << std::endl;
		std::cout << "Number of boolean expressions = " << r->numberOfExpressions() << std::endl;
	}
}

/*******************************************************************************
 *
 */
unsigned int
Model::findParameterIndex(const std::string& name) const
{
	for (unsigned int i = 0; i < parameterList_.size(); ++i) {
		if (parameterList_[i].name() == name) {
			return i;
		}
	}
	THROW_EXCEPTION(InvalidParameterException, "Parameter name not found: " << name << '.');
}

/*******************************************************************************
 * Find the Category.
 *
 * Returns an empty shared_ptr if the Category was not found.
 */
const std::shared_ptr<Category>
Model::findCategory(const std::string& name) const
{
	for (auto& category : categoryList_) {
		if (category->name() == name) {
			return category;
		}
	}
	return std::shared_ptr<Category>();
}

/*******************************************************************************
 * Find the Category.
 *
 * Returns an empty shared_ptr if the Category was not found.
 */
std::shared_ptr<Category>
Model::findCategory(const std::string& name)
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
bool
Model::findCategoryName(const std::string& name) const
{
	for (const auto& item : categoryList_) {
		if (item->name() == name) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
void
Model::clearFormulaSymbolList()
{
	formulaSymbolList_.fill(0.0);
}

/*******************************************************************************
 *
 */
void
Model::setFormulaSymbolValue(FormulaSymbol::Code symbol, float value)
{
	formulaSymbolList_[symbol] = value;
}

/*******************************************************************************
 *
 */
float
Model::getFormulaSymbolValue(FormulaSymbol::Code symbol) const
{
	return formulaSymbolList_[symbol];
}

/*******************************************************************************
 *
 */
void
Model::setDefaultFormulaSymbols(Transition::Type transitionType)
{
	setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION1, 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION2, 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION3, 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION4, 33.3333);

	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA1      , 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA2      , 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA3      , 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA4      , 33.3333);

	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB1      , 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB2      , 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB3      , 33.3333);
	setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB4      , 33.3333);

	setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO1, 1.0);
	setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO2, 1.0);
	setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO3, 1.0);
	setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO4, 1.0);

	setFormulaSymbolValue(FormulaSymbol::SYMB_BEAT ,  33.0);
	setFormulaSymbolValue(FormulaSymbol::SYMB_MARK1, 100.0);
	switch (transitionType) {
	case Transition::TYPE_DIPHONE:
		setFormulaSymbolValue(FormulaSymbol::SYMB_RD   , 100.0);
		setFormulaSymbolValue(FormulaSymbol::SYMB_MARK2,   0.0);
		setFormulaSymbolValue(FormulaSymbol::SYMB_MARK3,   0.0);
		break;
	case Transition::TYPE_TRIPHONE:
		setFormulaSymbolValue(FormulaSymbol::SYMB_RD   , 200.0);
		setFormulaSymbolValue(FormulaSymbol::SYMB_MARK2, 200.0);
		setFormulaSymbolValue(FormulaSymbol::SYMB_MARK3,   0.0);
		break;
	case Transition::TYPE_TETRAPHONE:
		setFormulaSymbolValue(FormulaSymbol::SYMB_RD   , 300.0);
		setFormulaSymbolValue(FormulaSymbol::SYMB_MARK2, 200.0);
		setFormulaSymbolValue(FormulaSymbol::SYMB_MARK3, 300.0);
		break;
	default:
		THROW_EXCEPTION(TRMControlModelException, "Invalid transition type: " << transitionType << '.');
	}
}

/*******************************************************************************
 *
 */
float
Model::evalEquationFormula(const std::string& equationName) const
{
	auto iter = equationMap_.find(equationName);
	if (iter == equationMap_.end()) {
		THROW_EXCEPTION(TRMControlModelException, "Equation not found: " << equationName << '.');
	}

	return iter->second->evalFormula(formulaSymbolList_);
}

/*******************************************************************************
 *
 */
bool
Model::findEquationGroupName(const std::string& name) const
{
	for (const auto& item : equationGroupList_) {
		if (item.name == name) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
bool
Model::findEquationName(const std::string& name) const
{
	for (const auto& group : equationGroupList_) {
		for (const auto& item : group.equationList) {
			if (item.name == name) {
				return true;
			}
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
bool
Model::findEquationIndex(const std::string& name, unsigned int& groupIndex, unsigned int& index) const
{
	for (unsigned int i = 0, groupListSize = equationGroupList_.size(); i < groupListSize; ++i) {
		const auto& group = equationGroupList_[i];
		for (unsigned int j = 0, size = group.equationList.size(); j < size; ++j) {
			if (group.equationList[j].name == name) {
				groupIndex = i;
				index = j;
				return true;
			}
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
float
Model::getParameterMinimum(unsigned int parameterIndex) const
{
	if (parameterIndex >= parameterList_.size()) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
	}

	return parameterList_[parameterIndex].minimum();
}

/*******************************************************************************
 *
 */
float
Model::getParameterMaximum(unsigned int parameterIndex) const
{
	if (parameterIndex >= parameterList_.size()) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
	}

	return parameterList_[parameterIndex].maximum();
}

/*******************************************************************************
 *
 */
const Parameter&
Model::getParameter(unsigned int parameterIndex) const
{
	if (parameterIndex >= parameterList_.size()) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
	}

	return parameterList_[parameterIndex];
}

/*******************************************************************************
 *
 */
bool
Model::findParameterName(const std::string& name) const
{
	for (const auto& item : parameterList_) {
		if (item.name() == name) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
bool
Model::findSymbolName(const std::string& name) const
{
	for (const auto& item : symbolList_) {
		if (item.name() == name) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 * Find a Posture with the given name.
 *
 * Returns a pointer to the Posture, or 0 (zero) if a Posture
 * was not found.
 */
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
 *
 */
void
Model::sortPostures()
{
	std::sort(postureList_.begin(), postureList_.end(), [](const std::unique_ptr<Posture>& p1, const std::unique_ptr<Posture>& p2) -> bool {
		return p1->name() < p2->name();
	});
}

/*******************************************************************************
 *
 */
bool
Model::findPostureName(const std::string& name) const
{
	for (const auto& item : postureList_) {
		if (item->name() == name) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 * Find a Transition with the given name.
 *
 * Returns a pointer to the Transition, or 0 (zero) if a Transition
 * was not found.
 */
const Transition*
Model::findTransition(const std::string& name) const
{
	auto transIter = transitionMap_.find(name);
	if (transIter == transitionMap_.end()) {
		return nullptr;
	}
	return transIter->second;
}

/*******************************************************************************
 *
 */
bool
Model::findTransitionGroupName(const std::string& name) const
{
	for (const auto& item : transitionGroupList_) {
		if (item.name == name) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
bool
Model::findTransitionName(const std::string& name) const
{
	for (const auto& group : transitionGroupList_) {
		for (const auto& item : group.transitionList) {
			if (item.name() == name) {
				return true;
			}
		}
	}
	return false;
}

/*******************************************************************************
 * Find a Special Transition with the given name.
 *
 * Returns a pointer to the Special Transition, or 0 (zero) if a Special
 * Transition was not found.
 */
const Transition*
Model::findSpecialTransition(const std::string& name) const
{
	auto transIter = specialTransitionMap_.find(name);
	if (transIter == specialTransitionMap_.end()) {
		return nullptr;
	}
	return transIter->second;
}

/*******************************************************************************
 *
 */
bool
Model::findSpecialTransitionGroupName(const std::string& name) const
{
	for (const auto& item : specialTransitionGroupList_) {
		if (item.name == name) {
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 *
 */
bool
Model::findSpecialTransitionName(const std::string& name) const
{
	for (const auto& group : specialTransitionGroupList_) {
		for (const auto& item : group.transitionList) {
			if (item.name() == name) {
				return true;
			}
		}
	}
	return false;
}

/*******************************************************************************
 * Finds the first Rule that matches the given sequence of Postures.
 */
const Rule*
Model::findFirstMatchingRule(const std::vector<const Posture*>& postureSequence, unsigned int& ruleIndex) const
{
	if (ruleList_.empty()) {
		ruleIndex = 0;
		return nullptr;
	}

	unsigned int i = 0;
	for (const auto& r : ruleList_) {
		if (r->numberOfExpressions() <= postureSequence.size()) {
			if (r->evalBooleanExpression(postureSequence)) {
				ruleIndex = i;
				return r.get();
			}
		}
		++i;
	}

	ruleIndex = 0;
	return nullptr;
}

} /* namespace TRMControlModel */
} /* namespace GS */
