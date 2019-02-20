/**
 * Intf_defs.h
 *
 *  Created on: Feb 19, 2010
 *      Author: Randy (randydu@gmail.com)
 *
 *  \file
 *  \brief Declaration of interface concept.
 */

#pragma once

#include <assert.h>

#if defined(_WIN32_)||defined(_WIN64_)
#include <windows.h>
#endif

#include <cstddef>

namespace xp {
/**
 * \file
 * \brief Interface Bus Support
 *
 * Here is a simple yet powerful COM-style interface engine implemented from scratch to
 * support flexible cross-module interface browsing and integration.
 *
 * Features:
 *
 * -# Interface browsing;
 * -# Interfaces can be aggregated on the fly via "Interface Bus";
 * -# Interface Bus has a built-in security (bus level) control.
 * 	  Interfaces (containing more secure apis) hosted on a low level bus can discover
 * 	  the interfaces hosted on high level buses.
 *
 */

/**
 * \typedef TIntfId
 * \brief The data type of interface identifier.
 *
 * Here we use a string instead of a GUID for more good code readability, it of course can be
 * a classical UUID string representation.
 */
typedef const char* TIntfId;

/**
 * \def DECLARE_IID
 * \brief defines the interface identifier in an implementation class.
 * \sa TInterface
 * \sa TInterfaceEx
 */
#define DECLARE_IID(x) static inline xp::TIntfId iid() {return #x;}

/**
 * \def IID
 * \brief extracts interface id from an interface name.
 */
#define IID(intf) intf::iid()

/**
 * \fn bool equalIID(const TIntfId id1, const TIntfId id2);
 * \brief tests if two IIDs are equal.
 */
extern bool equalIID(const TIntfId id1, const TIntfId id2);

#ifndef INTERFACE
#define INTERFACE struct
#endif


struct IRefObj {
	virtual ~IRefObj(){}
	/**
	 * increase reference count
	 */
	virtual void ref() = 0;
	/**
	 * decrease reference count, try destroying instance once reference count reaches zero.
	 */
	virtual void unref() = 0;
	/**
	 * decrease reference count, but do not destroy instance when reference count reaches zero.
	 */
	virtual void unrefNoDelete() = 0;
};

struct IBus;

struct IQueryState: public IRefObj {
	virtual void addSearchedBus(IBus* bus) = 0;
	virtual bool isBusSearched(IBus* bus) const = 0;
};

/**
 * \interface IInterface
 * \brief root of all interfaces
 *
 * It's platform independent and Windows COM interface can be converted to IInterface via an interface adapter.
 */
struct IInterface : public IRefObj
{
	DECLARE_IID(B4FF784E-2DDA-4CA2-BC84-4AAD35FCAAF3);

	/**
	 *	Interface browsing, returns 0 if successful, non-zero error code if fails.
	 */
	virtual int queryInterface(TIntfId iid, void** retIntf, IQueryState* qst) = 0;

	//helper: detect if another interface is accessible
	bool supports(TIntfId iid){
		IInterface* intf(NULL);
		if(0 == this->queryInterface(iid, (void**)&intf, NULL)){
			assert(intf);
			if(intf){
				intf->unrefNoDelete(); //balance queryInterface()
				return true;
			}
		}
		return false;
	}

	template<typename T> T* cast(){
		T* intf;
		if (this->queryInterface(T::iid(), (void**) &intf, NULL)) {
			return NULL;
		}else{
			intf->unrefNoDelete(); //Balance counter (incremented within queryInterface)
			return intf;
		}
	}
};


#define IID_IINTERFACE IID(IInterface)


struct IBus;

/**
 * \interface IInterfaceEx
 * \brief root of all bus-aware interfaces
 */
struct IInterfaceEx : public IInterface
{
	DECLARE_IID(632B176F-E7B9-4557-9657-15DB3AC94FBC);
	/**
	 * set the hosting bus
	 */
	virtual void setBus(IBus* bus)  = 0;
	/**
	 * Interface browsing without scanning external hosting bus.
	 */
	virtual int localQueryInterface(TIntfId iid, void** retIntf, IQueryState* qst) = 0;
};


#define IID_IINTERFACEEX IID(IInterfaceEx)

/**
 * \interface IBus
 * \brief Interface integration bus is used to connect multiple interfaces on the fly.
 *
 * Interface Bus itself can be inter-connected to form a more complex topology.
 *
 * Note: only IInterfaceEx-derived interfaces can be connected via IBus.
 * However, it is possible that other more generic IInterface or Windows IUnknown can be supported
 * as well by interface hooking.
 */
struct IBus : public IInterfaceEx
{
	DECLARE_IID(B7914714-4159-48C6-BFF3-A21C6F0BB1CA);

	/**
	 * Connect the intf to this bus.
	 *
	 * intf can be a normal interface or a IBus. For a IBus, only the bus with lower or equal bus level
	 * can be successfully connected to this bus for security concern, because by design the low level bus should
	 * hosting more secure interfaces which should not be browsed from less secure interfaces on higher level bus.
	 */
	virtual bool connect(IInterfaceEx* intf) = 0;
	/**
	 * Disconnect the intf ( a normal interface or an interace bus) from this bus.
	 * After disconnection, the intf itself and all interfaces hosted on it (for a bus) cannot be reached from
	 * interface browsing, however, if it is already retrieved and locked before the disconnection, the interface
	 * can still be used until it is released.
	 */
	virtual void disconnect(IInterfaceEx* intf) = 0;
	/**
	 * Get the Bus Level
	 *
	 * bus level 0 is the top secret level.
	 */
	virtual int getLevel()  = 0;
	/**
	 * Find a bus with specified bus level.
	 *
	 * Note there might be multiple buses with the same level in a complex bus network, and the programmer
	 * should know what he is doing.
	 */
	virtual IBus* findFirstBusByLevel(int busLevel)  = 0;
};

#define IID_IBUS IID(IBus)


/**
 * \class IEnumerator
 * \brief Generic value enumerator
 */
template<typename T>
struct IEnumerator : public IRefObj {
	///Is next value available?
	virtual bool hasNext() = 0;
	///The next values
	virtual T next() = 0;
};

/**
 * \class IEnumeratorEx
 * \brief Enhanced Generic value enumerator (with new \e size() )
 */
template<typename T>
struct IEnumeratorEx : public IRefObj {
	///Is next value available?
	virtual bool hasNext() = 0;
	///The next value
	virtual T next() = 0;
	///Get the total number of values.
	virtual unsigned int size() const = 0;
	///Random access by index.
	virtual T get(unsigned int index) const = 0;
  //go to first element to re-start the enumeration
  virtual void rewind() = 0;
};


//----- Helper ------
/**
 * \class auto_ref
 * \brief Helper class for automatic reference counting.
 *
 * Usage:
 *
 * \code
 *
 * auto_ref<ISecureBox> box (new Impl_SecureBox());
 *
 *
 * \endcode
 */
template<class T>
class auto_ref {
	typedef auto_ref<T> this_type;
private:
	T* _intf;

public:
	auto_ref():_intf((T*)NULL){}
	auto_ref(const this_type& rv):_intf(rv._intf){
		if(_intf) _intf->ref();
	}

	explicit auto_ref(IInterface* intf) {
		assert(intf);
		if (intf->queryInterface(T::iid(), (void**) &_intf, NULL)) {
			_intf = (T*)NULL; //fails
		}
	}

	/**
	 *
	 */
	auto_ref(T* intf) : _intf(intf) {
		if (intf)
			intf->ref();
	}

	auto_ref(T* intf, bool refIt) : _intf(intf) {
		if (intf && refIt)
			intf->ref();
	}
	~auto_ref() {
		if (_intf)
			_intf->unref();
	}

	void operator = (T* intf){
		if(_intf != intf){
			if (_intf) _intf->unref();
			_intf = intf;
			if(_intf) _intf->ref();
		}
	}

	void operator = (const this_type& rv){
		_intf = rv._intf;
		if(_intf) _intf->ref();
	}

	inline T& operator*() const {
		assert(_intf);
		return *_intf;
	}

	inline operator T*(void) const {
		return _intf;
	}
	//The caller just reference it and will *not* unref() the pointer when not using it anymore.
	inline T* get() const {
		return _intf;
	}
	/**
	 * The caller should call \e unref() to release the pointer when not using it anymore.
	 *
	 * \code
	 * 		auto_ref<INode> _node;
	 * 		INode* QueryInterface(IID id){
	 * 			if(id == IID(INode))
	 * 				return _node.getRef();
	 * 		}
	 * \endcode
	 */

	inline T* getRef(){
		if(_intf){
			_intf->ref();
		}
		return _intf;
	}
	inline T* operator->() const {
		return _intf;
	}

	inline operator bool() const {
		return _intf != NULL;
	}
};

template<>
class auto_ref<IInterface> {
	typedef auto_ref<IInterface> this_type;
private:
	auto_ref(const this_type&);
	const this_type& operator = (const this_type&);

	IInterface* _intf;
public:
	auto_ref():_intf(NULL){}

	auto_ref(IInterface* intf): _intf(intf) {
		assert(intf);
		intf->ref();
	}

	auto_ref(IInterface* intf, bool refIt) : _intf(intf) {
		if (intf && refIt) intf->ref();
	}
	~auto_ref() {
		if (_intf)
			_intf->unref();
	}

	void operator = (IInterface* intf){
		if(_intf != intf){
			if (_intf) _intf->unref();
			_intf = intf;
			if(_intf) _intf->ref();
		}
	}

	inline IInterface& operator*() const {
		assert(_intf);
		return *_intf;
	}

	inline operator IInterface*(void) const {
		return _intf;
	}
	//The caller just reference it and will *not* unref() the pointer when not using it anymore.
	inline IInterface* get() const {
		return _intf;
	}
	inline IInterface* getRef(){
		if(_intf){
			_intf->ref();
		}
		return _intf;
	}
	inline IInterface* operator->() const {
		return _intf;
	}

	inline operator bool() const {
		return _intf != NULL;
	}
};

/**
 * \class checked_unref
 * \brief Helper function operator to be used with std::for_each() to *release* ref-count managed objects.
 *
 * \code
 *
 * class TSample{
 * private:
 * 	 std::vector<INode> _nodes;
 * public:
 *   ~TSample(){
 *      std::for_each(_nodes.begin(),_nodes.end(), checked_unref<INode>());
 *   }
 * };
 *
 *
 * \endcode
 */
template<typename T>
struct checked_unref
{
    typedef void result_type;
    typedef T * argument_type;

    void operator()(T * p) const {
    	if(p){
    		p->unref();
    	}
    }
};

template<typename T>
struct checked_ref
{
    typedef void result_type;
    typedef T * argument_type;

    void operator()(T * p) const {
    	if(p){
    		p->ref();
    	}
    }
};


//Connect interface only if it is not plugged in.
#define BUS_CONNECT_INTERFACE(bus, intf, inst) { if(!bus->supports(intf::iid())) bus->connect(inst); }

} // xp