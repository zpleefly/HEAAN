#include "SchemeAlgo.h"

#include <cmath>
#include <iostream>
#include <vector>

#include "../czz/CZZ.h"
#include "Params.h"

Cipher SchemeAlgo::powerOf2(Cipher& cipher, const long& logDegree) {
	Cipher res = cipher;
	for (long i = 0; i < logDegree; ++i) {
		scheme.squareAndEqual(res);
		scheme.modSwitchAndEqual(res);
	}
	return res;
}

void SchemeAlgo::powerOf2Extended(vector<Cipher>& res, Cipher& cipher, const long& logDegree) {
	res.reserve(logDegree + 1);
	Cipher cpow = cipher;
	res.push_back(cpow);
	for (long i = 1; i < logDegree + 1; ++i) {
		scheme.squareAndEqual(cpow);
		scheme.modSwitchAndEqual(cpow);
		res.push_back(cpow);
	}
}

//-----------------------------------------

Cipher SchemeAlgo::power(Cipher& cipher, const long& degree) {
	long logDegree = log2(degree);
	long po2Degree = 1 << logDegree;

	Cipher res = powerOf2(cipher, logDegree);
	long remDegree = degree - po2Degree;
	if(remDegree > 0) {
		Cipher tmp = power(cipher, remDegree);
		scheme.modEmbedAndEqual(tmp, res.level);
		scheme.multModSwitchAndEqual(res, tmp);
	}
	return res;
}

void SchemeAlgo::powerExtended(vector<Cipher>& res, Cipher& cipher, const long& degree) {
	res.reserve(degree);
	long logDegree = log2(degree);
	vector<Cipher> cpows;
	powerOf2Extended(cpows, cipher, logDegree);
	for (long i = 0; i < logDegree; ++i) {
		long powi = (1 << i);
		res.push_back(cpows[i]);
		for (int j = 0; j < powi-1; ++j) {
			Cipher tmp = scheme.modEmbed(res[j], cpows[i].level);
			scheme.multModSwitchAndEqual(tmp, cpows[i]);
			res.push_back(tmp);
		}
	}
	res.push_back(cpows[logDegree]);
	long degree2 = (1 << logDegree);
	for (int i = 0; i < (degree - degree2); ++i) {
		Cipher tmp = scheme.modEmbed(res[i], cpows[logDegree].level);
		scheme.multModSwitchAndEqual(tmp, cpows[logDegree]);
		res.push_back(tmp);
	}
}

//-----------------------------------------

Cipher SchemeAlgo::prod2(vector<Cipher>& ciphers, const long& logDegree) {
	vector<Cipher> res = ciphers;
	for (long i = logDegree; i > 0; --i) {
		vector<Cipher> cprodvec;
		long powi = (1 << i);
		cprodvec.reserve(powi / 2);
		for (long j = 0; j < powi; j = j + 2) {
			Cipher cprod = scheme.mult(res[j], res[j + 1]);
			scheme.modSwitchAndEqual(cprod);
			cprodvec.push_back(cprod);
		}
		res = cprodvec;
	}
	return res[0];
}

void SchemeAlgo::prod2Extended(vector<vector<Cipher>>& res, vector<Cipher>& ciphers, const long& logDegree) {
	res.reserve(logDegree + 1);
	res.push_back(ciphers);
	for (long i = 0; i < logDegree; ++i) {
		vector<Cipher> cprodvec;
		long size = res[i].size();
		cprodvec.reserve(size / 2);
		for (long j = 0; j < size; j = j + 2) {
			Cipher cprod = scheme.mult(res[i][j], res[i][j + 1]);
			scheme.modSwitchAndEqual(cprod);
			cprodvec.push_back(cprod);
		}
		res.push_back(cprodvec);
	}
}

//-----------------------------------------

Cipher SchemeAlgo::inverse(Cipher& cipher, const long& steps) {
	Cipher cpow = cipher;
	Cipher tmp = scheme.addConst(cipher, scheme.params.p);
	scheme.modEmbedAndEqual(tmp);
	Cipher res = tmp;

	for (long i = 0; i < steps - 1; ++i) {
		scheme.squareAndEqual(cpow);
		scheme.modSwitchAndEqual(cpow);
		tmp = cpow;
		scheme.addConstAndEqual(tmp, scheme.params.p);
		scheme.multAndEqual(tmp, res);
		scheme.modSwitchAndEqual(tmp, i + 3);
		res = tmp;
	}
	return res;
}

void SchemeAlgo::inverseExtended(vector<Cipher>& res, Cipher& cipher, const long& steps) {
	res.reserve(steps);
	Cipher cpow = cipher;
	Cipher tmp = scheme.addConst(cipher, scheme.params.p);
	scheme.modEmbedAndEqual(tmp);
	res.push_back(tmp);

	for (long i = 0; i < steps - 1; ++i) {
		scheme.squareAndEqual(cpow);
		scheme.modSwitchAndEqual(cpow);
		tmp = cpow;
		scheme.addConstAndEqual(tmp, scheme.params.p);
		scheme.multAndEqual(tmp, res[i]);
		scheme.modSwitchAndEqual(tmp, i + 3);
		res.push_back(tmp);
	}
}

//-----------------------------------------

Cipher SchemeAlgo::function(Cipher& cipher, string& funcName, const long& degree) {
	vector<Cipher> cpows;
	powerExtended(cpows, cipher, degree);

	vector<ZZ> pows = scheme.params.taylorPows.powsMap.at(funcName);
	vector<double> coeffs = scheme.params.taylorPows.coeffsMap.at(funcName);

	Cipher res = scheme.multByConst(cpows[0], pows[1]);
	ZZ a0 = scheme.params.p * pows[0];
	scheme.addConstAndEqual(res, a0);


	for (int i = 1; i < degree; ++i) {
		if(abs(coeffs[i + 1]) > 1e-17) {
			Cipher aixi = scheme.multByConst(cpows[i], pows[i + 1]);
			scheme.modEmbedAndEqual(res, aixi.level);
			scheme.addAndEqual(res, aixi);
		}
	}
	scheme.modSwitchAndEqual(res);
	return res;
}

Cipher SchemeAlgo::functionLazy(Cipher& cipher, string& funcName, const long& degree) {
	vector<Cipher> cpows;
	powerExtended(cpows, cipher, degree);

	vector<ZZ> pows = scheme.params.taylorPows.powsMap.at(funcName);
	vector<double> coeffs = scheme.params.taylorPows.coeffsMap.at(funcName);

	Cipher res = scheme.multByConst(cpows[0], pows[1]);
	ZZ a0 = scheme.params.p * pows[0];
	scheme.addConstAndEqual(res, a0);

	for (int i = 1; i < degree; ++i) {
		if(abs(coeffs[i + 1]) > 1e-27) {
			Cipher aixi = scheme.multByConst(cpows[i], pows[i + 1]);
			scheme.modEmbedAndEqual(res, aixi.level);
			scheme.addAndEqual(res, aixi);
		}
	}
	return res;
}

void SchemeAlgo::functionExtended(vector<Cipher>& res, Cipher& cipher, string& funcName, const long& degree) {
	vector<Cipher> cpows;
	powerExtended(cpows, cipher, degree);

	vector<ZZ> pows = scheme.params.taylorPows.powsMap.at(funcName);
	vector<double> coeffs = scheme.params.taylorPows.coeffsMap.at(funcName);

	Cipher aixi = scheme.multByConst(cpows[0], pows[1]);
	ZZ a0 = scheme.params.p * pows[0];
	scheme.addConstAndEqual(aixi, a0);
	res.push_back(aixi);

	long idx = 0;
	for (long i = 1; i < degree; ++i) {
		if(abs(coeffs[i + 1]) > 1e-17) {
			aixi = scheme.multByConst(cpows[i], pows[i + 1]);
			Cipher tmp = scheme.modEmbed(res[idx++], aixi.level);
			scheme.addAndEqual(aixi, tmp);
			res.push_back(aixi);
		}
	}
	for (long i = 0; i < res.size(); ++i) {
		scheme.modSwitchAndEqual(res[i]);
	}
}

//-----------------------------------------

vector<Cipher> SchemeAlgo::fftRaw(vector<Cipher>& ciphers, const bool& isForward) {
	long csize = ciphers.size();

	if(csize == 1) {
		return ciphers;
	}

	long csizeh = csize >> 1;

	vector<Cipher> res, sub1, sub2;

	res.reserve(csize);
	sub1.reserve(csizeh);
	sub2.reserve(csizeh);

	for (long i = 0; i < csize; i = i+2) {
		sub1.push_back(ciphers[i]);
		sub2.push_back(ciphers[i+1]);
	}

	vector<Cipher> y1 = fftRaw(sub1, isForward);
	vector<Cipher> y2 = fftRaw(sub2, isForward);

	long shift = isForward ? (scheme.params.M / csize) : (scheme.params.M - scheme.params.M / csize);

	for (long i = 0; i < csizeh; ++i) {
		scheme.multByMonomialAndEqual(y2[i], shift * i);
		Cipher sum = y1[i];
		scheme.addAndEqual(sum, y2[i]);
		scheme.subAndEqual(y1[i], y2[i]);
		res.push_back(sum);
	}

	for (long i = 0; i < csizeh; ++i) {
		res.push_back(y1[i]);
	}
	return res;
}

vector<Cipher> SchemeAlgo::fft(vector<Cipher>& ciphers) {
	return fftRaw(ciphers, true);
}

vector<Cipher> SchemeAlgo::fftInv(vector<Cipher>& ciphers) {
	vector<Cipher> fftInv = fftRaw(ciphers, false);
	long N = fftInv.size();
	long logN = log2(N);
	long bits = scheme.params.logp - logN;
	for (long i = 0; i < N; ++i) {
		scheme.leftShiftAndEqual(fftInv[i], bits);
		scheme.modSwitchAndEqual(fftInv[i]);
	}
	return fftInv;
}

vector<Cipher> SchemeAlgo::fftInvLazy(vector<Cipher>& ciphers) {
	return fftRaw(ciphers, false);
}
