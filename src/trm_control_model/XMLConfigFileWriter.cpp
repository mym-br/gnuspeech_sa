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

#include "XMLConfigFileWriter.h"

#include "Exception.h"
#include "Log.h"
#include "Model.h"



namespace {

void
indentLine(std::ofstream& out, int n)
{
	if (n < 0) return;
	for (int i = 0; i < n; ++i) {
		out << "  ";
	}
}

class XMLAux {
public:
	XMLAux(std::ofstream& out, int indentLevel)
		: out_(out), indentLevel_(indentLevel) {}

	void openElement(const std::string& name) {
		indentLine(out_, indentLevel_++);
		out_ << '<' << name << ">\n";
	}
	void openInlineElement(const std::string& name) {
		indentLine(out_, indentLevel_++);
		out_ << '<' << name << '>';
	}
	void openElementWithAttributes(const std::string& name) {
		indentLine(out_, indentLevel_++);
		out_ << '<' << name;
	}

	void addAttribute(const std::string& name, int value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}
	void addAttribute(const std::string& name, float value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}
	void addAttribute(const std::string& name, unsigned int value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}
	void addAttribute(const std::string& name, const std::string& value) {
		out_ << ' ' << name << "=\"" << value << '"';
	}

	void endAttributes() {
		out_ << ">\n";
	}
	void endAttributesAndCloseElement() {
		--indentLevel_;
		out_ << "/>\n";
	}

	void addText(const std::string& text) {
		out_ << text;
	}

	void closeElement(const std::string& name) {
		indentLine(out_, --indentLevel_);
		out_ << "</" << name << ">\n";
	}
	void closeInlineElement(const std::string& name) {
		--indentLevel_;
		out_ << "</" << name << ">\n";
	}

	void indent() {
		indentLine(out_, indentLevel_);
	}

private:
	std::ofstream& out_;
	int indentLevel_;
};

} // namespace



namespace GS {
namespace TRMControlModel {

/*******************************************************************************
 * Constructor.
 */
XMLConfigFileWriter::XMLConfigFileWriter(const Model& model, const std::string& filePath)
		: model_(model)
		, out_(filePath)
{
	if (!out_) {
		THROW_EXCEPTION(IOException, "The output file " << filePath << " could not be created.");
	}
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
	out_ << "<?xml version='1.0' encoding='utf-8'?>\n<root version='1'>\n";

	writeElements();

	out_ << "</root>\n";
}

/*******************************************************************************
 *
 */
void
XMLConfigFileWriter::writeElements()
{
	XMLAux xml(out_, 1);

	LOG_DEBUG("categories ==================================================");
	xml.openElement("categories");
	for (const Category& category : model_.categoryList()) {
		xml.openElementWithAttributes("category");
		xml.addAttribute("name", category.name());
		if (category.comment().empty()) {
			xml.endAttributesAndCloseElement();
			continue;
		}
		xml.endAttributes();
		xml.openInlineElement("comment");
		xml.addText(category.comment());
		xml.closeInlineElement("comment");
		xml.closeElement("category");
	}
	xml.closeElement("categories");

	LOG_DEBUG("parameters ==================================================");
	xml.openElement("parameters");
	for (unsigned int i = 0, numParam = model_.getNumParameters(); i < numParam; ++i) {
		const Parameter& param = model_.getParameter(i);
		xml.openElementWithAttributes("parameter");
		xml.addAttribute("name", param.name());
		xml.addAttribute("minimum", param.minimum());
		xml.addAttribute("maximum", param.maximum());
		xml.addAttribute("default", param.defaultValue());
		xml.endAttributesAndCloseElement();
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
		xml.endAttributesAndCloseElement();
	}
	xml.closeElement("symbols");

	LOG_DEBUG("postures ==================================================");
	xml.openElement("postures");
	for (const Posture& posture : model_.postureList()) {
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
			xml.addAttribute("name", category.name());
			xml.endAttributesAndCloseElement();
		}
		xml.closeElement("posture-categories");

		xml.openElement("parameter-targets");
		for (unsigned int i = 0, numParam = model_.getNumParameters(); i < numParam; ++i) {
			const Parameter& param = model_.getParameter(i);
			const float target = posture.getParameterTarget(i);
			xml.openElementWithAttributes("target");
			xml.addAttribute("name", param.name());
			xml.addAttribute("value", target);
			xml.endAttributesAndCloseElement();
		}
		xml.closeElement("parameter-targets");

		xml.openElement("symbol-targets");

		xml.openElementWithAttributes("target");
		xml.addAttribute("name", "duration");
		xml.addAttribute("value", posture.symbols().duration);
		xml.endAttributesAndCloseElement();

		xml.openElementWithAttributes("target");
		xml.addAttribute("name", "transition");
		xml.addAttribute("value", posture.symbols().transition);
		xml.endAttributesAndCloseElement();

		xml.openElementWithAttributes("target");
		xml.addAttribute("name", "qssa");
		xml.addAttribute("value", posture.symbols().qssa);
		xml.endAttributesAndCloseElement();

		xml.openElementWithAttributes("target");
		xml.addAttribute("name", "qssb");
		xml.addAttribute("value", posture.symbols().qssb);
		xml.endAttributesAndCloseElement();

		xml.closeElement("symbol-targets");

		xml.closeElement("posture");
	}
	xml.closeElement("postures");

	LOG_DEBUG("equations ==================================================");
	xml.openElement("equations");
	for (const std::string groupName : model_.equationGroupList()) {
		xml.openElementWithAttributes("equation-group");
		xml.addAttribute("name", groupName);
		xml.endAttributes();
		for (const Equation& equation : model_.equationList()) {
			if (equation.groupName == groupName) {
				xml.openElementWithAttributes("equation");
				xml.addAttribute("name", equation.name);
				xml.addAttribute("formula", equation.formula);
				if (!equation.comment.empty()) {
					xml.endAttributes();
					xml.openInlineElement("comment");
					xml.addText(equation.comment);
					xml.closeInlineElement("comment");
					xml.closeElement("equation");
				} else {
					xml.endAttributesAndCloseElement();
				}
			}
		}
		xml.closeElement("equation-group");
	}
	xml.closeElement("equations");

	LOG_DEBUG("transitions ==================================================");
	xml.openElement("transitions");
	for (const std::string groupName : model_.transitionGroupList()) {
		xml.openElementWithAttributes("transition-group");
		xml.addAttribute("name", groupName);
		xml.endAttributes();

		xml.closeElement("transition-group");
	}
	xml.closeElement("transitions");

	LOG_DEBUG("special-transitions ==================================================");
	xml.openElement("special-transitions");
	for (const std::string groupName : model_.specialTransitionGroupList()) {
		xml.openElementWithAttributes("transition-group");
		xml.addAttribute("name", groupName);
		xml.endAttributes();

		xml.closeElement("transition-group");
	}
	xml.closeElement("special-transitions");

	LOG_DEBUG("rules ==================================================");
	xml.openElement("rules");
	unsigned int ruleNumber = 0;
	for (const Rule& rule : model_.ruleList()) {
		++ruleNumber;
		xml.indent();
		out_ << "<!-- Rule: " << ruleNumber << " -->\n";
		xml.openElement("rule");

		xml.openElement("boolean-expressions");
		for (const std::string& s : rule.booleanExpressionList()) {
			xml.openInlineElement("boolean-expression");
			xml.addText(s);
			xml.closeInlineElement("boolean-expression");
		}
		xml.closeElement("boolean-expressions");

		if (!rule.comment().empty()) {
			xml.openInlineElement("comment");
			xml.addText(rule.comment());
			xml.closeInlineElement("comment");
		}

		xml.openElement("parameter-profiles");
		for (unsigned int i = 0, numParam = model_.getNumParameters(); i < numParam; ++i) {
			xml.openElementWithAttributes("parameter-transition");
			xml.addAttribute("name", model_.getParameter(i).name());
			xml.addAttribute("transition", rule.paramProfileTransitionList()[i]);
			xml.endAttributesAndCloseElement();
		}
		xml.closeElement("parameter-profiles");

		bool hasSpecialTransition = false;
		for (const std::string& s : rule.specialProfileTransitionList()) {
			if (!s.empty()) {
				hasSpecialTransition = true;
				break;
			}
		}
		if (hasSpecialTransition) {
			xml.openElement("special-profiles");
			for (unsigned int i = 0, numParam = model_.getNumParameters(); i < numParam; ++i) {
				const std::string s = rule.specialProfileTransitionList()[i];
				if (!s.empty()) {
					xml.openElementWithAttributes("parameter-transition");
					xml.addAttribute("name", model_.getParameter(i).name());
					xml.addAttribute("transition", s);
					xml.endAttributesAndCloseElement();
				}
			}
			xml.closeElement("special-profiles");
		}

		xml.openElement("expression-symbols");

		xml.openElementWithAttributes("symbol-equation");
		xml.addAttribute("name", "rd");
		xml.addAttribute("equation", rule.exprSymbolEquations().ruleDuration);
		xml.endAttributesAndCloseElement();

		xml.openElementWithAttributes("symbol-equation");
		xml.addAttribute("name", "beat");
		xml.addAttribute("equation", rule.exprSymbolEquations().beat);
		xml.endAttributesAndCloseElement();

		xml.openElementWithAttributes("symbol-equation");
		xml.addAttribute("name", "mark1");
		xml.addAttribute("equation", rule.exprSymbolEquations().mark1);
		xml.endAttributesAndCloseElement();

		if (!rule.exprSymbolEquations().mark2.empty()) {
			xml.openElementWithAttributes("symbol-equation");
			xml.addAttribute("name", "mark2");
			xml.addAttribute("equation", rule.exprSymbolEquations().mark2);
			xml.endAttributesAndCloseElement();
		}

		xml.closeElement("expression-symbols");

		xml.closeElement("rule");
	}
	xml.closeElement("rules");
}

} /* namespace TRMControlModel */
} /* namespace GS */
