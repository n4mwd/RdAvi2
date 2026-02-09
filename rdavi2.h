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
*/

//#define VERSION   "1.0.1"
//#define RELEASE_DATE   "July 23, 2024"
#define VERSION   "1.0.2"
#define RELEASE_DATE "February 9, 2026"
#define COPYRIGHT    "2024-2026"


#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__TINYC__)
    #pragma GCC diagnostic ignored "-Wmultichar"

    // Large File Support for MinGW/Linux
    #define _FILE_OFFSET_BITS 64
    #ifndef _LARGEFILE64_SOURCE
        #define _LARGEFILE64_SOURCE 1
    #endif

    // Use standard types to avoid "redefinition" errors
    #include <stdint.h>
    typedef uint64_t  QWORD;
    typedef int64_t   QINT;
    typedef int32_t   LONG; // Guaranteed 4 bytes regardless of 32/64 bit
    typedef uint32_t  FOURCC;
    typedef uint32_t  DWORD;

    #define ASSERT_SIZE(type, expected_size) \
        _Static_assert(sizeof(type) == (expected_size), #type " size mismatch")

    #ifndef min
        #define min(a,b) (((a) < (b)) ? (a) : (b))
    #endif

    #if defined(__TINYC__)
        static inline uint32_t __builtin_bswap32(uint32_t x)
        {
            __asm__ ("bswap %0" : "=r" (x) : "0" (x));
            return x;
        }
    #endif
    #define FIX_LIT(n) ((uint32_t)__builtin_bswap32(n))
    #define FCC2STR(n) Fcc2Str(n)
#endif



#if defined(__BORLANDC__)
  // Borland C is a 32bit compiler.
  typedef unsigned __int64 QWORD;     // different
  typedef signed   __int64 QINT;
  typedef long             LONG;     // long is 8 bytes on 64bit compilers, must be 4 bytes here.
  typedef unsigned int     FOURCC;
  typedef unsigned int     DWORD;
  #define ASSERT_SIZE(type, expected_size) \
    typedef char type##_size_check[(sizeof(type) == (expected_size)) ? 1 : -1]
  #define FIX_LIT(n) (n)
  #define FCC2STR(n) ((char *)&(n))
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#define FALSE  0
#define TRUE   !FALSE

typedef unsigned short WORD;
typedef unsigned char  BYTE;

#ifdef __GNUC__
    #pragma pack(push, 1)
#elif defined(__BORLANDC__)
    /* Switch to 1-byte alignment */
    #pragma option -a1
#endif

ASSERT_SIZE(QWORD, 8);
ASSERT_SIZE(DWORD, 4);
ASSERT_SIZE(LONG, 4);
ASSERT_SIZE(WORD, 2);
ASSERT_SIZE(BYTE, 1);


#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

typedef struct
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;

typedef struct
{
    WORD Left;
    WORD Top;
    WORD Right;
    WORD Bottom;
} SMALL_RECT;

typedef struct
{
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
} GUID;



typedef struct
{
    FILE *fp;
    QWORD SeekBase;   // Base File Pointer
} MFILE;



typedef struct
{
    DWORD MicroSecPerFrame; // frame display rate (or 0)
    DWORD MaxBytesPerSec; // max. transfer rate
    DWORD PaddingGranularity; // pad to multiples of this size;
    DWORD Flags; // the ever-present flags
    DWORD TotalFrames; // # frames in file
    DWORD InitialFrames;
    DWORD NumStreams;
    DWORD SuggestedBufferSize;
    DWORD Width;
    DWORD Height;
    DWORD Reserved[4];
} MainAVIHeader;

// Flags for MainAVIHeader
#define 	AVIF_HASINDEX        0x00000010
#define 	AVIF_MUSTUSEINDEX    0x00000020
#define 	AVIF_ISINTERLEAVED   0x00000100
#define 	AVIF_TRUSTCKTYPE     0x00000800
#define 	AVIF_WASCAPTUREFILE  0x00010000
#define 	AVIF_COPYRIGHTED     0x00020000

// Due to multiple definitions of AVI Stream Header we have to define all three.

typedef struct     // strh - compact version - 48 bytes
{
    FOURCC fccType;     // Can be 'auds', 'mids', 'txts', or 'vids'
    FOURCC fccHandler;
    DWORD  Flags;
    WORD   Priority;
    WORD   Language;
    DWORD  InitialFrames;
    DWORD  TimeScale;
    DWORD  Rate;   /* Rate / TimeScale == samples/second */
    DWORD  StartTime;
    DWORD  Length;    /* In units above... */
    DWORD  SuggestedBufferSize;
    DWORD  Quality;
    DWORD  SampleSize;
} AVIStreamHeader48;

typedef struct     // strh - mid sized version - 56 bytes
{
    FOURCC fccType;     // Can be 'auds', 'mids', 'txts', or 'vids'
    FOURCC fccHandler;
    DWORD  Flags;
    WORD   Priority;
    WORD   Language;
    DWORD  InitialFrames;
    DWORD  TimeScale;
    DWORD  Rate;   /* Rate / TimeScale == samples/second */
    DWORD  StartTime;
    DWORD  Length;    /* In units above... */
    DWORD  SuggestedBufferSize;
    DWORD  Quality;
    DWORD  SampleSize;
    SMALL_RECT Frame;
} AVIStreamHeader56;

typedef struct     // strh - large version - 64 bytes
{
    FOURCC fccType;     // Can be 'auds', 'mids', 'txts', or 'vids'
    FOURCC fccHandler;
    DWORD  Flags;
    WORD   Priority;
    WORD   Language;
    DWORD  InitialFrames;
    DWORD  TimeScale;
    DWORD  Rate;   /* Rate / TimeScale == samples/second */
    DWORD  StartTime;
    DWORD  Length;    /* In units above... */
    DWORD  SuggestedBufferSize;
    DWORD  Quality;
    DWORD  SampleSize;
    RECT   Frame;
} AVIStreamHeader64;

// Flags for AVIStreamHeader

#define AVISF_DISABLED          0x00000001
#define AVISF_VIDEO_PALCHANGES  0x00010000


typedef struct       // used when format is 'vids'
{ // bmih
   DWORD  header_size;
   LONG   biWidth;
   LONG   biHeight;
   WORD   biPlanes;
   WORD   bits_per_pixel;
   DWORD  biCompression;
   DWORD  biSizeImage;   // total bytes in image
   LONG   biXPelsPerMeter;
   LONG   biYPelsPerMeter;
   DWORD  biClrUsed;
   DWORD  biClrImportant;
   // Palette goes here
} STREAMFORMATVID;                   // size=40 + palette size

typedef struct
{ // rgbq
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} VIDPALETTE;    // palette for STREAMFORMATVIDS


struct stream_header_auds_t
{
  int format_type;
  int number_of_channels;
  int sample_rate;
  int bytes_per_second;
  int block_size_of_data;
  int bits_per_sample;
  int byte_count_extended;
};


typedef struct
{
  WORD  wFormatTag;
  WORD  nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD  nBlockAlign;
  WORD  wBitsPerSample;
  WORD  cbSize;
} STREAMFORMATAUD;    // same as WAVEFORMATEX   size=18 bytes


typedef struct            // 22 bytes
{
    union
    {
        WORD wValidBitsPerSample;
        WORD wSamplesPerBlock;
        WORD wReserved;
    } Samples;
    DWORD        dwChannelMask;
    GUID         SubFormat;         // 16 bytes
} AUDIOEXTENSION;

typedef struct   // 12 bytes - for use when wFormatTag = 0x0055 (MP3)
{
    WORD  wID;
    DWORD fdwFlags;
    WORD  nBlockSize;
    WORD  nFramesPerBlock;
    WORD  nCodecDelay;
} MP3EXT;


typedef struct     // legacy index structure
{
    DWORD ckid;
    DWORD dwFlags;
    DWORD dwChunkOffset;
    DWORD dwChunkLength;
} AVIINDEXENTRY;

typedef struct
{
    DWORD CompressedBMHeight;
    DWORD CompressedBMWidth;
    DWORD ValidBMHeight;
    DWORD ValidBMWidth;
    DWORD ValidBMXOffset;
    DWORD ValidBMYOffset;
    DWORD VideoXOffsetInT;
    DWORD VideoYValidStartLine;
} VIDEO_FIELD_DESC;


typedef struct
{
    DWORD VideoFormatToken;
    DWORD VideoStandard;
    DWORD dwVerticalRefreshRate;
    DWORD dwHTotalInT;
    DWORD dwVTotalInLines;
    DWORD dwFrameAspectRatio;
    DWORD dwFrameWidthInPixels;
    DWORD dwFrameHeightInLines;
    DWORD nbFieldPerFrame;
    VIDEO_FIELD_DESC FieldInfo[];    // nbFieldPerFrame
} VideoPropHeader;


typedef struct
{
    DWORD dwTotalFrames;    // total frames in all riffs combined.
//    DWORD dwReserved[61];   // junk not used - note the spec doesn't actually have this
} AVIEXTHEADER;


// bIndexType codes
#define AVI_INDEX_OF_INDEXES 0x00   // when each entry in aIndex array points to an index chunk
#define AVI_INDEX_OF_CHUNKS  0x01   // when each entry in aIndex array points to a chunk in the file
#define AVI_INDEX_IS_DATA    0x80   // when each entry is aIndex is really the data

// bIndexSubtype codes for INDEX_OF_CHUNKS
#define AVI_INDEX_STANDARD   0x00   // Standard index chunks
#define AVI_INDEX_2FIELD     0x01   // when fields within frames are also indexed


typedef struct
{
    WORD   wLongsPerEntry;   // size of each entry in aIndex array
    BYTE   bIndexSubType;    // future use.  must be 0
    BYTE   bIndexType;       // one of AVI_INDEX_* codes
    DWORD  nEntriesInUse;    // index of first unused member in aIndex array
    DWORD  dwChunkId;        // fcc of what is indexed
    QWORD  qwBaseOffset;     // offsets in aIndex array are relative to this
    DWORD  dwReserved;       // must be 0
} INDX_CHUNK;


typedef struct
{
    DWORD dwOffset;    // qwBaseOffset + this is absolute file offset
    DWORD dwSize;      // bit 31 is set if this is NOT a keyframe
} STDINDEXENTRY;


typedef struct
{
    DWORD dwOffset;
    DWORD dwSize;      // size of all fields (bit 31 set for NON-keyframes)
    DWORD dwOffsetField2; // offset to second field
} FIELDINDEXENTRY;

typedef struct
{
    QWORD qwOffset;   // absolute file offset, offset 0 is unused entry??
    DWORD dwSize;     // size of index chunk at this offset
    DWORD dwDuration; // time span in stream ticks
} SUPERINDEXENTRY;


#ifdef __GNUC__
    #pragma pack(pop)
#elif defined(__BORLANDC__)
    /* Restore to default alignment (usually 4 or 8) */
    #pragma option -a. 
#endif

// codecs.c prototypes

char *LookupFourCC(DWORD InFcc);
char *LookupFormat(DWORD FmtNum);
char *LookupINFO(DWORD Info);


// File64.c prototypes

// Exported functions
void File64SetBase(MFILE *fp, QWORD NewBase);
QWORD File64GetBase(MFILE *fp);
MFILE *File64Open(char *fname, char *mode);
int  File64Close(MFILE *mfp);
size_t File64Read(MFILE *mfp, void *buffer, int len);
size_t File64Write(MFILE *mfp, void *buffer, int len);
int File64Qseek(MFILE *mfp, QWORD AbsAddr);
int File64SetPos(MFILE *mfp, LONG offset, int whence);
DWORD File64GetPos(MFILE *mfp);
BYTE File64Getchar(MFILE *mfp);
BYTE File64Putchar(MFILE *mfp, BYTE ch);
FOURCC ReadFCC(MFILE *in, int *StreamNum);
int WriteFCC(MFILE *out, FOURCC fccval, int StreamNum);

DWORD  ReverseLiteral(DWORD val);


// FileUtil.c prototypes

char *QWORD2HEX(QWORD val);
DWORD ReadDWORD(MFILE *in);
//FOURCC ReadFCC(FILE *in, int *StreamNum);







