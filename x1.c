// test Bits dt

#include <stdio.h>
#include "defs.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"

int main(int argc, char **argv)
{
	Bits b = newBits(60);
	printf("t=0: "); showBits(b); printf("\n");
	setBit(b, 5);
	printf("t=1: "); showBits(b); printf("\n");
	setBit(b, 0);
	setBit(b, 50);
	setBit(b, 59);
	printf("t=2: "); showBits(b); printf("\n");
	if (bitIsSet(b,5)) printf("Bit 5 is set\n");
	if (bitIsSet(b,10)) printf("Bit 10 is set\n");
	Bits a = copyBits(b);
	printf("t=3: "); showBits(a); printf("\n");
	setAllBits(b);
	printf("t=4: "); showBits(b); printf("\n");
	andBits(a ,b);
	printf("t=5: "); showBits(a); printf("\n");
	printf("t=6: "); showBits(b); printf("\n");
	unsetBit(b, 40);
	printf("t=7: "); showBits(b); printf("\n");
	if (bitIsSet(b,20)) printf("Bit 20 is set\n");
	if (bitIsSet(b,40)) printf("Bit 40 is set\n");
	setBit(b, 59);
	unsetAllBits(b);
	printf("t=8: "); showBits(b); printf("\n");
	printf("t=9: "); showBits(a); printf("\n");

    if(equalBits(a, b)) printf("A and B are equal");
    if(isSubset(a,b)) printf("B is the sebset of A");

	freeBits(a);
	freeBits(b);
	return 0;
}
