/*
 * Impl_serialize.cpp
 *
 *  Created on: Mar 21, 2010
 *      Author: Randy
 */
#include <string>
#include <memory>
#include "Intf_serialize.h"

namespace xp { namespace serialize {


//string
template <>
ISerialize& operator <<(ISerialize& sr, const std::string& t){
	uint16_t len = (uint16_t)t.length();
	sr << len;
	if(len)	sr.write(t.data(), len);
	return sr;
}
template <>
ISerialize& operator >>(ISerialize& sr, std::string& t){
	t = ""; //Force release shared internal data pointer
	uint16_t len;
	sr >> len;
	if(len){
		t.resize(len);
		sr.read((void*)t.data(), len);
	}
	return sr;
}

//wstring
template <>
ISerialize& operator <<(ISerialize& sr, const std::wstring& t){
	uint16_t len = (uint16_t)t.length();
	sr << len;
	sr.write(t.data(), len*sizeof(wchar_t));
	return sr;
}
template <>
ISerialize& operator >>(ISerialize& sr, std::wstring& t){
	t = L""; //Force release shared internal data pointer
	uint16_t len;
	sr >> len;
	if(len){
		t.resize(len);
		sr.read((void*)t.data(), len * sizeof(wchar_t));
	}
	return sr;
}

//bool
template <>
ISerialize& operator <<(ISerialize& sr, const bool& t){
	sr << (uint8_t)t;
	return sr;
}
template <>
ISerialize& operator >>(ISerialize& sr, bool& t){
	uint8_t v;
	sr >> v;
	t = (v != 0);
	return sr;
}

uint32_t copy(ISerialize& reader, ISerialize& writer, uint32_t copyLen){
	#define DATA_BUFSIZE 1024*16
	//char buf[DATA_BUFSIZE];
	std::auto_ptr<char> buf(new char[DATA_BUFSIZE]);
	uint32_t lenLeft = copyLen;
	while(lenLeft){
		uint32_t len = DATA_BUFSIZE;
		if(len > lenLeft) len = lenLeft;

		len = (uint32_t)reader.read(buf.get(), (int)len);
		if(len){
			writer.write(buf.get(), (int)len);
			lenLeft -= len;
		}else break; //end of reader
	}
	return copyLen - lenLeft;
}


}} //xp::serialize
