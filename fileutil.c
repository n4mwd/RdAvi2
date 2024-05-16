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

int read_long(FILE *in)
{
    int c;

    if (!in) return(-1);
    File64Read(in, &c, sizeof(long));

    return c;
}



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
    BYTE buf[4];
    FOURCC val;
    int  ret;

    if (!in)return(-1);  // no file to read
    if (StreamNum) *StreamNum = -1;
    ret = File64Read(in, buf, 4);
    if (ret != 4) return(-1);    // EOF

    val = *((FOURCC *)buf);


    // Note that both '##ix' and 'ix##' can exist
    if (StreamNum)   // attempt to get stream number
    {
        FOURCC v2, v3;

        // look for ##db, ##dc, ##wb, ##tx, ##ix or ix##
        v2 = (val >> 16) & 0x0000FFFF;     // left side
        v3 = val & 0x0000FFFF;             // right side
        if (v3 == 'ix')
        {
            buf[0] = buf[2];
            buf[1] = buf[3];
            buf[2] = 0;
            *StreamNum = strtol((char *) buf, NULL, 16);
            val = ('##' << 16) | v3;    // standardize FourCC
        }
        else if (v2 == 'db' || v2 == 'dc' || v2 == 'wb' ||
                 v2 == 'tx' || v2 == 'ix' || v2 == 'pc')
        {
            buf[2] = 0;
            *StreamNum = strtol((char *) buf, NULL, 16);
            val = (v2 << 16) | '##';    // standardize FourCC
        }
    }

    return(val);   // return FOURCC

}



