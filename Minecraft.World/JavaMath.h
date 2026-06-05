#pragma once
#include "Random.h"

class Math
{
private:
	static Random rand;

public:
	static double random();
#ifdef _LINUX
	// Headless port: allow seeding the global RNG deterministically (it is QPC-seeded by default).
	static void setRandomSeed(__int64 s) { rand.setSeed(s); }
#endif
	static __int64 round( double d );
	static int _max(int a, int b);
	static float _max(float a, float b);
	static int _min(int a, int b);
	static float _min(float a, float b);

	static float wrapDegrees(float input);
	static double wrapDegrees(double input);
};