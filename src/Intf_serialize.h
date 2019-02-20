/*
 * Intf_serialize.h
 *
 *  Created on: Mar 18, 2010
 *      Author: Randy
 */

#ifndef _XP_SERIALIZE_H_
#define _XP_SERIALIZE_H_

#if defined(_MSC_VER)
#include <cstdint>
#else
#include <boost/cstdint.hpp>
#endif

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/type_traits/is_same.hpp>

#include <string>

#include "Intf_defs.h"

namespace xp { namespace serialize {

typedef enum{
	seek_begin =0, seek_current = 1, seek_end = 2
} seek_tag;

typedef uint32_t pos_t;
typedef int32_t offset_t;

struct ISerialize : IRefObj {
private:
  int _ver; //user defined version
protected:
	ISerialize():_ver(0){}
public:
  inline void setVersion(int ver){
    _ver = ver;
  }
  inline int getVersion() const {
    return _ver;
  }

public:

		virtual bool toLoad() const = 0;
		/**
		 * \param buf Caller provided buffer to hold the data
		 * \param len The size of the data buffer provided
		 * \return the size of data actually read from the source.
		 */
		virtual int read(void* buf, int len) = 0;
		/**
		 * \param buf pointer to data to write
		 * \param len size of data to write
		 * \return the size of the data written successfully, -1 if fails.
		 */
		virtual int write(const void* buf, int len) = 0;

		virtual pos_t pos() const = 0;
		virtual pos_t seek(offset_t offset, seek_tag tag) = 0;


		template<typename T>
		ISerialize& write(const T& obj) {
			int i = this->write(&obj, sizeof(T));
			assert(i == sizeof(T));
			return *this;
		}

		template<typename T>
		ISerialize& read(T& obj) {
			int i = this->read(&obj, sizeof(T));
			assert(i == sizeof(T));
			return *this;
		}


		/**
		 * Pipe-style serializing
		 *
		 * Example:
		 * \code
		 *
		 * int a,b;
		 * string name;
		 * float salary;
		 *
		 * memory_writer writer;
		 * writer | a | b | name | salary;  //equals writer << a << b << name << salary
		 *
		 * file_reader file("c:/sample.dat");
		 * file | a | b | name | salary;  //equals file >> a >> b >> name >> salary
		 *
		 * void serialize(ISerialize& sr){
		 *   sr | a | b | name | salary;
		 * }
		 *
		 *
		 * \endcode
		 */
		template<typename T>
		inline ISerialize& operator |(T& v){
			if(toLoad()){
				*this >> v;
			}
			else{
				*this << v;
			}

			return *this;
		}
};


template<typename T>
inline ISerialize& operator <<(ISerialize& sr, const T& t){
	sr.write(&t, sizeof(T));
	return sr;
}

template<typename T>
inline ISerialize& operator >>(ISerialize& sr, T& t){
	sr.read(&t, sizeof(T));

	return sr;
}


//string (implemented in Impl_serialize.cpp)
template <> ISerialize& operator <<(ISerialize& sr, const std::string& t);
template <> ISerialize& operator >>(ISerialize& sr, std::string& t);

//wstring (implemented in Impl_serialize.cpp)
template <> ISerialize& operator <<(ISerialize& sr, const std::wstring& t);
template <> ISerialize& operator >>(ISerialize& sr, std::wstring& t);

//bool: sizeof(bool) ==4 for PPC
template <> ISerialize& operator <<(ISerialize& r, const bool& v);
template <> ISerialize& operator >>(ISerialize& r, bool& v);

////////////////// serialize() ////////////////////////////////
template<typename T>
inline void serialize(ISerialize& sr, T& v){
	if(sr.toLoad()) sr >> v;
	else sr << v;
}


struct ref_init
{
    template<typename T>
    void operator()(T * p) const {
    	if(p){
    		p->ref();
    	}
    }
};

struct dummy_init
{
    template<typename T>
    void operator()(T * p) const {
    }
};


template<typename T>
void serialize_pod_array(ISerialize& sr, T& container){
	typedef typename T::value_type value_type;
	typedef typename T::iterator it_type;

	BOOST_STATIC_ASSERT(boost::is_pod<value_type>::value || boost::is_same<value_type, std::string>::value);

	uint32_t N;
	if(sr.toLoad()){
		container.clear();
		sr >> N;
		value_type v;
		for(unsigned int i=0;i<N;i++){
			sr >> v;
			container.push_back(v);
		}
	}else{
		N = (uint32_t)container.size();
		sr << N;

		for(it_type it = container.begin(); it!=container.end(); ++it){
			sr << *it;
		}
	}
}; //serialize


template<typename T, typename Tinit>
void serialize_array(ISerialize& sr, T& container, Tinit finit){
	typedef typename T::value_type pvalue_type;
	typedef typename ::boost::remove_pointer<pvalue_type>::type value_type;
	typedef typename T::iterator it_type;

	BOOST_STATIC_ASSERT(boost::is_pointer<pvalue_type>::value);

	uint32_t N;
	if(sr.toLoad()){
		sr >> N;
		for(unsigned int i=0;i<N;i++){
			value_type* pv = new value_type;
			finit(pv);
			pv->serialize(sr);
			container.push_back(pv);
		}
	}else{
		N = container.size();
		sr << N;

		for(it_type it = container.begin(); it!=container.end(); ++it){
			(*it)->serialize(sr);
		}
	}
}; //serialize

template<typename T>
inline void serialize_array_ref(ISerialize& sr, T& container){
	serialize_array(sr, container, ref_init());
}

template<typename T>
inline void serialize_array_dummy(ISerialize& sr, T& container) {
	serialize_array(sr, container, dummy_init());
}

//helpers
/**
 * keep the serialize position untouched when leaving a scope
 *
 *
 *  example:
 *
 *     pos_t p0 = sr.pos();
 *     {
 *     	  pos_lock(sr);
 *     ... do anything with sr...
 *     }
 *     assert(p0 == sr.pos());
 *
 *
 */
class pos_lock {
private:
	ISerialize& _sr;
	pos_t _pos;
public:
	pos_lock(ISerialize& sr):_sr(sr){
		sr.ref();
		_pos = sr.pos();
	}
	~pos_lock(){
		_sr.seek(_pos, seek_begin);
		_sr.unref();
	}
};

/**
 * Bookmark for value update any time at the fixed position
 *
 */
class bookmark {
private:
	ISerialize& _sr;
	pos_t _pos;
	offset_t _offset;
public:
	bookmark(ISerialize& sr, int offset = 0) :
		_sr(sr), _offset(offset) {
		sr.ref();
		_pos = sr.pos();
	}
	~bookmark(){
		_sr.unref();
	}

  void rewind(){
    _sr.seek(_pos + _offset, seek_begin);
  }

	//no endian awareness
	void updateValue(const void* buf, size_t len){
		pos_lock lock(_sr);

    rewind();
		_sr.write(buf, (int)len);
	}

	//endian-aware thanks to (sr << v)
	template<typename T> void updateValue(const T& v) {
		pos_lock lock(_sr);

    rewind();
		_sr << v;
	}
};



/**
 * streaming cursor management
 *
 * Ex:
 *
 *  ISerialize& sr;
 *
 *  pos_cursor pc1(sr);
 *  pos_cursor pc2(sr);
 *
 *	{
 *		auto_cursor cr(pc1);
 *		sr << ...;
 *
 *		pos_A = sr.pos();
 *	}
 *
 *	sr << ...
 *	pos_B = sr.pos();
 *
 *	{ pos_lock(sr);
 *
 *	  auto_cursor cr(pc1);
 *	  sr << ...;  //continue from pos_A
 *	}
 *
 *	sr << ... //continue from pos_B
 *
 *
 *
 */
class pos_cursor{
private:
	ISerialize& _sr;
	pos_t _pos;
public:
	pos_cursor(ISerialize& sr):_sr(sr){
		sr.ref();
		_pos = sr.pos();
	}
	~pos_cursor(){
		_sr.unref();
	}

	void jump(pos_t newpos){
		_sr.seek(newpos, seek_begin);
		_pos = _sr.pos();
	}
	void offset(offset_t off){
		_sr.seek(off, seek_current);
		_pos = _sr.pos();
	}

	void enter(){
		_sr.seek(_pos, seek_begin);
	}
	void leave(){
		_pos = _sr.pos();
	}
};

class auto_cursor{
private:
	pos_cursor& _cursor;
public:
	auto_cursor(pos_cursor& cursor):_cursor(cursor){
		_cursor.enter();
	}
	~auto_cursor(){
		_cursor.leave();
	}
};


//helper
uint32_t copy(ISerialize& reader, ISerialize& writer, uint32_t copyLen);

}}//xp::serialize

using xp::serialize::ISerialize;

#endif /* SERIALIZE_H_ */
