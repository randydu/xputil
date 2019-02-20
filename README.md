# Cross-Platform C++ Utilities

This project consists of some C++ source files that can be useful in developing C++ application on Windows, Linux and Mac.

## Interface and Bus

Interface based programming is very powerful, a group of functions are defined in an abstract interface which can be implementated in different modules.

Interface Bus (IBus) is a special interface that can connect interfaces and route interface from one to another.

Each bus has an integer bus-level, interfaces hosted on bus level-A can browse to interfaces hosted on bus level-B if level-B >= level-A, it is designed for security control, for example, at the bus level-0 we can host low-level interfaces such as ILicense, IUserCtl that can only be accessed at ring-0, at the bus level-3, however, we can host any public interfaces.

It is very useful for plug-in development, as an example, our app can load multiple optional plugins at runtime, the interface between the main app and the plugin being loaded is quite simple and flexible:

```c++

//declare the abstract interfaces in common header file
common/intf_translate.h:

INTERFACE ITranslate : public IInterface
{
    virtual const char* lang() const = 0; //lang-id
    virtual std::string translate(std::string input) = 0;
};

//Translator Management
INTERFACE ITranslatorMan : public IInterfaceEx
{
    virtual void add(ITranslate* translator) = 0;
    virtual std::string translate(std::string id, std::string input) = 0;
}

```

in the main app source code, we implement interface bus and ITranslatorMan interface:

```c++
// main.cpp

#include <xputil/Impl_intfs.h>
#include <common/intf_translate.h>

#include <vector>

using namespace xp;

class CTranslatorMan: public ITranslatorMan {
private:
    std::vector<ITranslate*> _translators;
public:
    virtual void add(ITranslate* translator) override {
        translator->ref();
        _translators.push_back(translator);
    }
    virtual std::string translate(std::string id, std::string input) override {
        for(auto& trans: _translators){
            if(id == trans.lang()){
                return trans.translate(input);
            }
        }
        throw "lang not supported";
    }
    ~CTranslatorMan(){
        for(auto& trans: _translators) trans->unref();
        _translators.clear();
    }
};


auto_ref<IBus> bus(new Impl_IBus(1));

void init_main(){
    bus.connect(new TInterfaceEx< COtherInterfaceA >()); //Impl interfaceA
    bus.connect(new TInterfaceEx< COtherInterfaceB >()); //Impl interfaceB
    ...
    bus.connect(new TIntefaceEx< CTranslatorMan >()); // Translator Management
}

void init_plugins(){
    for(auto& plugin: plugins){
        plugin.init(bus); //wire-in translator interfaces (or other interfaces) implemented in plugin.
    }
}

```


in the plugin source code, we implement some ITranslate interfaces and add them to the translator management interface (implemented in main app):

```c++

// plugin.cpp:

#include <xputil/Impl_intfs.h>
#include <common/intf_translate.h>

 class CTranslateSpanish : public ITranslate {
 public:

    virtual const char* lang() const override {
        return "SP";
    }

    virtual std::string translate(std::string input) override {
        std::string result;
        ... //translate input to spanish
        return result;
    }
 };

 class CTranslateCN : public ITranslate {
 public:

    virtual const char* lang() const override {
        return "CN";
    }

    virtual std::string translate(std::string input) override {
        std::string result;
        ... //translate input to Chinese
        return result;
    }
 };

//This is the only api exposed by plugin, called by main app when the plugin is being loaded at runtime
void plugin_init(xp::IBus* srv){
    using namespace xp;

    auto_ref<ITranslatorMan> man(srv); //route to ITranslatorMan interface

    man->add(new TInterface<CTranslateSpanish>());
    man->add(new TInterface<CTranslateCN>()));
}

```

If an app can be implemented in many small sub-modules, each module implements a subset of related interfaces, then the whole system will be easy to maintain, one module can upgrade without affecting other modules, there is no need to export multiple module-specific apis across the module boundary, all modules communicate via predefined interface.
