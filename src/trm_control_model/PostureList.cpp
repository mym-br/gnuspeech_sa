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

#include "PostureList.h"

#include <algorithm> /* lower_bound */
#include <utility> /* move */

#include "Exception.h"
#include "Posture.h"



namespace {

struct NameLessThan {
	bool operator()(const std::unique_ptr<GS::TRMControlModel::Posture>& posture, const std::string& name) {
		return posture->name() < name;
	}
};

} // namespace

namespace GS {
namespace TRMControlModel {

PostureList::PostureList()
{
}

PostureList::~PostureList()
{
}

void
PostureList::clear()
{
	postureList_.clear();
}

void
PostureList::add(std::unique_ptr<Posture> posture)
{
	auto iter = std::lower_bound(postureList_.begin(), postureList_.end(), posture->name(), NameLessThan());
	if (iter == postureList_.end()) {
		postureList_.push_back(std::move(posture));
	} else if ((*iter)->name() == posture->name()) {
		THROW_EXCEPTION(InvalidParameterException, "Can't add posture: duplicate posture name: " << posture->name() << '.');
	} else {
		postureList_.insert(iter, std::move(posture));
	}
}

/*******************************************************************************
 *
 * The index is not checked.
 */
void
PostureList::remove(size_type index)
{
	postureList_.erase(postureList_.begin() + index);
}

/*******************************************************************************
 *
 * The index is not checked.
 */
const Posture&
PostureList::operator[](size_type index) const
{
	return *postureList_[index];
}
Posture&
PostureList::operator[](size_type index)
{
	return *postureList_[index];
}

/*******************************************************************************
 * Finds a Posture with the given name.
 *
 * Returns a pointer to the Posture, or nullptr if a Posture
 * was not found.
 */
const Posture*
PostureList::find(const std::string& name) const
{
	const auto iter = std::lower_bound(postureList_.begin(), postureList_.end(), name, NameLessThan());
	if (iter == postureList_.end() || (*iter)->name() != name) {
		return nullptr;
	}
	return iter->get();
}
Posture*
PostureList::find(const std::string& name)
{
	auto iter = std::lower_bound(postureList_.begin(), postureList_.end(), name, NameLessThan());
	if (iter == postureList_.end() || (*iter)->name() != name) {
		return nullptr;
	}
	return iter->get();
}

} /* namespace TRMControlModel */
} /* namespace GS */
