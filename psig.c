// psig.c ... functions on page signatures (psig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "hash.h"

Bits makePageSig(Reln r, Tuple t) {
    assert(r != NULL && t != NULL);
    Count pPageNum = nPsigPages(r) - 1;
    Page psigPage = getPage(psigFile(r), pPageNum);
    Count pageNum = nPages(r) - 1;
    Count pSigPageNum = pageNum / maxPsigsPP(r);
    Count pSigPageOffSet = pageNum % maxPsigsPP(r);
    Bits psig = newBits(psigBits(r));

    if (pSigPageNum > pPageNum) {
        addPage(psigFile(r));
        nPsigPages(r)++;
        free(psigPage);
        psigPage = newPage();
        if (psigPage == NULL) return NULL;
    }
    getBits(psigPage, pSigPageOffSet, psig);

    Count bitsPerAttr = codeBits(r);
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
                position = rand() % psigBits(r);
                setBit(psig, position);
            }
        }
        token = strtok(NULL, ",");
    }
    strcpy(t, tmp);
    nPsigs(r) = pageNum + 1;
    addOneItem(psigPage);
    putBits(psigPage, pSigPageOffSet, psig);
    addOneItem(psigPage);
    putPage(psigFile(r), nPsigPages(r) - 1, psigPage);

    return psig;
}

void findPagesUsingPageSigs(Query q) {
    assert(q != NULL);
    //TODO
    setAllBits(q->pages); // remove this
}

