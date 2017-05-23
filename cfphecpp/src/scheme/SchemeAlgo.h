#ifndef SCHEME_SCHEMEALGO_H_
#define SCHEME_SCHEMEALGO_H_

#include <iostream>

#include "Cipher.h"
#include "Scheme.h"

class SchemeAlgo {
public:
	Scheme& scheme;

	SchemeAlgo(Scheme& scheme) : scheme(scheme) {};

	//-----------------------------------------

	Cipher powerOf2(Cipher& cipher, const long& logDegree);
	Cipher* powerOf2Extended(Cipher& cipher, const long& logDegree);

	//-----------------------------------------

	Cipher power(Cipher& cipher, const long& degree);
	Cipher* powerExtended(Cipher& cipher, const long& degree);

	//-----------------------------------------

	Cipher prod2(Cipher*& ciphers, const long& logDegree);

	//-----------------------------------------

	Cipher inverse(Cipher& cipher, const long& steps);
	Cipher* inverseExtended(Cipher& cipher, const long& steps);

	//-----------------------------------------

	Cipher function(Cipher& cipher, string& funcName, const long& degree);
	Cipher functionLazy(Cipher& cipher, string& funcName, const long& degree);
	Cipher* functionExtended(Cipher& cipher, string& funcName, const long& degree);

	//-----------------------------------------

	Cipher* fftRaw(Cipher*& ciphers, const long& size, const bool& isForward);
	Cipher* fft(Cipher*& ciphers, const long& size);
	Cipher* fftInv(Cipher*& ciphers, const long& size);
	Cipher* fftInvLazy(Cipher*& ciphers, const long& size);

	//-----------------------------------------

};

#endif /* SCHEME_SCHEMEALGO_H_ */
