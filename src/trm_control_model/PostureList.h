/***************************************************************************
 *  Copyright 2015 Marcelo Y. Matuda                                       *
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

#ifndef TRM_CONTROL_MODEL_POSTURE_LIST_H_
#define TRM_CONTROL_MODEL_POSTURE_LIST_H_

#include <string>
#include <memory>
#include <vector>



namespace GS {
namespace TRMControlModel {

class Posture;

class PostureList {
public:
	typedef std::vector<std::unique_ptr<Posture>>::size_type size_type;

	PostureList();
	~PostureList();

	size_type size() const { return postureList_.size(); }
	void clear();

	void add(std::unique_ptr<Posture> posture);
	void remove(size_type index);

	const Posture& operator[](size_type index) const;
	Posture& operator[](size_type index);

	const Posture* find(const std::string& name) const;
	Posture* find(const std::string& name);
private:
	std::vector<std::unique_ptr<Posture>> postureList_; // always sorted, by name
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_POSTURE_LIST_H_ */
