// test tsig dt

#include <stdio.h>
#include "defs.h"
#include "tuple.h"
#include "tsig.h"
#include "bits.h"
#include "reln.h"

int main(int argc, char **argv)
{
    Reln r = openRelation("R");
//	Tuple t = readTuple(r, stdin);
//	Bits b = makeTupleSig(r, t);
//	showBits(b); putchar('\n');
//	int n0, n1, tot; n0 = n1 = tot = 0;
//	for (int i = 0; i < tsigBits(r); i++) {
//		if (bitIsSet(b,i)) n1++; else n0++;
//		tot++;
//	}
//	printf("%d total, %d set, %d zero\n", tot, n1, n0);
    Page  p;
    Bits b = newBits(tsigBits(r));
    for (PageID i = 0; i < nTsigPages(r); ++i) {
        p = getPage(r->tsigf, i);
        for (int j = 0; j < pageNitems(p); ++j) {
            getBits(p, j, b);
            printf("%d: ", j); showBits(b); printf("\n");
        }
        free(p);
    }
    freeBits(b);
	return 0;
}
