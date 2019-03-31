#include "O1SA_Arduino_Common.h"

char * Common::removeWhiteSpaces(const char * str)
{
  int i;
  int count = 0;
  char * t = new char[strlen(str)] ;
  for(i=0; i<strlen(str); i++)
  {
    t[i] = 0;
    if(str[i] != ' ')
    {
      t[i] = 1;
      count++;
    }
  }

  char * o = new char[count+1];

  int k = 0;
  for(i=0; i< strlen(str); i++)
  {
    if(t[i]==1)
    {
      o[k] = str[i];
      k++;
    }
  }
  o[k] = '\0';

  delete[] t;
  return o;
}

char ** Common::splitBy(const char * str, const char * sep, uint8_t * num_strings)
{
  bool b;
	char * t;
	t = new char[strlen(str)] ;

	// set to 1
	int i;
	for(i=0;i<strlen(str);i++)
	{
		t[i] = 1;
	}

	// find separator in array

	int k;
	int j;

	for(i=0;i<strlen(str);i++)
	{
		if(str[i] == sep[0])
		{
			k = 1;
			b = true;
			for(j=i+1;j<strlen(sep)+i; j++)
			{
				if(str[j] != sep[k])
				{
					b = false;
					break;
				}
				k++;
			}
			if(b)
			{
				for(k=i; k<j; k++)
				{
					// set to 0 if separator
					t[k] = 0;
				}
			}
		}
	}

	// get substrings
	char** a = new char*[ strlen(str) ];
	for (i = 0; i < strlen(str); i++)
	{
		a[i] = new char[strlen(str)];
	}

	uint8_t last = 1;
	uint16_t numSubString = 0;
	uint16_t current  =0;

	for(i=0;i<strlen(str);i++)
	{
		// new substring
		if(last == 0 && t[i] == 1)
		{
			a[numSubString][current] ='\0';
			current=0;
			numSubString++;
		}
		if(t[i]== 1)
		{
			a[numSubString][current] = str[i];
			current++;
		}
		last = t[i];
	}
	a[numSubString][current] ='\0';

	// check if stuff after lass separator

	 // cleanup output

	 char** o = new char*[ numSubString+1];

	 for(i=0;i<numSubString+1;i++)
	 {
		 uint8_t size = strlen((const char *)a[i]);
		 o[i] = new char[size];
		 char * temp = a[i];
		 memcpy( o[i], a[i], size+1 );
	 }

	 *num_strings = numSubString+1;

	// delete dynamic allocations

	 for(i = 0; i < strlen(str); i++)
	 {
		 delete a[i];
	 }
	 delete[] a;   // check for delete[] a;
	 delete[] t;

	 return o;
}
