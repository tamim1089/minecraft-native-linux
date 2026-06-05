#pragma once

class FoliageColor
{
	// We don't want to use this any more
//private:
//	static intArray pixels;
//public:
//	static void init(intArray pixels);
//    static int get(double temp, double rain);

public:
	static int getEvergreenColor();
	static int getBirchColor();
	static int getDefaultColor();
};