#include "xp_exception.h"

#include <stdarg.h>
#include <stdio.h>

namespace xp {

NORETURN void xp_exception::raise(int code, const char* message, ...){
	  va_list list;
	  va_start(list,message);
	  char buf[1024];
#ifdef _MSC_VER
	  vsnprintf_s( buf, sizeof(buf), _TRUNCATE, message, list);	  
#else
	  vsnprintf(buf, sizeof(buf)-1, message, list);
#endif
	  va_end(list);
	  
	  throw xp_exception(code, buf);
	}

}//xp