#ifndef STRING_INCLUDED
#define STRING_INCLUDED


#include "types.c"


Byte to_upper_case(Byte c)
{
	if(c >= 'a' && c <= 'z') {
		c = c + 'A' - 'a';
	}
	
	return c;
}


Byte to_lower_case(Byte c)
{
	if(c >= 'A' && c <= 'Z') {
		c = c + 'a' - 'A';
	}
	
	return c;
}


Signed_Number compare_strings(Byte* string1, Byte* string2)
{
	Signed_Number difference;

	while(*string1 && *string2) {
		difference = (Signed_Number)*string1 - (Signed_Number)*string2;

		if(difference) {
			return difference;
		}

		++string1;
		++string2;
	}

	return (Signed_Number)*string1 - (Signed_Number)*string2;
}


#endif//STRING_INCLUDED