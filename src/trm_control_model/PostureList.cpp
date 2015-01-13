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

#include <algorithm> /* find_if */
#include <utility> /* make_pair, move */

#include "Exception.h"
#include "Posture.h"



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
	postureMap_.clear();
}

void
PostureList::add(std::unique_ptr<Posture> posture)
{
	auto checkIter = postureMap_.find(posture->name());
	if (checkIter != postureMap_.end()) {
		THROW_EXCEPTION(InvalidParameterException, "Can't add posture: duplicate posture name: " << posture->name() << '.');
	}

	// postureList_ is always sorted.
	auto findIter = std::find_if(postureList_.begin(), postureList_.end(),
				[&](const std::unique_ptr<Posture>& p) {
					return p->name() > posture->name();
				});
	auto newIter = postureList_.insert(findIter, std::move(posture));

	postureMap_.insert(std::make_pair((*newIter)->name(), (*newIter).get()));
}

/*******************************************************************************
 *
 * The index is not checked.
 */
void
PostureList::remove(size_type index)
{
	postureMap_.erase(postureList_[index]->name());
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

/*******************************************************************************
 *
 * The index is not checked.
 */
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
	auto findIter = postureMap_.find(name);
	if (findIter == postureMap_.end()) {
		return nullptr;
	}
	return findIter->second;
}

/*******************************************************************************
 * Finds a Posture with the given name.
 *
 * Returns a pointer to the Posture, or nullptr if a Posture
 * was not found.
 */
Posture*
PostureList::find(const std::string& name)
{
	auto findIter = postureMap_.find(name);
	if (findIter == postureMap_.end()) {
		return nullptr;
	}
	return findIter->second;
}

} /* namespace TRMControlModel */
} /* namespace GS */
