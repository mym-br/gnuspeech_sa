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

#include <memory>
#include <ostream>
#include <string>
#include <utility> /* move, swap */
#include <vector>

#include "Exception.h"



namespace GS {
namespace TRMControlModel {

class Category;
class Equation;
class Model;
class Posture;
class Transition;

class RuleBooleanNode {
public:
	virtual ~RuleBooleanNode();

	virtual bool eval(const Posture& posture) const = 0;
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

	virtual bool eval(const Posture& posture) const;
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

	virtual bool eval(const Posture& posture) const;
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

	virtual bool eval(const Posture& posture) const;
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

	virtual bool eval(const Posture& posture) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	RuleBooleanNode_ptr child_;
};

class RuleBooleanTerminal : public RuleBooleanNode {
public:
	RuleBooleanTerminal(const std::shared_ptr<Category>& category, bool matchAll)
			: RuleBooleanNode(), category_(category), matchAll_(matchAll) {}
	virtual ~RuleBooleanTerminal();

	virtual bool eval(const Posture& posture) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	const std::shared_ptr<Category> category_;
	bool matchAll_;
};

class Rule {
public:
	struct ExpressionSymbolEquations {
		std::shared_ptr<Equation> ruleDuration;
		std::shared_ptr<Equation> beat;
		std::shared_ptr<Equation> mark1;
		std::shared_ptr<Equation> mark2;
		std::shared_ptr<Equation> mark3;
	};

	Rule(unsigned int numParameters)
		: paramProfileTransitionList_(numParameters)
		, specialProfileTransitionList_(numParameters)
	{
	}

	std::size_t numberOfExpressions() const;
	bool evalBooleanExpression(const std::vector<const Posture*>& postureSequence) const;
	bool evalBooleanExpression(const Posture& posture, unsigned int expressionIndex) const;
	void printBooleanNodeTree() const;

	ExpressionSymbolEquations& exprSymbolEquations() { return exprSymbolEquations_; }
	const ExpressionSymbolEquations& exprSymbolEquations() const { return exprSymbolEquations_; }

	const std::shared_ptr<Transition>& getParamProfileTransition(unsigned int parameterIndex) const {
		if (parameterIndex >= paramProfileTransitionList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
		}

		return paramProfileTransitionList_[parameterIndex];
	}
	void setParamProfileTransition(unsigned int parameterIndex, const std::shared_ptr<Transition>& transition) {
		if (parameterIndex >= paramProfileTransitionList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
		}

		paramProfileTransitionList_[parameterIndex] = transition;
	}

	const std::shared_ptr<Transition>& getSpecialProfileTransition(unsigned int parameterIndex) const {
		if (parameterIndex >= specialProfileTransitionList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
		}

		return specialProfileTransitionList_[parameterIndex]; // may return an empty string
	}
	void setSpecialProfileTransition(unsigned int parameterIndex, const std::shared_ptr<Transition>& transition) {
		if (parameterIndex >= specialProfileTransitionList_.size()) {
			THROW_EXCEPTION(InvalidParameterException, "Invalid parameter index: " << parameterIndex << '.');
		}

		specialProfileTransitionList_[parameterIndex] = transition;
	}

	void evaluateExpressionSymbols(const double* tempos, const std::vector<const Posture*>& postures, Model& model, double* ruleSymbols) const;

	const std::vector<std::string>& booleanExpressionList() const { return booleanExpressionList_; }
	void setBooleanExpressionList(const std::vector<std::string>& exprList, const Model& model);

	const std::vector<std::shared_ptr<Transition>>& paramProfileTransitionList() const { return paramProfileTransitionList_; }
	std::vector<std::shared_ptr<Transition>>& paramProfileTransitionList() { return paramProfileTransitionList_; }

	const std::vector<std::shared_ptr<Transition>>& specialProfileTransitionList() const { return specialProfileTransitionList_; }
	std::vector<std::shared_ptr<Transition>>& specialProfileTransitionList() { return specialProfileTransitionList_; }

	const std::string& comment() const { return comment_; }
	void setComment(const std::string& comment) { comment_ = comment; }
private:
	std::vector<std::string> booleanExpressionList_;
	std::vector<std::shared_ptr<Transition>> paramProfileTransitionList_;
	std::vector<std::shared_ptr<Transition>> specialProfileTransitionList_;
	ExpressionSymbolEquations exprSymbolEquations_;
	std::string comment_;
	RuleBooleanNodeList booleanNodeList_;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_RULE_H_ */
