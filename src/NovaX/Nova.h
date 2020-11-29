
// header gaurd
#ifndef _NOVA_PRIVATE_H
#define _NOVA_PRIVATE_H

// main includes
#include <NovaX.hpp>

// readablity defines
#define FOREVER true
#define NOCODE
#define GLOBAL
#define NOARG
#define NOTHROW
#define MICROSOFT    // must redefine because public interface undefines it 

// list of classes in nova
namespace NOVA
{
    // all classes
    template <typename T> class MODULUS;
    template <class T> class SHARED;
    class TIMER;
    class THREAD;
    class THREAD_FORK;

    // exports
    class THREAD_IMPL;
    class TIMER_IMPL;
};


#endif // _NOVA_PRIVATE_H
