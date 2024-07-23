/*
RdAvi2.exe - Copyright (c) 2024 by Dennis Hawkins. All rights reserved.
Inspired by: ReadAvi.exe by Michael Kohn<mike@mikekohn.net> (http://www.mikekohn.net/)

BSD License

Redistribution and use in source and binary forms are permitted provided
that the above copyright notice and this paragraph are duplicated in all
such forms and that any documentation, advertising materials, and other
materials related to such distribution and use acknowledge that the
software was developed by the copyright holder. The name of the copyright
holder may not be used to endorse or promote products derived from this
software without specific prior written permission.  THIS SOFTWARE IS
PROVIDED `'AS IS? AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.

Although not required, attribution is requested for any source code
used by others.
*/

#include "rdavi2.h"


// Convert a quad word to a 16 byte hex string.

char *QWORD2HEX(QWORD val)
{
    static char outstr[20];

//    return(_ui64toa(val, outstr, 16));   // bug in borland library - prints as long not i64.

    sprintf(outstr, "%08X%08X", *((DWORD *) &val + 1), *((DWORD *) &val + 0));

    return(outstr);
}

LONG read_long(FILE *in)
{
    int c;

    if (!in) return(-1);
    File64Read(in, &c, sizeof(LONG));

    return c;
}


// This function is only used if the compiler treats multi-character literals
// as BIG ENDIAN order.
// It reverses the order of a literal into Little Endian order and is called
// by FIX_LIT() macro


#if defined(BE_MC_LIT)

DWORD ReverseLiteral(DWORD val)
{
    BYTE out[4], *in = (BYTE *) &val;

    out[0] = in[3];
    out[1] = in[2];
    out[2] = in[1];
    out[3] = in[0];

    return(*(DWORD *) out);
}

#endif


// Read four chars from the file and convert to a Little Endian integer.
// If StreamNum is not NULL, and the chars are ##db, ##dc, ##wb, ##tx, or ix##,
// the stream number is returned in StreamNum.  Also, the FCC is converted
// to a standard form like ##dc instead of the original like 00dc or 01dc.
// This is so the program can compare against a consistant value.
// The ## stream number portion of the fourcc is a 2 digit hex number and is
// not allowed to contain lower case hex digits (a-f), these must be uppercase
// hex digits (A-F). If lower case was allowed, then a fourcc like 'dcdb'
// would cause ambiguity.

FOURCC ReadFCC(FILE *in, int *StreamNum)
{
    char Buf[15];
    FOURCC val;
    int  ret;

    memset(Buf, 0, sizeof(Buf));
    if (!in)return(-1);  // no file to read
    if (StreamNum) *StreamNum = -1;
    ret = File64Read(in, Buf, 4);
    if (ret != 4) return(-1);    // EOF

    val = *((FOURCC *)Buf);  // val is LE when CPU is LE


    // Note that both '##ix' and 'ix##' can exist
    if (StreamNum)   // attempt to get stream number
    {
        char *BufLeft = Buf + 5;
        char *BufRight = Buf + 10;

        // look for ##db, ##dc, ##wb, ##tx, ##ix or ix##
        memcpy(BufLeft, Buf, 2);
        memcpy(BufRight, Buf + 2, 2);
        BufRight[2] = ',';
        if (strcmp(BufLeft, "ix") == 0)   // special case for 'ix##'
        {
            *StreamNum = strtol(BufRight, NULL, 16);
            memcpy(Buf + 2, "##", 2);        // standardize FourCC
        }
        else if (strstr("dc,db,wb,ix,tx,pc,", BufRight))  // '##dc' etc
        {
            *StreamNum = strtol(BufLeft, NULL, 16);
            memcpy(Buf, "##", 2);        // standardize FourCC
        }
        val = *((FOURCC *)Buf);
    }

    return(val);   // return FOURCC

}



