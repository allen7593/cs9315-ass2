// reln.c ... functions on Relations
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "reln.h"
#include "page.h"
#include "tuple.h"
#include "tsig.h"
#include "bits.h"
#include "hash.h"
#include "psig.h"
// open a file with a specified suffix
// - always open for both reading and writing

File openFile(char *name, char *suffix) {
    char fname[MAXFILENAME];
    sprintf(fname, "%s.%s", name, suffix);
    File f = open(fname, O_RDWR | O_CREAT, 0644);
    assert(f >= 0);
    return f;
}

// create a new relation (five files)
// data file has one empty data page

Status newRelation(char *name, Count nattrs, float pF,
                   Count tk, Count tm, Count pm, Count bm) {
    Reln r = malloc(sizeof(RelnRep));
    RelnParams *p = &(r->params);
    assert(r != NULL);
    p->nattrs = nattrs;
    p->pF = pF,
            p->tupsize = 28 + 7 * (nattrs - 2);
    Count available = (PAGESIZE - sizeof(Count));
    p->tupPP = available / p->tupsize;
    p->tk = tk;
    if (tm % 8 > 0) tm += 8 - (tm % 8); // round up to byte size
    p->tm = tm;
    p->tsigSize = tm / 8;
    p->tsigPP = available / (tm / 8);
    if (pm % 8 > 0) pm += 8 - (pm % 8); // round up to byte size
    p->pm = pm;
    p->psigSize = pm / 8;
    p->psigPP = available / (pm / 8);
    if (p->psigPP < 2) {
        free(r);
        return -1;
    }
    if (bm % 8 > 0) bm += 8 - (bm % 8); // round up to byte size
    p->bm = bm;
    p->bsigSize = bm / 8;
    p->bsigPP = available / (bm / 8);
    if (p->bsigPP < 2) {
        free(r);
        return -1;
    }
    r->infof = openFile(name, "info");
    r->dataf = openFile(name, "data");
    r->tsigf = openFile(name, "tsig");
    r->psigf = openFile(name, "psig");
    // printf("r->psigf: %d\n", r->psigf);
    r->bsigf = openFile(name, "bsig");
    // printf("r->bsigf: %d\n", r->bsigf);

    addPage(r->dataf);
    p->npages = 1;
    p->ntups = 0;
    addPage(r->tsigf);
    p->tsigNpages = 1;
    p->ntsigs = 0;
    addPage(r->psigf);
    p->psigNpages = 1;
    p->npsigs = 0;

    // by Lily
    // Create a file containing "pm" all-zeroes bit-strings,
    // each of which has length "bm" bits
    addBsigPage(r);
    closeRelation(r);
    return 0;
}

void addBsigPage(Reln r) {
    Count bsigPageNume = r->params.pm / maxBsigsPP(r);
    if ((r->params.pm % maxBsigsPP(r)) > 0) bsigPageNume++;

    for (int i = 0; i < bsigPageNume; ++i) {
        addPage(bsigFile(r));
    }
    r->params.bsigNpages += bsigPageNume;
    r->params.nbsigs = r->params.bsigNpages / bsigPageNume * psigBits(r);
}

// check whether a relation already exists

Bool existsRelation(char *name) {
    char fname[MAXFILENAME];
    sprintf(fname, "%s.info", name);
    File f = open(fname, O_RDONLY);
    if (f < 0)
        return FALSE;
    else {
        close(f);
        return TRUE;
    }
}

// set up a relation descriptor from relation name
// open files, reads information from rel.info

Reln openRelation(char *name) {
    Reln r = malloc(sizeof(RelnRep));
    assert(r != NULL);
    r->infof = openFile(name, "info");
    r->dataf = openFile(name, "data");
    r->tsigf = openFile(name, "tsig");
    r->psigf = openFile(name, "psig");
    r->bsigf = openFile(name, "bsig");
    read(r->infof, &(r->params), sizeof(RelnParams));
    return r;
}

// release files and descriptor for an open relation
// copy latest information to .info file
// note: we don't write ChoiceVector since it doesn't change

void closeRelation(Reln r) {
    // make sure updated global data is put in info file
    lseek(r->infof, 0, SEEK_SET);
    int n = write(r->infof, &(r->params), sizeof(RelnParams));
    assert(n == sizeof(RelnParams));
    close(r->infof);
    close(r->dataf);
    close(r->tsigf);
    close(r->psigf);
    close(r->bsigf);
    free(r);
}

// insert a new tuple into a relation
// returns page where inserted
// returns NO_PAGE if insert fails completely

PageID addToRelation(Reln r, Tuple t) {
    assert(r != NULL && t != NULL && strlen(t) == tupSize(r));
    Page p;
    PageID pid;
    RelnParams *rp = &(r->params);
    Bits tsig = NULL;
    Bits psig;

    // add tuple to last page
    pid = rp->npages - 1;
    p = getPage(r->dataf, pid);
    // check if room on last page; if not add new page
    if (pageNitems(p) == rp->tupPP) {
        addPage(r->dataf);
        rp->npages++;
        pid++;
        free(p);
        p = newPage();
        if (p == NULL) return NO_PAGE;
    }
    addTupleToPage(r, p, t);
    rp->ntups++;  //written to disk in closeRelation()
    putPage(r->dataf, pid, p);

    // compute tuple signature and add to tsigf

    tsig = makeTupleSig(r, t);
    addToTSig(r, tsig);

    // compute page signature and add to psigf

    psig = makePageSig(r, t);
//    showBits(psig);printf("\n");
    // use page signature to update bit-slices

    //by Lily
    Bits slice = newBits(bsigBits(r));
    Count nPage = nPsigs(r) - 1;
    Count bsigPageOffset = 0;
    Count bsigItemOffset = 0;
    Page currentBsig;
    for (int i = 0; i < psigBits(r); ++i) {
        bsigPageOffset = i / maxBsigsPP(r);
        bsigItemOffset = i % maxBsigsPP(r);
        if (bitIsSet(psig, i)) {
            currentBsig = getPage(bsigFile(r), bsigPageOffset);
            getBits(currentBsig, bsigItemOffset, slice);
            setBit(slice, nPage);
            putBits(currentBsig, bsigItemOffset, slice);
            putPage(bsigFile(r), bsigPageOffset, currentBsig);
        }
    }

    return nPages(r) - 1;
}

PageID addToTSig(Reln r, Bits sig) {
    Page p;
    PageID pid;
    pid = nTsigPages(r) - 1;
    p = getPage(tsigFile(r), pid);
    if (pageNitems(p) == maxTsigsPP(r)) {
        addPage(tsigFile(r));
        nTsigPages(r)++;
        pid++;
        free(p);
        p = newPage();
        if (p == NULL) return NO_PAGE;
    }
    addSigToPage(r, p, sig);
    nTsigs(r)++;
    putPage(tsigFile(r), pid, p);
    freeBits(sig);
    return nTsigPages(r) - 1;
}

Status addSigToPage(Reln r, Page p, Bits sig) {
    if (pageNitems(p) == maxTsigsPP(r)) return NOT_OK;
    int size = tsigBytes(sig);
    Byte *addr = addrInPage(p, pageNitems(p), size);
    memcpy(addr, getSigString(sig), size);
    addOneItem(p);
    return OK;
}

// displays info about open Reln (for debugging)

void relationStats(Reln r) {
    RelnParams *p = &(r->params);
    printf("Global Info:\n");
    printf("Dynamic:\n");
    printf("  #items:  tuples: %d  tsigs: %d  psigs: %d  bsigs: %d\n",
           p->ntups, p->ntsigs, p->npsigs, p->nbsigs);
    printf("  #pages:  tuples: %d  tsigs: %d  psigs: %d  bsigs: %d\n",
           p->npages, p->tsigNpages, p->psigNpages, p->bsigNpages);
    printf("Static:\n");
    printf("  tups   #attrs: %d  size: %d bytes  max/page: %d\n",
           p->nattrs, p->tupsize, p->tupPP);
    printf("  sigs   bits/attr: %d\n", p->tk);
    printf("  tsigs  size: %d bits (%d bytes)  max/page: %d\n",
           p->tm, p->tsigSize, p->tsigPP);
    printf("  psigs  size: %d bits (%d bytes)  max/page: %d\n",
           p->pm, p->psigSize, p->psigPP);
    printf("  bsigs  size: %d bits (%d bytes)  max/page: %d\n",
           p->bm, p->bsigSize, p->bsigPP);
}
