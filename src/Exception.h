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

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>
#include <sstream>
#include <string>

#ifdef __GNUC__
# define FUNCTION_NAME __PRETTY_FUNCTION__
#else
# define FUNCTION_NAME __func__
#endif


// __func__ is defined in C99/C++11.
// __PRETTY_FUNCTION__ is a gcc extension.
#define THROW_EXCEPTION(E,M) \
	do {\
		GS::ErrorMessage em;\
		E exc;\
		try { em << M << "\n[file: " << __FILE__ << "]\n[function: " << FUNCTION_NAME << "]\n[line: " << __LINE__ << "]"; } catch (...) {}\
		exc.setMessage(em);\
		throw exc;\
	} while (false)



namespace GS {

/*******************************************************************************
 *
 *
 * This class may throw std::bad_alloc.
 */
class ErrorMessage {
public:
	ErrorMessage() {}
	~ErrorMessage() {}

	template<typename T>
	ErrorMessage& operator<<(const T& messagePart)
	{
		buffer_ << messagePart;
		return *this;
	}

	ErrorMessage& operator<<(const std::exception& e)
	{
		buffer_ << e.what();
		return *this;
	}

	std::string getString() const
	{
		return buffer_.str();
	}
private:
	ErrorMessage(const ErrorMessage&);
	ErrorMessage& operator=(const ErrorMessage&);

	std::ostringstream buffer_;
};

/*******************************************************************************
 *
 */
struct Exception : public virtual std::exception {
	Exception() throw() {}
	~Exception() throw() {}

	virtual const char* what() const throw()
	{
		const char* cs = "";
		try {
			cs = message.c_str();
		} catch (...) {
			// Ignore.
		}
		return cs;
	}

	void setMessage(const ErrorMessage& em)
	{
		try {
			message = em.getString();
		} catch (...) {
			// Ignore.
		}
	}

	std::string message;
};

struct EndOfBufferException : public virtual Exception {};
struct ExternalProgramExecutionException : public virtual Exception {};
struct InvalidCallException : public virtual Exception {};
struct InvalidDirectoryException : public virtual Exception {};
struct InvalidFileException : public virtual Exception {};
struct InvalidParameterException : public virtual Exception {};
struct InvalidStateException : public virtual Exception {};
struct InvalidValueException : public virtual Exception {};
struct IOException : public virtual Exception {};
struct MissingValueException : public virtual Exception {};
struct TextParserException : public virtual Exception {};
struct TRMControlModelException : public virtual Exception {};
struct TRMException : public virtual Exception {};
struct UnavailableResourceException : public virtual Exception {};
struct WrongBufferSizeException : public virtual Exception {};
struct XMLException : public virtual Exception {};

} /* namespace GS */

#endif /* EXCEPTION_H_ */
