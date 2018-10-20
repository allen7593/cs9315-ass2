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
    Bits qsig= newBits(psigBits(q->rel));
    makePageSignature(q->rel, q->qstring, qsig);
    Bits slice = newBits(bitBits(q->pages));
    Count bsigPageOffset = 0;
    Count bsigItemOffset = 0;
    Page bsigPage;
    Count page = 0;
    for (int i = 0; i < psigBits(q->rel); ++i) {
        bsigPageOffset = i / maxBsigsPP(q->rel);
        bsigItemOffset = i % maxBsigsPP(q->rel);
        if(bitIsSet(qsig, i)) {
            page++;
            q->nsigpages++;
            q->nsigs++;
            bsigPage = getPage(bsigFile(q->rel), bsigPageOffset);
            getBits(bsigPage, bsigItemOffset, slice);
            andBits(q->pages, slice);
        }
    }

}

