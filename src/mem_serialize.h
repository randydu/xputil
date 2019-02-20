/**
 * \file mem_serialize.h
 * \brief memory-based serialize
 *
 *  Created on: Mar 18, 2010
 *      Author: Randy
 */

#ifndef _XP_MEM_SERIALIZE_H_
#define _XP_MEM_SERIALIZE_H_

#include "Intf_serialize.h"

#include <cstdlib> //malloc
#include <cstring> //memcpy
#include <cassert>

#include "Impl_intfs.h"

namespace xp { namespace serialize {

namespace _detail{

class memory_base {
protected:
	pos_t _pos, _maxPos;
	bool _bFreeMem;
	char* _pMem;
	pos_t _totalSize;

	void updateMaxPos(){
		if(_maxPos < _pos) _maxPos = _pos;
	}
public:
	memory_base(uint32_t initSize):_pos(0),_maxPos(0),_bFreeMem(true){
    	_pMem = (char*)malloc(initSize);
    	assert(_pMem);
   		_totalSize = _pMem? initSize : 0;
    }

	memory_base(char* ptr, int len):_pos(0),_maxPos(len),_bFreeMem(false),_pMem(ptr),_totalSize(len){

	}
    ~memory_base(){
    	if(_bFreeMem && _pMem){
    		free(_pMem);
    	}
    }

    //The caller need free() the pointer later
    char* release(){
      char* result = _pMem;
      _bFreeMem = false;
      _pMem = NULL;

      return result;
    }

    pos_t length() const {
    	return _maxPos;
    }

    char* memory() const {
    	return _pMem;
    }

    pos_t pos() const{
    	return _pos;
    }

    pos_t seek(offset_t offset, seek_tag tag){
    	switch(tag){
    	case seek_begin:
    		_pos = offset;
    		break;
    	case seek_current:
    		_pos += offset;
    		break;
    	case seek_end:
    		_pos = _maxPos + offset;
    	}

		if(_pos > _totalSize) _pos = _totalSize;

    	updateMaxPos();
    	return _pos;
    }
};

}//_detail

class memory_sink : public _detail::memory_base{
	typedef _detail::memory_base inherited;
public:
	int write(const void* buf, int len){
		pos_t newPos = _pos + len;
    	if(newPos > _totalSize){
    		unsigned int newSize = 2*_totalSize; //double size
    		if(newSize < newPos) newSize = newPos;

    		char* p = (char*)realloc(_pMem, newSize);
    		if(p){
    			_totalSize = newSize;
    			_pMem = p;
    		}else{
    			//cannot enlarge
    			assert(0);
    			return -1;
    		}
    	}

		memcpy(_pMem + _pos, buf , len);
		_pos = newPos;

    	updateMaxPos();

    	return len;
	}
    memory_sink(uint32_t initSize = 4096):inherited(initSize){
    }
};

class memory_source : public _detail::memory_base{
	typedef _detail::memory_base inherited;
public:
	int read(void* buf, int len){
		assert(buf);
		if(buf == NULL) return -1;

		int iLeft = _maxPos - _pos;
		int n = len;
		if (n > iLeft) n = iLeft;
		if(n > 0){
			memcpy(buf, _pMem + _pos, n);
			_pos += n;
	    	return n;
		}else
			return -1;
	}
	memory_source():inherited(NULL, 0){}
	memory_source(char* ptr, int len):inherited(ptr, len){}

	void attach(const void* ptr, int len){
		assert(_pMem == NULL );
		_pMem = (char*)ptr;

		_maxPos = len;
		_totalSize = len;
	}
};



class memory_writer : public TRefObj<ISerialize> {
private:
	memory_sink _sink;

	memory_writer(){}
public:

	static inline memory_writer* create(){
		return new memory_writer();
	}

	virtual bool toLoad() const {
		return false;
	}
	virtual int write(const void* buf, int len){
		return _sink.write(buf, len);
	}
	virtual int read(void* buf, int len){
		(void)buf;
		(void)len;

		assert(false);
		return -1; //not supported!
	}
	virtual pos_t pos() const{
		return _sink.pos();
	}
	virtual pos_t seek(offset_t offset, seek_tag tag){
		return _sink.seek(offset, tag);
	}

	inline int length() const {
		return _sink.length();
	}
	inline const void* memory() const {
		return _sink.memory();
	}
  inline const void* release(){
    return _sink.release();
  }
};

class memory_reader : public TRefObj<ISerialize> {
private:
	memory_source _src;
	void* _ptr;
	/*
	 * LocalCopy:
	 *
	 *   Since ISerialize can be referenced by other classes for later usage, the localCopy can be false
	 *   only when you are sure the input buffer is persistent within the lifetime of this memory_reader instance.
	 */
	memory_reader(const void* ptr, int len, bool localCopy){
		if(localCopy){
			//do a local copy in case the input ptr is released outside.
			assert(len > 0);
			_ptr = (char*)malloc(len);
			assert(_ptr);
			if(_ptr){
			  memcpy(_ptr, ptr, len);
			  _src.attach(_ptr, len);
			}
		}else{
			_ptr = NULL;
			_src.attach(ptr, len);
		}
	}

	virtual ~memory_reader(){
		if(_ptr){//local copy
			free(_ptr);
		}
	}

public:
	static inline memory_reader* create(const void* ptr, int len, bool localCopy){
		return new memory_reader(ptr, len, localCopy);
	}
	virtual bool toLoad() const {
		return true;
	}
	virtual int write(const void* buf, int len){
		(void)buf; (void)len;
		assert(false);
		return -1; //not supported!
	}
	virtual int read(void* buf, int len){
		return _src.read(buf, len);
	}
	virtual pos_t pos() const{
		return _src.pos();
	}
	virtual pos_t seek(offset_t offset, seek_tag tag){
		return _src.seek(offset, tag);
	}
	inline int length() const {
		return _src.length();
	}
	inline const void* memory() const {
		return _src.memory();
	}
  inline const void* release(){
    return _src.release();
  }
};



}}//xp::serialize

#endif /* MEM_SERIALIZE_H_ */
