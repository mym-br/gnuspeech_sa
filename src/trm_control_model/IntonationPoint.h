/***************************************************************************
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
// This file was copied from Gnuspeech and modified by Marcelo Y. Matuda.

#ifndef TRM_CONTROL_MODEL_INTONATION_POINT_H_
#define TRM_CONTROL_MODEL_INTONATION_POINT_H_



namespace GS {
namespace TRMControlModel {

class EventList;

class IntonationPoint {
public:
	IntonationPoint(EventList* eventList);
	~IntonationPoint() {}

	void setSemitone(double newValue) { semitone_ = newValue; }
	double semitone() const { return semitone_; }

	void setOffsetTime(double newValue) { offsetTime_ = newValue; }
	double offsetTime() const { return offsetTime_; }

	void setSlope(double newValue) { slope_ = newValue; }
	double slope() const { return slope_; }

	void setRuleIndex(int newIndex) { ruleIndex_ = newIndex; }
	int ruleIndex() const { return ruleIndex_; }

	double absoluteTime() const;
	double beatTime() const;

private:
	double semitone_;      /* Value of the in semitones */
	double offsetTime_;    /* Points are timed wrt a beat + this offset */
	double slope_;         /* Slope of point */
	int ruleIndex_;        /* Index of posture which is the focus of this point */
	EventList* eventList_; /* Current EventList */
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_INTONATION_POINT_H_ */
