// bsig.c ... functions on Tuple Signatures (bsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"

#include "psig.h"

void findPagesUsingBitSlices(Query q) {
    assert(q != NULL);;
    setAllBits(q->pages);
    Bits qsig = makePageSig(q->rel, q->qstring);
    Bits slice = newBits(bitBits(q->pages));
    Count itemOffset = 0;
    Count pageBase = psigBits(q->rel) / maxBsigsPP(q->rel);
    Count nBasePage = (nBsigPages(q->rel) / pageBase) - 1;
    Page bsigPage;
    if ((psigBits(q->rel) % maxBsigsPP(q->rel)) > 0) pageBase++;
    for (int i = 0; i < psigBits(q->rel); ++i) {
        if (bitIsSet(qsig, i)) {
            for (int j = 0; j < nBasePage; ++j) {
                itemOffset = i % maxBsigsPP(q->rel);
                for (Count l = 0; l < pageBase; ++l) {
                    bsigPage = getPage(bsigFile(q->rel), j * pageBase + l);
                    getBits(bsigPage, itemOffset, slice);
//                    for (int k = 0; k < bitBits(q->pages); ++k) {
//                        if (!bitIsSet(slice, k)) {
//                            unsetBit(q->pages, k);
//                        }
//                    }
                    orBits(q->pages, slice);
                    showBits(q->pages);printf("\n");
                    free(bsigPage);
                }
            }
        }
    }
}

