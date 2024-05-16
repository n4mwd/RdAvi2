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


static DWORD G_movi_offset = 0;   // offset of the 'movi' tag
static int Level = 0;
static char indent[80] = {0};
#define CHARS_PER_TAB   2



// Take a numerical offset relative to the Base file address and return
// a pointer to an ascii string representing the full 64bit file location.
// If the base is 0, then only the 32bit file location is returned.
// For legacy files, this will always be 32bits.
// Because the string is returned as a pointer to a static buffer, this
// cannot be used more than once in a function call.

static char *GetOffsetStr(DWORD offset)
{
    QWORD base = File64GetBase();
    static char tmpstr[20];

    if (base)
        strcpy(tmpstr, QWORD2HEX(base + (QWORD) offset));
    else sprintf(tmpstr, "%08X", offset);

    return(tmpstr);
}


// This function modifies the global string 'indent' by a making
// a spring of (CHARS_PER_TAB X Level) number of spaces.  Level
// is also a global.  The resulting string is used for output
// formatting purposes.

static char *MakeIndent(void)
{
    int x;

    memset(indent, ' ', sizeof(indent));
    x = Level * CHARS_PER_TAB;
    if (x >= sizeof(indent)) x = sizeof(indent) - 2;
    indent[x] = 0;

    return(indent);
}


// These functions are called before going deeper in the output nesting and
// after returning.  They will print an opening and closing bracket as
// appropriate and adjust the nesting level.

static void OpenLevel(void)
{
    printf("%s{\n", indent);
    Level++;
    MakeIndent();
}

static void CloseLevel(void)
{
    Level--;
    MakeIndent();
    printf("%s}\n", indent);
}


// Produce a HEX dump for an output.
// Exactly chunk_len bytes are read and output.
// Return 0 on success and -1 if EOF.
// Output is suppressed after 16 lines.

#define HEXSTART   (buf + 9)
#define CHARSTART  (buf + 57)
#define ENDNULL    73

static int hex_dump_chunk(FILE *in, int chunk_len)
{
    char CharStr[17];
    int ch, n, i, linecnt = 0, bcnt, ret, printing = TRUE, poff;
    DWORD offset = File64GetPos(in);
    char buf[80], tmpstr[32];

/*
00000000 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 1234567890123456
*/

    memset(buf, ' ', sizeof(buf));
    buf[ENDNULL] = 0;

    for (n = 0; n < chunk_len; )
    {
        // eject the previous line
        if (linecnt++ == 16) printing = FALSE;

        // start new line
        strset(buf, ' ');                  // clear print buffer
        sprintf(tmpstr, "%08X ", offset & 0xFFFFFFF0);  // print address
        memcpy(buf, tmpstr, 8);            // copy in address

        poff = (offset & 0x0000000F);
        bcnt = min(16 - poff, chunk_len - n);
        offset += bcnt;
        n += bcnt;
        ret = File64Read(in, CharStr, bcnt);
        if (ret != bcnt)
        {
            printf("*** Unexpected EOF ***\n");
            return(-1);
        }

        if (poff)    // fill in initial missing bytes
        {
            for (i = 0; i < 16; i++)
                memcpy(HEXSTART + i * 3, "<>", 2);
        }

        // add hex and char bytes to buffer
        for (i = 0; i < bcnt; i++)
        {
            ch = CharStr[i];
            sprintf(tmpstr, "%02X", ch);
            memcpy(HEXSTART + (i + poff) * 3, tmpstr, 2);
            CHARSTART[i + poff] = (char)(isprint(ch) ? ch : '.');
        }

        // print line
        if (printing)
            printf("    %s\n", buf);  // print chars of previous line
    }

    // print anything left in buffer
    if (!printing) printf("\n        **TRUNCATED**\n");

    return 0;
}


// Displays a legacy AVI index.
// Returns 0 on success and non-zero on failure.
// Output suppressed after 16 lines.


#define AVIIF_LIST          0x00000001L // chunk is a 'LIST'
#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
#define AVIIF_FIRSTPART     0x00000020L // this frame is the start of a partial frame.
#define AVIIF_LASTPART      0x00000040L // this frame is the end of a partial frame.
#define AVIIF_NO_TIME	    0x00000100L // this frame doesn't take any time
#define AVIIF_COMPUSE       0x0FFF0000L


static int parse_idx1(FILE *in, int chunk_len)
{
    AVIINDEXENTRY index_entry;
    int t, br;
    DWORD flags;
    char buf[80];

    printf("%sCkId  Flags                           Location    Length\n", indent);
    printf("%s====  ==============================  ==========  ==========\n", indent);

    for (t = 0; t < (int)(chunk_len / sizeof(AVIINDEXENTRY)); t++)
    {

        br = File64Read(in, &index_entry, sizeof(AVIINDEXENTRY));
        if (br != sizeof(AVIINDEXENTRY))
        {
            printf("*** Unexpected EOF ***\n");
            return(-1);
        }

        if (t < 16)
        {
            buf[0] = 0;
            printf("%s%.4s  ", indent, &index_entry.ckid);
            flags = index_entry.dwFlags;
            strcat(buf, (flags & AVIIF_KEYFRAME)  ? "KEYFRM " : "       ");
            strcat(buf, (flags & AVIIF_LIST)     ? "RECLIST " : "        ");
            strcat(buf, (flags & AVIIF_NO_TIME)  ? "NOTIME " : "       ");
            strcat(buf, (flags & AVIIF_FIRSTPART) ? "1st " : "    ");
            strcat(buf, (flags & AVIIF_LASTPART)  ? "LAST " : "     ");
            printf("%s 0x%08X  0x%08X\n", buf,
                G_movi_offset - 4 + index_entry.dwChunkOffset,
                index_entry.dwChunkLength);
        }
    }

    if (t >= 16) printf("%s**Suppressed %d index entries**\n", indent, t - 16);
    else printf("\n");

    return 0;
}




// Read and print Main AVI Header.
// Return 0 on success or non-zero if not.

static int read_avi_header(FILE *in)
{
    MainAVIHeader avi_header;
    DWORD offset = File64GetPos(in);
    DWORD flags, br;
    char flagstr[64];

    memset(flagstr, 0, sizeof(flagstr));
    br = File64Read(in, &avi_header, sizeof(MainAVIHeader));
    if (br != sizeof(MainAVIHeader)) return(-1);

    // Format flag string
    flags = avi_header.Flags;
    if (flags & AVIF_HASINDEX)       strcat(flagstr, "IDX ");    //4
    if (flags & AVIF_MUSTUSEINDEX)   strcat(flagstr, "IDXREQ "); //7
    if (flags & AVIF_ISINTERLEAVED)  strcat(flagstr, "INTLV ");  //6
    if (flags & AVIF_WASCAPTUREFILE) strcat(flagstr, "CAPFILE ");//8
    if (flags & AVIF_COPYRIGHTED)    strcat(flagstr, "(c) ");    //4
    if (flags & AVIF_TRUSTCKTYPE)    strcat(flagstr, "CKOK ");   //5
    if (flags == 0) strcpy(flagstr, "No Flags");                 //9

    printf("         offset=0x%lx\n", offset);
    printf("             TimeBetweenFrames: %d\n", avi_header.MicroSecPerFrame);
    printf("               MaximumDataRate: %d\n", avi_header.MaxBytesPerSec);
    printf("            PaddingGranularity: %d\n", avi_header.PaddingGranularity);
    printf("                         Flags: %08x - %s\n", avi_header.Flags, flagstr);
    printf("           TotalNumberOfFrames: %d\n", avi_header.TotalFrames);
    printf("         NumberOfInitialFrames: %d\n", avi_header.InitialFrames);
    printf("               NumberOfStreams: %d\n", avi_header.NumStreams);
    printf("           SuggestedBufferSize: %d\n", avi_header.SuggestedBufferSize);
    printf("                         Width: %d\n", avi_header.Width);
    printf("                        Height: %d\n", avi_header.Height);

    return 0;
}



// Stream Header.
// StrType is 0 for Video and 1 for audio streams.
// Return 0 if OK, or -1 on EOF.

static int read_stream_header(FILE *in, DWORD size)
{
    AVIStreamHeader56 stream_header;
    DWORD offset = File64GetPos(in);
    DWORD br, flags;
    char flagstr[32];
    SMALL_RECT r;



    // There are 3 different versions of AVIStreamHeader in use.  Figure
    // out which one based on length.
    memset(&stream_header, 0, sizeof(AVIStreamHeader56));
    if (size == sizeof(AVIStreamHeader48) || size == sizeof(AVIStreamHeader56))
    {
        br = File64Read(in, &stream_header, size);
        if (br != size)
        {
            printf("**Unexpected End of File**\n");
            return(-1);
        }
    }
    else if (size == sizeof(AVIStreamHeader64))
    {
        AVIStreamHeader64 tHdr;

        br = File64Read(in, &tHdr, sizeof(AVIStreamHeader64));
        if (br != sizeof(AVIStreamHeader64))
        {
            printf("**Unexpected End of File**\n");
            return(-1);
        }

        // convert to 56 byte version
        memcpy(&stream_header, &tHdr, sizeof(AVIStreamHeader56));
        stream_header.Frame.Top = (WORD) tHdr.Frame.top;
        stream_header.Frame.Left = (WORD) tHdr.Frame.left;
        stream_header.Frame.Bottom = (WORD) tHdr.Frame.bottom;
        stream_header.Frame.Right = (WORD) tHdr.Frame.right;
    }
    else
    {
        printf("**Unknown structure type**\n");
        return(-2);    // unexpected structure size
    }

    r = stream_header.Frame;
    flags = stream_header.Flags;
    memset(flagstr, 0, sizeof(flagstr));
    if (flags & AVISF_DISABLED) strcat(flagstr, "DISABLED ");
    if (flags & AVISF_VIDEO_PALCHANGES) strcat(flagstr, "PALCHG");
    if (flags == 0) strcpy(flagstr, "No Flags Set");

    printf("                offset=0x%lx\n", offset);
    printf("         Stream Header Version: %.4s (%d byte) version\n",
                                  (char *)&stream_header.fccType, size);
    printf("                   FourCC Type: %.4s\n", (char *)&stream_header.fccType);
    if (stream_header.fccType == 'auds')
    {
        printf("                FourCC Handler: Not Used\n");
    }
    else
    {
        printf("                FourCC Handler: %.4s - %s\n",
                            (char *)&stream_header.fccHandler,
                            LookupFourCC(stream_header.fccHandler));
    }
    printf("                         Flags: %08x - %s\n", stream_header.Flags, flagstr);
    printf("                      Priority: %d\n", stream_header.Priority);
    printf("                 InitialFrames: %d\n", stream_header.InitialFrames);
    printf("                     TimeScale: %d\n", stream_header.TimeScale);
    printf("                      DataRate: %d\n", stream_header.Rate);
    printf("                     StartTime: %d\n", stream_header.StartTime);
    printf("                    DataLength: %d\n", stream_header.Length);
    printf("           SuggestedBufferSize: %d\n", stream_header.SuggestedBufferSize);
    printf("                       Quality: %d\n", stream_header.Quality);
    printf("                    SampleSize: %d\n", stream_header.SampleSize);
    printf("                         Frame: { Top: %d, Left: %d, Bottom: %d, Right: %d }\n",
             r.Top, r.Left, r.Bottom, r.Right);

    return 0;
}


// Read Video Stream Format

static int read_stream_format_vid(FILE *in, DWORD size)
{
    STREAMFORMATVID stream_format;
    VIDPALETTE pal[256];
    DWORD offset = File64GetPos(in);
    DWORD br, t;

    // Read structure
    memset(pal, 0, sizeof(pal));
    if (size < sizeof(STREAMFORMATVID))
    {
        printf("*** Unexpected short chunk ***\n");
        return(-1);
    }

    br = File64Read(in, &stream_format, sizeof(STREAMFORMATVID));
    size -= br;
    if (br != sizeof(STREAMFORMATVID))
    {
        printf("*** Unexpected End of File ***\n");
        return(-1);
    }

    // Read Palette
    if (stream_format.biClrUsed != 0)
    {
        // read palette but do not store
        t = sizeof(VIDPALETTE) * stream_format.biClrUsed;
        if (size < t)
        {
            printf("*** Unexpected short chunk ***\n");
            return(-1);
        }

        br = File64Read(in, pal, t);
        size -= br;
        if (br != t)
        {
            printf("**Unexpected End of File**\n");
            return(-1);
        }
    }


    // Print structure
    printf("                offset=0x%lx\n", offset);
    printf("                   header_size: %d\n", stream_format.header_size);
    printf("                   image_width: %d\n", stream_format.biWidth);
    printf("                  image_height: %d\n", stream_format.biHeight);
    printf("              number_of_planes: %d\n", stream_format.biPlanes);
    printf("                bits_per_pixel: %d\n", stream_format.bits_per_pixel);
    printf("              compression_type: %.4s - %s\n",
                       (char *) &stream_format.biCompression,
                       LookupFourCC(stream_format.biCompression));
    printf("           image_size_in_bytes: %d\n", stream_format.biSizeImage);
    printf("              x_pels_per_meter: %d\n", stream_format.biXPelsPerMeter);
    printf("              y_pels_per_meter: %d\n", stream_format.biYPelsPerMeter);
    printf("                   colors_used: %d\n", stream_format.biClrUsed);
    printf("              colors_important: %d\n", stream_format.biClrImportant);

    // print palette
    if (stream_format.biClrUsed != 0)
    {
        int i;

        printf("\n%sVideo Palette:\n%s", indent, indent);
        printf("### RR:GG:BB    ### RR:GG:BB    ### RR:GG:BB    ### RR:GG:BB\n");

        for (i = 0; i < (int) stream_format.biClrUsed; i++)
        {
            printf("%3d %2X:%2X:%2X    ", i,
                pal[i].rgbRed, pal[i].rgbGreen, pal[i].rgbBlue);
            if (i % 4 == 3) printf("\n");
        }
        printf("\n");
    }

    if (size)    // error
    {
        printf("%sUnrecognized Bitmap Header Extension, skipping!!\n", indent);
        File64SetPos(in, size, SEEK_CUR);

    }

    return 0;
}


// Read stream format structure for audio

static int read_stream_format_auds(FILE *in, int size)
{
    STREAMFORMATAUD stream_format;
    MP3EXT mp3fmt;
    AUDIOEXTENSION extfmt;
    DWORD offset = File64GetPos(in);
    DWORD br, rl;

    memset(&stream_format, 0, sizeof(STREAMFORMATAUD));

    // structure could be either WAVEFORMATEX or WAVEFORMATEXTENSIBLE
    // If the FormatTag field is 1 (PCM), then the cbSize field can be omitted.
    // This results in a shorter structure and is why we go by the chunk
    // size here instead of just the structure size.  For a short structure,
    // cbSize will read as zero due to being initialized that way

    rl = min(sizeof(STREAMFORMATAUD), size);
    br = File64Read(in, &stream_format, rl);
    if (br != rl)
    {
        printf("*** Unexpected End of File ***\n");
        return(-1);
    }

    printf("                        offset=0x%lx\n", offset);
    printf("                        format: 0x%04X - %s\n",
                  stream_format.wFormatTag,
                  LookupFormat(stream_format.wFormatTag));
    printf("                      channels: %d\n", stream_format.nChannels);
    printf("            samples_per_second: %d\n", stream_format.nSamplesPerSec);
    printf("              bytes_per_second: %d\n", stream_format.nAvgBytesPerSec);
    printf("            block_size_of_data: %d\n", stream_format.nBlockAlign);
    printf("               bits_per_sample: %d\n", stream_format.wBitsPerSample);
    printf("              Extensible bytes: %d\n", stream_format.cbSize);

    if (stream_format.cbSize)   // there are additional bytes that follow
    {
        size -= br;
        if (stream_format.wFormatTag == 0x0055)   // mp3
        {
            rl = min(sizeof(MP3EXT), size);
            memset(&mp3fmt, 0, sizeof(mp3fmt));
            br = File64Read(in, &mp3fmt, rl);
            if (br != rl)
            {
                printf("*** Unexpected End of File ***\n");
                return(-1);
            }

            printf("                           wID: %d\n", mp3fmt.wID);
            printf("                      fdwFlags: 0x%08X\n", mp3fmt.fdwFlags);
            printf("                    nBlockSize: %d\n", mp3fmt.nBlockSize);
            printf("               nFramesPerBlock: %d\n", mp3fmt.nFramesPerBlock);
            printf("                   nCodecDelay: %d\n", mp3fmt.nCodecDelay);
        }
        else if (stream_format.cbSize == sizeof(AUDIOEXTENSION))
        {
            // its just a guess, but if cbSize is 22 bytes,, then its likely
            // a standard audio extension.
            GUID t;

            rl = min(sizeof(AUDIOEXTENSION), size);
            memset(&extfmt, 0, sizeof(extfmt));
            br = File64Read(in, &extfmt, rl);
            if (br != rl)
            {
                printf("*** Unexpected End of File ***\n");
                return(-1);
            }

            printf("         Valid_Bits_per_Sample: %d\n", extfmt.Samples.wReserved);
            printf("             Samples_per_Block: %d\n", extfmt.Samples.wReserved);
            printf("                  Channel_Mask: %d\n", extfmt.dwChannelMask);
            printf("                Subformat GUID: ");
//                                                  {00000000-0000-0000-0000-000000000000 }
            t = extfmt.SubFormat;
            printf("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                    t.Data1, t.Data2, t.Data3,
                    t.Data4[0], t.Data4[1],
                    t.Data4[2], t.Data4[3],
                    t.Data4[4], t.Data4[5],
                    t.Data4[6], t.Data4[7]);
        }
        else
        {
            hex_dump_chunk(in, size);
        }
    }


    return 0;
}


// Read stream format for closed captioning
// Placeholder - not really supported

static int read_stream_format_txts(FILE *in, int size)
{
    return(hex_dump_chunk(in, size));

}


// Display an open-DML index

static int ProcessIndx(FILE *in, int chunk_size)
{
    INDX_CHUNK idx;   // Generic open-dml index header
    DWORD rb, irb, BytesLeft, br, pad, casetype;
    int i, max;

    // Read base structure if open-dml index
    memset(&idx, 0, sizeof(idx));
    rb = min(sizeof(idx), chunk_size);
    BytesLeft = chunk_size - rb;
    br = File64Read(in, &idx, rb);
    if (br != rb)
    {
        printf("*** Unexpected End of File.\n");
        return(-1);
    }
    pad = chunk_size - sizeof(idx) - (idx.wLongsPerEntry * 4 * idx.nEntriesInUse);

    irb = idx.wLongsPerEntry * 4;   // bytes to read for one index entry

    // Modify indexType to include subtype for use in switch() below
    casetype = idx.bIndexType;
    if (casetype == AVI_INDEX_OF_CHUNKS)
        casetype |= (idx.bIndexSubType << 8);

    switch (casetype)
    {
        case AVI_INDEX_OF_INDEXES:
            printf("%sThis is an Open-DML SuperIndex of Indexes", indent);
            if (sizeof(SUPERINDEXENTRY) != irb)
            {
                printf("%swLongsPerEntry is not correct for this type "
                           "of index.\n", indent);
                irb = min(sizeof(SUPERINDEXENTRY), irb);
            }
            printf(" for the stream '%.4s'.\n", (char *) &idx.dwChunkId);

            printf("%sEach index entry has %d bytes ", indent, idx.wLongsPerEntry * 4);
            printf("with %d entries in use.\n\n", idx.nEntriesInUse);

            printf("%sAbsolute Location   Size        Duration\n", indent);
            printf("%s==================  ==========  ==========\n", indent);
            for (i = 0; i < (int) idx.nEntriesInUse; i++)
            {
                SUPERINDEXENTRY entry;

                memset(&entry, 0, sizeof(entry));
                BytesLeft -= irb;
                br = File64Read(in, &entry, irb);
                if (br != irb)
                {
                    printf("*** Unexpected End of File.\n");
                    return(-1);
                }
                printf("%s0x%s  0x%08X  0x%08X\n", indent,
                    QWORD2HEX(entry.qwOffset),
                    entry.dwSize, entry.dwDuration);
            }
            break;

        case AVI_INDEX_OF_CHUNKS:   // standard index
            if (irb != sizeof(STDINDEXENTRY))
            {
                printf("%swLongsPerEntry is not correct for this type "
                           "of index.\n", indent);
                irb = min(sizeof(STDINDEXENTRY), irb);
            }

            printf("%sAbsolute Location    Size        Keyframe\n", indent);
            printf("%s==================   ==========  ========\n", indent);
            max = min(16, idx.nEntriesInUse);
            for (i = 0; i < (int) max; i++)
            {
                STDINDEXENTRY entry;

                memset(&entry, 0, sizeof(entry));
                BytesLeft -= irb;
                br = File64Read(in, &entry, irb);
                if (br != irb)
                {
                    printf("*** Unexpected End of File.\n");
                    return(-1);
                }
                printf("%s0x%s   0x%08X  %s\n", indent,
                    QWORD2HEX(idx.qwBaseOffset + (QWORD) entry.dwOffset),
                    entry.dwSize & 0x7FFFFFFF,
                    (entry.dwSize & 0x80000000) ? "NO" : "YES");
            }

            if (max != (int) idx.nEntriesInUse)
                printf("%s**Suppressed %d index entries**\n", indent, idx.nEntriesInUse - max);

            break;

        case AVI_INDEX_OF_CHUNKS | (AVI_INDEX_2FIELD << 8):   // field index
            if (irb != sizeof(FIELDINDEXENTRY))
            {
                printf("%swLongsPerEntry is not correct for this type "
                           "of index.\n", indent);
                irb = min(sizeof(FIELDINDEXENTRY), irb);
            }

            printf("%sAbsolute Location   2nd Field Loc       Size        Keyframe\n", indent);
            printf("%s==================  ==================  ==========  ========\n", indent);
            max = min(16, idx.nEntriesInUse);
            for (i = 0; i < (int) max; i++)
            {
                FIELDINDEXENTRY entry;

                memset(&entry, 0, sizeof(entry));
                BytesLeft -= irb;
                br = File64Read(in, &entry, irb);
                if (br != irb)
                {
                    printf("*** Unexpected End of File.\n");
                    return(-1);
                }
                // QWORD2HEX() cannot be called more than once at a time.
                printf("%s0x%s  ", indent,
                    QWORD2HEX(idx.qwBaseOffset + (QWORD) entry.dwOffset));
                printf("0x%s  0x%08X  %s\n",
                    QWORD2HEX(idx.qwBaseOffset + (QWORD) entry.dwOffsetField2),
                    entry.dwSize & 0x7FFFFFFF,
                    (entry.dwSize & 0x80000000) ? "NO" : "YES");
            }

            if (max != (int) idx.nEntriesInUse)
                printf("%s**Suppressed %d index entries**\n", indent, idx.nEntriesInUse - max);

            break;

        case AVI_INDEX_IS_DATA:   // not really supported
            printf("Data containing Index\n");
            break;

        default:
            printf("Index of an unknown type (0x%02X)", (DWORD) idx.bIndexType);
            break;
    }


    printf("\n");
    if (pad)
        printf("%sThis index contains %d extra bytes of padding.\n", indent, pad);



    if (BytesLeft)
        File64SetPos(in, BytesLeft, SEEK_CUR);


    return(0);
}


// Parse and display frames in the movi list.
// It will call itself recursively is a 'LIST rec' is encountered.

static int parse_movi(FILE *in, DWORD size)
{
    FOURCC movi_fcc, NewListName;
    DWORD  movi_size, offset, file_movi_size;
    QWORD  AbsLoc, filebase;
    int    stream, adv, ret;
    DWORD  cnt = 4;   // 4 for 'movi'
    int    dcCnt, txCnt, wbCnt, pcCnt, ix2Cnt, defCnt;   // ix1Cnt,
    char   fccbuf[8], *fccptr, ChunkDesc[64];


    dcCnt = txCnt = wbCnt = pcCnt = ix2Cnt = defCnt = 0;  // ix1Cnt = 0;
    filebase = File64GetBase();

    // print header
    printf("\n%sCkId  Chunk Type                Absolute Location   Length\n", indent);
    printf(  "%s====  ========================  ==================  ==========\n", indent);

    while (cnt < size)
    {
        offset = File64GetPos(in);

        movi_fcc = ReadFCC(in, &stream);
        movi_size = read_long(in);
        AbsLoc = filebase + (QWORD) offset;

        // reconstitute fourcc
        fccptr = (char *) &movi_fcc;
        fccbuf[0] = 0;
        if (stream != -1)
        {
            if (movi_fcc == 'ix##')
                sprintf(fccbuf, "ix%02X", stream);
            else
                sprintf(fccbuf, "%02X%.2s", stream, fccptr + 2);
        }
        else
        {
            memcpy(fccbuf, &movi_fcc, 4);
            fccbuf[4] = 0;
        }

        // There is a poorly documented fact regarding chunks that says they
        // must be WORD aligned.  So chunk_size refers to the size of the chunk
        // and not to the number of bytes on the disk.  Bump up the file
        // pointer by one byte if the chunk_size is an odd number.

        file_movi_size = movi_size;
        if (file_movi_size & 0x00000001) file_movi_size++;
        adv = file_movi_size + 8;   // default advance

        ChunkDesc[0] = 0;

        switch (movi_fcc)
        {
            case 'LIST':
                NewListName = ReadFCC(in, NULL);  // should be 'rec ', but we handle them all
                printf("%sLIST '%.4s'      (Location=0x%s length=0x%08X)\n",
                        indent, (char *)&NewListName,
                        QWORD2HEX(File64GetBase() + (QWORD) offset), movi_size);
                OpenLevel();
                ret = parse_movi(in, file_movi_size);
                CloseLevel();
                adv = 0;   // file advancing already done
                cnt += file_movi_size + 8;
                if (ret) return(ret);
                break;

            case '##db':
                if (dcCnt++ < 16) strcpy(ChunkDesc, "Uncompressed Video");
                break;

            case '##dc':
                if (dcCnt++ < 16) strcpy(ChunkDesc, "Compressed Video");
                break;

            case '##tx':
                if (txCnt++ < 16) strcpy(ChunkDesc, "Subtitle Text");
                break;

            case '##wb':
                if (wbCnt++ < 16) strcpy(ChunkDesc, "Audio");
                break;

            case '##pc':
                if (pcCnt++ < 16) strcpy(ChunkDesc, "Palette Change");
                break;

            case 'ix##':
                // peek at index type
                {
                    INDX_CHUNK idx;
                    int rb;
                    char tmpstr[32];

                    // Read base structure if open-dml index
                    memset(&idx, 0, sizeof(idx));
                    rb = min(sizeof(idx), file_movi_size);
                    rb = File64Read(in, &idx, rb);
                    strcpy(tmpstr, "ODML Standard Index");
                    if (idx.bIndexSubType == AVI_INDEX_2FIELD)
                        strcpy(tmpstr, "ODML Frame Index");
                    printf("%s%s  %.4s %-19s  0x%s  0x%08X\n",
                          indent, fccbuf, (char *)&idx.dwChunkId,
                          tmpstr, QWORD2HEX(AbsLoc), movi_size);

                    // return to previous position
                    File64SetPos(in, -rb, SEEK_CUR);
                }

                OpenLevel();
                ret = ProcessIndx(in, file_movi_size);
//                ret = hex_dump_chunk(in, movi_size);
                CloseLevel();
                adv = 0;   // file advancing already done
                cnt += file_movi_size + 8;
                if (ret) return(ret);
                break;

            case '##ix':
                if (ix2Cnt++ < 16) strcpy(ChunkDesc, "Data chunk for timecode stream");
                break;

            case 'JUNK':
                strcpy(ChunkDesc, "Wasted Space");
                break;

            default:
                if (defCnt++ < 16) strcpy(ChunkDesc, "Unsupported FourCC tag");
                break;

        }


        if (ChunkDesc[0])
        {
            printf("%s%s  %-24s  0x%s  0x%08X\n",
                          indent, fccbuf, ChunkDesc, QWORD2HEX(AbsLoc), movi_size);
        }


        if (adv)
        {
            cnt += adv;
            File64SetPos(in, file_movi_size, SEEK_CUR);
        }


    }

    if (dcCnt > 16) printf("%s**Suppressed %d video frames**\n", indent, dcCnt - 16);
    if (wbCnt > 16) printf("%s**Suppressed %d audio frames**\n", indent, wbCnt - 16);
    printf("\n");


    return(0);
}


// Display the VPRP Video Property Header

static int ProcessVPRP(FILE *in, int chunk_size)
{
    DWORD i, s, t;
    VideoPropHeader vprp;
    VIDEO_FIELD_DESC vfld;

    static char *VidTokStr[] =
    {
        "FORMAT_UNKNOWN", "FORMAT_PAL_SQUARE", "FORMAT_PAL_CCIR_601",
        "FORMAT_NTSC_SQUARE", "FORMAT_NTSC_CCIR_601", NULL
    };
    static char *VidStdStr[] =
    {
        "STANDARD_UNKNOWN", "STANDARD_PAL", "STANDARD_NTSC",
        "STANDARD_SECAM", NULL
    };


    File64Read(in, (char *) &vprp, sizeof(VideoPropHeader));

//    File64SetPos(in, sizeof(VideoPropHeader), SEEK_CUR); vprp.nbFieldPerFrame=1;


    t = vprp.VideoFormatToken;
    printf("               Video Format Token: %d - %s\n", t, (t < 5) ? VidTokStr[t] : "INVALID");
    t = vprp.VideoStandard;
    printf("                   Video standard: %d - %s\n", t, (t < 4) ? VidStdStr[t] : "INVALID");
    printf("            Vertical refresh rate: %d\n", vprp.dwVerticalRefreshRate);
    printf("            Horizontal Total in T: %d\n", vprp.dwHTotalInT);
    printf("          Vertical Total in Lines: %d\n", vprp.dwVTotalInLines);
    t = vprp.dwFrameAspectRatio;
    printf("                FrameAspect Ratio: %d:%d\n", (t & 0xFFFF0000) >> 16, t & 0x0000FFFF);

    printf("    Active Frame Width in Pixels : %d\n", vprp.dwFrameWidthInPixels);
    printf("    Active Frame Height in Lines : %d\n", vprp.dwFrameHeightInLines);
    printf("      Number of Fields Per Frame : %d\n", vprp.nbFieldPerFrame);

    // Number of fields is usually one or two
    s = sizeof(VideoPropHeader);
    for (i = 0; i < vprp.nbFieldPerFrame && s < (DWORD) chunk_size; i++)
    {
        File64Read(in, (char *) &vfld, sizeof(VIDEO_FIELD_DESC));
//    File64SetPos(in, sizeof(VIDEO_FIELD_DESC), SEEK_CUR);

        s += sizeof(VIDEO_FIELD_DESC);

        printf("\n       Video Field #%d Description\n", i);
        printf("     Compressed Bitmap Size (WxH): %d X %d\n", vfld.CompressedBMWidth, vfld.CompressedBMHeight);
        printf("         Valid Bitmap Size (WxH) : %d X %d\n", vfld.ValidBMWidth, vfld.ValidBMHeight);
        printf("         Valid Bitmap Offet (X,Y): %d, %d\n", vfld.ValidBMXOffset, vfld.ValidBMYOffset);
        printf("             Valid X-Offset In T : %d\n", vfld.VideoXOffsetInT);
        printf("              Valid Y Start Line : %d\n", vfld.VideoYValidStartLine);

    }

    return(0);
}


// Read a null terminated string from the file and then display it.
// Non-printable characters are converted to spaces.

static int ProcessString(FILE *in, int chunk_size)
{
    char buffer[32];
    int BytesLeft = chunk_size;
    int br, rt, i;

    printf("\"");
    while (BytesLeft)
    {
        memset(buffer, 0, sizeof(buffer));
        br = min(sizeof(buffer) - 1, BytesLeft);
        BytesLeft -= br;
        rt = File64Read(in, buffer, br);
        if (rt != br)    // EOF
        {
            printf("*** Unexpected EOF\n");
            return(-1);
        }

        // get rid of unprintable characters
        for (i = 0; buffer[i]; i++)
            if (!isprint(buffer[i])) buffer[i] = ' ';

        printf("%s", buffer);
    }
    printf("\"\n");

    return(0);
}



// Special INFO List Processor
// As far as I can tell, there are several possible elements for the INFO
// list and all of them are null terminated strings.

static int ProcessINFO(FILE *in, int chunk_size)
{
    DWORD offset = File64GetPos(in);
    DWORD end_of_chunk = offset + chunk_size - 4;
    DWORD InfoName, InfoSize;
    int ret;


    while (offset < end_of_chunk)
    {
        InfoName = ReadFCC(in, NULL);
        InfoSize = read_long(in);    // length of list element
        if (InfoSize & 0x00000001) InfoSize++;


        printf("%s%s(%.4s): ", indent, LookupINFO(InfoName), (char *)&InfoName);
        ret = ProcessString(in, InfoSize);
        if (ret) return(ret);

        offset = File64GetPos(in);   // current offset
    }

    return(0);
}


// Read the DMLH header info
// The DML specs say that the DMLH structure contains only one DWORD, but
// every implementation also adds a 61 DWORD "reserved" field.  This function
// will handle all sizes.

static int ProcessDmlh(FILE *in, int chunk_size)
{
    AVIEXTHEADER rec;
    DWORD rb, BytesLeft, br;


    memset(&rec, 0, sizeof(rec));
    rb = min(sizeof(rec), chunk_size);
    BytesLeft = chunk_size - rb;
    br = File64Read(in, &rec, rb);
    if (br != rb)
    {
        printf("*** Unexpected End of File.\n");
        return(-1);
    }

    // move to end of chunk if necessary
    if (BytesLeft)
        File64SetPos(in, BytesLeft, SEEK_CUR);

    printf("%sGrand Total of All Frames in File: %u\n",
            indent, rec.dwTotalFrames);

    return(0);
}





// Parse tags under a LIST chunk.
// Title and indentation applied before calling.
// This function is called recursively.

static int parse_list(FILE *in, FOURCC ListName, DWORD ListLen)
{
    DWORD NewListName, StrhType = 0;
    int ret;  //, chunk_size, ret;
    DWORD ListElem, ListElemSize;
    DWORD end_of_chunk;
    DWORD offset = File64GetPos(in);

    if (ListName == 'movi')    // special case for movi lists
    {
        G_movi_offset = offset;     // changes with each new movi list
        ret = parse_movi(in, ListLen);
        return(ret);
    }
    else if (ListName == 'INFO')    // spcial case for INFO lists
    {
        ret = ProcessINFO(in, ListLen);
       return(ret);
    }

    end_of_chunk = offset + ListLen - 4;

    while (offset < end_of_chunk)
    {
        ListElem = ReadFCC(in, NULL);    // get next list element
        ListElemSize = read_long(in);    // length of list element

// printf("ListElem: %.4s\n", (char *)&ListElem);

        switch (ListElem)
        {
            case 'LIST':     // yep, its recursive
                NewListName = ReadFCC(in, NULL);
                printf("%sAVI LIST '%.4s' Element '%.4s' (Location=0x%s length=0x%06X)\n",
                        indent, (char *)&ListName, (char *)&NewListName,
                        GetOffsetStr(offset), ListElemSize);
                OpenLevel();
                ret = parse_list(in, NewListName, ListElemSize);
                CloseLevel();
                if (ret) return(ret);
                break;

            case 'avih':     // AVI header
                if (ListName != 'hdrl') goto syntax;
                printf("%sAVI Main Header 'avih' (Location=0x%08X length=0x%06X)\n",
                        indent, offset, ListElemSize);
                OpenLevel();
                ret = read_avi_header(in);
                CloseLevel();
                if (ret) return(ret);
                break;

            case 'strh':
                if (ListName != 'strl') goto syntax;
                // Peek at stream type
                StrhType = ReadFCC(in, NULL);    // should be  'vids' or 'auds'
                File64SetPos(in, -4, SEEK_CUR);   // move FP back
                printf("%sAVI 'strh' Stream Header for '%.4s' (Location=0x%08X length=0x%06X)\n",
                        indent, (char *)&StrhType,
                        offset, ListElemSize);
                OpenLevel();
                if (StrhType != 'vids' && StrhType != 'auds' && StrhType != 'txts')  // unknown
                {
                    printf("%sUnsupported Stream Header 'strh' type %.4s\n",
                              indent, (char *)&StrhType);
                    File64SetPos(in, ListElemSize, SEEK_CUR);
                    break;
                }

                ret = read_stream_header(in, ListElemSize);
                CloseLevel();
                if (ret) return(ret);
                break;

            case 'strf':
                if (ListName != 'strl') goto syntax;
                printf("%sAVI 'strf' Stream Format for '%.4s' (Location=0x%08X length=0x%06X)\n",
                        indent, (char *)&StrhType,
                        offset, ListElemSize);
                OpenLevel();
                if (StrhType == 'vids')  // video
                {
                    ret = read_stream_format_vid(in, ListElemSize);
                    if (ret) return(ret);
                }
                else if (StrhType == 'auds')  // audio
                {
                    ret = read_stream_format_auds(in, ListElemSize);
                    if (ret) return(ret);
                }
                else if (StrhType == 'txts')   // subtitles
                {
                    ret = read_stream_format_txts(in, ListElemSize);
                    if (ret) return(ret);
                }
                else    // unsupported
                {
                    if (StrhType == 0)
                        printf("*** 'strf' without preceeding 'strh'\n");
                    else printf("*** Unsupported Stream Format '%.4s'\n", (char *)&StrhType);
                    File64SetPos(in, ListElemSize, SEEK_CUR);
                }
                CloseLevel();
                break;

            case 'vprp':        // video properties header
                if (ListName != 'strl') goto syntax;
                printf("%sAVI 'vprp' Video Property Header (Location=0x%08X length=0x%06X)\n",
                        indent, offset, ListElemSize);
                OpenLevel();
                ret = ProcessVPRP(in, ListElemSize);
                if (ret) return(ret);
                CloseLevel();
                break;

            case 'dmlh':
                if (ListName != 'odml') goto syntax;
                printf("%sAVI 'dmlh' Extended Header (Location=0x%08X length=0x%06X)\n",
                        indent,
                        offset, ListElemSize);
                OpenLevel();
                ret = ProcessDmlh(in, ListElemSize);
                if (ret) return(ret);
                CloseLevel();
                break;

            case 'strn':      // null terminated string stream name
                if (ListName != 'strl') goto syntax;
                printf("%sStream Name(strn): ", indent);
                ret = ProcessString(in, ListElemSize);
                if (ret) return(ret);
                break;


            case 'strd':
                if (ListName != 'strl') goto syntax;
                printf("%sAVI 'strd' Stream Data (Location=0x%08X length=0x%06X)\n",
                        indent,
                        offset, ListElemSize);
                OpenLevel();
                ret = hex_dump_chunk(in, ListElemSize);
                if (ret) return(ret);
                CloseLevel();
                break;

            case 'indx':       // super DML index
                printf("%sAVI 'indx' Open DML Index (Location=0x%08X length=0x%06X)\n",
                        indent,
                        offset, ListElemSize);
                OpenLevel();
//                ret = hex_dump_chunk(in, ListElemSize);
                ret = ProcessIndx(in, ListElemSize);
                if (ret) return(ret);
                CloseLevel();
                break;


            case 0:  // special case for PRMI
                if (ListName == 'PRMI')
                {
                    printf("%sPRMI: ", indent);
                    ret = ProcessString(in, ListElemSize);
                    if (ret) return(ret);
                    break;
                }


            case 'JUNK':
            default:
                printf("%sAVI '%.4s' Chunk (Location=0x%s length=0x%06X)\n",
                        indent, (char *)&ListElem,
                        GetOffsetStr(offset), ListElemSize);
                OpenLevel();
                printf("%sSkipping %d %.4s bytes.\n", indent,
                    ListElemSize, (char *)&ListElem);
                CloseLevel();
                File64SetPos(in, ListElemSize, SEEK_CUR);
                break;
        }


        offset = File64GetPos(in);   // current offset
    }
    return(0);

syntax:
    printf("*** A File syntax error was detected near offset 0x%X ***\n", offset);
    return(-1);


}


// Process the AVI or AVIX file
// We accept LIST and idx1, eveything else is treated as JUNK.

static int ProcessAVI(FILE *in, DWORD riff_size)
{
    DWORD fcc_id, chunk_size, ListName;
    DWORD offset, endofs;
    int ret;

    offset = File64GetPos(in);   // get offset of the start of this chunk
    endofs = offset + riff_size - 4;


    while (offset < endofs)
    {
        fcc_id = ReadFCC(in, NULL);   // LIST, idx1, etc
        chunk_size = read_long(in);

        switch (fcc_id)
        {
            case 'LIST':         // get list type
                ListName = ReadFCC(in, NULL);

                printf("%sAVI LIST '%.4s' (Location=0x%s length=0x%06X)\n",
                            indent, (char *)&ListName,
                            GetOffsetStr(offset), chunk_size);
                OpenLevel();
                ret = parse_list(in, ListName, chunk_size);
                CloseLevel();
                if (ret) return(ret);
                break;

            case 'idx1':
                printf("%sAVI Legacy Index 'idx1' (Location=0x%08X length=0x%06X)\n",
                            indent, offset, chunk_size);
                OpenLevel();
                ret = parse_idx1(in, chunk_size);
                CloseLevel();
                if (ret) return(ret);
                break;

            case 'DISP':    // junk
                printf("%sAVI 'DISP' Chunk (Location=0x%s length=0x%08X)\n",
                        indent, GetOffsetStr(offset), chunk_size);
                OpenLevel();
                ret = hex_dump_chunk(in, chunk_size);
                if (ret) return(ret);
                CloseLevel();
                break;

            case 'JUNK':    // junk
            default:  // unsupported
                printf("%sAVI '%.4s' Chunk (Location=0x%s length=0x%08X)\n",
                        indent, (char *)&fcc_id, GetOffsetStr(offset), chunk_size);
                OpenLevel();
                printf("%sSkipping %d '%.4s' bytes.\n", indent, chunk_size, (char *)&fcc_id);
                CloseLevel();
                File64SetPos(in, chunk_size, SEEK_CUR);
                break;
        }

        offset = File64GetPos(in);   // get offset of the start of this chunk

    }
    return(0);
}



static void parse_riff(FILE *in)
{
    int fcc_id, fcc_type, riff_size, riff_count = 0;

    while ((fcc_id = ReadFCC(in, NULL)) != -1)  // should be RIFF
    {
        riff_size = read_long(in);

        if (fcc_id != 'RIFF')
        {
            if (riff_count == 0) printf("This is not a AVI/RIFF file.\n");
            else printf("Unexpected garbage detected at end of file.\n");
            return;
        }

        fcc_type = ReadFCC(in, NULL);
        switch (fcc_type)
        {
            case 'AVIX':
                // Set current base file pointer
                File64SetBase(in, -12);   // set to start of RIFF
                // fall through

            case 'AVI ':
                riff_count++;
                printf("%sRIFF#%d %.4s (Base=0x%s Length=0x%08X)\n", indent,
                    riff_count, (char *)&fcc_type, QWORD2HEX(File64GetBase()),
                    riff_size);
                OpenLevel();      // increase nested level
                ProcessAVI(in, riff_size); // Process AVI or AVIX
                CloseLevel();      // decrease nexted level
                break;

            default:
                printf("Unknown RIFF chunk.  Are you sure this is an AVI file?\n");
                break;
        }
    }

    return;
}




int main(int argc, char *argv[])
{
    FILE *in ;

    printf("\n"
           "Display the contents and file structure of an AVI file.\n"
           "This program will work on most AVI files including Open-DML\n"
           "files that are bigger than 4GB.\n\n"
           "RdAvi2 - RIFF AVI 2.0 Format Reader (April 18, 2024) By Dennis Hawkins\n"
           "Based on readavi by Michael Kohn (http://www.mikekohn.net)\n"
           "Copyright 2024 by Dennis Hawkins, BSD License applies.\n\n");

    if (argc != 2)
    {
        printf("Usage: readavi <filename>\n\n");
        exit(0);
    }

    in = File64Open(argv[1], "rb");
    if (in == 0)
    {
        printf("Could not open %s for input\n", argv[1]);
        exit(1);
    }


    parse_riff(in);

    File64Close(in);

    return 0;
}



