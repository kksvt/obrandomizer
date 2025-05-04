#include "rng.h"

OblivionRng::OblivionRng(): seeded(false) { 
}

OblivionRng::OblivionRng(UInt32 seed): seeded(true) { //initialize with a fixed seed
	generator.seed(seed);
}

UInt32 OblivionRng::rng(UInt32 min, UInt32 max) { //returns a pseudorandom number within [min, max]
	if (!seeded) {
		_ERROR("RNG used before being initialized.");
		return 0;
	}
	std::uniform_int_distribution<UInt32> dist(min, max);
	return dist(generator);
}

UInt32 OblivionRng::operator()(UInt32 min, UInt32 max) {
	return this->rng(min, max);
}

bool OblivionRng::seed() {
	std::random_device rd;
	generator.seed(rd());
	seeded = true;
	return true;
}

bool OblivionRng::seed(UInt32 seed) {
	generator.seed(seed);
	seeded = true;
	return true;
}