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

#include <iostream>

#include "ConfigFile.h"
#include "Log.h"



namespace {

//const char* PHONEME_TRANSLATOR_CONFIG_FILE_NAME = "/phoneme_translation.config";
//const char* PHONE_REWRITER_CONFIG_FILE_NAME = "/phone_rewriting.config";

} /* namespace */

//==============================================================================

namespace GS {
namespace TRMControlModel {

/*******************************************************************************
 * Constructor.
 */
Model::Model()
		//: phonemeRewriter_(configDirPath + PHONEME_TRANSLATOR_CONFIG_FILE_NAME)
		//, phoneRewriter_(configDirPath + PHONE_REWRITER_CONFIG_FILE_NAME, phoneMap_)
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
Model::load(const char* configDirPath, const char* configFileName)
{
	std::string filePath = std::string(configDirPath) + configFileName;

	// Load the configuration file.
	LOG_DEBUG("Loading xml configuration: " << filePath);
	ConfigFile cfg(*this, filePath);
	cfg.loadModel();

	preparePhones();
	prepareEquations();
	prepareRules();
}

/*******************************************************************************
 *
 */
void
Model::preparePhones()
{
	LOG_DEBUG("Preparing phones...");

	// Fill the category code in the category list of each Phone.
	for (auto itPost = phoneMap_.begin(); itPost != phoneMap_.end(); ++itPost) {
		const PhoneMap::value_type& v = *itPost;
		for (auto itCat = v.second->categoryList().begin(); itCat != v.second->categoryList().end(); ++itCat) {
			auto itMap = categoryMap_.find((*itCat)->name);
			if (itMap != categoryMap_.end()) {
				(*itCat)->code = itMap->second->code;
			}
		}
	}
}

/*******************************************************************************
 *
 */
void
Model::prepareEquations()
{
	LOG_DEBUG("Preparing equations...");

	// Convert the formula expression to a tree.
	for (EquationMap::const_iterator iter = equationMap_.begin(); iter != equationMap_.end(); ++iter) {
		Equation& e = *iter->second;
		e.parseFormula(formulaSymbol_);
	}
}

/*******************************************************************************
 *
 */
void
Model::prepareRules()
{
	LOG_DEBUG("Preparing rules...");

	// Convert the boolean expression string of each Rule to a tree.
	for (RuleList::const_iterator iter = ruleList_.begin(); iter != ruleList_.end(); ++iter) {
		Rule& r = **iter;
		r.parseBooleanExpression(categoryMap_);
	}
}

/*******************************************************************************
 *
 */
void
Model::filterInput(InputFile& inFile)
{
	//inFile.rewritePhonemes(phonemeRewriter_);
	//inFile.rewritePhones(phoneRewriter_);

	//TODO: remove? move?
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
	for (auto iter = categoryMap_.begin();
			iter != categoryMap_.end(); ++iter) {
		std::cout << "category: " << iter->first << " code: " << iter->second->code << std::endl;
	}

	//---------------------------------------------------------
	// Phones.
	std::cout << std::string(40, '-') << "\nPhones:\n" << std::endl;
	for (PhoneMap::const_iterator iter = phoneMap_.begin(); iter != phoneMap_.end(); ++iter) {
		const PhoneMap::value_type& v = *iter;
		std::cout << "phone symbol: " << v.second->name() << " r8: " << v.second->getParameterValue(Parameter::PARAMETER_R8) << std::endl;

		for (CategoryList::const_iterator it2 = v.second->categoryList().begin();
				it2 != v.second->categoryList().end(); ++it2) {
			std::cout << "  categ: " << (*it2)->name << "_" << (*it2)->code << std::endl;
		}
		std::cout << "  symbols.duration: "   << v.second->symbols().duration   << std::endl;
		std::cout << "  symbols.transition: " << v.second->symbols().transition << std::endl;
		std::cout << "  symbols.qssa: "       << v.second->symbols().qssa       << std::endl;
		std::cout << "  symbols.qssb: "       << v.second->symbols().qssb       << std::endl;
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
	for (EquationMap::const_iterator iter = equationMap_.begin(); iter != equationMap_.end(); ++iter) {
		const EquationMap::value_type& v = *iter;
		std::cout << "=== Equation: [" << v.second->name << "] group: " << v.second->groupName << std::endl;
		std::cout << "    [" << v.second->formula << "]" << std::endl;
		std::cout << *v.second->formulaRoot << std::endl;
		std::cout << "*** EVAL=" << v.second->formulaRoot->eval(symbolList) << std::endl;
	}

	//---------------------------------------------------------
	// Transitions.
	std::cout << std::string(40, '-') << "\nTransitions:" << std::endl << std::endl;
	for (auto& item : transitionMap_) {
		const Transition& t = *item.second;
		std::cout << "### Transition: [" << t.name() << "] group: " << t.groupName()  << std::endl;
		std::cout << "    type=" << t.type() << " special=" << t.special() << std::endl;

		for (auto& pointOrSlope : t.pointOrSlopeList()) {
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

	//---------------------------------------------------------
	// Special transitions.
	std::cout << std::string(40, '-') << "\nSpecial transitions:" << std::endl << std::endl;
	for (auto& item : specialTransitionMap_) {
		const Transition& t = *item.second;
		std::cout << "### Transition: [" << t.name() << "] group: " << t.groupName()  << std::endl;
		std::cout << "    type=" << t.type() << " special=" << t.special() << std::endl;

		for (const auto& pointOrSlope : t.pointOrSlopeList()) {
			const auto& point = dynamic_cast<const Transition::Point&>(*pointOrSlope);

			std::cout << "       point: type=" << point.type << " value=" << point.value
				<< " freeTime=" << point.freeTime << " timeExpression=" << point.timeExpression
				<< " isPhantom=" << point.isPhantom << std::endl;
		}
	}

	//---------------------------------------------------------
	// Rules.

	// Print boolean expression tree for each Rule.
	std::cout << std::string(40, '-') << "\nRules:\n" << std::endl;
	for (RuleList::const_iterator iter = ruleList_.begin(); iter != ruleList_.end(); ++iter) {
		const Rule& r = **iter;
		std::cout << "--------------------------------------" << std::endl;
		std::cout << "Rule number: " << r.number() << '\n' << std::endl;
		r.printBooleanNodeTree();
	}
	std::cout << "--------------------------------------" << std::endl;
	PhoneSequence postSeq;
	const Phone* pp = findPhone("m");
	if (pp) postSeq.push_back(pp);
	pp = findPhone("ah");
	if (pp) postSeq.push_back(pp);
	pp = findPhone("s");
	if (pp) postSeq.push_back(pp);
	for (RuleList::const_iterator iter = ruleList_.begin(); iter != ruleList_.end(); ++iter) {
		const Rule& r = **iter;
		std::cout << "---" << std::endl;
		std::cout << "Rule number: " << r.number() << std::endl;
		std::cout << "bool=" << r.evalBooleanExpression(postSeq) << std::endl;
	}
	std::cout << "--------------------------------------" << std::endl;
	for (RuleList::const_iterator iter = ruleList_.begin(); iter != ruleList_.end(); ++iter) {
		const Rule& r = **iter;
		std::cout << "---" << std::endl;
		std::cout << "Rule number: " << r.number() << std::endl;
		std::cout << "Number of boolean expressions = " << r.numberOfExpressions() << std::endl;
	}
}

} /* namespace TRMControlModel */
} /* namespace GS */
