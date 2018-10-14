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

    makePageSignature(r, t, psig);
    nPsigs(r) = pageNum + 1;
    if (nTuples(r) % maxTupsPP(r) == 1)
        addOneItem(psigPage);
    putBits(psigPage, pSigPageOffSet, psig);
    putPage(psigFile(r), nPsigPages(r) - 1, psigPage);

    return psig;
}

void makePageSignature(Reln r, Tuple t, Bits psig) {
    Count bitsPerAttr = codeBits(r);
    Tuple tmp = malloc(sizeof(char) * strlen(t));
    strcpy(tmp, t);
    char *token = strtok(t, ",");
    Word hashVal = 0;
    int position = 0;

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
    free(tmp);
}

void findPagesUsingPageSigs(Query q) {
    assert(q != NULL);
    Bits qsig = newBits(psigBits(q->rel));
    Bits psig = newBits(psigBits(q->rel));;
    makePageSignature(q->rel, q->qstring, qsig);
    Count numPsigPages = nPsigPages(q->rel);
    Page sigPage;
    Count pageNItem;
    Count pageNum = 0;
    for (PageID i = 0; i < numPsigPages; ++i) {
        q->nsigpages++;
        sigPage = getPage(psigFile(q->rel), i);
        pageNItem = pageNitems(sigPage);
        for (Offset j = 0; j < pageNItem; ++j) {
            q->nsigs++;
            getBits(sigPage, j, psig);
            if (isSubset(psig, qsig)) {
                pageNum = (i * maxPsigsPP(q->rel)) + j;
                setBit(q->pages, pageNum);
            }
        }
        free(sigPage);
    }
    freeBits(qsig);
    freeBits(psig);
}

