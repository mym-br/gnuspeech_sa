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

#include "ConfigFile.h"

#include <iostream>

#include "Category.h"
#include "Exception.h"
#include "Log.h"
#include "Model.h"
#include "Parameter.h"
#include "Phone.h"
#include "Text.h"



namespace GS {
namespace TRMControlModel {

void
ConfigFile::parseCategories()
{
	typedef CategoryMap::iterator MI;
	typedef CategoryMap::value_type VT;

	int n = 0;
	for (const std::string* category = parser_.getFirstChild(categoryTagName_);
				category;
				category = parser_.getNextSibling(categoryTagName_)) {
		const std::string& name = parser_.getAttribute(nameAttrName_);

		Category_ptr cat(new Category(name, ++n)); // starts with 1
		VT item(name, std::move(cat));
		std::pair<MI, bool> res = model_.categoryMap_.insert(std::move(item));
		if (!res.second) {
			THROW_EXCEPTION(TRMControlModelException, "Duplicate category: " << name << '.');
		}

		if (parser_.getFirstChild()) { // skip comment
			parser_.getNextSibling();
		}
	}
}

void
ConfigFile::parseParameters()
{
	for (const std::string* parameter = parser_.getFirstChild(parameterTagName_);
				parameter;
				parameter = parser_.getNextSibling(parameterTagName_)) {
		Parameter::setInfo(
			model_.parameterInfoArray_,
			model_.parameterCodeMap_,
			parser_.getAttribute(nameAttrName_),
			Text::parseString<float>(parser_.getAttribute(minimumAttrName_)),
			Text::parseString<float>(parser_.getAttribute(maximumAttrName_)),
			Text::parseString<float>(parser_.getAttribute(defaultAttrName_)));
	}
}

void
ConfigFile::parseSymbols()
{
	for (const std::string* symbol = parser_.getFirstChild(symbolTagName_);
				symbol;
				symbol = parser_.getNextSibling(symbolTagName_)) {
		// Ignore the attributes.
	}
}

void
ConfigFile::parsePostureSymbols(Phone& phone)
{
	for (const std::string* target = parser_.getFirstChild(targetTagName_);
				target;
				target = parser_.getNextSibling(targetTagName_)) {
		const std::string& name = parser_.getAttribute(nameAttrName_);
		const std::string& value = parser_.getAttribute(valueAttrName_);
		if (name == durationSymbolName_) {
			phone.symbols_.duration = Text::parseString<float>(value);
		} else if (name == transitionSymbolName_) {
			phone.symbols_.transition = Text::parseString<float>(value);
		} else if (name == qssaSymbolName_) {
			phone.symbols_.qssa = Text::parseString<float>(value);
		} else if (name == qssbSymbolName_) {
			phone.symbols_.qssb = Text::parseString<float>(value);
		}
	}
}

void
ConfigFile::parsePostureCategories(Phone& phone)
{
	for (const std::string* catRef = parser_.getFirstChild(categoryRefTagName_);
				catRef;
				catRef = parser_.getNextSibling(categoryRefTagName_)) {
		Category_ptr cat(new Category(parser_.getAttribute(nameAttrName_), 0));
		phone.categoryList_.push_back(std::move(cat));
	}
}

void
ConfigFile::parsePostureParameters(Phone& phone)
{
	for (const std::string* target = parser_.getFirstChild(targetTagName_);
				target;
				target = parser_.getNextSibling(targetTagName_)) {
		Parameter::setValue(
			model_.parameterCodeMap_,
			phone.parameters_,
			parser_.getAttribute(nameAttrName_),
			Text::parseString<float>(parser_.getAttribute(valueAttrName_)));
	}
}

void
ConfigFile::parsePosture()
{
	Phone_ptr p(new Phone(parser_.getAttribute(symbolAttrName_)));

	for (const std::string* child = parser_.getFirstChild();
				child;
				child = parser_.getNextSibling()) {
		if (*child == postureCategoriesTagName_) {
			parsePostureCategories(*p);
		} else if (*child == parameterTargetsTagName_) {
			parsePostureParameters(*p);
		} else if (*child == symbolTargetsTagName_) {
			parsePostureSymbols(*p);
		}
	}

	const std::string& name = p->name_;
	insert(model_.phoneMap_, name, std::move(p));
}

void
ConfigFile::parsePostures()
{
	for (const std::string* posture = parser_.getFirstChild(postureTagName_);
				posture;
				posture = parser_.getNextSibling(postureTagName_)) {
		parsePosture();
	}
}

void
ConfigFile::parseEquationsGroup()
{
	typedef EquationMap::iterator MI;
	typedef EquationMap::value_type VT;

	const std::string groupName = parser_.getAttribute(nameAttrName_);
	// Can't be a reference because it is used many times in the loop.

	for (const std::string* equation = parser_.getFirstChild(equationTagName_);
				equation;
				equation = parser_.getNextSibling(equationTagName_)) {
		Equation_ptr p(new Equation());
		p->groupName = groupName;
		p->name = parser_.getAttribute(nameAttrName_);
		p->formula = parser_.getAttribute(formulaAttrName_);

		if (p->formula.empty()) {
			LOG_ERROR("Equation " << p->name << " without formula (ignored)."); // should not happen
		} else {
			const std::string& name = p->name;
			std::pair<MI,bool> res = model_.equationMap_.insert(VT(name, std::move(p)));
			if (!res.second) {
				THROW_EXCEPTION(TRMControlModelException, "Duplicate equation: " << name << '.');
			}
		}

		if (parser_.getFirstChild()) {
			parser_.getNextSibling(); // skip comment
		}
	}
}

void
ConfigFile::parseEquations()
{
	for (const std::string* group = parser_.getFirstChild(equationGroupTagName_);
				group;
				group = parser_.getNextSibling(equationGroupTagName_)) {
		parseEquationsGroup();
	}
}

void
ConfigFile::parseSlopeRatio(Transition& transition)
{
	std::unique_ptr<Transition::SlopeRatio> p(new Transition::SlopeRatio());

	for (const std::string* child = parser_.getFirstChild();
				child;
				child = parser_.getNextSibling()) {
		if (*child == pointsTagName_) {
			for (const std::string* point = parser_.getFirstChild(pointTagName_);
						point;
						point = parser_.getNextSibling(pointTagName_)) {
				std::unique_ptr<Transition::Point> p2(new Transition::Point());
				p2->type = Transition::Point::getTypeFromName(parser_.getAttribute(typeAttrName_));
				p2->value = Text::parseString<float>(parser_.getAttribute(valueAttrName_));
				p2->timeExpression = parser_.getAttribute(timeExpressionAttrName_);
				if (p2->timeExpression.empty()) {
					p2->freeTime = Text::parseString<float>(parser_.getAttribute(freeTimeAttrName_));
				}
				if (parser_.getAttribute(isPhantomAttrName_) == "yes") {
					p2->isPhantom = true;
				}
				p->pointList.push_back(std::move(p2));
			}
		} else if (*child == slopesTagName_) {
			for (const std::string* slope = parser_.getFirstChild(slopeTagName_);
						slope;
						slope = parser_.getNextSibling(slopeTagName_)) {
				std::unique_ptr<Transition::Slope> p2(new Transition::Slope());
				p2->slope = Text::parseString<float>(parser_.getAttribute(slopeAttrName_));
				p2->displayTime = Text::parseString<float>(parser_.getAttribute(displayTimeAttrName_));
				p->slopeList.push_back(std::move(p2));
			}
		}
	}

	transition.pointOrSlopeList_.push_back(std::move(p));
}

void
ConfigFile::parseTransitionPointOrSlopes(Transition& transition)
{
	for (const std::string* child = parser_.getFirstChild();
				child;
				child = parser_.getNextSibling()) {
		if (*child == pointTagName_) {
			std::unique_ptr<Transition::Point> p(new Transition::Point());
			p->type = Transition::Point::getTypeFromName(parser_.getAttribute(typeAttrName_));
			p->value = Text::parseString<float>(parser_.getAttribute(valueAttrName_));
			p->timeExpression = parser_.getAttribute(timeExpressionAttrName_);
			if (p->timeExpression.empty()) {
				p->freeTime = Text::parseString<float>(parser_.getAttribute(freeTimeAttrName_));
			}
			if (parser_.getAttribute(isPhantomAttrName_) == "yes") {
				p->isPhantom = true;
			}
			transition.pointOrSlopeList_.push_back(std::move(p));
		} else if (*child == slopeRatioTagName_) {
			parseSlopeRatio(transition);
		}
	}
}

void
ConfigFile::parseTransitionsGroup(bool special)
{
	typedef TransitionMap::iterator MI;
	typedef TransitionMap::value_type VT;

	const std::string groupName = parser_.getAttribute(nameAttrName_);
	// Can't be a reference because it is used many times in the loop.

	TransitionMap& map = special ? model_.specialTransitionMap_ : model_.transitionMap_;

	for (const std::string* child = parser_.getFirstChild(transitionTagName_);
				child;
				child = parser_.getNextSibling(transitionTagName_)) {
		Transition_ptr p(new Transition(special));
		p->groupName_ = groupName;
		p->name_ = parser_.getAttribute(nameAttrName_);
		p->type_ = Transition::getTypeFromName(parser_.getAttribute(typeAttrName_));

		for (const std::string* transitionChild = parser_.getFirstChild();
					transitionChild;
					transitionChild = parser_.getNextSibling()) {
			if (*transitionChild == pointOrSlopesTagName_) {
				parseTransitionPointOrSlopes(*p);
			} // else: comment
		}

		const std::string name = p->name_;
		std::pair<MI, bool> res = map.insert(VT(name, std::move(p)));
		if (!res.second) {
			//THROW_EXCEPTION(TRMControlModelException, "Duplicate transition: " << name << '.');
			LOG_ERROR("Duplicate transition: " << name << " (ignored).");
		}
	}
}

void
ConfigFile::parseTransitions(bool special)
{
	for (const std::string* group = parser_.getFirstChild(transitionGroupTagName_);
				group;
				group = parser_.getNextSibling(transitionGroupTagName_)) {
		parseTransitionsGroup(special);
	}
}

void
ConfigFile::parseRuleParameterProfiles(Rule& rule)
{
	for (const std::string* paramTrans = parser_.getFirstChild(parameterTransitionTagName_);
				paramTrans;
				paramTrans = parser_.getNextSibling(parameterTransitionTagName_)) {
		Parameter::setValue(
			model_.parameterCodeMap_,
			rule.paramProfileTransitions_,
			parser_.getAttribute(nameAttrName_),
			parser_.getAttribute(transitionAttrName_));
	}
}

void
ConfigFile::parseRuleSpecialProfiles(Rule& rule)
{
	for (const std::string* paramTrans = parser_.getFirstChild(parameterTransitionTagName_);
				paramTrans;
				paramTrans = parser_.getNextSibling(parameterTransitionTagName_)) {
		Parameter::setValue(
			model_.parameterCodeMap_,
			rule.specialProfileTransitions_,
			parser_.getAttribute(nameAttrName_),
			parser_.getAttribute(transitionAttrName_));
	}
}

void
ConfigFile::parseRuleExpressionSymbols(Rule& rule)
{
	for (const std::string* symbEqu = parser_.getFirstChild(symbolEquationTagName_);
				symbEqu;
				symbEqu = parser_.getNextSibling(symbolEquationTagName_)) {
		const std::string& name = parser_.getAttribute(nameAttrName_);
		const std::string& equation = parser_.getAttribute(equationAttrName_);
		if (name == rdSymbolName_) {
			rule.exprSymbolEquations_.ruleDuration = equation;
		} else if (name == beatSymbolName_) {
			rule.exprSymbolEquations_.beat = equation;
		} else if (name == mark1SymbolName_) {
			rule.exprSymbolEquations_.mark1 = equation;
		} else if (name == mark2SymbolName_) {
			rule.exprSymbolEquations_.mark2 = equation;
		} else if (name == mark3SymbolName_) {
			rule.exprSymbolEquations_.mark3 = equation;
		}
	}
}

void
ConfigFile::parseRuleBooleanExpressions(Rule& rule)
{
	for (const std::string* boolExpr = parser_.getFirstChild(booleanExpressionTagName_);
				boolExpr;
				boolExpr = parser_.getNextSibling(booleanExpressionTagName_)) {
		rule.booleanExpressionList_.push_back(parser_.getText());
	}
}

void
ConfigFile::parseRule()
{
	Rule_ptr p(new Rule());
	p->number_ = model_.ruleList_.size() + 1;

	for (const std::string* child = parser_.getFirstChild();
				child;
				child = parser_.getNextSibling()) {
		if (*child == booleanExpressionsTagName_) {
			parseRuleBooleanExpressions(*p);
		} else if (*child == parameterProfilesTagName_) {
			parseRuleParameterProfiles(*p);
		} else if (*child == specialProfilesTagName_) {
			parseRuleSpecialProfiles(*p);
		} else if (*child == expressionSymbolsTagName_) {
			parseRuleExpressionSymbols(*p);
		}
	}

	model_.ruleList_.push_back(std::move(p));
}

void
ConfigFile::parseRules()
{
	for (const std::string* rule = parser_.getFirstChild(ruleTagName_);
				rule;
				rule = parser_.getNextSibling(ruleTagName_)) {
		parseRule();
	}
}



/*******************************************************************************
 * Constructor.
 */
ConfigFile::ConfigFile(Model& model, const std::string& filePath)
		: model_(model)

		, booleanExpressionTagName_  ("boolean-expression")
		, booleanExpressionsTagName_ ("boolean-expressions")
		, categoriesTagName_         ("categories")
		, categoryTagName_           ("category")
		, categoryRefTagName_        ("category-ref")
		, equationTagName_           ("equation")
		, equationGroupTagName_      ("equation-group")
		, equationsTagName_          ("equations")
		, expressionSymbolsTagName_  ("expression-symbols")
		, parameterTagName_          ("parameter")
		, parameterProfilesTagName_  ("parameter-profiles")
		, parametersTagName_         ("parameters")
		, parameterTargetsTagName_   ("parameter-targets")
		, parameterTransitionTagName_("parameter-transition")
		, pointOrSlopesTagName_      ("point-or-slopes")
		, pointTagName_              ("point")
		, pointsTagName_             ("points")
		, postureCategoriesTagName_  ("posture-categories")
		, posturesTagName_           ("postures")
		, postureTagName_            ("posture")
		, ruleTagName_               ("rule")
		, rulesTagName_              ("rules")
		, slopeTagName_              ("slope")
		, slopeRatioTagName_         ("slope-ratio")
		, slopesTagName_             ("slopes")
		, specialProfilesTagName_    ("special-profiles")
		, specialTransitionsTagName_ ("special-transitions")
		, symbolEquationTagName_     ("symbol-equation")
		, symbolsTagName_            ("symbols")
		, symbolTagName_             ("symbol")
		, symbolTargetsTagName_      ("symbol-targets")
		, targetTagName_             ("target")
		, transitionTagName_         ("transition")
		, transitionGroupTagName_    ("transition-group")
		, transitionsTagName_        ("transitions")

		, defaultAttrName_       ("default")
		, displayTimeAttrName_   ("display-time")
		, equationAttrName_      ("equation")
		, formulaAttrName_       ("formula")
		, freeTimeAttrName_      ("free-time")
		, isPhantomAttrName_     ("is-phantom")
		, maximumAttrName_       ("maximum")
		, minimumAttrName_       ("minimum")
		, nameAttrName_          ("name")
		, p12AttrName_           ("p12")
		, p23AttrName_           ("p23")
		, p34AttrName_           ("p34")
		, slopeAttrName_         ("slope")
		, symbolAttrName_        ("symbol")
		, timeExpressionAttrName_("time-expression")
		, typeAttrName_          ("type")
		, transitionAttrName_    ("transition")
		, valueAttrName_         ("value")

		, beatSymbolName_      ("beat")
		, durationSymbolName_  ("duration")
		, mark1SymbolName_     ("mark1")
		, mark2SymbolName_     ("mark2")
		, mark3SymbolName_     ("mark3")
		, qssaSymbolName_      ("qssa")
		, qssbSymbolName_      ("qssb")
		, rdSymbolName_        ("rd")
		, transitionSymbolName_("transition")

		, parser_(filePath)
{
	const std::string* root = parser_.getFirstChild();
	if (root == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Root element not found.");
	}
}

/*******************************************************************************
 * Destructor.
 */
ConfigFile::~ConfigFile()
{
}

/*******************************************************************************
 *
 */
void
ConfigFile::loadModel()
{
	LOG_DEBUG("categories");
	if (parser_.getFirstChild(categoriesTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Categories element not found.");
	}
	parseCategories();

	LOG_DEBUG("parameters");
	if (parser_.getNextSibling(parametersTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Parameters element not found.");
	}
	parseParameters();

	LOG_DEBUG("symbols");
	if (parser_.getNextSibling(symbolsTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Categories element not found.");
	}
	parseSymbols();

	LOG_DEBUG("postures");
	if (parser_.getNextSibling(posturesTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Postures element not found.");
	}
	parsePostures();

	LOG_DEBUG("equations");
	if (parser_.getNextSibling(equationsTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Equations element not found.");
	}
	parseEquations();

	LOG_DEBUG("transitions");
	if (parser_.getNextSibling(transitionsTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Transitions element not found.");
	}
	parseTransitions(false);

	LOG_DEBUG("special-transitions");
	if (parser_.getNextSibling(specialTransitionsTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Special-transitions element not found.");
	}
	parseTransitions(true);

	LOG_DEBUG("rules");
	if (parser_.getNextSibling(rulesTagName_) == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Rules element not found.");
	}
	parseRules();
}

} /* namespace TRMControlModel */
} /* namespace GS */
