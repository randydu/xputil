/*
 * Impl_Intfs.cpp
 *
 *  Created on: Feb 19, 2010
 *      Author: Randy (randydu@gmail.com)
 */

#include "stdafx.h"
#include "Impl_intfs.h"

#include <string.h>

namespace xp {
	bool equalIID(const TIntfId id1, const TIntfId id2) {
		return 0 == strcmp(id1, id2);
	}
}//xp
