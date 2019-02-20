/*
 * \file yl_exception.h
 * \brief
 *
 *  Created on: Mar 23, 2010
 *      Author: Randy
 */

#ifndef _XP_EXCEPTION_H_
#define _XP_EXCEPTION_H_

#include "type_defs.h"

#include <exception>
#include <string>

//Disable annoying message: warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#ifdef _MSC_VER
#pragma warning(disable:4290)
#endif


namespace xp {
/**
 * \class xp_exception
 *
 * \brief base type for all exception thrown from xputil classes.
 */
class xp_exception : public std::exception{
private:
	int _code;				//and err_code
	std::string _msg;
public:
	xp_exception(int code, const char* message) throw() :std::exception(),_code(code),_msg(message){}
	virtual ~xp_exception() throw (){}

	int code() const throw() { return _code; }
	virtual const char* what() const throw() { return _msg.c_str(); }

	///explicit error message
	static NORETURN void raise(int code, const char* message, ...);
};

enum{ EC_STD_EXCEPTION = -1, EC_UNKNOWN_EXCEPTION = -2 };

};//xp

#define RAISE_EXCEPTION xp::xp_exception::raise


#endif /* XP_EXCEPTION_H_ */
