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

#ifndef TRM_CONTROL_MODEL_RULE_H_
#define TRM_CONTROL_MODEL_RULE_H_

#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <utility> /* move */
#include <vector>

#include "Phone.h"



namespace TRMControlModel {

class Model;

class RuleBooleanNode {
public:
	virtual ~RuleBooleanNode();

	virtual bool eval(const Phone& phone) const = 0;
	virtual void print(std::ostream& out, int level = 0) const = 0;
};

typedef std::unique_ptr<RuleBooleanNode> RuleBooleanNode_ptr;
// Type of container that manages the RuleBooleanNode instances.
typedef std::vector<RuleBooleanNode_ptr> RuleBooleanNodeList;

class RuleBooleanAndExpression : public RuleBooleanNode {
public:
	RuleBooleanAndExpression(RuleBooleanNode_ptr c1, RuleBooleanNode_ptr c2)
			: RuleBooleanNode(), child1_(std::move(c1)), child2_(std::move(c2)) {}
	virtual ~RuleBooleanAndExpression();

	virtual bool eval(const Phone& phone) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	RuleBooleanNode_ptr child1_;
	RuleBooleanNode_ptr child2_;
};

class RuleBooleanOrExpression : public RuleBooleanNode {
public:
	RuleBooleanOrExpression(RuleBooleanNode_ptr c1, RuleBooleanNode_ptr c2)
			: RuleBooleanNode(), child1_(std::move(c1)), child2_(std::move(c2)) {}
	virtual ~RuleBooleanOrExpression();

	virtual bool eval(const Phone& phone) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	RuleBooleanNode_ptr child1_;
	RuleBooleanNode_ptr child2_;
};

class RuleBooleanXorExpression : public RuleBooleanNode {
public:
	RuleBooleanXorExpression(RuleBooleanNode_ptr c1, RuleBooleanNode_ptr c2)
			: RuleBooleanNode(), child1_(std::move(c1)), child2_(std::move(c2)) {}
	virtual ~RuleBooleanXorExpression();

	virtual bool eval(const Phone& phone) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	RuleBooleanNode_ptr child1_;
	RuleBooleanNode_ptr child2_;
};

class RuleBooleanNotExpression : public RuleBooleanNode {
public:
	RuleBooleanNotExpression(RuleBooleanNode_ptr c)
			: RuleBooleanNode(), child_(std::move(c)) {}
	virtual ~RuleBooleanNotExpression();

	virtual bool eval(const Phone& phone) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	RuleBooleanNode_ptr child_;
};

class RuleBooleanTerminal : public RuleBooleanNode {
public:
	RuleBooleanTerminal(const std::string& text, bool matchAll, int categoryCode)
			: RuleBooleanNode(), text_(text), matchAll_(matchAll), categoryCode_(categoryCode) {}
	virtual ~RuleBooleanTerminal();

	virtual bool eval(const Phone& phone) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	std::string text_;
	bool matchAll_;
	int categoryCode_;
};

class Rule;

typedef std::unique_ptr<Rule> Rule_ptr;
typedef std::vector<Rule_ptr>::size_type RuleNumber;

class Rule {
public:
	typedef std::array<std::string, Parameter::NUM_PARAMETERS> TransitionNameArray;

	struct ExpressionSymbolEquations {
		std::string ruleDuration;
		std::string beat;
		std::string mark1;
		std::string mark2;
		std::string mark3;
		ExpressionSymbolEquations() {}
	};

	Rule()
		: number_(0)
	{
	}

	std::size_t numberOfExpressions() const;
	bool evalBooleanExpression(const PhoneSequence& phoneSequence) const;
	void printBooleanNodeTree() const;
	void parseBooleanExpression(const CategoryMap& categoryMap);

	RuleNumber number() const {
		return number_;
	}
	const ExpressionSymbolEquations& exprSymbolEquations() const {
		return exprSymbolEquations_;
	}
	const std::string& getParamProfileTransition(/*Parameter::Code*/ int parameter) const {
		return paramProfileTransitions_[parameter];
	}
	const std::string& getSpecialProfileTransition(/*Parameter::Code*/ int parameter) const {
		return specialProfileTransitions_[parameter]; // may return an empty string
	}
	void evaluateExpressionSymbols(const double* tempos, const PhoneSequence& phones, Model& model, double* ruleSymbols) const;
private:
	typedef std::vector<std::string> BooleanExpressionList;

	RuleNumber number_;
	BooleanExpressionList booleanExpressionList_;
	TransitionNameArray paramProfileTransitions_;
	TransitionNameArray specialProfileTransitions_;
	ExpressionSymbolEquations exprSymbolEquations_;
	RuleBooleanNodeList booleanNodeList_;

	friend class ConfigFile;
};

typedef std::vector<Rule_ptr> RuleList;

} /* namespace TRMControlModel */

#endif /* TRM_CONTROL_MODEL_RULE_H_ */
