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

#ifndef TRM_CONTROL_MODEL_XML_CONFIG_FILE_WRITER_H_
#define TRM_CONTROL_MODEL_XML_CONFIG_FILE_WRITER_H_

#include <fstream>
#include <string>



namespace GS {
namespace TRMControlModel {

class Model;

/*******************************************************************************
 *
 */
class XMLConfigFileWriter {
public:
	XMLConfigFileWriter(const Model& model, const std::string& filePath);
	~XMLConfigFileWriter();

	void saveModel();
private:
	XMLConfigFileWriter(const XMLConfigFileWriter&) = delete;
	XMLConfigFileWriter& operator=(const XMLConfigFileWriter&) = delete;

	void writeElements();

	const Model& model_;
	std::ofstream out_;
	std::string filePath_;
};

} /* namespace TRMControlModel */
} /* namespace GS */

#endif /* TRM_CONTROL_MODEL_XML_CONFIG_FILE_WRITER_H_ */
