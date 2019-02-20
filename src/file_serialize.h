/**
 * \file file_serialize.h
 * \brief file-based serialize
 *
 *  Created on: Mar 25, 2010
 *      Author: Randy
 */

#ifndef _XP_FILE_SERIALIZE_H_
#define _XP_FILE_SERIALIZE_H_

#include "Intf_serialize.h"
#include "xp_exception.h"

#include <assert.h>
#include <fstream>

#include "Impl_intfs.h"

namespace xp { namespace serialize {

//Error codes
const int XPERR_OPEN_FILE = -100;
const int XPERR_OP_NOTSUPPORTED = -101;

namespace _detail {

class file_base : public TRefObj<ISerialize> {
protected:
	FILE* _file;
	bool _autoClose;

	//
	file_base(const char* file, const char* mode) throw(xp_exception) :_file(NULL), _autoClose(true){
		_file = fopen(file, mode);
		if(NULL == _file) RAISE_EXCEPTION(XPERR_OPEN_FILE, file, mode);
	}
	file_base(FILE* fp, bool autoClose):_file(fp), _autoClose(autoClose){}

	~file_base(){
		if(_file && _autoClose) fclose(_file);
	}
public:
	virtual pos_t pos() const{
		return (pos_t)ftell(_file);
	}
	virtual pos_t seek(offset_t offset, seek_tag tag){
		fseek(_file, offset, (int)tag);
		return pos();
	}

};

}//_detail


class file_writer : public _detail::file_base {
private:
	//
	file_writer(FILE* fp, bool autoClose): _detail::file_base(fp, autoClose){
	}
	file_writer(const char* file, const char* mode) throw(xp_exception)
			:_detail::file_base(file, mode){
	};

public:
	virtual bool toLoad() const {
		return false;
	}
	virtual int write(const void* buf, int len){
		return (int)fwrite(buf, 1, len, _file);
	}
	virtual int read(void* buf, int len){
		(void)buf; (void)len;
		RAISE_EXCEPTION(XPERR_OP_NOTSUPPORTED, "serialize::read");
		return -1; //not supported!
	}
	//
	static inline file_writer* create(const char* file, const char* mode = "wb" ) throw(xp_exception){
		return new file_writer(file, mode);
	}
	static inline file_writer* create(FILE* fp, bool autoClose = true){
		return new file_writer(fp, autoClose);
	}
};

class file_reader : public _detail::file_base {
private:
	file_reader(const char* file, const char* mode = "rb" ) throw(xp_exception)
			:_detail::file_base(file, mode){
	}
	file_reader(FILE* fp, bool autoClose = true): _detail::file_base(fp, autoClose){}
public:
	virtual bool toLoad() const {
		return true;
	}
	virtual int write(const void* buf, int len){
		(void)buf; (void)len;
		RAISE_EXCEPTION(XPERR_OP_NOTSUPPORTED, "serialize::write");
		return -1; //not supported!
	}
	virtual int read(void* buf, int len){
		return (int)fread(buf, 1, len, _file);
	}

	static inline file_reader* create(const char* file, const char* mode = "rb" ) throw(xp_exception){
		return new file_reader(file, mode);
	}
	static inline file_reader* create(FILE* fp, bool autoClose = true){
		return new file_reader(fp, autoClose);
	}
};

class file_io : public _detail::file_base {
private:
	file_io(const char* file, const char* mode = "r+b" ) throw(xp_exception)
			:_detail::file_base(file, mode){
	}
	file_io(FILE* fp, bool autoClose = true): _detail::file_base(fp, autoClose){}
public:
	virtual bool toLoad() const {
		assert(false); //Do not call this api!
		return true;
	}
	virtual int write(const void* buf, int len){
		fseek(_file, 0, SEEK_CUR);
		return (int)fwrite(buf, 1, len, _file);
	}
	virtual int read(void* buf, int len){
		fseek(_file, 0, SEEK_CUR);
		return (int)fread(buf, 1, len, _file);
	}
	static inline file_io* create(const char* file, const char* mode = "r+b" ) throw(xp_exception){
		return new file_io(file, mode);
	}
	static inline file_io* create(FILE* fp, bool autoClose = true){
		return new file_io(fp, autoClose);
	}
};



}}//xp::serialize

#endif /* FILE_SERIALIZE_H_ */
