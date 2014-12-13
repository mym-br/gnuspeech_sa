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
// 2014-12
// This file was created by Marcelo Y. Matuda, and code/information
// from Gnuspeech was added to it later.

#ifndef TRM_CONTROL_MODEL_SYMBOL_H_
#define TRM_CONTROL_MODEL_SYMBOL_H_

#include <memory>
#include <string>



namespace GS {
namespace TRMControlModel {

class Symbol {
public:
	Symbol(const std::string& name, float minimum, float maximum, float defaultValue)
		: name_(name)
		, minimum_(minimum)
		, maximum_(maximum)
		, defaultValue_(defaultValue)
	{}

	const std::string& name() const { return name_; }
	float minimum() const { return minimum_; }
	float maximum() const { return maximum_; }
	float defaultValue() const { return defaultValue_; }
private:
	std::string name_;
	float minimum_;
	float maximum_;
	float defaultValue_;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_SYMBOL_H_ */
