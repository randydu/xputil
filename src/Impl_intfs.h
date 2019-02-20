/**
 * Impl_Intfs.h
 *
 *  Created on: Feb 19, 2010
 *      Author: Randy (randydu@gmail.com)
 *
 *  \file
 *  \brief Implementation of interface concept.
 */

#pragma once

#include "Intf_defs.h"
#include <assert.h>
#include <vector>
#include <algorithm>
#include <memory>

namespace xp {


template<class T>
class TRefObj: public T {
protected:
	int _count;


public:
	TRefObj() :	_count(0) {
	}

	virtual ~TRefObj() {
		assert((_count == 0) && "TRefObj::~TRefObj >> non-zero count!");
	}
	int refCount() const { return _count; }

	template<typename T1> TRefObj(T1 t1):T(t1),_count(0){}
	template<typename T1, typename T2> TRefObj(T1 t1, T2 t2):T(t1,t2),_count(0){}
	template<typename T1, typename T2, typename T3> TRefObj(T1 t1, T2 t2, T3 t3):T(t1,t2,t3),_count(0){}
	//IRefObj
	virtual void ref() {
		++_count;
	}
	virtual void unref() {
		if (--_count == 0) {
			delete this;
		}
	}
	virtual void unrefNoDelete() {
		--_count;
		assert(_count >=0);
	}
};


/**
 * \class TInterface<>
 * \brief Implements IInterface
 *
 *  Usage:
 *
 *  - defines a new interface
 *  \code
 *  INTERFACE IHello : public IInterface {
 *     virtual void sayHello() = 0;
 *  };
 *	\endcode
 *  - Implementation
 *  \code
 *  class Impl_Hello : public IHello {
 *    public:
 *      void sayHello(){
 *         cout << "Hello World!" << endl;
 *      }
 *  };
 * \endcode
 *  - use it:
 *  \code
 *  auto_ref<IHello> hello(new TInterface<Impl_Hello>());
 *  hello->sayHello(); //say "Hello World!";
 *  \endcode
 *
 */

template<class T>
class TInterface: public T {
protected:
	int _count;
public:
	TInterface():_count(0) {
	}
	virtual ~TInterface() {
		assert((_count == 0) && "TInterface::~TInterface >> non-zero count!");
	}
	template<typename T1> TInterface(T1 t1):T(t1),_count(0){}
	template<typename T1, typename T2> TInterface(T1 t1, T2 t2):T(t1,t2),_count(0){}
	template<typename T1, typename T2, typename T3> TInterface(T1 t1, T2 t2, T3 t3):T(t1,t2,t3),_count(0){}

	//IInterface
	virtual int queryInterface(TIntfId iid, void** retIntf, IQueryState* qst) {
		if (equalIID(iid, T::iid()) || equalIID(iid, IID_IINTERFACE)) {
			this->ref();
			*retIntf = (IInterface*) (this);
			return 0;
		}

		return 1;
	}
	virtual void ref() {
		++_count;
	}
	virtual void unref() {
		if (--_count == 0) {
			delete this;
		}
	}
	virtual void unrefNoDelete() {
		--_count;
		assert(_count >= 0);
	}
};

/**
 * \class TInterfaceEx<>
 * \brief Implements IInterfaceEx
 *
 *  Usage:
 *
 *  - defines a new interface
 *  \code
 *  INTERFACE IHello : public IInterfaceEx {
 *     DECLARE_IID(IHello);
 *
 *     virtual void sayHello() = 0;
 *  };
 *	\endcode
 *  - Implementation
 *  \code
 *  class Impl_Hello : public IHello {
 *    public:
 *      void sayHello(){
 *         cout << "Hello World!" << endl;
 *      }
 *  };
 * \endcode
 *  - use it:
 *  \code
 *
 *  IBus* bus; //initialized elsewhere
 *
 *  bus->connect(new TInterfaceEx<Impl_Hello>()); //bus connection.
 *
 *  IHello* hello;
 *  if(0 == bus->queryInterface("IHello", (void**)&hello){
 *    hello->sayHello(); //==> "Hello World!"
 *    hello->unref();
 *  }
 *  \endcode
 *
 *
 */
template<class T>
class TInterfaceEx: public T {
protected:
	int _count;
	IBus* _bus;
public:
	TInterfaceEx() :
		_count(0), _bus(NULL) {
	}
	virtual ~TInterfaceEx() {
		//might not has been connected with any bus
		//assert((_bus == NULL)&& "TInterfaceEx::~TInterfaceEx >> should has been unplugged from hub!");
		assert((_count == 0) && "TInterfaceEx::~TInterfaceEx >> non-zero count!");
	}
	template<typename T1> TInterfaceEx(T1 t1):T(t1),_count(0), _bus(NULL){}
	template<typename T1, typename T2> TInterfaceEx(T1 t1, T2 t2):T(t1,t2),_count(0),_bus(NULL){}
	template<typename T1, typename T2, typename T3> TInterfaceEx(T1 t1, T2 t2, T3 t3):T(t1,t2,t3),_count(0),_bus(NULL){}

	virtual int localQueryInterface(TIntfId iid, void** retIntf, IQueryState* qst) {
		if (equalIID(iid, T::iid()) || equalIID(iid, IID_IINTERFACEEX)|| equalIID(iid, IID_IINTERFACE)) {
			this->ref();
			*retIntf = (IInterface*) (this);
			return 0;
		}
		return 1;
	}
	//IInterface
	virtual int queryInterface(TIntfId iid, void** retIntf, IQueryState* qst) {
		if (0 == localQueryInterface(iid, retIntf, qst))
			return 0;
		if (_bus) {
			if ((qst == NULL) || !qst->isBusSearched(_bus)) {
				return _bus->queryInterface(iid, retIntf, qst);
			}
		}
		return 1;
	}
	virtual void ref() {
		++_count;
	}
	virtual void unref() {
		if (--_count == 0) {
			delete this;
		}
	}
	virtual void unrefNoDelete() {
		--_count;
		assert(_count >= 0);
	}
	//IInterfaceEx
	virtual void setBus(IBus* bus) {
		_bus = bus;
	}
};

#define BEGIN_INTERFACES  public: bool supportIntf(TIntfId iid){ 

#define IMPL_INTERFACE(intf) { if(equalIID(iid, intf::iid())) return true; }

#define END_INTERFACES return false; }

template<class T>
class TMultiInterfaceEx: public T {
protected:
	int _count;
	IBus* _bus;
public:
  TMultiInterfaceEx() :
		_count(0), _bus(NULL) {
	}
  virtual ~TMultiInterfaceEx() {
		//might not has been connected with any bus
		//assert((_bus == NULL)&& "TInterfaceEx::~TInterfaceEx >> should has been unplugged from hub!");
		assert((_count == 0) && "TMultiInterfaceEx::~TMultiInterfaceEx >> non-zero count!");
	}
  template<typename T1> TMultiInterfaceEx(T1 t1) :T(t1), _count(0), _bus(NULL){}
  template<typename T1, typename T2> TMultiInterfaceEx(T1 t1, T2 t2) : T(t1, t2), _count(0), _bus(NULL){}
  template<typename T1, typename T2, typename T3> TMultiInterfaceEx(T1 t1, T2 t2, T3 t3) : T(t1, t2, t3), _count(0), _bus(NULL){}

	virtual int localQueryInterface(TIntfId iid, void** retIntf, IQueryState* qst) {
		if (T::supportIntf(iid) || equalIID(iid, IID_IINTERFACEEX)|| equalIID(iid, IID_IINTERFACE)) {
			this->ref();
			*retIntf = (IInterface*) (this);
			return 0;
		}
		return 1;
	}
	//IInterface
	virtual int queryInterface(TIntfId iid, void** retIntf, IQueryState* qst) {
		if (0 == localQueryInterface(iid, retIntf, qst))
			return 0;
		if (_bus) {
			if ((NULL == qst) || !qst->isBusSearched(_bus)) {
				return _bus->queryInterface(iid, retIntf, qst);
			}
		}
		return 1;
	}
	virtual void ref() {
		++_count;
	}
	virtual void unref() {
		if (--_count == 0) {
			delete this;
		}
	}
	virtual void unrefNoDelete() {
		--_count;
		assert(_count >= 0);
	}
	//IInterfaceEx
	virtual void setBus(IBus* bus) {
		_bus = bus;
	}
};

/**
 * \class Impl_IBus
 * \brief Implements IBus
 *
 *  Usage:
 *
 *  In the following example, we create 4 different interfaces and 2 level of interface buses, then connect
 *  them together as a two-layer network:
 *  <pre>
 *      Secured BUS0: ( IA, IB)
 *  Non-Secured BUS1: ( IC, ID, ( +BUS0 ))
 * </pre>
 *  It demonstrates cross-bus interface browsing and integration.
 * <pre>
 *  IA ==> (IB, IC, ID)
 *  IB ==> (IA, IC, ID)
 *  IC ==> (ID)
 *  ID ==> (IC)
 * </pre>
 *
 *  \code
 *
 #include <iostream>
 using namespace std;

 #include "Impl_Intfs.h"

 INTERFACE IHello : public IInterfaceEx
 {
 virtual void sayHello(const char* name) = 0;
 };

 class CHello : public IHello{
 public:
 DECLARE_IID("IHello");

 CHello(){
 cout << "!!!Hello World!!!" << endl;
 }
 ~CHello(){
 cout << "!!!bye!!!" << endl;
 }

 virtual void sayHello(const char* name){
 cout << "Hello " << name << '!' << endl;
 }
 };


 INTERFACE IA : public IInterfaceEx
 {
 virtual void runA() = 0;
 };

 INTERFACE IB : public IInterfaceEx
 {
 virtual void runB() = 0;
 };

 INTERFACE IC : public IInterfaceEx
 {
 virtual void runC() = 0;
 };
 INTERFACE ID : public IInterfaceEx
 {
 virtual void runD() = 0;
 };

 class Impl_IA : public IA {
 public:
 DECLARE_IID("IA");

 void runA(){
 cout << "runA" << endl;
 }
 ~Impl_IA(){
 cout <<"~Impl_IA" <<endl;
 }
 };

 class Impl_IB : public IB {
 public:
 DECLARE_IID("IB");
 void runB(){
 cout << "IB::runB" <<endl;
 }
 ~Impl_IB(){
 cout <<"~Impl_IB" <<endl;
 }
 };

 class Impl_IC : public IC {
 public:
 DECLARE_IID("IC");
 void runC(){
 cout << "IC::runC" <<endl;
 }
 ~Impl_IC(){
 cout <<"~Impl_IC" <<endl;
 }
 };

 class Impl_ID : public ID {
 public:
 DECLARE_IID("ID");
 void runD(){
 cout << "ID::runD" <<endl;
 }
 ~Impl_ID(){
 cout <<"~Impl_ID" <<endl;
 }
 };

 int main() {
 auto_ref<Impl_IBus> bus0 ( new Impl_IBus(0)); //bus level = 0
 bus0->connect(new TInterfaceEx<Impl_IA>());
 bus0->connect(new TInterfaceEx<Impl_IB>());

 IA * ia;
 if (0 == bus0->queryInterface("IA", (void**)&ia)){ //BUS0 ==> IA
 ia->runA();
 ia->unref();
 }

 IB* ib;
 if( 0 == ia->queryInterface("IB",(void**)&ib)){ //IA ==> IB
 ib->runB();
 ib->unref();
 }


 cout <<"Testing bus cascading..."<<endl;
 auto_ref<Impl_IBus> bus1 ( new Impl_IBus(1)); //bus level 1
 bus1->connect(new TInterfaceEx<Impl_IC>());
 bus1->connect(new TInterfaceEx<Impl_ID>());

 bus1->connect(bus0.get()); //connect two buses

 //IA ==> IC (cross-bus)
 IC* ic;
 if( 0 == ia->queryInterface("IC",(void**)&ic)){
 ic->runC();
 ic->unref();
 }

 //IC ==> IA (cross-bus) Upstream browsing, should fail.
 IA* ia1;
 if( 0 == ic->queryInterface("IA",(void**)&ia1)){
 cout << "Upstream browsing, success." << endl;
 ia1->runA();
 ia1->unref();
 }else{
 cout << "Upstream browsing, fail." << endl;
 }

 return 0;
 }

 *  \endcode
 *
 *
 */

class TQueryState : public TRefObj<IQueryState> {
private:
	std::vector<IBus*> _buses;
public:
	virtual void addSearchedBus(IBus* bus) override {
		bus->ref();
		_buses.push_back(bus);
	}
	virtual bool isBusSearched(IBus* bus) const override {
		return std::find(_buses.cbegin(), _buses.cend(), bus) != _buses.cend();
	}

	virtual ~TQueryState() override {
		for (auto e : _buses) e->unref();
	}
};

//IBus
class Impl_IBus: public IBus {
protected:
	int _level; //busLevel
	int _count;
	IBus* _bus; //outbound bus to connect to
	std::vector<IInterfaceEx*> _intfs;
	std::vector<IBus*> _buses; //connected inbound buses
public:
	Impl_IBus(int busLevel) :
		_level(busLevel), _count(0), _bus(NULL) {
	}
	~Impl_IBus() {
		assert((_bus == NULL)&& "Impl_IBus::~Impl_IBus >> should has been unplugged from hub!");
		assert((_count == 0) && "Impl_IBus::~Impl_IBus >> non-zero count!");

		for (std::vector<IInterfaceEx*>::reverse_iterator it = _intfs.rbegin(); it
				!= _intfs.rend(); ++it) {
			IInterfaceEx* intf = *it;
			intf->setBus(NULL);
			intf->unref();
		}
		for (std::vector<IBus*>::reverse_iterator it = _buses.rbegin(); it
				!= _buses.rend(); ++it) {
			IBus* bus = *it;
			bus->setBus(NULL);
			bus->unref();
		}
	}
	//IHub
	virtual bool connect(IInterfaceEx* intf)  {
		IBus* bus;
		if (0 == intf->queryInterface(IID_IBUS, (void**) &bus, NULL)) {
			if (bus->getLevel() <= _level) {
				_buses.push_back(bus); //queryInterace already ref it.
				bus->setBus(this);
				return true;
			} else {
				//bus level mismatch, connection fails.
				bus->unref(); //balance queryInterface
				return false;
			}
		} else {
			intf->ref();
			_intfs.push_back(intf);
			intf->setBus(this);
			return true;
		}
	}
	virtual void disconnect(IInterfaceEx* intf) {
		{//interfaces first
			std::vector<IInterfaceEx*>::iterator it = find(_intfs.begin(),
					_intfs.end(), intf);
			if (it != _intfs.end()) {
				_intfs.erase(it);
				intf->setBus(NULL);
				intf->unref();
				return;
			}
		}
		{//buses later
			std::vector<IBus*>::iterator it = find(_buses.begin(),
					_buses.end(), intf);
			if (it != _buses.end()) {
				_buses.erase(it);
				intf->setBus(NULL);
				intf->unref();
			}
		}
	}
	virtual int getLevel() {
		return _level;
	}
	virtual IBus* findFirstBusByLevel(int busLevel) {
		for(auto bus: _buses){
			if (bus->getLevel() == busLevel) {
				return bus;
			}
		}
		return NULL;
	}
	//IInterface
	virtual int localQueryInterface(TIntfId iid, void** retIntf, IQueryState* qst) {

		if (equalIID(iid, IID_IBUS) || equalIID(iid, IID_IINTERFACEEX) || equalIID(iid, IID_IINTERFACE)){
			*retIntf = (IInterface*) (this);
			this->ref();
			return 0;
		} else {
			if (qst) qst->addSearchedBus(this);

			{//scanning pure interfaces
				for(auto intf: _intfs){
					if (intf->localQueryInterface(iid, retIntf, qst) == 0) {
						return 0;
					}
				}
			}
			{//scanning connected buses
				for(auto bus: _buses){
					if (bus->getLevel() >= _level) {
						if ((NULL == qst) || !qst->isBusSearched(bus)) {
							if (bus->localQueryInterface(iid, retIntf, qst) == 0) {
								return 0;
							}
						}
					}
				}
			}

			return 1;
		}
	}
	virtual int queryInterface(TIntfId iid, void** retIntf, IQueryState* qst) {
		auto_ref<IQueryState> st(qst);
		if (qst == NULL) {
			st = new TQueryState();
		}
		if (0 == localQueryInterface(iid, retIntf, st.get()))
			return 0;
		if (_bus && !st->isBusSearched(_bus)) {
			return _bus->queryInterface(iid, retIntf, st.get());
		}
		return 1;
	}
	virtual void ref() {
		++_count;
	}
	virtual void unref() {
		if (--_count == 0) {
			delete this;
		}
	}
	virtual void unrefNoDelete() {
		--_count;
		assert(_count >= 0);
	}
	//IInterfaceEx
	virtual void setBus(IBus* bus) {
		_bus = bus;
	}
};


} // iw

