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

#include "XMLConfigFileWriter.h"

#include <cstring> /* strerror */
#include <fstream>
#include <sstream>

#include "Exception.h"
#include "Log.h"
#include "Model.h"
#include "StreamXMLWriter.h"



namespace GS {
namespace TRMControlModel {

/*******************************************************************************
 * Constructor.
 */
XMLConfigFileWriter::XMLConfigFileWriter(const Model& model, const std::string& filePath)
		: model_(model)
		, filePath_(filePath)
{
}

/*******************************************************************************
 * Destructor.
 */
XMLConfigFileWriter::~XMLConfigFileWriter()
{
}

/*******************************************************************************
 *
 */
void
XMLConfigFileWriter::saveModel()
{
	std::ofstream out(filePath_, std::ios_base::out | std::ios_base::binary);
	if (!out) {
		THROW_EXCEPTION(IOException, "The output file " << filePath_ << " could not be created.");
	}

	StreamXMLWriter xml(out, 2);
	xml.writeDeclaration();

	xml.openElementWithAttributes("root");
	xml.addAttribute("version", 1);
	xml.endAttributes();

	writeElements(xml);

	xml.closeElement("root");

	out.flush();
	if (!out) {
		THROW_EXCEPTION(IOException, "Could not write to the file " << filePath_ << ". Reason: " << std::strerror(errno) << '.');
	}
	out.close();
	if (!out) {
		THROW_EXCEPTION(IOException, "Could not close the file " << filePath_ << '.');
	}
}

/*******************************************************************************
 *
 */
void
XMLConfigFileWriter::writeElements(StreamXMLWriter& xml)
{
	LOG_DEBUG("categories ==================================================");
	xml.openElement("categories");
	for (const auto& category : model_.categoryList()) {
		xml.openElementWithAttributes("category");
		xml.addAttribute("name", category->name());
		if (category->comment().empty()) {
			xml.endAttributesAndCloseElement();
			continue;
		}
		xml.endAttributes();
		xml.openInlineElement("comment");
		xml.addText(category->comment());
		xml.closeInlineElement("comment");
		xml.closeElement("category");
	}
	xml.closeElement("categories");

	LOG_DEBUG("parameters ==================================================");
	xml.openElement("parameters");
	for (const Parameter& param : model_.parameterList()) {
		xml.openElementWithAttributes("parameter");
		xml.addAttribute("name", param.name());
		xml.addAttribute("minimum", param.minimum());
		xml.addAttribute("maximum", param.maximum());
		xml.addAttribute("default", param.defaultValue());
		if (param.comment().empty()) {
			xml.endAttributesAndCloseElement();
			continue;
		}
		xml.endAttributes();
		xml.openInlineElement("comment");
		xml.addText(param.comment());
		xml.closeInlineElement("comment");
		xml.closeElement("parameter");
	}
	xml.closeElement("parameters");

	LOG_DEBUG("symbols ==================================================");
	xml.openElement("symbols");
	for (const Symbol& symbol : model_.symbolList()) {
		xml.openElementWithAttributes("symbol");
		xml.addAttribute("name", symbol.name());
		xml.addAttribute("minimum", symbol.minimum());
		xml.addAttribute("maximum", symbol.maximum());
		xml.addAttribute("default", symbol.defaultValue());
		if (symbol.comment().empty()) {
			xml.endAttributesAndCloseElement();
			continue;
		}
		xml.endAttributes();
		xml.openInlineElement("comment");
		xml.addText(symbol.comment());
		xml.closeInlineElement("comment");
		xml.closeElement("symbol");
	}
	xml.closeElement("symbols");

	LOG_DEBUG("postures ==================================================");
	xml.openElement("postures");

	for (unsigned int i = 0, size = model_.postureList().size(); i < size; ++i) {
		const Posture& posture = model_.postureList()[i];

		xml.openElementWithAttributes("posture");
		xml.addAttribute("symbol", posture.name());
		xml.endAttributes();

		if (!posture.comment().empty()) {
			xml.openInlineElement("comment");
			xml.addText(posture.comment());
			xml.closeInlineElement("comment");
		}

		xml.openElement("posture-categories");
		for (const auto& category : posture.categoryList()) {
			xml.openElementWithAttributes("category-ref");
			xml.addAttribute("name", category->name());
			xml.endAttributesAndCloseElement();
		}
		xml.closeElement("posture-categories");

		xml.openElement("parameter-targets");
		for (unsigned int i = 0, numParam = model_.parameterList().size(); i < numParam; ++i) {
			const Parameter& param = model_.getParameter(i);
			const float target = posture.getParameterTarget(i);
			xml.openElementWithAttributes("target");
			xml.addAttribute("name", param.name());
			xml.addAttribute("value", target);
			xml.endAttributesAndCloseElement();
		}
		xml.closeElement("parameter-targets");

		xml.openElement("symbol-targets");

		for (unsigned int i = 0, size = model_.symbolList().size(); i < size; ++i) {
			const Symbol& symbol = model_.symbolList()[i];
			xml.openElementWithAttributes("target");
			xml.addAttribute("name", symbol.name());
			xml.addAttribute("value", posture.getSymbolTarget(i));
			xml.endAttributesAndCloseElement();
		}

		xml.closeElement("symbol-targets");

		xml.closeElement("posture");
	}
	xml.closeElement("postures");

	LOG_DEBUG("equations ==================================================");
	xml.openElement("equations");
	for (const EquationGroup& group : model_.equationGroupList()) {
		xml.openElementWithAttributes("equation-group");
		xml.addAttribute("name", group.name);
		xml.endAttributes();
		for (const auto& equation : group.equationList) {
			xml.openElementWithAttributes("equation");
			xml.addAttribute("name", equation->name());
			xml.addAttribute("formula", equation->formula());
			if (!equation->comment().empty()) {
				xml.endAttributes();
				xml.openInlineElement("comment");
				xml.addText(equation->comment());
				xml.closeInlineElement("comment");
				xml.closeElement("equation");
			} else {
				xml.endAttributesAndCloseElement();
			}
		}
		xml.closeElement("equation-group");
	}
	xml.closeElement("equations");

	LOG_DEBUG("transitions ==================================================");
	xml.openElement("transitions");
	for (const TransitionGroup& group : model_.transitionGroupList()) {
		xml.openElementWithAttributes("transition-group");
		xml.addAttribute("name", group.name);
		xml.endAttributes();
		for (const auto& transition : group.transitionList) {
			xml.openElementWithAttributes("transition");
			xml.addAttribute("name", transition->name());
			xml.addAttribute("type", Transition::getNameFromType(transition->type()));
			xml.endAttributes();
			if (!transition->comment().empty()) {
				xml.openInlineElement("comment");
				xml.addText(transition->comment());
				xml.closeInlineElement("comment");
			}
			xml.openElement("point-or-slopes");
			for (const auto& pointOrSlope : transition->pointOrSlopeList()) {
				if (pointOrSlope->isSlopeRatio()) {
					const Transition::SlopeRatio& slopeRatio = dynamic_cast<const Transition::SlopeRatio&>(*pointOrSlope);
					xml.openElement("slope-ratio");
					xml.openElement("points");
					for (const auto& point : slopeRatio.pointList) {
						xml.openElementWithAttributes("point");
						xml.addAttribute("type", Transition::Point::getNameFromType(point->type));
						xml.addAttribute("value", point->value);
						if (!point->timeExpression) {
							xml.addAttribute("free-time", point->freeTime);
						} else {
							xml.addAttribute("time-expression", point->timeExpression->name());
						}
						if (point->isPhantom) {
							xml.addAttribute("is-phantom", "yes");
						}
						xml.endAttributesAndCloseElement();
					}
					xml.closeElement("points");
					xml.openElement("slopes");
					for (const auto& slope : slopeRatio.slopeList) {
						xml.openElementWithAttributes("slope");
						xml.addAttribute("slope", slope->slope);
						xml.addAttribute("display-time", slope->displayTime);
						xml.endAttributesAndCloseElement();
					}
					xml.closeElement("slopes");
					xml.closeElement("slope-ratio");
				} else {
					const Transition::Point& point = dynamic_cast<const Transition::Point&>(*pointOrSlope);
					xml.openElementWithAttributes("point");
					xml.addAttribute("type", Transition::Point::getNameFromType(point.type));
					xml.addAttribute("value", point.value);
					if (!point.timeExpression) {
						xml.addAttribute("free-time", point.freeTime);
					} else {
						xml.addAttribute("time-expression", point.timeExpression->name());
					}
					if (point.isPhantom) {
						xml.addAttribute("is-phantom", "yes");
					}
					xml.endAttributesAndCloseElement();
				}
			}
			xml.closeElement("point-or-slopes");
			xml.closeElement("transition");
		}
		xml.closeElement("transition-group");
	}
	xml.closeElement("transitions");

	LOG_DEBUG("special-transitions ==================================================");
	xml.openElement("special-transitions");
	for (const TransitionGroup& group : model_.specialTransitionGroupList()) {
		xml.openElementWithAttributes("transition-group");
		xml.addAttribute("name", group.name);
		xml.endAttributes();
		for (const auto& transition : group.transitionList) {
			xml.openElementWithAttributes("transition");
			xml.addAttribute("name", transition->name());
			xml.addAttribute("type", Transition::getNameFromType(transition->type()));
			xml.endAttributes();
			if (!transition->comment().empty()) {
				xml.openInlineElement("comment");
				xml.addText(transition->comment());
				xml.closeInlineElement("comment");
			}
			xml.openElement("point-or-slopes");
			for (const auto& pointOrSlope : transition->pointOrSlopeList()) {
				if (pointOrSlope->isSlopeRatio()) {
					THROW_EXCEPTION(InvalidValueException, "Unexpected slope ratio in special transition.");
				} else {
					const Transition::Point& point = dynamic_cast<const Transition::Point&>(*pointOrSlope);
					xml.openElementWithAttributes("point");
					xml.addAttribute("type", Transition::Point::getNameFromType(point.type));
					xml.addAttribute("value", point.value);
					if (!point.timeExpression) {
						xml.addAttribute("free-time", point.freeTime);
					} else {
						xml.addAttribute("time-expression", point.timeExpression->name());
					}
					if (point.isPhantom) {
						xml.addAttribute("is-phantom", "yes");
					}
					xml.endAttributesAndCloseElement();
				}
			}
			xml.closeElement("point-or-slopes");
			xml.closeElement("transition");
		}
		xml.closeElement("transition-group");
	}
	xml.closeElement("special-transitions");

	LOG_DEBUG("rules ==================================================");
	xml.openElement("rules");
	unsigned int ruleNumber = 0;
	for (const auto& rule : model_.ruleList()) {
		++ruleNumber;
		xml.indent();
		std::ostringstream comment;
		comment << "Rule: " << ruleNumber;
		xml.writeComment(comment.str());

		xml.openElement("rule");

		xml.openElement("boolean-expressions");
		for (const std::string& s : rule->booleanExpressionList()) {
			xml.openInlineElement("boolean-expression");
			xml.addText(s);
			xml.closeInlineElement("boolean-expression");
		}
		xml.closeElement("boolean-expressions");

		if (!rule->comment().empty()) {
			xml.openInlineElement("comment");
			xml.addText(rule->comment());
			xml.closeInlineElement("comment");
		}

		xml.openElement("parameter-profiles");
		for (unsigned int i = 0, numParam = model_.parameterList().size(); i < numParam; ++i) {
			xml.openElementWithAttributes("parameter-transition");
			xml.addAttribute("name", model_.getParameter(i).name());
			xml.addAttribute("transition", rule->paramProfileTransitionList()[i]->name());
			xml.endAttributesAndCloseElement();
		}
		xml.closeElement("parameter-profiles");

		bool hasSpecialTransition = false;
		for (const auto& s : rule->specialProfileTransitionList()) {
			if (s) {
				hasSpecialTransition = true;
				break;
			}
		}
		if (hasSpecialTransition) {
			xml.openElement("special-profiles");
			for (unsigned int i = 0, numParam = model_.parameterList().size(); i < numParam; ++i) {
				const auto& s = rule->specialProfileTransitionList()[i];
				if (s) {
					xml.openElementWithAttributes("parameter-transition");
					xml.addAttribute("name", model_.getParameter(i).name());
					xml.addAttribute("transition", s->name());
					xml.endAttributesAndCloseElement();
				}
			}
			xml.closeElement("special-profiles");
		}

		xml.openElement("expression-symbols");

		xml.openElementWithAttributes("symbol-equation");
		xml.addAttribute("name", "rd");
		xml.addAttribute("equation", rule->exprSymbolEquations().ruleDuration ? rule->exprSymbolEquations().ruleDuration->name() : "");
		xml.endAttributesAndCloseElement();

		xml.openElementWithAttributes("symbol-equation");
		xml.addAttribute("name", "beat");
		xml.addAttribute("equation", rule->exprSymbolEquations().beat ? rule->exprSymbolEquations().beat->name() : "");
		xml.endAttributesAndCloseElement();

		xml.openElementWithAttributes("symbol-equation");
		xml.addAttribute("name", "mark1");
		xml.addAttribute("equation", rule->exprSymbolEquations().mark1 ? rule->exprSymbolEquations().mark1->name() : "");
		xml.endAttributesAndCloseElement();

		if (rule->exprSymbolEquations().mark2) {
			xml.openElementWithAttributes("symbol-equation");
			xml.addAttribute("name", "mark2");
			xml.addAttribute("equation", rule->exprSymbolEquations().mark2->name());
			xml.endAttributesAndCloseElement();
		}

		if (rule->exprSymbolEquations().mark3) {
			xml.openElementWithAttributes("symbol-equation");
			xml.addAttribute("name", "mark3");
			xml.addAttribute("equation", rule->exprSymbolEquations().mark3->name());
			xml.endAttributesAndCloseElement();
		}

		xml.closeElement("expression-symbols");

		xml.closeElement("rule");
	}
	xml.closeElement("rules");
}

} /* namespace TRMControlModel */
} /* namespace GS */
