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

This is a library extension to enable reading and writing AVI files
larger than 4GB.  Because Borland C does not handle 64 bit files directly,
we are forced to call the Windows API for this.

*/

// Define NO_HUGE_FILES in rdavi2.h to only use 32bit file addressing.

#include "rdavi2.h"


// SeekBase is an unsigned 64 value that gets added to the offset to form
// the absolute seek location.  The SeekBase is the location of the first
// byte of the RIFF chunk.
// For AVI 1.0 files, this is always zero.  Any File64SetPos(DWORD) will
// get added to SeekBase to seek to the address.  Calls to GetPos() will
// return a DWORD value from the current SeekBase.  It is important to
// remember that this is a base not simply the upper 32 bits.  Use
// File64SetBase() and File64GetBase() to set/get the SeekBase value.
// It is not set automatically.

static QWORD SeekBase = 0;   // base of file pos, gets added to seeks.



// Set the SeekBase to the current file location + delta
// File postions will be based off this location until changed.
// Delta can be negative.


void File64SetBase(FILE *fp, int delta)
{
#if defined(NO_HUGE_FILES)
    SeekBase = (DWORD)(ftell(fp) & 0xFFFFFFFF) + delta;
#elif defined(__TINYC__)
    SeekBase = ftell(fp) + delta;
#else
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(fp));
    DWORD offset;
    LONG OfsHigh = 0;

    // Get current file location as a two part pointer
    offset = SetFilePointer(hFile, 0, (LONG *) &OfsHigh, FILE_CURRENT);

    SeekBase = ((QWORD) OfsHigh << 32) | (offset + delta);

#endif
}


// Return the current base location

QWORD File64GetBase(void)
{
    return(SeekBase);
}


// Open a file using fopen() parameters and return a FILE pointer.

FILE *File64Open(char *fname, char *mode)
{
    FILE *fp = fopen(fname, mode);

    SeekBase = 0;
    return(fp);
}


// Close a file pointer.

void File64Close(FILE *fp)
{
    fclose(fp);
}




// Read a block of bytes from a file.
// Returns the number of bytes actually read.

size_t File64Read(FILE *fp, void *buffer, int len)
{
#if defined(NO_HUGE_FILES)
    return(fread(buffer, 1, (size_t) len, fp));
#elif defined(__TINYC__)
    return(fread(buffer, 1, (size_t) len, fp));
#else
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(fp));
    DWORD cnt = 0;

    ReadFile(hFile, buffer, len, &cnt, NULL);

    return(cnt);
#endif
}


// Sets a 32 or 64 bit file position.
// whence can be FILE_BEGIN, FILE_CURRENT, FILE_END, SEEK_SET, SEEK_CUR, or SEEK_END.
// This function returns zero if successful, or
// else it returns a non-zero value.
// Note that Offset is based off SeekBase.

int File64SetPos(FILE *fp, LONG offset, int whence)
{
#if defined(NO_HUGE_FILES) || defined(__TINYC__)
    QWORD LongOffset = offset;

    // if whence is SEEK_SET (FILE_BEGIN) we must apply SeekBase
    if (whence == SEEK_SET)
    {
        LongOffset += SeekBase;   // Seekbase is 32 bits when NO_HUGE_FILES is defined
    }

    return(fseek(fp, (long int) LongOffset, whence));
#else
    LONG ret, SaveHigh, OfsHigh = 0, *pOffHigh = NULL;
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(fp));
    QWORD AbsPos;

    // if whence is SEEK_SET (FILE_BEGIN) we must apply SeekBase
    if (whence == SEEK_SET)
    {
        AbsPos = SeekBase + offset;
        OfsHigh = (LONG)(AbsPos >> 32);
        offset = (LONG)(AbsPos & 0xFFFFFFFF);
        pOffHigh = &OfsHigh;
    }
    SaveHigh = OfsHigh;
    ret = SetFilePointer(hFile, offset, pOffHigh, whence);

    return(ret != offset || SaveHigh != OfsHigh);
#endif
}


// Returns the current file position relative to SeekBase.
// Only the 32 bit offset is returned.
// If the current pos is less than SeekBase, or more than 4GB from
// SeekBase, then return -1 as an error code.
// Note that this function will not change SeekBase.

DWORD File64GetPos(FILE *fp)
{
#if defined(NO_HUGE_FILES) || defined(__TINYC__)
    QWORD Offset = (QWORD) ftell(fp);   // Get 64 bit absolute offset

    return((DWORD)(Offset - SeekBase));
#else
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(fp));
    DWORD Offset, OfsHigh = 0;
    QWORD AbsPos, NewOfs;


    // Get current file pos as high:low
    Offset = SetFilePointer(hFile, 0, (LONG *) &OfsHigh, FILE_CURRENT);

    AbsPos = ((QWORD) OfsHigh << 32) | Offset;      // Calculate absolute
    if (SeekBase > AbsPos) return(-1);           // Out of bounds
    NewOfs = AbsPos - SeekBase;                  // calc diifference
    if (NewOfs & 0xFFFFFFFF00000000) return(-1);  // Out of bounds
    return((DWORD) NewOfs & 0xFFFFFFFF);
#endif

}








