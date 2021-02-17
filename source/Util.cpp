#include "ReVolt.h"
#include "Util.h"


/////////////////////////////////////////////////////////////////////
//
// GoodWrap: wrap a number within a range without assuming it is
// less than one "range" away from the range
//
/////////////////////////////////////////////////////////////////////
REAL GoodWrap(REAL *var, REAL min, REAL max)
{ 
	int n; 
	REAL diff, range; 
	if (*var < min) { 
		range = max - min; 
		diff = min - *var; 
		n = (int) (diff / range); 
		*var += range * (n + 1); 
	} 
	else if (*var > max) { 
		range = max - min; 
		diff = *var - max; 
		n = (int) (diff / range); 
		*var -= range * (n + 1); 
	} 
	return *var;
}

