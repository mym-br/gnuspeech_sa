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

#include "Transition.h"

#include "Model.h"



namespace GS {
namespace TRMControlModel {

double
Transition::SlopeRatio::totalSlopeUnits() const
{
	double temp = 0.0;
	for (const auto& slope : slopeList) {
		temp += slope->slope;
	}
	return temp;
}

double
Transition::getPointTime(const Transition::Point& point, const Model& model)
{
	if (!point.timeExpression) {
		return point.freeTime;
	} else {
		return model.evalEquationFormula(*point.timeExpression);
	}
}

void
Transition::getPointData(const Transition::Point& point, const Model& model,
				double& time, double& value)
{
	if (!point.timeExpression) {
		time = point.freeTime;
	} else {
		time = model.evalEquationFormula(*point.timeExpression);
	}

	value = point.value;
}

void
Transition::getPointData(const Transition::Point& point, const Model& model,
				double baseline, double delta, double min, double max,
				double& time, double& value)
{
	if (!point.timeExpression) {
		time = point.freeTime;
	} else {
		time = model.evalEquationFormula(*point.timeExpression);
	}

	value = baseline + ((point.value / 100.0) * delta);
	if (value < min) {
		value = min;
	} else if (value > max) {
		value = max;
	}
}

} /* namespace TRMControlModel */
} /* namespace GS */
