/***************************************************************************
 *  Copyright 2014 Marcelo Y. Matuda                                       *
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

#ifndef TRM_CONTROL_MODEL_EQUATION_H_
#define TRM_CONTROL_MODEL_EQUATION_H_

#include <iostream>
#include <string>
#include <memory>
#include <utility> /* move */

#include "FormulaSymbol.h"



namespace GS {
namespace TRMControlModel {

class FormulaNode {
public:
	virtual ~FormulaNode() {}

	virtual float eval(const FormulaSymbolList& symbolList) const = 0;
	virtual void print(std::ostream& out, int level = 0) const = 0;
};

typedef std::unique_ptr<FormulaNode> FormulaNode_ptr;

class FormulaMinusUnaryOp : public FormulaNode {
public:
	FormulaMinusUnaryOp(FormulaNode_ptr c)
			: FormulaNode(), child_(std::move(c)) {}
	virtual ~FormulaMinusUnaryOp() {}

	virtual float eval(const FormulaSymbolList& symbolList) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	FormulaNode_ptr child_;
};

class FormulaAddBinaryOp : public FormulaNode {
public:
	FormulaAddBinaryOp(FormulaNode_ptr c1, FormulaNode_ptr c2)
			: FormulaNode(), child1_(std::move(c1)), child2_(std::move(c2)) {}
	virtual ~FormulaAddBinaryOp() {}

	virtual float eval(const FormulaSymbolList& symbolList) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	FormulaNode_ptr child1_;
	FormulaNode_ptr child2_;
};

class FormulaSubBinaryOp : public FormulaNode {
public:
	FormulaSubBinaryOp(FormulaNode_ptr c1, FormulaNode_ptr c2)
			: FormulaNode(), child1_(std::move(c1)), child2_(std::move(c2)) {}
	virtual ~FormulaSubBinaryOp() {}

	virtual float eval(const FormulaSymbolList& symbolList) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	FormulaNode_ptr child1_;
	FormulaNode_ptr child2_;
};

class FormulaMultBinaryOp : public FormulaNode {
public:
	FormulaMultBinaryOp(FormulaNode_ptr c1, FormulaNode_ptr c2)
			: FormulaNode(), child1_(std::move(c1)), child2_(std::move(c2)) {}
	virtual ~FormulaMultBinaryOp() {}

	virtual float eval(const FormulaSymbolList& symbolList) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	FormulaNode_ptr child1_;
	FormulaNode_ptr child2_;
};

class FormulaDivBinaryOp : public FormulaNode {
public:
	FormulaDivBinaryOp(FormulaNode_ptr c1, FormulaNode_ptr c2)
			: FormulaNode(), child1_(std::move(c1)), child2_(std::move(c2)) {}
	virtual ~FormulaDivBinaryOp() {}

	virtual float eval(const FormulaSymbolList& symbolList) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	FormulaNode_ptr child1_;
	FormulaNode_ptr child2_;
};

class FormulaConst : public FormulaNode {
public:
	FormulaConst(float value)
			: FormulaNode(), value_(value) {}
	virtual ~FormulaConst() {}

	virtual float eval(const FormulaSymbolList& symbolList) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	float value_;
};

class FormulaSymbolValue : public FormulaNode {
public:
	FormulaSymbolValue(FormulaSymbol::Code symbol)
			: FormulaNode(), symbol_(symbol) {}
	virtual ~FormulaSymbolValue() {}

	virtual float eval(const FormulaSymbolList& symbolList) const;
	virtual void print(std::ostream& out, int level = 0) const;
private:
	FormulaSymbol::Code symbol_;
};

std::ostream& operator<<(std::ostream& out, const FormulaNode& node);

struct Equation {
	std::string groupName;
	std::string name;
	std::string formula;
	std::string comment;
	FormulaNode_ptr formulaRoot;

	Equation() : groupName(), name(), formula(), comment(), formulaRoot() {}

	void parseFormula(const FormulaSymbol& formulaSymbol);
	float evalFormula(const FormulaSymbolList& symbolList) const;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_EQUATION_H_ */
