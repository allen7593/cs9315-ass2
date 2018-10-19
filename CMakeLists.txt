cmake_minimum_required(VERSION 3.12)
project(cs9315 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(cs9315
        bits.c
        bits.h
        bsig.c
        bsig.h
        chvec.c
        chvec.h
#        create.c
        defs.h
#        dump.c
#        gendata.c
        hash.c
        hash.h
        insert.c
        page.c
        page.h
#        pages.c
        psig.c
        psig.h
        query.c
        query.h
        reln.c
        reln.h
#        select.c
#        stats.c
        tsig.c
        tsig.h
        tuple.c
        tuple.h
        util.c
        util.h
#        x1.c
#        x2.c
#        x3.c
        )
