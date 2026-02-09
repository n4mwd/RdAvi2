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

DWORD ReadDWORD(MFILE *in)
{
    int c;

    if (!in) return(-1);
    File64Read(in, &c, sizeof(DWORD));

    return c;
}



// Read four bytes from the current position in the file and convert
// to a Little Endian integer.  If StreamNum is not NULL, and the FCC
// chars are '##db', '##dc', '##wb', '##tx', or 'ix##' (where ##
// represents a stream number like '00' or '02'), the stream number
// portion is returned in StreamNum as an integer.  Also, the FCC is
// converted to a standard form like '##dc' (actual '#' characters) and
// returned instead of the original like '00dc' or '01dc'.  This is so
// the program can compare against a consistent value.  The ## stream
// number portion of the fcc is a 2 digit decimal number represented by
// two ascii characters with a leading '0' if the stream number is less
// than 10.  If StreamNum is not null, and no '##' digits were found,
// StreamNum is set to -1 to indicate that the stream number is invalid.
// If there was a problem reading the file, the function returns -1.

FOURCC ReadFCC(MFILE *in, int *StreamNum)
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
        Buf[4] = ',';  // add comma for search
        *StreamNum = -1;

        // Check if 'ix##'
        if (Buf[0] == 'i' && Buf[1] == 'x')  // special case for 'ix##'
        {
            if (isdigit(Buf[2]) && isdigit(Buf[3]))
                *StreamNum = (Buf[2] - '0') * 10 + (Buf[3] - '0');
            Buf[2] = Buf[3] = '#';           // standardize FourCC
        }

        // check if '##db', '##dc', '##wb', '##tx', '##ix'
        else if (strstr("dc,db,wb,ix,tx,pc,", Buf + 2))  // '##dc' etc
        {
            if (isdigit(Buf[0]) && isdigit(Buf[1]))
                *StreamNum = (Buf[0] - '0') * 10 + (Buf[1] - '0');
            Buf[0] = Buf[1] = '#';           // standardize FourCC
        }

        if (*StreamNum == -1) return(-1);
        val = *((FOURCC *)Buf);
    }

    val = FIX_LIT(val);      // FOURCC should now be in correct endian order

    return(val);   // return FOURCC

}


