#ifndef _DUALWAY_SERIALIZE_H_
#define _DUALWAY_SERIALIZE_H_

/*
  dual-way compatibility object serialize schema

  It keeps version compatibility in dual-way.
*/

#include "Intf_serialize.h"
#include "mem_serialize.h"
#include "xp_exception.h"

namespace xp {

	template<typename T> void dualway_serialize(T* obj, ISerialize & sr, bool skipOnError = false) {
		if (sr.toLoad()) {
			unsigned int len;
			sr >> len;
			XP_TRACE("dualway: loading len [%d]", len);
			{
				xp::serialize::pos_lock lock(sr);
				try {
					obj->serialize(sr);
				}
				catch (xp_exception& e) {
					XP_TRACE_WARN("xp_exception: code [%d] message [%s]!", e.code(), e.what());
					if (!skipOnError) {
						throw;
					}
				}
				catch (std::exception& e) {
					XP_TRACE_WARN("exception: [%s]!", e.what());
					if (!skipOnError) {
						throw;
					}
				}
				catch (...) {
					XP_TRACE0("Unknown exception occurs!");
					if (!skipOnError) {
						throw;
					}
				}
			}
			sr.seek(len, xp::serialize::seek_current);
		}
		else {
			auto_ref<xp::serialize::memory_writer> writer(xp::serialize::memory_writer::create());
			obj->serialize(*writer);

			unsigned int len = writer->length();
			XP_TRACE("dualway: saving len [%d]", len);
			sr << len;
			sr.write(writer->memory(), len);
		}
	}

};//xp


#endif