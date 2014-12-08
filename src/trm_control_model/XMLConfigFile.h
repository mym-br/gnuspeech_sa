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

#ifndef TRM_CONTROL_MODEL_XML_CONFIG_FILE_H_
#define TRM_CONTROL_MODEL_XML_CONFIG_FILE_H_

#include <string>

#include "Equation.h"
#include "Posture.h"
#include "Rule.h"
#include "SimpleXMLParser.h"
#include "Transition.h"



namespace GS {
namespace TRMControlModel {

class Model;

/*******************************************************************************
 * This class supposes that the XML is in the correct format.
 */
class XMLConfigFile {
public:
	XMLConfigFile(Model& model, const std::string& filePath);
	~XMLConfigFile();

	void loadModel();
private:
	Model& model_;

	//TODO: why these strings? To avoid temporaries?
	std::string booleanExpressionTagName_;
	std::string booleanExpressionsTagName_;
	std::string categoriesTagName_;
	std::string categoryTagName_;
	std::string categoryRefTagName_;
	std::string equationTagName_;
	std::string equationGroupTagName_;
	std::string equationsTagName_;
	std::string expressionSymbolsTagName_;
	std::string parameterTagName_;
	std::string parameterProfilesTagName_;
	std::string parametersTagName_;
	std::string parameterTargetsTagName_;
	std::string parameterTransitionTagName_;
	std::string pointOrSlopesTagName_;
	std::string pointTagName_;
	std::string pointsTagName_;
	std::string postureCategoriesTagName_;
	std::string posturesTagName_;
	std::string postureTagName_;
	std::string ruleTagName_;
	std::string rulesTagName_;
	std::string slopeTagName_;
	std::string slopeRatioTagName_;
	std::string slopesTagName_;
	std::string specialProfilesTagName_;
	std::string specialTransitionsTagName_;
	std::string symbolEquationTagName_;
	std::string symbolsTagName_;
	std::string symbolTagName_;
	std::string symbolTargetsTagName_;
	std::string targetTagName_;
	std::string transitionTagName_;
	std::string transitionGroupTagName_;
	std::string transitionsTagName_;

	std::string defaultAttrName_;
	std::string displayTimeAttrName_;
	std::string equationAttrName_;
	std::string formulaAttrName_;
	std::string freeTimeAttrName_;
	std::string isPhantomAttrName_;
	std::string maximumAttrName_;
	std::string minimumAttrName_;
	std::string nameAttrName_;
	std::string p12AttrName_;
	std::string p23AttrName_;
	std::string p34AttrName_;
	std::string slopeAttrName_;
	std::string symbolAttrName_;
	std::string timeExpressionAttrName_;
	std::string typeAttrName_;
	std::string transitionAttrName_;
	std::string valueAttrName_;

	std::string beatSymbolName_;
	std::string durationSymbolName_;
	std::string mark1SymbolName_;
	std::string mark2SymbolName_;
	std::string mark3SymbolName_;
	std::string qssaSymbolName_;
	std::string qssbSymbolName_;
	std::string rdSymbolName_;
	std::string transitionSymbolName_;

	SimpleXMLParser parser_;

	XMLConfigFile(const XMLConfigFile&);
	XMLConfigFile& operator=(const XMLConfigFile&);

	void parseCategories();

	void parseParameters();

	void parseSymbols();

	void parsePostureSymbols(Posture& posture);
	void parsePostureCategories(Posture& posture);
	void parsePostureParameters(Posture& posture);
	void parsePosture();
	void parsePostures();

	void parseEquationsGroup();
	void parseEquations();

	void parseSlopeRatio(Transition& transition);
	void parseTransitionPointOrSlopes(Transition& transition);
	void parseTransitionsGroup(bool special);
	void parseTransitions(bool special);

	void parseRuleParameterProfiles(Rule& list);
	void parseRuleSpecialProfiles(Rule& list);
	void parseRuleExpressionSymbols(Rule& list);
	void parseRuleBooleanExpressions(Rule& rule);
	void parseRule();
	void parseRules();
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_XML_CONFIG_FILE_H_ */
