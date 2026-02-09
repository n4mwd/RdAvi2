
/*
RdAvi2.exe - Copyright (c) 2025 by Dennis Hawkins. All rights reserved.
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


Compilation
Borland C (Windows)
bcc32 rdavi2.c codecs.c fileutil.c file64.c

MinGW (Windows) - 32-bit
i686-w64-mingw32-gcc file64.c codecs.c rdavi2.c fileutil.c -o rdavi2.exe

GCC (Linux) - 32-bit
gcc -m32 file64.c codecs.c rdavi2.c fileutil.c -o rdavi2

GCC (Linux) - 64-bit
gcc -m64 file64.c codecs.c rdavi2.c fileutil.c -o rdavi2

Note: Both 32-bit and 64-bit builds are fully supported.
The code automatically adapts based on available functions.

*/



#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE 1
#include <stdio.h>
#include <stdlib.h>

// Platform-specific includes
#if defined(__BORLANDC__)
    // Borland C compiler - use Windows API for 64-bit file operations
    #define USE_WINDOWS_FILE_IO
    #include <windows.h>
    #include <io.h>
    typedef unsigned __int64 QWORD;     // different

    #define FIX_LIT(n) (n)


#elif defined(_WIN32)
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <stdint.h>
    #include <io.h>

    #ifdef _MSC_VER
        // Disable "deprecated" warnings for fopen, etc.
        #define _CRT_SECURE_NO_WARNINGS
        #include <stdlib.h>
        // Use MSVC intrinsic for byte swapping
        #define FIX_LIT(n) ((uint32_t)_byteswap_ulong(n))
    #endif

    #ifndef _WINDEF_ // Guard against Windows.h redefinitions
        typedef uint64_t QWORD;
        typedef uint32_t DWORD;
        typedef uint32_t FOURCC;
        typedef int32_t  LONG;
        typedef uint8_t  BYTE;
    #endif
#else
    // Linux/Unix - use POSIX functions
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <stdint.h>
    typedef uint64_t QWORD;
    typedef uint32_t DWORD;
    typedef  uint32_t FOURCC;
    typedef int32_t  LONG;
    typedef uint8_t  BYTE;

#endif


#if defined(NO_HUGE_FILES)
    #define FILE64_FSEEK(f,o,w)    fseek(f,(long)(o),w)
    #define FILE64_FTELL(f)        ftell(f)

#elif defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
    // MSVC and MinGW both support the i64 underscores
    #define FILE64_FSEEK(f,o,w)    _fseeki64(f,(long long)(o),w)
    #define FILE64_FTELL(f)        _ftelli64(f)

#elif defined(__GNUC__) || defined(__TINYC__)
    // Standard Linux/Unix 64-bit offsets
    #define FILE64_FOPEN(fn, fm)   fopen64(fn, fm)  // Added 64 here
    #define FILE64_FCLOSE(f)       fclose(f)
    #define FILE64_FREAD(b,s,c,f)  fread(b, s, c, f)
    #define FILE64_FWRITE(b,s,c,f) fwrite(b, s, c, f)
    #define FILE64_FSEEK(f,o,w)    fseeko64(f,(off64_t)o,w)
    #define FILE64_FTELL(f)        ftello64(f)
#endif

// Generic fallbacks for standard functions
#ifndef FILE64_FOPEN
    #define FILE64_FOPEN(fn, fm)   fopen(fn, fm)
    #define FILE64_FCLOSE(f)       fclose(f)
    #define FILE64_FREAD(b,s,c,f)  fread(b, s, c, f)
    #define FILE64_FWRITE(b,s,c,f) fwrite(b, s, c, f)
#endif




// Error Defines
enum errvals
{
    AVIERR_NO_ERROR=0,
    AVIERR_BAD_PARAMETER=17,
};


typedef struct
{
    FILE *fp;
    QWORD SeekBase;   // Base File Pointer
} MFILE;



// Exported functions
void File64SetBase(MFILE *fp, QWORD NewBase);
QWORD File64GetBase(MFILE *fp);
MFILE *File64Open(char *fname, char *mode);
int  File64Close(MFILE *mfp);
size_t File64Read(MFILE *mfp, void *buffer, int len);
size_t File64Write(MFILE *mfp, void *buffer, int len);
int File64Qseek(MFILE *mfp, QWORD AbsAddr);
int File64QseekFrom(MFILE *mfp, QWORD AbsAddr, int whence);
int File64SetPos(MFILE *mfp, LONG offset, int whence);
DWORD File64GetPos(MFILE *mfp);
BYTE File64Getchar(MFILE *mfp);
BYTE File64Putchar(MFILE *mfp, BYTE ch);
FOURCC ReadFCC(MFILE *in, int *StreamNum);
int WriteFCC(MFILE *out, FOURCC fccval, int StreamNum);









// SeekBase is an unsigned 64 value that gets added to the offset to form
// the absolute seek location.  The SeekBase is the location of the first
// byte of the RIFF chunk.
// For AVI 1.0 files, this is always zero.  Any File64SetPos(DWORD) will
// get added to SeekBase to seek to the address.  Calls to GetPos() will
// return a DWORD value from the current SeekBase.  It is important to
// remember that this is a base not simply the upper 32 bits.  Use
// File64SetBase() and File64GetBase() to set/get the SeekBase value.
// It is not set automatically, but remains constant for each file pointer.


// Set the SeekBase to the current file location + delta
// File postions will be based off this location until changed.
// Delta can be negative.


void File64SetBase(MFILE *fp, QWORD NewBase)
{
#if defined(NO_HUGE_FILES)
    fp->SeekBase = 0;   // Always 0 for legacy avi
#else
    fp->SeekBase = NewBase;
#endif
}


// Return the current base location

QWORD File64GetBase(MFILE *fp)
{
    return(fp->SeekBase);
}


// Open a file using fopen() parameters and return a FILE pointer.

MFILE *File64Open(char *fname, char *mode)
{
    MFILE *mfp;
    FILE *fp;

    fp = FILE64_FOPEN(fname, mode);
    if (!fp) return(NULL);

    mfp = malloc(sizeof(MFILE));
    if (!mfp)
    {
        fclose(fp);
        return(NULL);
    }
    mfp->fp = fp;
    mfp->SeekBase = 0;

    return(mfp);
}


// Close a file pointer.

int  File64Close(MFILE *mfp)
{
    if (!mfp)
        return(AVIERR_BAD_PARAMETER);
    FILE64_FCLOSE(mfp->fp);
    free(mfp);

    return(AVIERR_NO_ERROR);
}




// Read a block of bytes from a file.
// Returns the number of bytes actually read.

size_t File64Read(MFILE *mfp, void *buffer, int len)
{
#if defined(USE_WINDOWS_FILE_IO)
    // use Windows API
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(mfp->fp));
    DWORD cnt = 0;

    ReadFile(hFile, buffer, len, &cnt, NULL);

    return(cnt);
#else
    return(FILE64_FREAD(buffer, 1, (size_t) len, mfp->fp));
#endif
}


// Write a block of bytes to a file.
// Returns the number of bytes actually written.

size_t File64Write(MFILE *mfp, void *buffer, int len)
{
#if defined(USE_WINDOWS_FILE_IO)
    // use Windows API
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(mfp->fp));
    DWORD cnt = 0;

    WriteFile(hFile, buffer, len, &cnt, NULL);

    return(cnt);
#else

    return(FILE64_FWRITE(buffer, 1, (size_t) len, mfp->fp));
#endif
}


// This function bypasses the Base addressing and seeks
// to an absolute 64 bit location in the file.
// The Base Address is not used or modified.

int File64QseekFrom(MFILE *mfp, QWORD AbsAddr, int whence)
{
#if defined(USE_WINDOWS_FILE_IO)
    // use Windows API
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(mfp->fp));
    LONG OfsHigh, Offset;

    OfsHigh = (LONG)(AbsAddr >> 32);
    Offset = (LONG)(AbsAddr & 0xFFFFFFFF);

    SetFilePointer(hFile, Offset, &OfsHigh, whence);
    return(0);
#else
  #if defined(NO_HUGE_FILES)
    // Note that the offset to fseek() is a signed integer which
    // limits the function to positive numbers < 2GB.
    if (AbsAddr & 0xFFFFFFFF80000000ULL) return(-1);   // out of bounds
  #endif
    return(FILE64_FSEEK(mfp->fp, AbsAddr, whence));
#endif
}

int File64Qseek(MFILE *mfp, QWORD AbsAddr)
{
    return(File64QseekFrom(mfp, AbsAddr, SEEK_SET));
}


// Return a full 64 bit file pointer to the current location
// Base address is not used or modified

QWORD File64Qtell(MFILE *fp)
{
#if defined(USE_WINDOWS_FILE_IO)
    // Use Windows API
    HANDLE hFile = (HANDLE)_get_osfhandle(fileno(fp->fp));
    DWORD Offset, OfsHigh = 0;

    // Get current file pos as high:low with windows
    Offset = SetFilePointer(hFile, 0, (LONG *) &OfsHigh, FILE_CURRENT);

    return(((QWORD) OfsHigh << 32) | Offset );
#else
    return((QWORD) FILE64_FTELL(fp->fp));   // Get absolute offset
#endif

}



// Sets a 64 bit file position.
// whence can be FILE_BEGIN, FILE_CURRENT, FILE_END, SEEK_SET, SEEK_CUR, or SEEK_END.
// This function returns zero if successful, or
// else it returns a non-zero value.
// Note that Offset is based off SeekBase.

int File64SetPos(MFILE *mfp, LONG offset, int whence)
{
    QWORD LongOffset = (QWORD) offset;

    // if whence is SEEK_SET (FILE_BEGIN) we must apply SeekBase
    if (whence == SEEK_SET)
        LongOffset += mfp->SeekBase;   // Seekbase is 32 bits when NO_HUGE_FILES is defined

    return(File64QseekFrom(mfp, LongOffset, whence));
}


// Returns the current file position relative to SeekBase.
// Only the 32 bit offset is returned.
// If the current pos is less than SeekBase, or more than 4GB from
// SeekBase, then return -1 as an error code.
// Note that this function will not change SeekBase.

DWORD File64GetPos(MFILE *mfp)
{
    QWORD Offset = File64Qtell(mfp);   // Get 64 bit absolute offset

    return((DWORD)(Offset - mfp->SeekBase));
}

BYTE File64Getchar(MFILE *mfp)
{
    BYTE ch;

    File64Read(mfp, &ch, 1);
    return(ch);
}

BYTE File64Putchar(MFILE *mfp, BYTE ch)
{
    int ret;

    ret = File64Write(mfp, &ch, 1);

    return((BYTE) ret);
}



