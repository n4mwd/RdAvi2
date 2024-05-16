
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

This file handles CODEC and other lookups.

*/



#include "rdavi2.h"

typedef struct
{
    DWORD fcc;
    char  *Desc;
} CODEC_DESC;


// Registered FourCC codes from:
// https://www.iana.org/assignments/wave-avi-codec-registry/wave-avi-codec-registry.xhtml

static CODEC_DESC CodecTbl[] =
{
    { 0,      "NONE Specified" },
    { 'ANIM', "Intel RDX" },
    { 'AUR2', "AuraVision Aura 2" },
    { 'AURA', "AuraVision Aura 1" },
    { 'BT20', "Brooktree MediaStream" },
    { 'BTCV', "Brooktree Composite Video" },
    { 'CC12', "Intel YUV12" },
    { 'CDVC', "Canopus DV" },
    { 'CHAM', "Winnov Caviara Cham" },
    { 'CLJR', "Proprietary YUV 4 pixels" },
    { 'CMYK', "Common Data Format in Printing" },
    { 'CPLA', "Weitek 4:2:0 YUV Planar" },
    { 'CVID', "Cinepak by Supermac" },
    { 'CWLT', "Microsoft Color WLT DIB" },
    { 'CYUV', "Creative Labs YUV" },
    { 'D261', "H.261" },
    { 'D263', "H.263" },
    { 'DIV3', "Low motion DivX MPEG-4" },
    { 'DIV4', "Fast motion DivX MPEG-4" },
    { 'DUCK', "True Motion 1.0" },
    { 'DVE2', "DVE-2 Videoconferencing" },
    { 'FLJP', "Field Encoded Motion JPEG" },
    { 'FVF1', "Fractal Video Frame" },
    { 'GWLT', "Microsoft Greyscale WLT DIB" },
    { 'H260', "H.260" },
    { 'H261', "H.261" },
    { 'H262', "H.262" },
    { 'H263', "H.263" },
    { 'H264', "H.264" },
    { 'H265', "H.265" },
    { 'H266', "H.266" },
    { 'H267', "H.267" },
    { 'H268', "H.268" },
    { 'H269', "H.260" },
    { 'I263', "I263" },
    { 'I420', "Intel Indeo 4" },
    { 'IAN ', "Intel RDX" },
    { 'ICLB', "CellB Videoconferencing Codec" },
    { 'ILVC', "Intel Layered Video" },
    { 'ILVR', "ITU-T H.263+" },
    { 'IRAW', "Intel YUV Uncompressed" },
    { 'IV30', "Intel Indeo Video 3" },
    { 'IV31', "Intel Indeo Video 3.1" },
    { 'IV32', "Intel Indeo Video 3.2" },
    { 'IV33', "Intel Indeo Video 3.3" },
    { 'IV34', "Intel Indeo Video 3.4" },
    { 'IV35', "Intel Indeo Video 3.5" },
    { 'IV36', "Intel Indeo Video 3.6" },
    { 'IV37', "Intel Indeo Video 3.7" },
    { 'IV38', "Intel Indeo Video 3.8" },
    { 'IV39', "Intel Indeo Video 3.9" },
    { 'IV40', "Intel Indeo Video 4.0" },
    { 'IV41', "Intel Indeo Video 4.1" },
    { 'IV42', "Intel Indeo Video 4.2" },
    { 'IV43', "Intel Indeo Video 4.3" },
    { 'IV44', "Intel Indeo Video 4.4" },
    { 'IV45', "Intel Indeo Video 4.5" },
    { 'IV46', "Intel Indeo Video 4.6" },
    { 'IV47', "Intel Indeo Video 4.7" },
    { 'IV48', "Intel Indeo Video 4.8" },
    { 'IV49', "Intel Indeo Video 4.9" },
    { 'IV50', "Intel Indeo Video 5.0" },
    { 'JPEG', "Still Image JPEG DIB" },
    { 'MJPG', "Motion JPEG DIB" },
    { 'MP42', "Microsoft MPEG-4 Video Codec" },
    { 'MPEG', "MPEG 1 Video Frame" },
    { 'MRCA', "MR Codec" },
    { 'MRLE', "Run Length Encoding" },
    { 'MSVC', "Video 1" },
    { 'PHMO', "Photomotion" },
    { 'QPEQ', "QPEG 1.1 Format Video" },
    { 'RGBT', "RGBT" },
    { 'RLE4', "Run Length Encoded 4" },
    { 'RLE8', "Run Length Encoded 8" },
    { 'RT21', "Indeo 2.1" },
    { 'RVX ', "Intel RDX" },
    { 'SDCC', "Sun Digital Camera Codec" },
    { 'SFMC', "Crystal Net SFM Codec" },
    { 'SMSC', "SMSC" },
    { 'SMSD', "SMSD" },
    { 'SPLC', "Splash Studios ACM Audio Codec" },
    { 'SQZ2', "Microsoft VXtreme Video Codec" },
    { 'SV10', "Sorenson Video R1" },
    { 'TLMS', "TeraLogic Motion Infraframe Codec A" },
    { 'TLST', "TeraLogic Motion Infraframe Codec B" },
    { 'TM20', "TrueMotion 2.0" },
    { 'TMIC', "TeraLogic Motion Intraframe Codec 2" },
    { 'TMOT', "TrueMotion Video Compression" },
    { 'TR20', "TrueMotion RT 2.0" },
    { 'ULTI', "Ultimotion" },
    { 'UYVY', "UYVY 4:2:2 byte ordering" },
    { 'V422', "24 bit YUV 4:2:2 Format" },
    { 'V655', "16 bit YUV 4:2:2 Format" },
    { 'VCR1', "ATI VCR 1.0" },
    { 'VCR2', "ATI VCR 2.0" },
    { 'VCR3', "ATI VCR 3.0" },
    { 'VCR4', "ATI VCR 4.0" },
    { 'VCR5', "ATI VCR 5.0" },
    { 'VCR6', "ATI VCR 6.0" },
    { 'VCR7', "ATI VCR 7.0" },
    { 'VCR8', "ATI VCR 8.0" },
    { 'VCR9', "ATI VCR 9.0" },
    { 'VDCT', "Video Maker Pro DIB" },
    { 'VIDS', "YUV 4:2:2 CCIR 601 for V422" },
    { 'VIVO', "Vivo H.263" },
    { 'VIXL', "VIXL" },
    { 'VLV1', "VLCAP.DRV" },
    { 'WBVC', "W9960" },
    { 'X263', "X263" },
    { 'XLV0', "XL Video Decoder" },
    { 'Y211', "YUV 2:1:1 Packed" },
    { 'Y411', "YUV 4:1:1 Packed" },
    { 'Y41B', "YUV 4:1:1 Planar" },
    { 'Y41P', "PC1 4:1:1" },
    { 'Y41T', "PC1 4:1:1 with transparency" },
    { 'Y42B', "YUV 4:2:2 Planar" },
    { 'Y42T', "PCI 4:2:2 with transparency" },
    { 'YC12', "Intel YUV12 Codec" },
    { 'YUV8', "Winnov Caviar YUV8" },
    { 'YUV9', "YUV9" },
    { 'YUY2', "YUYV 4:2:2 byte ordering packed" },
    { 'YUYV', "BI_YUYV, Canopus" },
    { 'YV12', "YVU12 Planar" },
    { 'YVU9', "YVU9 Planar" },
    { 'YVYU', "YVYU 4:2:2 byte ordering" },
    { 'ZPEG', "Video Zipper" },
    { NULL,   NULL }
};

// Registered formats from:
// https://www.iana.org/assignments/wave-avi-codec-registry/wave-avi-codec-registry.xhtml

CODEC_DESC AudTbl[] =
{
    { 0x0000, "Microsoft Unknown Wave Format" },    // WAVE_FORMAT_UNKNOWN
    { 0x0001, "Microsoft PCM Format" },              // WAVE_FORMAT_PCM
    { 0x0002, "Microsoft ADPCM Format" },    // WAVE_FORMAT_ADPCM
    { 0x0003, "IEEE Float" },    // WAVE_FORMAT_IEEE_FLOAT
    { 0x0004, "Compaq Computer's VSELP" },    // WAVE_FORMAT_VSELP
    { 0x0005, "IBM CVSD" },    // WAVE_FORMAT_IBM_CVSD
    { 0x0006, "Microsoft ALAW" },    // WAVE_FORMAT_ALAW
    { 0x0007, "Microsoft MULAW" },    // WAVE_FORMAT_MULAW
    { 0x0010, "OKI ADPCM" },    // WAVE_FORMAT_OKI_ADPCM
    { 0x0011, "Intel's DVI ADPCM" },    // WAVE_FORMAT_DVI_ADPCM
    { 0x0012, "Videologic's MediaSpace ADPCM" },    // WAVE_FORMAT_MEDIASPACE_ADPCM
    { 0x0013, "Sierra ADPCM" },    // WAVE_FORMAT_SIERRA_ADPCM
    { 0x0014, "G.723 ADPCM" },    // WAVE_FORMAT_G723_ADPCM
    { 0x0015, "DSP Solution's DIGISTD" },    // WAVE_FORMAT_DIGISTD
    { 0x0016, "DSP Solution's DIGIFIX" },    // WAVE_FORMAT_DIGIFIX
    { 0x0017, "Dialogic OKI ADPCM" },    // WAVE_FORMAT_DIALOGIC_OKI_ADPCM
    { 0x0018, "MediaVision ADPCM" },    // WAVE_FORMAT_MEDIAVISION_ADPCM
    { 0x0019, "HP CU" },    // WAVE_FORMAT_CU_CODEC
    { 0x0020, "Yamaha ADPCM" },    // WAVE_FORMAT_YAMAHA_ADPCM
    { 0x0021, "Speech Compression's Sonarc" },    // WAVE_FORMAT_SONARC
    { 0x0022, "DSP Group's True Speech" },    // WAVE_FORMAT_DSPGROUP_TRUESPEECH
    { 0x0023, "Echo Speech's EchoSC1" },    // WAVE_FORMAT_ECHOSC1
    { 0x0024, "Audiofile AF36" },    // WAVE_FORMAT_AUDIOFILE_AF36
    { 0x0025, "APTX" },    // WAVE_FORMAT_APTX
    { 0x0026, "AudioFile AF10" },    // WAVE_FORMAT_AUDIOFILE_AF10
    { 0x0027, "Prosody 1612" },    // WAVE_FORMAT_PROSODY_1612
    { 0x0028, "LRC" },    // WAVE_FORMAT_LRC
    { 0x0030, "Dolby AC2" },    // WAVE_FORMAT_DOLBY_AC2
    { 0x0031, "GSM610" },    // WAVE_FORMAT_GSM610
    { 0x0032, "MSNAudio" },    // WAVE_FORMAT_MSNAUDIO
    { 0x0033, "Antex ADPCME" },    // WAVE_FORMAT_ANTEX_ADPCME
    { 0x0034, "Control Res VQLPC" },    // WAVE_FORMAT_CONTROL_RES_VQLPC
    { 0x0035, "Digireal" },    // WAVE_FORMAT_DIGIREAL
    { 0x0036, "DigiADPCM" },    // WAVE_FORMAT_DIGIADPCM
    { 0x0037, "Control Res CR10" },    // WAVE_FORMAT_CONTROL_RES_CR10
    { 0x0038, "NMS VBXADPCM" },    // WAVE_FORMAT_NMS_VBXADPCM
    { 0x0039, "Roland RDAC" },    // WAVE_FORMAT_ROLAND_RDAC
    { 0x003A, "EchoSC3" },    // WAVE_FORMAT_ECHOSC3
    { 0x003B, "Rockwell ADPCM" },    // WAVE_FORMAT_ROCKWELL_ADPCM
    { 0x003C, "Rockwell Digit LK" },    // WAVE_FORMAT_ROCKWELL_DIGITALK
    { 0x003D, "Xebec" },    // WAVE_FORMAT_XEBEC
    { 0x0040, "Antex Electronics G.721" },    // WAVE_FORMAT_G721_ADPCM
    { 0x0041, "G.728 CELP" },    // WAVE_FORMAT_G728_CELP
    { 0x0042, "MSG723" },    // WAVE_FORMAT_MSG723
    { 0x0050, "MPEG" },    // WAVE_FORMAT_MPEG
    { 0x0052, "RT24" },    // WAVE_FORMAT_RT24
    { 0x0053, "PAC" },    // WAVE_FORMAT_PAC
    { 0x0055, "MPEG Layer 3" },    // WAVE_FORMAT_MPEGLAYER3
    { 0x0059, "Lucent G.723" },    // WAVE_FORMAT_LUCENT_G723
    { 0x0060, "Cirrus" },    // WAVE_FORMAT_CIRRUS
    { 0x0061, "ESPCM" },    // WAVE_FORMAT_ESPCM
    { 0x0062, "Voxware" },    // WAVE_FORMAT_VOXWARE
    { 0x0063, "Canopus Atrac" },    // WAVE_FORMAT_CANOPUS_ATRAC
    { 0x0064, "G.726 ADPCM" },    // WAVE_FORMAT_G726_ADPCM
    { 0x0065, "G.722 ADPCM" },    // WAVE_FORMAT_G722_ADPCM
    { 0x0066, "DSAT" },    // WAVE_FORMAT_DSAT
    { 0x0067, "DSAT Display" },    // WAVE_FORMAT_DSAT_DISPLAY
    { 0x0069, "Voxware Byte Aligned" },    // WAVE_FORMAT_VOXWARE_BYTE_ALIGNED
    { 0x0070, "Voxware AC8" },    // WAVE_FORMAT_VOXWARE_AC8
    { 0x0071, "Voxware AC10" },    // WAVE_FORMAT_VOXWARE_AC10
    { 0x0072, "Voxware AC16" },    // WAVE_FORMAT_VOXWARE_AC16
    { 0x0073, "Voxware AC20" },    // WAVE_FORMAT_VOXWARE_AC20
    { 0x0074, "Voxware MetaVoice" },    // WAVE_FORMAT_VOXWARE_RT24
    { 0x0075, "Voxware MetaSound" },    // WAVE_FORMAT_VOXWARE_RT29
    { 0x0076, "Voxware RT29HW" },    // WAVE_FORMAT_VOXWARE_RT29HW
    { 0x0077, "Voxware VR12" },    // WAVE_FORMAT_VOXWARE_VR12
    { 0x0078, "Voxware VR18" },    // WAVE_FORMAT_VOXWARE_VR18
    { 0x0079, "Voxware TQ40" },    // WAVE_FORMAT_VOXWARE_TQ40
    { 0x0080, "Softsound" },    // WAVE_FORMAT_SOFTSOUND
    { 0x0081, "Voxware TQ60" },    // WAVE_FORMAT_VOXWARE_TQ60
    { 0x0082, "MSRT24" },    // WAVE_FORMAT_MSRT24
    { 0x0083, "G.729A" },    // WAVE_FORMAT_G729A
    { 0x0084, "MVI MV12" },    // WAVE_FORMAT_MVI_MV12
    { 0x0085, "DF G.726" },    // WAVE_FORMAT_DF_G726
    { 0x0086, "DF GSM610" },    // WAVE_FORMAT_DF_GSM610
    { 0x0088, "ISIAudio" },    // WAVE_FORMAT_ISIAUDIO
    { 0x0089, "Onlive" },    // WAVE_FORMAT_ONLIVE
    { 0x0091, "SBC24" },    // WAVE_FORMAT_SBC24
    { 0x0092, "Dolby AC3 SPDIF" },    // WAVE_FORMAT_DOLBY_AC3_SPDIF
    { 0x0097, "ZyXEL ADPCM" },    // WAVE_FORMAT_ZYXEL_ADPCM
    { 0x0098, "Philips LPCBB" },    // WAVE_FORMAT_PHILIPS_LPCBB
    { 0x0099, "Packed" },    // WAVE_FORMAT_PACKED
    { 0x0100, "Rhetorex ADPCM" },    // WAVE_FORMAT_RHETOREX_ADPCM
    { 0x0101, "BeCubed Software's IRAT" },    // WAVE_FORMAT_IRAT
    { 0x0011, "Vivo G.723" },    // WAVE_FORMAT_VIVO_G723
    { 0x0112, "Vivo Siren" },    // WAVE_FORMAT_VIVO_SIREN
    { 0x0123, "Digital G.723" },    // WAVE_FORMAT_DIGITAL_G723
    { 0x0200, "Creative ADPCM" },    // WAVE_FORMAT_CREATIVE_ADPCM
    { 0x0202, "Creative FastSpeech8" },    // WAVE_FORMAT_CREATIVE_FASTSPEECH8
    { 0x0203, "Creative FastSpeech10" },    // WAVE_FORMAT_
    { 0x0220, "Quarterdeck" },    // WAVE_FORMAT_QUARTERDECK
    { 0x0300, "FM Towns Snd" },    // WAVE_FORMAT_FM_TOWNS_SND
    { 0x0400, "BTV Digital" },    // WAVE_FORMAT_BTV_DIGITAL
    { 0x0680, "VME VMPCM" },    // WAVE_FORMAT_VME_VMPCM
    { 0x1000, "OLIGSM" },    // WAVE_FORMAT_OLIGSM
    { 0x1001, "OLIADPCM" },    // WAVE_FORMAT_OLIADPCM
    { 0x1002, "OLICELP" },    // WAVE_FORMAT_OLICELP
    { 0x1003, "OLISBC" },    // WAVE_FORMAT_OLISBC
    { 0x1004, "OLIOPR" },    // WAVE_FORMAT_OLIOPR
    { 0x1100, "LH Codec" },    // WAVE_FORMAT_LH_CODEC
    { 0x1400, "Norris" },    // WAVE_FORMAT_NORRIS
    { 0x1401, "ISIAudio" },    // WAVE_FORMAT_ISIAUDIO
    { 0x1500, "Soundspace Music Compression" },    // WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS
    { 0x2000, "AC3 DVM" },    // WAVE_FORMAT_DVM
    { 0x2001, "DTS" },        // WAVE_FORMAT_DTS
    { 0xFFFE, "Wave Format Extensible" }, // WAVE_FORMAT_EXTENSIBLE - special format flag
    { NULL, NULL }
};


static CODEC_DESC InfoTbl[] =
{
    { 0,      "None Specified" },
    { 'IARL', "Archival Location" },
    { 'IART', "Artist" },
    { 'ICMS', "Commissioned" },
    { 'ICMT', "Comment" },
    { 'ICOP', "Copyright" },
    { 'ICRD', "Creation date" },
    { 'ICRP', "Cropped" },
    { 'IDIM', "Dimensions" },
    { 'IDPI', "Dots Per Inch" },
    { 'IENG', "Engineer" },
    { 'IGNR', "Genre" },
    { 'IKEY', "Keywords" },
    { 'ILGT', "Lightness" },
    { 'IMED', "Storage Medium" },
    { 'INAM', "Name" },
    { 'IPLT', "Num Palette Colors" },
    { 'IPRD', "Product" },
    { 'ISBJ', "Subject" },
    { 'ISFT', "Software" },
    { 'ISHP', "Sharpness" },
    { 'ISRC', "Source" },
    { 'ISRF', "Source Form" },
    { 'ITCH', "Technician" },
    { NULL, NULL }
};


// Main search
char *LookupFCCsub(DWORD inFcc, CODEC_DESC *Tbl, char *NotFoundStr)
{
    int i;

    // Do a simple linear search because its only done once in the program
    for (i = 0; Tbl[i].Desc; i++)
    {
        if (Tbl[i].fcc == inFcc)
            return(Tbl[i].Desc);
    }

    return(NotFoundStr);
}


// Return the description of the FourCC codec
// or NULL if not found.

char *LookupFourCC(DWORD inFcc)
{
    int i;
    char *str;

    // convert to all caps - can't use strupr() because its not a string.
    str = (char *) &inFcc;
    for (i = 0; i < 4; i++)
        str[i] = (char) toupper(str[i]);

    return(LookupFCCsub(inFcc, CodecTbl, "Unknown FourCC"));
}

// lookup audio handler

char *LookupFormat(DWORD FmtNum)
{
    return(LookupFCCsub(FmtNum, AudTbl, "Unknown Format"));
}


// Return the description of the INFO list element

char *LookupINFO(DWORD inInfo)
{
    return(LookupFCCsub(inInfo, InfoTbl, "Unknown INFO element"));
}



