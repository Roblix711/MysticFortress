/* =====================

    Nova: Modular 

===================== */

#ifndef _NOVA_MODULAR_H
#define _NOVA_MODULAR_H

// nova headers
#include "Nova.h"

// class to hide modular arithmetic 
template <typename TYPE>
class NOVA::MODULUS
{
    public:
    TYPE Value;
    TYPE Mod;

    // constructor takes care of the modulus data
    MODULUS(TYPE InitValue, TYPE InitMod): Value(InitValue), Mod(InitMod){}

    // this is a cast to our base type -> this should get our original value
    template <class TYPE>
    inline operator TYPE()
    {
        return Value; 
    }

    // overload the increment to hide the modulus: note, the second parameter must be exactly int always
    TYPE operator++(int)
    {
        TYPE Temp = Value;
        Value++;
        Value %= Mod; 
        // NOTE: if you're getting a divide by 0 error here: get rid of static global initialization classes
        // The order of initializtions is not gaurenteed and thus if a class uses another uninitialized class, whos 
        // member contains a MODULAR, then it will not have gone through the constructor, and its Mod value is undef
        // with a good chance of being 0 -> causing a div/0 error...
        return Temp;
    }
    
    // overload the decrement operator to hide modulus
    TYPE operator--(int)
    {
        TYPE Temp = Value;
        Value += Mod - 1; // mod trick to avoid -VE casting / branching
        Value %= Mod; 
        // NOTE: if you're getting a divide by 0 error here: get rid of static global initialization classes
        // The order of initializtions is not gaurenteed and thus if a class uses another uninitialized class, whos 
        // member contains a MODULAR, then it will not have gone through the constructor, and its Mod value is undef
        // with a good chance of being 0 -> causing a div/0 error...
        return Temp;
    }

	// operation between integers
	int operator+(int x)
	{
        return (Value + x) % Mod;
	}
	// operation between integers
	int operator-(int x)
	{
		int New = Value;
		New -= x;
		while (New < 0) New += Mod;
        return New;
	}
};





#endif
