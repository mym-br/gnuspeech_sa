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

#include "Category.h"
#include "Exception.h"
#include "Model.h"
#include "Text.h"



namespace {

using namespace GS::TRMControlModel;

const char rightParenChar = ')';
const char leftParenChar  = '(';
const char matchAllChar   = '*';
const std::string  orOpSymb = "or";
const std::string notOpSymb = "not";
const std::string xorOpSymb = "xor";
const std::string andOpSymb = "and";

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

bool isSeparator(char c)
{
	switch (c) {
	case rightParenChar: return true;
	case leftParenChar:  return true;
	case ' ':            return true;
	default: return false;
	}
}

class Parser {
public:
	Parser(const std::string& s, const CategoryMap& categoryMap)
				: categoryMap_(categoryMap)
				, s_(s)
				, pos_(0)
				, symbolType_(SYMBOL_TYPE_INVALID) {
		if (s.empty()) {
			THROW_EXCEPTION(GS::TRMControlModelException, "Boolean expression parser error: Empty string.");
		}
		nextSymbol();
	}

	void throwException(const char* errorDescription) const;
	template<typename T> void throwException(const char* errorDescription, const T& complement) const;
	bool finished() const {
		return pos_ >= s_.size();
	}
	void nextSymbol();
	RuleBooleanNode_ptr getBooleanNode();
private:
	const CategoryMap& categoryMap_;
	const std::string s_;
	std::string::size_type pos_;
	std::string symbol_;
	SymbolType symbolType_;

	void skipSpaces();
};

void
Parser::throwException(const char* errorDescription) const
{
	THROW_EXCEPTION(GS::TRMControlModelException, "Boolean expression parser error: "
					<< errorDescription << " at position " << pos_ << " of string [" << s_ << "].");
}

template<typename T>
void
Parser::throwException(const char* errorDescription, const T& complement) const
{
	THROW_EXCEPTION(GS::TRMControlModelException, "Boolean expression parser error: "
					<< errorDescription << complement << " at position " << pos_ << " of string [" << s_ << "].");
}

void
Parser::skipSpaces()
{
	while (!finished() && s_[pos_] == ' ') ++pos_;
}

void
Parser::nextSymbol()
{
	skipSpaces();

	symbol_.clear();

	if (finished()) {
		symbolType_ = SYMBOL_TYPE_INVALID;
		return;
	}

	char c = s_[pos_++];
	symbol_ += c;
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
			nextSymbol();
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
			default:
				throwException("Invalid operator: ", symbolType_);
			}
		}

		nextSymbol();
		if (symbolType_ != SYMBOL_TYPE_RIGHT_PAREN) {
			throwException("Right parenthesis not found");
		}
		return p;
	}
	case SYMBOL_TYPE_STRING:
	{
		if (symbolType_ == SYMBOL_TYPE_INVALID) {
			throwException("Could not find the category");
		}

		const std::string& symbolTmp = symbol_;
		bool matchAll = false;
		if (symbolTmp[symbolTmp.size() - 1] == matchAllChar) {
			matchAll = true;
		}

		int categoryCode = 0;
		if (!matchAll) {
			// Try to find a code for the category.
			auto itMap = categoryMap_.find(symbolTmp);
			if (itMap != categoryMap_.end()) {
				categoryCode = itMap->second->code;
			}
		}

		return RuleBooleanNode_ptr(new RuleBooleanTerminal(
						matchAll ? symbolTmp.substr(0, symbolTmp.size() - 1) : symbolTmp,
						matchAll,
						categoryCode));
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
		throwException("Invalid symbol");
	}
	return RuleBooleanNode_ptr(); // unreachable
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
	if (matchAll_) {
		return posture.isMemberOfCategory(text_, Text::MatchWithOptionalCharSuffix('\''));
	} else {
		if (categoryCode_ != 0) {
			return posture.isMemberOfCategory(categoryCode_);
		} else {
			return posture.isMemberOfCategory(text_, true);
		}
	}
}

void
RuleBooleanTerminal::print(std::ostream& out, int level) const
{
	std::string prefix(level * 8, ' ');

	out << prefix << "category [" << text_ << '_' << categoryCode_;
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
Rule::evalBooleanExpression(const PostureSequence& postureSequence) const
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
std::size_t
Rule::numberOfExpressions() const
{
	return booleanNodeList_.size();
}

/*******************************************************************************
 *
 */
void
Rule::parseBooleanExpression(const CategoryMap& categoryMap)
{
	for (std::vector<std::string>::size_type size = booleanExpressionList_.size(), i = 0; i < size; ++i) {
		Parser p(booleanExpressionList_[i], categoryMap);
		booleanNodeList_.push_back(p.getBooleanNode());
	}

	RuleBooleanNodeList::size_type size = booleanNodeList_.size();
	if (size < 2 || size > 4) {
		THROW_EXCEPTION(TRMControlModelException, "Invalid number of boolean expressions: " << size << '.');
	}
}

// ruleSymbols: {rd, beat, mark1, mark2, mark3}
// tempos[4]
void
Rule::evaluateExpressionSymbols(const double* tempos, const PostureSequence& postures, Model& model, double* ruleSymbols) const
{
	double localTempos[4];

	model.clearFormulaSymbolList();
	if (postures.size() >= 2) {
		const Posture& posture = *postures[0];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION1, posture.symbols().transition);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA1      , posture.symbols().qssa);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB1      , posture.symbols().qssb);
		const Posture& posture2 = *postures[1];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION2, posture2.symbols().transition);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA2      , posture2.symbols().qssa);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB2      , posture2.symbols().qssb);
		localTempos[0] = tempos[0];
		localTempos[1] = tempos[1];
	} else {
		localTempos[0] = 0.0;
		localTempos[1] = 0.0;
	}
	if (postures.size() >= 3) {
		const Posture& posture = *postures[2];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION3, posture.symbols().transition);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA3      , posture.symbols().qssa);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB3      , posture.symbols().qssb);
		localTempos[2] = tempos[2];
	} else {
		localTempos[2] = 0.0;
	}
	if (postures.size() == 4) {
		const Posture& posture = *postures[3];
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_TRANSITION4, posture.symbols().transition);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSA4      , posture.symbols().qssa);
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_QSSB4      , posture.symbols().qssb);
		localTempos[3] = tempos[3];
	} else {
		localTempos[3] = 0.0;
	}
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO1, localTempos[0]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO2, localTempos[1]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO3, localTempos[2]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_TEMPO4, localTempos[3]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_RD   , ruleSymbols[0]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_BEAT , ruleSymbols[1]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK1, ruleSymbols[2]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK2, ruleSymbols[3]);
	model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK3, ruleSymbols[4]);

	// Execute in this order.
	if (!exprSymbolEquations_.ruleDuration.empty()) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_RD   , model.evalEquationFormula(exprSymbolEquations_.ruleDuration));
	}
	if (!exprSymbolEquations_.mark1.empty()) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK1, model.evalEquationFormula(exprSymbolEquations_.mark1));
	}
	if (!exprSymbolEquations_.mark2.empty()) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK2, model.evalEquationFormula(exprSymbolEquations_.mark2));
	}
	if (!exprSymbolEquations_.mark3.empty()) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_MARK3, model.evalEquationFormula(exprSymbolEquations_.mark3));
	}
	if (!exprSymbolEquations_.beat.empty()) {
		model.setFormulaSymbolValue(FormulaSymbol::SYMB_BEAT , model.evalEquationFormula(exprSymbolEquations_.beat));
	}

	ruleSymbols[0] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_RD);
	ruleSymbols[1] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_BEAT);
	ruleSymbols[2] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_MARK1);
	ruleSymbols[3] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_MARK2);
	ruleSymbols[4] = model.getFormulaSymbolValue(FormulaSymbol::SYMB_MARK3);
}

} /* namespace TRMControlModel */
} /* namespace GS */
