#pragma once
#include <random>
#include "obse_headers.h"

class OblivionRng {
private:
	std::mt19937 generator;
	bool seeded;

public:
	OblivionRng();

	OblivionRng(UInt32 seed); //initialize with a fixed seed

	UInt32 rng(UInt32 min, UInt32 max); //returns a pseudorandom number within [min, max]

	UInt32 operator()(UInt32 min, UInt32 max);

	bool seed(); // random seed

	bool seed(UInt32 seed); //fixed seed
};