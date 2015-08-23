/***************************************************************************
 *  Copyright 2014, 2015 Marcelo Y. Matuda                                 *
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

#include "XMLConfigFileReader.h"

#include <memory>

#include "Exception.h"
#include "Log.h"
#include "Model.h"
#include "RapidXmlUtil.h"
#include "Text.h"



using namespace rapidxml;

namespace {

const std::string booleanExpressionTagName   = "boolean-expression";
const std::string booleanExpressionsTagName  = "boolean-expressions";
const std::string categoriesTagName          = "categories";
const std::string categoryTagName            = "category";
const std::string categoryRefTagName         = "category-ref";
const std::string commentTagName             = "comment";
const std::string equationTagName            = "equation";
const std::string equationGroupTagName       = "equation-group";
const std::string equationsTagName           = "equations";
const std::string expressionSymbolsTagName   = "expression-symbols";
const std::string parameterTagName           = "parameter";
const std::string parameterProfilesTagName   = "parameter-profiles";
const std::string parametersTagName          = "parameters";
const std::string parameterTargetsTagName    = "parameter-targets";
const std::string parameterTransitionTagName = "parameter-transition";
const std::string pointOrSlopesTagName       = "point-or-slopes";
const std::string pointTagName               = "point";
const std::string pointsTagName              = "points";
const std::string postureCategoriesTagName   = "posture-categories";
const std::string posturesTagName            = "postures";
const std::string postureTagName             = "posture";
const std::string ruleTagName                = "rule";
const std::string rulesTagName               = "rules";
const std::string slopeTagName               = "slope";
const std::string slopeRatioTagName          = "slope-ratio";
const std::string slopesTagName              = "slopes";
const std::string specialProfilesTagName     = "special-profiles";
const std::string specialTransitionsTagName  = "special-transitions";
const std::string symbolEquationTagName      = "symbol-equation";
const std::string symbolsTagName             = "symbols";
const std::string symbolTagName              = "symbol";
const std::string symbolTargetsTagName       = "symbol-targets";
const std::string targetTagName              = "target";
const std::string transitionTagName          = "transition";
const std::string transitionGroupTagName     = "transition-group";
const std::string transitionsTagName         = "transitions";

const std::string defaultAttrName        = "default";
const std::string displayTimeAttrName    = "display-time";
const std::string equationAttrName       = "equation";
const std::string formulaAttrName        = "formula";
const std::string freeTimeAttrName       = "free-time";
const std::string isPhantomAttrName      = "is-phantom";
const std::string maximumAttrName        = "maximum";
const std::string minimumAttrName        = "minimum";
const std::string nameAttrName           = "name";
const std::string p12AttrName            = "p12";
const std::string p23AttrName            = "p23";
const std::string p34AttrName            = "p34";
const std::string slopeAttrName          = "slope";
const std::string symbolAttrName         = "symbol";
const std::string timeExpressionAttrName = "time-expression";
const std::string typeAttrName           = "type";
const std::string transitionAttrName     = "transition";
const std::string valueAttrName          = "value";

const std::string beatSymbolName       = "beat";
const std::string durationSymbolName   = "duration";
const std::string mark1SymbolName      = "mark1";
const std::string mark2SymbolName      = "mark2";
const std::string mark3SymbolName      = "mark3";
const std::string qssaSymbolName       = "qssa";
const std::string qssbSymbolName       = "qssb";
const std::string rdSymbolName         = "rd";
const std::string transitionSymbolName = "transition";

} /* namespace */

namespace GS {
namespace TRMControlModel {

void
XMLConfigFileReader::parseCategories(rapidxml::xml_node<char>* categoriesElem)
{
	for (xml_node<char>* categoryElem = firstChild(categoriesElem, categoryTagName);
				categoryElem;
				categoryElem = nextSibling(categoryElem, categoryTagName)) {

		std::shared_ptr<Category> newCategory(new Category(attributeValue(categoryElem, nameAttrName)));
		model_.categoryList().push_back(newCategory);

		xml_node<char>* commentElem = firstChild(categoryElem, commentTagName);
		if (commentElem) {
			model_.categoryList().back()->setComment(commentElem->value());
		}
	}
}

void
XMLConfigFileReader::parseParameters(rapidxml::xml_node<char>* parametersElem)
{
	for (xml_node<char>* parameterElem = firstChild(parametersElem, parameterTagName);
				parameterElem;
				parameterElem = nextSibling(parameterElem, parameterTagName)) {

		std::string name   = attributeValue(parameterElem, nameAttrName);
		float minimum      = Text::parseString<float>(attributeValue(parameterElem, minimumAttrName));
		float maximum      = Text::parseString<float>(attributeValue(parameterElem, maximumAttrName));
		float defaultValue = Text::parseString<float>(attributeValue(parameterElem, defaultAttrName));
		std::string comment;

		xml_node<char>* commentElem = firstChild(parameterElem, commentTagName);
		if (commentElem) {
			comment = commentElem->value();
		}

		model_.parameterList().emplace_back(name, minimum, maximum, defaultValue, comment);
	}
}

void
XMLConfigFileReader::parseSymbols(rapidxml::xml_node<char>* symbolsElem)
{
	for (xml_node<char>* symbolElem = firstChild(symbolsElem, symbolTagName);
				symbolElem;
				symbolElem = nextSibling(symbolElem, symbolTagName)) {

		std::string name   = attributeValue(symbolElem, nameAttrName);
		float minimum      = Text::parseString<float>(attributeValue(symbolElem, minimumAttrName));
		float maximum      = Text::parseString<float>(attributeValue(symbolElem, maximumAttrName));
		float defaultValue = Text::parseString<float>(attributeValue(symbolElem, defaultAttrName));
		std::string comment;

		xml_node<char>* commentElem = firstChild(symbolElem, commentTagName);
		if (commentElem) {
			comment = commentElem->value();
		}

		model_.symbolList().emplace_back(name, minimum, maximum, defaultValue, comment);
	}
}

void
XMLConfigFileReader::parsePostureSymbols(rapidxml::xml_node<char>* symbolTargetsElem, Posture& posture)
{
	for (xml_node<char>* targetElem = firstChild(symbolTargetsElem, targetTagName);
				targetElem;
				targetElem = nextSibling(targetElem, targetTagName)) {

		const std::string name = attributeValue(targetElem, nameAttrName);
		const std::string value = attributeValue(targetElem, valueAttrName);
		for (unsigned int i = 0, size = model_.symbolList().size(); i < size; ++i) {
			const Symbol& symbol = model_.symbolList()[i];
			if (symbol.name() == name) {
				posture.setSymbolTarget(i, Text::parseString<float>(value));
			}
		}
	}
}

void
XMLConfigFileReader::parsePostureCategories(rapidxml::xml_node<char>* postureCategoriesElem, Posture& posture)
{
	for (xml_node<char>* catRefElem = firstChild(postureCategoriesElem, categoryRefTagName);
				catRefElem;
				catRefElem = nextSibling(catRefElem, categoryRefTagName)) {

		const std::string name = attributeValue(catRefElem, nameAttrName);
		if (name != posture.name()) {
			std::shared_ptr<Category> postureCat = model_.findCategory(name);
			if (!postureCat) {
				THROW_EXCEPTION(TRMControlModelException, "Posture category not found: " << name << '.');
			}
			posture.categoryList().push_back(postureCat);
		}
	}
}

void
XMLConfigFileReader::parsePostureParameters(rapidxml::xml_node<char>* parameterTargetsElem, Posture& posture)
{
	for (xml_node<char>* targetElem = firstChild(parameterTargetsElem, targetTagName);
				targetElem;
				targetElem = nextSibling(targetElem, targetTagName)) {

		std::string parameterName = attributeValue(targetElem, nameAttrName);
		unsigned int parameterIndex = model_.findParameterIndex(parameterName);

		posture.setParameterTarget(parameterIndex, Text::parseString<float>(attributeValue(targetElem, valueAttrName)));
	}
}

void
XMLConfigFileReader::parsePosture(rapidxml::xml_node<char>* postureElem)
{
	std::unique_ptr<Posture> posture(new Posture(
						attributeValue(postureElem, symbolAttrName),
						model_.parameterList().size(),
						model_.symbolList().size()));

	for (xml_node<char>* childElem = firstChild(postureElem);
				childElem;
				childElem = nextSibling(childElem)) {
		if (compareElementName(childElem, postureCategoriesTagName)) {
			parsePostureCategories(childElem, *posture);
		} else if (compareElementName(childElem, parameterTargetsTagName)) {
			parsePostureParameters(childElem, *posture);
		} else if (compareElementName(childElem, symbolTargetsTagName)) {
			parsePostureSymbols(childElem, *posture);
		} else if (compareElementName(childElem, commentTagName)) {
			posture->setComment(childElem->value());
		}
	}

	model_.postureList().add(std::move(posture));
}

void
XMLConfigFileReader::parsePostures(rapidxml::xml_node<char>* posturesElem)
{
	for (xml_node<char>* postureElem = firstChild(posturesElem, postureTagName);
				postureElem;
				postureElem = nextSibling(postureElem, postureTagName)) {
		parsePosture(postureElem);
	}
}

void
XMLConfigFileReader::parseEquationsGroup(rapidxml::xml_node<char>* equationGroupElem)
{
	EquationGroup group;
	group.name = attributeValue(equationGroupElem, nameAttrName);

	for (xml_node<char>* equationElem = firstChild(equationGroupElem, equationTagName);
				equationElem;
				equationElem = nextSibling(equationElem, equationTagName)) {

		std::string name = attributeValue(equationElem, nameAttrName);
		std::string formula = attributeValue(equationElem, formulaAttrName, true);
		std::string comment;
		xml_node<char>* commentElem = firstChild(equationElem, commentTagName);
		if (commentElem) {
			comment = commentElem->value();
		}

		if (formula.empty()) {
			LOG_ERROR("Equation " << name << " without formula (ignored).");
		} else {
			std::shared_ptr<Equation> eq(new Equation(name));
			eq->setFormula(formula);
			if (!comment.empty()) {
				eq->setComment(comment);
			}

			group.equationList.push_back(eq);
		}
	}

	model_.equationGroupList().push_back(std::move(group));
}

void
XMLConfigFileReader::parseEquations(rapidxml::xml_node<char>* equationsElem)
{
	for (xml_node<char>* groupElem = firstChild(equationsElem, equationGroupTagName);
				groupElem;
				groupElem = nextSibling(groupElem, equationGroupTagName)) {
		parseEquationsGroup(groupElem);
	}
}

void
XMLConfigFileReader::parseSlopeRatio(rapidxml::xml_node<char>* slopeRatioElem, Transition& transition)
{
	std::unique_ptr<Transition::SlopeRatio> p(new Transition::SlopeRatio());

	for (xml_node<char>* childElem = firstChild(slopeRatioElem);
				childElem;
				childElem = nextSibling(childElem)) {
		if (compareElementName(childElem, pointsTagName)) {
			for (xml_node<char>* pointElem = firstChild(childElem, pointTagName);
						pointElem;
						pointElem = nextSibling(pointElem, pointTagName)) {
				std::unique_ptr<Transition::Point> p2(new Transition::Point());
				p2->type = Transition::Point::getTypeFromName(attributeValue(pointElem, typeAttrName));
				p2->value = Text::parseString<float>(attributeValue(pointElem, valueAttrName));

				const std::string timeExpr = attributeValue(pointElem, timeExpressionAttrName, true);
				if (timeExpr.empty()) {
					p2->freeTime = Text::parseString<float>(attributeValue(pointElem, freeTimeAttrName));
				} else {
					std::shared_ptr<Equation> equation = model_.findEquation(timeExpr);
					if (!equation) {
						THROW_EXCEPTION(UnavailableResourceException, "Equation not found: " << timeExpr << '.');
					}
					p2->timeExpression = equation;
				}

				if (std::string("yes") == attributeValue(pointElem, isPhantomAttrName, true)) {
					p2->isPhantom = true;
				}
				p->pointList.push_back(std::move(p2));
			}
		} else if (compareElementName(childElem, slopesTagName)) {
			for (xml_node<char>* slopeElem = firstChild(childElem, slopeTagName);
						slopeElem;
						slopeElem = nextSibling(slopeElem, slopeTagName)) {
				std::unique_ptr<Transition::Slope> p2(new Transition::Slope());
				p2->slope = Text::parseString<float>(attributeValue(slopeElem, slopeAttrName));
				p2->displayTime = Text::parseString<float>(attributeValue(slopeElem, displayTimeAttrName));
				p->slopeList.push_back(std::move(p2));
			}
		}
	}

	transition.pointOrSlopeList().push_back(std::move(p));
}

void
XMLConfigFileReader::parseTransitionPointOrSlopes(rapidxml::xml_node<char>* pointOrSlopesElem, Transition& transition)
{
	for (xml_node<char>* childElem = firstChild(pointOrSlopesElem);
				childElem;
				childElem = nextSibling(childElem)) {
		if (compareElementName(childElem, pointTagName)) {
			std::unique_ptr<Transition::Point> p(new Transition::Point());
			p->type = Transition::Point::getTypeFromName(attributeValue(childElem, typeAttrName));
			p->value = Text::parseString<float>(attributeValue(childElem, valueAttrName));

			const std::string timeExpr = attributeValue(childElem, timeExpressionAttrName, true);
			if (timeExpr.empty()) {
				p->freeTime = Text::parseString<float>(attributeValue(childElem, freeTimeAttrName));
			} else {
				std::shared_ptr<Equation> equation = model_.findEquation(timeExpr);
				if (!equation) {
					THROW_EXCEPTION(UnavailableResourceException, "Equation not found: " << timeExpr << '.');
				}
				p->timeExpression = equation;
			}

			if (std::string("yes") == attributeValue(childElem, isPhantomAttrName, true)) {
				p->isPhantom = true;
			}
			transition.pointOrSlopeList().push_back(std::move(p));
		} else if (compareElementName(childElem, slopeRatioTagName)) {
			parseSlopeRatio(childElem, transition);
		}
	}
}

void
XMLConfigFileReader::parseTransitionsGroup(rapidxml::xml_node<char>* transitionGroupElem, bool special)
{
	TransitionGroup group;
	group.name = attributeValue(transitionGroupElem, nameAttrName);

	for (xml_node<char>* childElem = firstChild(transitionGroupElem,transitionTagName);
				childElem;
				childElem = nextSibling(childElem, transitionTagName)) {

		std::string name = attributeValue(childElem, nameAttrName);
		Transition::Type type = Transition::getTypeFromName(attributeValue(childElem, typeAttrName));

		std::shared_ptr<Transition> tr(new Transition(name, type, special));

		for (xml_node<char>* transitionChildElem = firstChild(childElem);
					transitionChildElem;
					transitionChildElem = nextSibling(transitionChildElem)) {
			if (compareElementName(transitionChildElem, pointOrSlopesTagName)) {
				parseTransitionPointOrSlopes(transitionChildElem, *tr);
			} else if (compareElementName(transitionChildElem, commentTagName)) {
				tr->setComment(transitionChildElem->value());
			}
		}

		group.transitionList.push_back(tr);
	}

	if (special) {
		model_.specialTransitionGroupList().push_back(std::move(group));
	} else {
		model_.transitionGroupList().push_back(std::move(group));
	}
}

void
XMLConfigFileReader::parseTransitions(rapidxml::xml_node<char>* transitionsElem, bool special)
{
	for (xml_node<char>* groupElem = firstChild(transitionsElem, transitionGroupTagName);
				groupElem;
				groupElem = nextSibling(groupElem, transitionGroupTagName)) {
		parseTransitionsGroup(groupElem, special);
	}
}

void
XMLConfigFileReader::parseRuleParameterProfiles(rapidxml::xml_node<char>* parameterProfilesElem, Rule& rule)
{
	for (xml_node<char>* paramTransElem = firstChild(parameterProfilesElem,parameterTransitionTagName);
				paramTransElem;
				paramTransElem = nextSibling(paramTransElem, parameterTransitionTagName)) {

		std::string parameterName = attributeValue(paramTransElem, nameAttrName);
		unsigned int parameterIndex = model_.findParameterIndex(parameterName);

		const std::string transitionName = attributeValue(paramTransElem, transitionAttrName);
		std::shared_ptr<Transition> transition = model_.findTransition(transitionName);
		if (!transition) {
			THROW_EXCEPTION(UnavailableResourceException, "Transition not found: " << transitionName << '.');
		}

		rule.setParamProfileTransition(parameterIndex, transition);
	}
}

void
XMLConfigFileReader::parseRuleSpecialProfiles(rapidxml::xml_node<char>* specialProfilesElem, Rule& rule)
{
	for (xml_node<char>* paramTransElem = firstChild(specialProfilesElem, parameterTransitionTagName);
				paramTransElem;
				paramTransElem = nextSibling(paramTransElem, parameterTransitionTagName)) {

		std::string parameterName = attributeValue(paramTransElem, nameAttrName);
		unsigned int parameterIndex = model_.findParameterIndex(parameterName);

		const std::string transitionName = attributeValue(paramTransElem, transitionAttrName);
		std::shared_ptr<Transition> transition = model_.findSpecialTransition(transitionName);
		if (!transition) {
			THROW_EXCEPTION(UnavailableResourceException, "Special transition not found: " << transitionName << '.');
		}

		rule.setSpecialProfileTransition(parameterIndex, transition);
	}
}

void
XMLConfigFileReader::parseRuleExpressionSymbols(rapidxml::xml_node<char>* expressionSymbolsElem, Rule& rule)
{
	for (xml_node<char>* symbEquElem = firstChild(expressionSymbolsElem, symbolEquationTagName);
				symbEquElem;
				symbEquElem = nextSibling(symbEquElem, symbolEquationTagName)) {

		const std::string name = attributeValue(symbEquElem, nameAttrName);
		const std::string equationName = attributeValue(symbEquElem, equationAttrName);

		std::shared_ptr<Equation> equation = model_.findEquation(equationName);
		if (!equation) {
			THROW_EXCEPTION(UnavailableResourceException, "Equation not found: " << equationName << '.');
		}

		if (name == rdSymbolName) {
			rule.exprSymbolEquations().ruleDuration = equation;
		} else if (name == beatSymbolName) {
			rule.exprSymbolEquations().beat = equation;
		} else if (name == mark1SymbolName) {
			rule.exprSymbolEquations().mark1 = equation;
		} else if (name == mark2SymbolName) {
			rule.exprSymbolEquations().mark2 = equation;
		} else if (name == mark3SymbolName) {
			rule.exprSymbolEquations().mark3 = equation;
		}
	}
}

void
XMLConfigFileReader::parseRuleBooleanExpressions(rapidxml::xml_node<char>* booleanExpressionsElem, Rule& rule)
{
	std::vector<std::string> exprList;
	for (xml_node<char>* boolExprElem = firstChild(booleanExpressionsElem, booleanExpressionTagName);
				boolExprElem;
				boolExprElem = nextSibling(boolExprElem, booleanExpressionTagName)) {
		exprList.push_back(boolExprElem->value());
	}
	rule.setBooleanExpressionList(exprList, model_);
}

void
XMLConfigFileReader::parseRule(rapidxml::xml_node<char>* ruleElem)
{
	std::unique_ptr<Rule> rule(new Rule(model_.parameterList().size()));

	for (xml_node<char>* childElem = firstChild(ruleElem);
				childElem;
				childElem = nextSibling(childElem)) {
		if (compareElementName(childElem, booleanExpressionsTagName)) {
			parseRuleBooleanExpressions(childElem, *rule);
		} else if (compareElementName(childElem, parameterProfilesTagName)) {
			parseRuleParameterProfiles(childElem, *rule);
		} else if (compareElementName(childElem, specialProfilesTagName)) {
			parseRuleSpecialProfiles(childElem, *rule);
		} else if (compareElementName(childElem, expressionSymbolsTagName)) {
			parseRuleExpressionSymbols(childElem, *rule);
		} else if (compareElementName(childElem, commentTagName)) {
			rule->setComment(childElem->value());
		}
	}

	model_.ruleList().push_back(std::move(rule));
}

void
XMLConfigFileReader::parseRules(rapidxml::xml_node<char>* rulesElem)
{
	for (xml_node<char>* ruleElem = firstChild(rulesElem, ruleTagName);
				ruleElem;
				ruleElem = nextSibling(ruleElem, ruleTagName)) {
		parseRule(ruleElem);
	}
}



/*******************************************************************************
 * Constructor.
 */
XMLConfigFileReader::XMLConfigFileReader(Model& model, const std::string& filePath)
		: model_(model)
		, filePath_(filePath)
{
}

/*******************************************************************************
 * Destructor.
 */
XMLConfigFileReader::~XMLConfigFileReader()
{
}

void
XMLConfigFileReader::loadModel()
{
	std::string source = readXMLFile(filePath_);
	xml_document<char> doc;
	doc.parse<parse_no_data_nodes | parse_validate_closing_tags>(&source[0]);

	xml_node<char>* root = doc.first_node();
	if (root == 0) {
		THROW_EXCEPTION(XMLException, "Root element not found.");
	}

	LOG_DEBUG("categories");
	xml_node<char>* categoriesElem = firstChild(root, categoriesTagName);
	if (categoriesElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Categories element not found.");
	}
	parseCategories(categoriesElem);

	LOG_DEBUG("parameters");
	xml_node<char>* parametersElem = firstChild(root, parametersTagName);
	if (parametersElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Parameters element not found.");
	}
	parseParameters(parametersElem);

	LOG_DEBUG("symbols");
	xml_node<char>* symbolsElem = firstChild(root, symbolsTagName);
	if (symbolsElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Categories element not found.");
	}
	parseSymbols(symbolsElem);

	LOG_DEBUG("postures");
	xml_node<char>* posturesElem = firstChild(root, posturesTagName);
	if (posturesElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Postures element not found.");
	}
	parsePostures(posturesElem);

	LOG_DEBUG("equations");
	xml_node<char>* equationsElem = firstChild(root, equationsTagName);
	if (equationsElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Equations element not found.");
	}
	parseEquations(equationsElem);

	LOG_DEBUG("transitions");
	xml_node<char>* transitionsElem = firstChild(root, transitionsTagName);
	if (transitionsElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Transitions element not found.");
	}
	parseTransitions(transitionsElem, false);

	LOG_DEBUG("special-transitions");
	xml_node<char>* specialTransitionsElem = firstChild(root, specialTransitionsTagName);
	if (specialTransitionsElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Special-transitions element not found.");
	}
	parseTransitions(specialTransitionsElem, true);

	LOG_DEBUG("rules");
	xml_node<char>* rulesElem = firstChild(root, rulesTagName);
	if (rulesElem == 0) {
		THROW_EXCEPTION(TRMControlModelException, "Rules element not found.");
	}
	parseRules(rulesElem);
}

} /* namespace TRMControlModel */
} /* namespace GS */
