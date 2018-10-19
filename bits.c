// bits.c ... functions on bit-strings
// part of SIMC signature files
// Bit-strings are arbitrarily long byte arrays
// Least significant bits (LSB) are in array[0]
// Most significant bits (MSB) are in array[nbytes-1]

// Written by John Shepherd, September 2018

#include <assert.h>
#include "defs.h"
#include "bits.h"
#include "page.h"

typedef struct _BitsRep {
    Count nbits;          // how many bits
    Count nbytes;          // how many bytes in array
    Byte bitstring[1];  // array of bytes to hold bits
    // actual array size is nbytes
} BitsRep;

// create a new Bits object

Bits newBits(int nbits) {
    Count nbytes = iceil(nbits, 8);
    Bits new = malloc(2 * sizeof(Count) + nbytes);
    new->nbits = nbits;
    new->nbytes = nbytes;
    for (int i = 0; i < new->nbytes; i++) {
        memset(&(new->bitstring[i]), 0, 1);
    }
    return new;
}

// release memory associated with a Bits object

void freeBits(Bits b) {
    if (b != NULL) {
        free(b);
    }
}

// check if the bit at position is 1

Bool bitIsSet(Bits b, int position) {
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);

    int chunck = position / 8;
    int offset = position % 8;
    return (Bool) ((b->bitstring[chunck]) & (1 << offset));
}

// check whether one Bits b1 is a subset of Bits b2

Bool isSubset(Bits b1, Bits b2) {
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    Bits tmp = copyBits(b1);
    andBits(b1, b2);
    Bool equal = equalBits(b1, b2);
    memcpy(b1, tmp, b1->nbytes);
    free(tmp);
    return equal; // remove this
}

Bool equalBits(Bits b1, Bits b2) {
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    for (int i = 0; i < b1->nbytes; ++i) {
        if (b1->bitstring[i] != b2->bitstring[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

// set the bit at position to 1

void setBit(Bits b, int position) {
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    int chunck = position / 8;
    int offset = position % 8;

    b->bitstring[chunck] |= (1 << offset);
}

// set all bits to 1

void setAllBits(Bits b) {
    assert(b != NULL);
    int i = 0;
    for (i = 0; i < b->nbits; i++) {
        setBit(b, i);
    }
}

// set the bit at position to 0

void unsetBit(Bits b, int position) {
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    int chunck = position / 8;
    int offset = position % 8;

    b->bitstring[chunck] &= ~(1 << offset);
}

// set all bits to 0

void unsetAllBits(Bits b) {
    assert(b != NULL);
    memset(&(b->bitstring[0]), 0, b->nbytes);
}

// bitwise AND ... b1 = b1 & b2

void andBits(Bits b1, Bits b2) {
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    for (int i = 0; i < b1->nbytes; i++) {
        b1->bitstring[i] &= b2->bitstring[i];
    }
}

// bitwise OR ... b1 = b1 | b2

void orBits(Bits b1, Bits b2) {
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    for (int i = 0; i < b1->nbytes; i++) {
        b1->bitstring[i] |= b2->bitstring[i];
    }
}

Bits copyBits(Bits b) {
    Bits new = newBits(b->nbits);
    orBits(new, b);
    return new;
}


// get a bit-string (of length b->nbytes)
// from specified position in Page buffer
// and place it in a BitsRep structure

void getBits(Page p, Offset pos, Bits b) {
    Byte *addr = addrInPage(p, pos, b->nbytes);
    memcpy(getSigString(b), addr, b->nbytes);
}

// copy the bit-string array in a BitsRep
// structure to specified position in Page buffer

void putBits(Page p, Offset pos, Bits b) {
    Byte *addr = addrInPage(p, pos, b->nbytes);
    memcpy(addr, getSigString(b), b->nbytes);
}

// show Bits on stdout
// display in order MSB to LSB
// do not append '\n'

void showBits(Bits b) {
    assert(b != NULL);
    //printf("(%d,%d)",b->nbits,b->nbytes);
    for (int i = b->nbytes - 1; i >= 0; i--) {
        for (int j = 7; j >= 0; j--) {
            Byte mask = (1 << j);
            if (b->bitstring[i] & mask)
                putchar('1');
            else
                putchar('0');
        }
    }
}

Count tsigBytes(Bits bits) { return bits->nbytes; }

Count bitBits(Bits bits) { return bits->nbits; }

Byte *getSigString(Bits bits) { return bits->bitstring; }
