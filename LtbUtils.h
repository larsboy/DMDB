/*!
* \file LtbUtils.h
* \author Lars Thomas Boye
*
* My useful functions, defined in the namespace LtbUtils:
* - Functions for converting numbers to strings
* - Function for rounding off double to int
* - A class for generating random numbers
*/

#ifndef LTBUTILS_H
#define LTBUTILS_H

#include <string>

using std::string;
//Not using std:: in LtbRandom-functions?

namespace LtbUtils
{
	const char CHAR_DIGITS[]={'0','1','2','3','4','5','6','7','8','9'};
	
	/*!
	* string intToString(int)
	* Converts an int to a string representation.
	*/
	string intToString(int number);

	/*!
	* string doubleToString(double,int)
	* Converts a double to a string representation.
	* You can specify the maximum number of decimals, up to 10.
	* It includes correct roundoff.
	*/
	string doubleToString(double number, int decimals=5);
	
	/*!
	* int roundDoubleToInt(double) converts a double to an int,
	* with correct roundoff to nearest int.
	*/
	int roundDoubleToInt(double number);
	
	/*!
	* LtbRandom is a class for generating random numbers of various types.
	* It is a shell for the standard library rand-function, that provides
	* easy access to random numbers of different types, and that takes care
	* of seeding the random number generator to provide truly random numbers.
	*/
	class LtbRandom
	{
		public:
		//! The constructor takes a bool to decide if it is to be randomized
		LtbRandom(bool seedIt=false);

		//! Randomizes the random number generator
		int seed();

		//! Provides a random int from zero and up to, but not including, max
		int uniformInt(int max);

		//! Provides a random double between zero and max, default max is 1
		double uniformDouble(double max=1.0);

		//Die-functions give a die roll, an int from 1 to the number of the
		//function name. So die6() gives a number from 1 to 6.
		int die4() {return 1+uniformInt(4);} //!< 1-4
		int die6() {return 1+uniformInt(6);} //!< 1-6
		int die8() {return 1+uniformInt(8);} //!< 1-8
		int die10() {return 1+uniformInt(10);} //!< 1-10
		int die12() {return 1+uniformInt(12);} //!< 1-12
		int die20() {return 1+uniformInt(20);} //!< 1-20
		int die100() {return 1+uniformInt(100);} //!< 1-100
	};
}

#endif
