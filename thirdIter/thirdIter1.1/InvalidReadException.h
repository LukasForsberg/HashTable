#ifndef INVALIDREADEXCEPTION_H
#define INVALIDREADEXCEPTION_H

#include <exception>


struct InvalidReadExeption : public std::exception
{
	const char * what () const throw ()
    {
    	return "Invalid read, Key dosen't exist";
    }
};

#endif
