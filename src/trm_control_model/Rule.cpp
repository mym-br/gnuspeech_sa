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

#include "Rule.h"

#include <cassert>
#include <cctype> /* isspace */
#include <iostream>

#include "Category.h"
#include "Equation.h"
#include "Model.h"
#include "Posture.h"
#include "Text.h"
#include "Transition.h"



namespace {

using namespace GS::TRMControlModel;

const char rightParenChar = ')';
const char  leftParenChar = '(';
const char   matchAllChar = '*';
const std::string  orOpSymb = "or";
const std::string notOpSymb = "not";
const std::string xorOpSymb = "xor";
const std::string andOpSymb = "and";

class Parser {
public:
	Parser(const std::string& s, const Model& model)
				: model_(model)
				, s_(GS::Text::trim(s))
				, pos_(0)
				, symbolType_(SYMBOL_TYPE_INVALID) {
		if (s_.empty()) {
			THROW_EXCEPTION(GS::TRMControlModelException, "Boolean expression parser error: Empty string.");
		}
		nextSymbol();
	}

	RuleBooleanNode_ptr parse();
private:
	enum SymbolType {
		SYMBOL_TYPE_INVALID,
		SYMBOL_TYPE_OR_OP,
		SYMBOL_TYPE_NOT_OP,
		SYMBOL_TYPE_XOR_OP,
		SYMBOL_TYPE_AND_OP,
		SYMBOL_TYPE_RIGHT_PAREN,
		SYMBOL_TYPE_LEFT_PAREN,
		SYMBOL_TYPE_STRING
	};

	[[noreturn]] void throwException(const char* errorDescription) const;
	template<typename T> [[noreturn]] void throwException(const char* errorDescription, const T& complement) const;
	bool finished() const {
		return pos_ >= s_.size();
	}
	void nextSymbol();
	RuleBooleanNode_ptr getBooleanNode();
	void skipSpaces();

	static bool isSeparator(char c) {
		switch (c) {
		case rightParenChar: return true;
		case leftParenChar:  return true;
		default:             return std::isspace(c) != 0;
		}
	}

	const Model& model_;
	const std::string s_;
	std::string::size_type pos_;
	std::string symbol_;
	SymbolType symbolType_;
};

void
Parser::throwException(const char* errorDescription) const
{
	THROW_EXCEPTION(GS::TRMControlModelException, "Boolean expression parser error: "
				<< errorDescription
				<< " at position " << (pos_ - symbol_.size()) << " of string [" << s_ << "].");
}

template<typename T>
void
Parser::throwException(const char* errorDescription, const T& complement) const
{
	THROW_EXCEPTION(GS::TRMControlModelException, "Boolean expression parser error: "
				<< errorDescription << complement
				<< " at position " << (pos_ - symbol_.size()) << " of string [" << s_ << "].");
}

void
Parser::skipSpaces()
{
	while (!finished() && std::isspace(s_[pos_])) ++pos_;
}

void
Parser::nextSymbol()
{
	skipSpaces();

	symbol_.resize(0);

	if (finished()) {
		symbolType_ = SYMBOL_TYPE_INVALID;
		return;
	}

	char c = s_[pos_++];
	symbol_ = c;
	switch (c) {
	case rightParenChar:
		symbolType_ = SYMBOL_TYPE_RIGHT_PAREN;
		break;
	case leftParenChar:
		symbolType_ = SYMBOL_TYPE_LEFT_PAREN;
		break;
	default:
		while ( !finished() && !isSeparator(c = s_[pos_]) ) {
			symbol_ += c;
			++pos_;
		}
		if (symbol_ == orOpSymb) {
			symbolType_ = SYMBOL_TYPE_OR_OP;
		} else if (symbol_ == andOpSymb) {
			symbolType_ = SYMBOL_TYPE_AND_OP;
		} else if (symbol_ == notOpSymb) {
			symbolType_ = SYMBOL_TYPE_NOT_OP;
		} else if (symbol_ == xorOpSymb) {
			symbolType_ = SYMBOL_TYPE_XOR_OP;
		} else {
			symbolType_ = SYMBOL_TYPE_STRING;
		}
	}
}

RuleBooleanNode_ptr
Parser::getBooleanNode()
{
	switch (symbolType_) {
	case SYMBOL_TYPE_LEFT_PAREN:
	{
		RuleBooleanNode_ptr p;

		nextSymbol();
		if (symbolType_ == SYMBOL_TYPE_NOT_OP) {

			// Operand.
			nextSymbol();
			RuleBooleanNode_ptr op(getBooleanNode());

			p.reset(new RuleBooleanNotExpression(std::move(op)));
		} else {
			// 1st operand.
			RuleBooleanNode_ptr op1(getBooleanNode());

			// Operator.
			switch (symbolType_) {
			case SYMBOL_TYPE_OR_OP:
			{	// 2nd operand.
				nextSymbol();
				RuleBooleanNode_ptr op2(getBooleanNode());

				p.reset(new RuleBooleanOrExpression(std::move(op1), std::move(op2)));
				break;
			}
			case SYMBOL_TYPE_AND_OP:
			{	// 2nd operand.
				nextSymbol();
				RuleBooleanNode_ptr op2(getBooleanNode());

				p.reset(new RuleBooleanAndExpression(std::move(op1), std::move(op2)));
				break;
			}
			case SYMBOL_TYPE_XOR_OP:
			{	// 2nd operand.
				nextSymbol();
				RuleBooleanNode_ptr op2(getBooleanNode());

				p.reset(new RuleBooleanXorExpression(std::move(op1), std::move(op2)));
				break;
			}
			case SYMBOL_TYPE_NOT_OP:
				throwException("Invalid operator");
			default:
				throwException("Missing operator");
			}
		}

		if (symbolType_ != SYMBOL_TYPE_RIGHT_PAREN) {
			throwException("Right parenthesis not found");
		}
		nextSymbol();
		return p;
	}
	case SYMBOL_TYPE_STRING:
	{
		bool matchAll = false;
		if (symbol_.size() >= 2 && symbol_[symbol_.size() - 1] == matchAllChar) {
			matchAll = true;
		}

		std::string name;
		if (matchAll) {
			name = symbol_.substr(0, symbol_.size() - 1);
		} else {
			name = symbol_;
		}

		std::shared_ptr<Category> category;
		const Posture* posture = model_.postureList().find(name);
		if (posture != nullptr) {
			category = posture->findCategory(name);
		} else {
			if (matchAll) {
				throwException("Asterisk at the end of a category name");
			}
			category = model_.findCategory(name);
		}
		if (!category) {
			throwException("Could not find category: ", name);
		}

		nextSymbol();
		return RuleBooleanNode_ptr(new RuleBooleanTerminal(category, matchAll));
	}
	case SYMBOL_TYPE_OR_OP:
		throwException("Unexpected OR op.");
	case SYMBOL_TYPE_NOT_OP:
		throwException("Unexpected NOT op.");
	case SYMBOL_TYPE_XOR_OP:
		throwException("Unexpected XOR op.");
	case SYMBOL_TYPE_AND_OP:
		throwException("Unexpected AND op.");
	case SYMBOL_TYPE_RIGHT_PAREN:
		throwException("Unexpected right parenthesis");
	default:
		throwException("Missing symbol");
	}
	return RuleBooleanNode_ptr(); // unreachable
}

RuleBooleanNode_ptr
Parser::parse()
{
	RuleBooleanNode_ptr booleanRoot = getBooleanNode();
	if (symbolType_ != SYMBOL_TYPE_INVALID) { // there is a symbol available
		throwException("Invalid text");
	}
	return booleanRoot;
}

} /* namespace */

//==============================================================================

namespace GS {
namespace TRMControlModel {

/*******************************************************************************
 * Destructor.
 */
RuleBooleanNode::~RuleBooleanNode()
{
}

/*******************************************************************************
 * Destructor.
 */
RuleBooleanAndExpression::~RuleBooleanAndExpression()
{
}

bool
RuleBooleanAndExpression::eval(const Posture& posture) const
{
	assert(child1_.get() != 0 && child2_.get() != 0);

	return child1_->eval(posture) && child2_->eval(posture);
}

void
RuleBooleanAndExpression::print(std::ostream& out, int level) const
{
	assert(child1_.get() != 0 && child2_.get() != 0);

	std::string prefix(level * 8, ' ');
	out << prefix << andOpSymb << " [\n";

	child1_->print(out, level + 1);
	child2_->print(out, level + 1);

	out << prefix << "]" << std::endl;
}

/*******************************************************************************
 * Destructor.
 */
RuleBooleanOrExpression::~RuleBooleanOrExpression()
{
}

bool
RuleBooleanOrExpression::eval(const Posture& posture) const
{
	assert(child1_.get() != 0 && child2_.get() != 0);

	return child1_->eval(posture) || child2_->eval(posture);
}

void
RuleBooleanOrExpression::print(std::ostream& out, int level) const
{
	assert(child1_.get() != 0 && child2_.get() != 0);

	std::string prefix(level * 8, ' ');
	out << prefix << orOpSymb << " [\n";

	child1_->print(out, level + 1);
	child2_->print(out, level + 1);

	out << prefix << "]" << std::endl;
}

/*******************************************************************************
 * Destructor.
 */
RuleBooleanXorExpression::~RuleBooleanXorExpression()
{
}

bool
RuleBooleanXorExpression::eval(const Posture& posture) const
{
	assert(child1_.get() != 0 && child2_.get() != 0);

	return child1_->eval(posture) != child2_->eval(posture);
}

void
RuleBooleanXorExpression::print(std::ostream& out, int level) const
{
	assert(child1_.get() != 0 && child2_.get() != 0);

	std::string prefix(level * 8, ' ');
	out << prefix << xorOpSymb << " [\n";

	child1_->print(out, level + 1);
	child2_->print(out, level + 1);

	out << prefix << "]" << std::endl;
}

/*******************************************************************************
 * Destructor.
 */
RuleBooleanNotExpression::~RuleBooleanNotExpression()
{
}

bool
RuleBooleanNotExpression::eval(const Posture& posture) const
{
	assert(child_.get() != 0);

	return !(child_->eval(posture));
}

void
RuleBooleanNotExpression::print(std::ostream& out, int level) const
{
	assert(child_.get() != 0);

	std::string prefix(level * 8, ' ');
	out << prefix << notOpSymb << " [\n";

	child_->print(out, level + 1);

	out << prefix << "]" << std::endl;
}

/*******************************************************************************
 * Destructor.
 */
RuleBooleanTerminal::~RuleBooleanTerminal()
{
}

bool
RuleBooleanTerminal::eval(const Posture& posture) const
{
	if (posture.isMemberOfCategory(*category_)) {
		return true;
	} else if (matchAll_) {
		return posture.name() == category_->name() + '\'';
	}
	return false;
}

void
RuleBooleanTerminal::print(std::ostream& out, int level) const
{
	std::string prefix(level * 8, ' ');

	out << prefix << "category [" << category_->name();
	if (matchAll_) out << "[*]";
	out << "]" << std::endl;
}

/*******************************************************************************
 *
 */
void
Rule::printBooleanNodeTree() const
{
	for (RuleBooleanNodeList::size_type size = booleanNodeList_.size(), i = 0; i < size; ++i) {
		std::cout << "Posture: " << (i + 1) << std::endl;
		booleanNodeList_[i]->print(std::cout);
	}
}

/*******************************************************************************
 *
 */
bool
Rule::evalBooleanExpression(const std::vector<const Posture*>& postureSequence) const
{
	if (postureSequence.size() < booleanNodeList_.size()) return false;
	if (booleanNodeList_.empty()) return false;

	for (RuleBooleanNodeList::size_type size = booleanNodeList_.size(), i = 0; i < size; ++i) {
		if ( !(booleanNodeList_[i]->eval(*postureSequence[i])) ) {
			return false;
		}
	}
	return true;
}

/*******************************************************************************
 *
 */
bool
Rule::evalBooleanExpression(const Posture& posture, unsigned int expressionIndex) const
{
	if (expressionIndex >= booleanNodeList_.size()) return false;

	return booleanNodeList_[expressionIndex]->eval(posture);
}

/*******************************************************************************
 *
 */
std::size_t
Rule::numberOfExpressions() const
{
	return booleanNodeList_.size();
}

// ruleSymbols: {rd, beat, mark1, mark2, mark3}
// tempos[4]
void
Rule::evaluateExpressionSymbols(const double* tempos, const std::vector<const Posture*>& postures, Model& model, double* ruleSymbols) const
{
	double localTempos[4];

	model.clearFormulaSymbolList();
	if (postures.size() >= 2) {
		const Posture& posture = *postures[0];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION1, posture.getSymbolTarget(1 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA1      , posture.getSymbolTarget(2 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB1      , posture.getSymbolTarget(3 /* hardcoded */));
		const Posture& posture2 = *postures[1];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION2, posture2.getSymbolTarget(1 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA2      , posture2.getSymbolTarget(2 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB2      , posture2.getSymbolTarget(3 /* hardcoded */));
		localTempos[0] = tempos[0];
		localTempos[1] = tempos[1];
	} else {
		localTempos[0] = 0.0;
		localTempos[1] = 0.0;
	}
	if (postures.size() >= 3) {
		const Posture& posture = *postures[2];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION3, posture.getSymbolTarget(1 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA3      , posture.getSymbolTarget(2 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB3      , posture.getSymbolTarget(3 /* hardcoded */));
		localTempos[2] = tempos[2];
	} else {
		localTempos[2] = 0.0;
	}
	if (postures.size() == 4) {
		const Posture& posture = *postures[3];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION4, posture.getSymbolTarget(1 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA4      , posture.getSymbolTarget(2 /* hardcoded */));
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB4      , posture.getSymbolTarget(3 /* hardcoded */));
		localTempos[3] = tempos[3];
	} else {
		localTempos[3] = 0.0;
	}
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO1, static_cast<float>(localTempos[0]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO2, static_cast<float>(localTempos[1]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO3, static_cast<float>(localTempos[2]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO4, static_cast<float>(localTempos[3]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_RD   , static_cast<float>(ruleSymbols[0]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_BEAT , static_cast<float>(ruleSymbols[1]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK1, static_cast<float>(ruleSymbols[2]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK2, static_cast<float>(ruleSymbols[3]));
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK3, static_cast<float>(ruleSymbols[4]));

	// Execute in this order.
	if (exprSymbolEquations_.ruleDuration) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_RD   , model.evalEquationFormula(*exprSymbolEquations_.ruleDuration));
	}
	if (exprSymbolEquations_.mark1) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK1, model.evalEquationFormula(*exprSymbolEquations_.mark1));
	}
	if (exprSymbolEquations_.mark2) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK2, model.evalEquationFormula(*exprSymbolEquations_.mark2));
	}
	if (exprSymbolEquations_.mark3) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK3, model.evalEquationFormula(*exprSymbolEquations_.mark3));
	}
	if (exprSymbolEquations_.beat) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_BEAT , model.evalEquationFormula(*exprSymbolEquations_.beat));
	}

	ruleSymbols[0] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_RD);
	ruleSymbols[1] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_BEAT);
	ruleSymbols[2] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_MARK1);
	ruleSymbols[3] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_MARK2);
	ruleSymbols[4] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_MARK3);
}

void
Rule::setBooleanExpressionList(const std::vector<std::string>& exprList, const Model& model)
{
	unsigned int size = exprList.size();
	if (size < 2U || size > 4U) {
		THROW_EXCEPTION(InvalidParameterException, "Invalid number of boolean expressions: " << size << '.');
	}

	RuleBooleanNodeList testBooleanNodeList;

	for (unsigned int i = 0; i < size; ++i) {
		Parser p(exprList[i], model);
		testBooleanNodeList.push_back(p.parse());
	}

	booleanExpressionList_ = exprList;
	std::swap(booleanNodeList_, testBooleanNodeList);
}

} /* namespace TRMControlModel */
} /* namespace GS */
