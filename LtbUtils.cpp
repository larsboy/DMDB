/*
* LtbUtils.cpp
* Implementation of the LtbUtils namespace
*/

#include <stdlib.h>
#include <time.h>
#include <cmath> //For roundoff functions
#include "LtbUtils.h"

//Convert an int to a string representation
string LtbUtils::intToString(int number)
{
	//Get zero out of the way first:
	if (number==0) return "0";

	int digit(0);
	string result="";
	string temp="";
	
	if (number<0)
	{
		number*=-1;
		result="-";
	}

	while (number>0)
	{
		digit=number%10;
		temp+=CHAR_DIGITS[digit];
		number/=10;
	}
	for (int i=temp.size()-1; i>=0; i--)
		result+=temp[i];

	return result;
}

//Convert a double to a string representation, including roundoff
string LtbUtils::doubleToString(double number, int decimals)
{
	int digit(0);
	string fracPart="";
	string result="";

	//Maximum ten decimals:
	int decimalTab[]={0,0,0,0,0,0,0,0,0,0,0}; //One extra for roundoff
	if (decimals>10) decimals=10;
	
	if (number<0.0)
	{
		number*=-1.0;
		result="-";
	}
	int integerPart=int(number);
	double decimalPart=number-integerPart;
	int overflow=0;
	
	//Make the decimal part:
	if (decimalPart>0.0)
	{
		int decimalCount=0;
		while ( (decimalPart>0.0) && (decimalCount<11) )
		{
			decimalPart*=10.0;
			digit=int(decimalPart);
			decimalTab[decimalCount]=digit;
			decimalPart-=digit;
			decimalCount++;
		}
		//Round off:
		for (decimalCount=10; decimalCount>=decimals; decimalCount--)
		{
			if (decimalTab[decimalCount]>=5)
				if (decimalCount>0) decimalTab[decimalCount-1]++;
				else overflow=1;
		}
		for (decimalCount=decimals-1; decimalCount>=0; decimalCount--)
			if (decimalTab[decimalCount]>9)
			{
				decimalTab[decimalCount]=0;
				if (decimalCount>0) decimalTab[decimalCount-1]++;
				else overflow=1;
			}
		//Don't use trailing zeros:
		while ( (decimals>0) && (decimalTab[decimals-1]==0) )
			decimals--;
		//Make string:
		for (decimalCount=0; decimalCount<decimals; decimalCount++)
			fracPart+=CHAR_DIGITS[decimalTab[decimalCount]];
	}

	//Make the integer part:
	integerPart+=overflow;
	if (integerPart>0)
	{
		string temp="";
		while (integerPart>0)
		{
			digit=integerPart%10;
			temp+=CHAR_DIGITS[digit];
			integerPart/=10;
		}
		for (int i=temp.size()-1; i>=0; i--)
			result+=temp[i];
	}
	else result+="0";

	if ((decimals>0) && (fracPart!="")) result=result+"."+fracPart;
	if (result=="-0") result="0";
	return result;
}

//Convert a double to an int with correct roundoff
int LtbUtils::roundDoubleToInt(double number)
{
	int x=0;
	
	if (number>=0.0)
	{
		double frac = number - std::floor(number);
		x = (frac<0.5) ? int(number) : int(number)+1;
	}
	else
	{
		double frac = number - std::ceil(number);
		x = (frac>-0.5) ? int(number) : int(number)-1; //>=?
	}
	
	return x;
}

//************************ LtbRandom ************************

//The constructor takes a bool to decide if it is to be randomized
LtbUtils::LtbRandom::LtbRandom(bool seedIt)
{
	clock_t clck=clock(); //Always zero first time (?)
	if (seedIt) //Get seed from system clock
	{
		time_t tm2;
		tm2=time(0);
		srand(tm2%999);
	}
}

//Randomizes the random number generator
int LtbUtils::LtbRandom::seed()
{
	//Use time and clock functions to make a random seed:
	time_t tm2;
	tm2=time(0);
	clock_t clck=clock();
	int rSeed = ( tm2%999 ) * ( (500+clck)%999 );
	//Seed the random number generator of the standard library
	srand(rSeed);
	//Returns the seed value, for debugging
	return rSeed;
}

//Provides a random int from zero and up to, but not including, max
int LtbUtils::LtbRandom::uniformInt(int max)
{
	if (max<1) return 0;
	double basic=(double(rand())/RAND_MAX);
	//Can rand() be equal to RAND_MAX?
	int returnVal=int(basic*max);
	//Better make sure its not equal to max
	return (returnVal==max)? max-1: returnVal;
}

//Provides a random double between zero and max, default max is 1
double LtbUtils::LtbRandom::uniformDouble(double max)
{
	if (max<0.0) return 0.0;
	double basic=(double(rand())/RAND_MAX);
	//Can rand() be equal to RAND_MAX?
	return basic*max;
}
