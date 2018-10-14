// query.c ... query scan functions
// part of SIMC signature files
// Manage creating and using Query objects
// Written by John Shepherd, September 2018

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"
#include "tsig.h"
#include "psig.h"
#include "bsig.h"
#include "hash.h"

// check whether a query is valid for a relation
// e.g. same number of attributes

int checkQuery(Reln r, char *q) {
    if (*q == '\0') return 0;
    char *c;
    int nattr = 1;
    for (c = q; *c != '\0'; c++)
        if (*c == ',') nattr++;
    return (nattr == nAttrs(r));
}

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q, char sigs) {
    Query new = malloc(sizeof(QueryRep));
    assert(new != NULL);
    if (!checkQuery(r, q)) return NULL;
    new->rel = r;
    new->qstring = q;
    new->nsigs = new->nsigpages = 0;
    new->ntuples = new->ntuppages = new->nfalse = 0;
    new->pages = newBits(nPages(r));
    switch (sigs) {
        case 't':
            findPagesUsingTupSigs(new);
            break;
        case 'p':
            findPagesUsingPageSigs(new);
            break;
        case 'b':
            findPagesUsingBitSlices(new);
            break;
        default:
            setAllBits(new->pages);
            break;
    }
    new->curpage = 0;
    return new;
}

// scan through selected pages (q->pages)
// search for matching tuples and show each
// accumulate query stats

void scanAndDisplayMatchingTuples(Query q) {
    assert(q != NULL);
    Count nPages = nPages(q->rel);
    Page p;
    Tuple t;
    Bool match = FALSE;
    for (int i = 0; i < nPages; ++i) {
        if (bitIsSet(q->pages, i)) {
            q->ntuppages++;
            p = getPage(dataFile(q->rel), i);
            for (int j = 0; j < pageNitems(p); j++) {
                q->ntuples++;
                t = getTupleFromPage(q->rel, p, j);
                if (tupleEqualsToQuery(t, q)) {
                    showTuple(q->rel, t);
                    match = TRUE;
                }
                free(t);
            }
            if (!match) {
                q->nfalse++;
            }
            match = FALSE;
            free(p);
        }
    }
}

Bool tupleEqualsToQuery(Tuple t, Query q) {
    Count nAtrr = nAttrs(q->rel);
    char **tList = splitStr(t, nAtrr);
    char **qList = splitStr(q->qstring, nAtrr);
    for (int i = 0; i < nAtrr; ++i) {
        if (*qList[i] != '?' && strcmp(tList[i], qList[i]) != 0) {
            freeList(tList,nAtrr);
            freeList(qList,nAtrr);
            return FALSE;
        }
    }
    freeList(tList,nAtrr);
    freeList(qList,nAtrr);
    return TRUE;
}

void freeList(char **list, Count nAttrs) {
    for (int i = 0; i < nAttrs; ++i) {
        free(list[i]);
    }
    free(list);
}

char **splitStr(Tuple t, Count nAtrr) {
    Tuple tmp = malloc(strlen(t));
    strcpy(tmp, t);
    char **strList = malloc(sizeof(char *) * nAtrr);
    char *token = strtok(t, ",");
    int count = 0;
    while (count < nAtrr) {
        strList[count] = malloc(sizeof(char) * strlen(token));
        strcpy(strList[count], token);
        token = strtok(NULL, ",");
        count++;
    }
    strcpy(t, tmp);
    free(tmp);
    return strList;
}

// print statistics on query

void queryStats(Query q) {
    printf("# signatures read:   %d\n", q->nsigs);
    printf("# sig pages read:    %d\n", q->nsigpages);
    printf("# tuples examined:   %d\n", q->ntuples);
    printf("# data pages read:   %d\n", q->ntuppages);
    printf("# false match pages: %d\n", q->nfalse);
}

// clean up a QueryRep object and associated data

void closeQuery(Query q) {
    free(q->pages);
    free(q);
}

