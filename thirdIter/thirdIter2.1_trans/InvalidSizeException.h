#ifndef INVALIDSIZEEXCEPTION_H
#define INVALIDSIZEEXCEPTION_H

#include <exception>


struct InvalidSizeException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Invalid size to HashTable constructor, has to be a power of two";
    }
};

#endif
