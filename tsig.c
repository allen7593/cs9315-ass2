// tsig.c ... functions on Tuple Signatures (tsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"

// make a tuple signature

Bits makeTupleSig(Reln r, Tuple t) {
    assert(r != NULL && t != NULL);
    Count bitsPerAttr = codeBits(r);
    Bits tupleSig = newBits(tsigBits(r));
    char *token = strtok(t, ",");
    Word hashVal = 0;
    int position = 0;

    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != NULL) {
        hashVal = hash_any(token, strlen(token));
        srand(hashVal);
        for (int count = 0; count < bitsPerAttr; count++) {
            position = rand() % tsigBits(r);
            setBit(tupleSig, position);
        }
        token = strtok(NULL, ",");
    }
    return tupleSig;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q) {
    assert(q != NULL);
    //TODO

    setAllBits(q->pages); // remove this

    // The printf below is primarily for debugging
    // Remove it before submitting this function
    printf("Matched Pages:");
    showBits(q->pages);
    putchar('\n');
}
