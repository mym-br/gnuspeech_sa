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

#ifndef TRM_CONTROL_MODEL_XML_CONFIG_FILE_READER_H_
#define TRM_CONTROL_MODEL_XML_CONFIG_FILE_READER_H_

#include <string>



namespace rapidxml {
template<typename Ch> class xml_node;
}

namespace GS {
namespace TRMControlModel {

class Model;
class Posture;
class Rule;
class Transition;

/*******************************************************************************
 * This class supposes that the XML is in the correct format.
 */
class XMLConfigFileReader {
public:
	XMLConfigFileReader(Model& model, const std::string& filePath);
	~XMLConfigFileReader();

	// Loads the model from the XML.
	//
	// Precondition: the model is empty.
	void loadModel();
private:
	XMLConfigFileReader(const XMLConfigFileReader&) = delete;
	XMLConfigFileReader& operator=(const XMLConfigFileReader&) = delete;

	void parseCategories(rapidxml::xml_node<char>* categoriesElem);

	void parseParameters(rapidxml::xml_node<char>* parametersElem);

	void parseSymbols(rapidxml::xml_node<char>* symbolsElem);

	void parsePostureSymbols(rapidxml::xml_node<char>* symbolTargetsElem, Posture& posture);
	void parsePostureCategories(rapidxml::xml_node<char>* postureCategoriesElem, Posture& posture);
	void parsePostureParameters(rapidxml::xml_node<char>* parameterTargetsElem, Posture& posture);
	void parsePosture(rapidxml::xml_node<char>* postureElem);
	void parsePostures(rapidxml::xml_node<char>* posturesElem);

	void parseEquationsGroup(rapidxml::xml_node<char>* equationGroupElem);
	void parseEquations(rapidxml::xml_node<char>* equationsElem);

	void parseSlopeRatio(rapidxml::xml_node<char>* slopeRatioElem, Transition& transition);
	void parseTransitionPointOrSlopes(rapidxml::xml_node<char>* pointOrSlopesElem, Transition& transition);
	void parseTransitionsGroup(rapidxml::xml_node<char>* transitionGroupElem, bool special);
	void parseTransitions(rapidxml::xml_node<char>* transitionsElem, bool special);

	void parseRuleParameterProfiles(rapidxml::xml_node<char>* parameterProfilesElem, Rule& list);
	void parseRuleSpecialProfiles(rapidxml::xml_node<char>* specialProfilesElem, Rule& list);
	void parseRuleExpressionSymbols(rapidxml::xml_node<char>* expressionSymbolsElem, Rule& list);
	void parseRuleBooleanExpressions(rapidxml::xml_node<char>* booleanExpressionsElem, Rule& rule);
	void parseRule(rapidxml::xml_node<char>* ruleElem);
	void parseRules(rapidxml::xml_node<char>* rulesElem);

	Model& model_;
	std::string filePath_;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_XML_CONFIG_FILE_READER_H_ */
