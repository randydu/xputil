/*
 * dummy_serialize.h
 *
 *  Created on: Sep 2, 2010
 *      Author: randy
 */

#ifndef _XP_DUMMY_SERIALIZE_H_
#define _XP_DUMMY_SERIALIZE_H_

#include "Intf_serialize.h"
#include "Impl_intfs.h"

namespace xp { namespace serialize {

/**
 * dummy_writer can detect the size of the storage for later *real* usage.
 */
class dummy_writer : public TRefObj<ISerialize>{
private:
	pos_t _pos;
	pos_t _end;

	dummy_writer():_pos(0), _end(0){}
public:
	static inline dummy_writer* create(){
		return new dummy_writer();
	}

	virtual bool toLoad() const {
		return false;
	}
	virtual int read(void* buf, int len){
		assert(false);
		(void)buf; (void)len;
		return 0;
	}
	virtual int write(const void* buf, int len){
		(void)buf;
		_pos += len;
		if(_end < _pos) _end = _pos;
		return len;
	}

	virtual pos_t pos() const {
		return _pos;
	}
	virtual pos_t seek(offset_t offset, seek_tag tag){
		switch(tag){
		case seek_begin: _pos = offset; break;
		case seek_current: _pos += offset; break;
		case seek_end: _pos = _end + offset; break;
		default:
			assert(false); //invalid seek mode
		}
		return _pos;
	}

	size_t size() const {
		return _end;
	}
};

}}//xp::serialize


#endif /* DUMMY_SERIALIZE_H_ */
