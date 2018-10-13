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
    Tuple tmp = malloc(sizeof(char) * strlen(t));
    strcpy(tmp, t);
    char *token = strtok(t, ",");
    Word hashVal = 0;
    int position = 0;

    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != NULL) {
        if (strcmp(token, "?") != 0) {
            hashVal = hash_any(token, strlen(token));
            srand(hashVal);
            for (int count = 0; count < bitsPerAttr; count++) {
                position = rand() % tsigBits(r);
                setBit(tupleSig, position);
            }
        }
        token = strtok(NULL, ",");
    }
    strcpy(t, tmp);
    return tupleSig;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q) {
    assert(q != NULL);
    Bits qsig = makeTupleSig(q->rel, q->qstring);
    Count maxTuplePP = maxTupsPP(q->rel);
    Count maxSigPP = maxTsigsPP(q->rel);
    Page page;
    PageID pid = 0;
    Bits tsig;
    for (int i = 0; i < nTsigPages(q->rel); ++i) {
        q->nsigpages++;
        page = getPage(q->rel->tsigf, i);
        for (int j = 0; j < pageNitems(page); ++j) {
            q->nsigs++;
            tsig = newBits(tsigBits(q->rel));
            getBits(page, j, tsig);
            if (isSubset(tsig, qsig)) {
                q->ntuples++;
                pid = (j + i * maxSigPP) / maxTuplePP;
                setBit(q->pages, pid);
            }
            freeBits(tsig);
        }
        free(page);
    }
    free(qsig);
}
