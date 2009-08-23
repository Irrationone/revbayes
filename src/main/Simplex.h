#ifndef Simplex_H
#define Simplex_H

#include <vector>
#include "RbDataType.h"

class Simplex : public RbDataType {

public:
	Simplex(int n);
	Simplex(const Simplex &s);
	Simplex(std::vector<double> &s);
	Simplex* copy(void) const;
	double operator[](int i) { return value[i]; } 
	void print(void) const;
	int size(void) { return value.size(); }
	bool store(void) { return true; }
	bool restore(void) { return true; }

private:
	std::vector<double> value;
};

#endif