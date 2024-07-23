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

#if defined(__GNUC__)
#error GCC is not tested.  Comment out this line and compile at your own risk.
#define __TINYC__
#endif

#if defined(__clang__)
#error clang is not tested.  Comment out this line and compile at your own risk.
#define __TINYC__
#endif

#if defined(__TINYC__)
  // TINYC is a 64 bit compiler.
  #pragma pack(push, 1)
  #define NO_HUGE_FILES
  #define min(X, Y) (((X) < (Y)) ? (X) : (Y))
  typedef unsigned long long QWORD;   // different

#endif

#if defined(__BORLANDC__)
  // Borland C is a 32bit compiler.
  typedef unsigned __int64 QWORD;     // different
#endif

#if defined(__WIN32__)        // Large file support via windows calls
  #include <windows.h>
  #include <stdio.h>
  #include <io.h>
#else
  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #include <ctype.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #define FALSE  0
  #define TRUE   !FALSE

  typedef unsigned int   FOURCC;
  typedef unsigned int   DWORD;
  typedef int            LONG;     // long is 8 bytes on 64bit compilers, must be 4 bytes here.
  typedef unsigned short WORD;
  typedef unsigned char  BYTE;

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

#endif


// *********** Uncomment define before to only allow files < 4GB
//#define NO_HUGE_FILES


// Test whether compiler uses LE or BE order for multi-character literals
#if '0123' == 0x33323130
  #define LE_MC_LIT   // Borland LE
#else
  #define BE_MC_LIT   // Most other compilers BE
#endif

#if defined(BE_MC_LIT)    // make sure literal is in Little Endian Order
  // must reverse
  #define FIX_LIT(x)  ReverseLiteral((DWORD)(x))
#else
  #define FIX_LIT(x)  ((DWORD)(x))
#endif

#define WAVE_FORMAT_EXTENSIBLE 0xFFFE



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


// codecs.c prototypes

char *LookupFourCC(DWORD InFcc);
char *LookupFormat(DWORD FmtNum);
char *LookupINFO(DWORD Info);


// File64.c prototypes

void   File64SetBase(FILE *fp, int delta);
QWORD  File64GetBase(void);
FILE  *File64Open(char *fname, char *mode);
void   File64Close(FILE *fp);
size_t File64Read(FILE *fp, void *buffer, int len);
int    File64SetPos(FILE *fp, LONG offset, int whence);
DWORD  File64GetPos(FILE *fp);
DWORD  ReverseLiteral(DWORD val);


// FileUtil.c prototypes

char *QWORD2HEX(QWORD val);
LONG read_long(FILE *in);
FOURCC ReadFCC(FILE *in, int *StreamNum);


