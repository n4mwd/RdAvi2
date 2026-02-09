# **RdAvi2 -- Read AVI 2.0**

#### AVI File Format Display Program - Written by Dennis Hawkins April 18, 2024.
#### Version 1.0.2 released on February 9, 2026

This program is a command line utility that will read an AVI file, including those that are over 2GB in size, and display the internal AVI file structure contents. Its purpose is for debugging and educational purposes. It works for most AVI files, so generally speaking, if the file extension is '.AVI' then it will probably work fine.

This program was developed in C using Borland C++ v5.02 running under Wine on a Linux development system. So it runs under Windows, from Win95 through Windows 11, or under Linux using Wine.

Starting in version 1.0.2, it can be compiled by GCC or Tiny C for Linux and Borland C or MinGW for Windows. MinGW will run on both Linux and Windows to produce a Windows executable. With the exception of Borland, which is 32-bit only, the program will compile both as a 32-bit and 64-bit executable.

## Usage

To run it from a Windows command prompt, enter the following:

>  RdAvi2 YourAviFile.avi

To run it from a Linux command prompt, enter:

>  ./RdAvi2.exe YourAviFile.avi

### Command Line Switches
There is one command line switch. Normally RdAvi2 will suppress movi and index output after about 10 lines. In some cases, it is necessary to view all lines. To enable this, the '-a' switch will disable truncation and display all movi and index lines.

Upon pressing enter, your file will be loaded and then displayed in the command line console. Because the output can sometimes be overwhelming and larger than the console's look-back buffer, it is recommended to redirect the output to a file:

>  RdAvi2 YourAviFile.avi > output.txt

---

## Changes from Version 1.0.1

In addition to making the code compile with multiple compilers, I changed the index listing output a bit to make the information more useful. The offset and the absolute addresses are now printed. In addition, the base address used is also printed.

---

## Building from Sources

### General 
The program uses multi-character literal constants extensively to facilitate efficient coding. These allow FourCC comparisons with a single machine cycle rather than a function call. Unfortunately, with the exception of Borland, all of the compilers I have used do not properly support these. Instead, they reverse the bytes which necessitates the use of a FIT_LIT() macro to put things in the correct order. Unfortunately, there is no C standard for how they should be ordered. In addition, most compilers will generate warnings about the use of multi-character literals, but fear not, I know what I'm doing.

Borland C v5.02 is the only supported compiler that does not generate 64-bit executables. To achieve 64-bit file support, it leverages the Windows API. All other compilers, including 32-bit ones, have full native 64-bit file support.

For Linux, I use MX Linux which is a Debian derivative.  The code has not been tested on other versions of Linux like Arch, Red Hat or Open BSD.  But building it from sources should work fine.

### Compilation Commands

#### Borland C (Windows)

>  ``` bcc32 rdavi2.c codecs.c fileutil.c file64.c```

#### MinGW (Windows) - 32-bit

>  ``` i686-w64-mingw32-gcc file64.c codecs.c rdavi2.c fileutil.c -o rdavi2.exe```

#### GCC (Linux) - 32-bit

>  ``` gcc -m32 file64.c codecs.c rdavi2.c fileutil.c -o rdavi2```

#### GCC (Linux) - 64-bit

>  ``` gcc -m64 file64.c codecs.c rdavi2.c fileutil.c -o rdavi2```

#### TINY C (Linux) - 32-bit

>  ``` tcc -m32 -w file64.c codecs.c rdavi2.c fileutil.c -o rdavi2```

#### TINY C (Linux) - 64-bit

>  ``` tcc -m64 -w file64.c codecs.c rdavi2.c fileutil.c -o rdavi2```

Note: Both 32-bit and 64-bit builds are fully supported. The code automatically adapts based on available functions.

If anyone has a suggestion for possible command line switches, let me know at n4mwd@yahoo.com.



## Sample Output
The following is a snippet of an actual output:

---

```text

Display the contents and file structure of an AVI file.
This program will work on most AVI files including Open-DML
files that are bigger than 4GB.

RdAvi2 - RIFF AVI 2 Format Reader (April 18, 2024) By Dennis Hawkins
Version 1.0.2 released on February 9, 2026 
Based on readavi by Michael Kohn (http://www.mikekohn.net)
Copyright 2026 by Dennis Hawkins, BSD License applies.

RIFF#1 AVI (Base=0x0000000000000000 Length=0x0012D090)
{
  AVI LIST 'hdrl' (Location=0x0000000C length=0x000124)
  {
    AVI Main Header 'avih' (Location=0x00000018 length=0x000038)
    {
             offset=0x20
          TimeBetweenFrames: 66666
            MaximumDataRate: 147882
         PaddingGranularity: 0
                      Flags: 00000810 - IDX CKOK
        TotalNumberOfFrames: 127
      NumberOfInitialFrames: 0
            NumberOfStreams: 2
        SuggestedBufferSize: 47040
                      Width: 320
                     Height: 240
    }
    AVI LIST 'hdrl' Element 'strl' (Location=0x00000058 length=0x000074)
    {
      AVI 'strh' Stream Header for 'vids' (Location=0x00000064 length=0x000038)
      {
            offset=0x6c
        Stream Header Version: vids (56 byte) version
                  FourCC Type: vids
               FourCC Handler: cvid - Cinepak by Supermac
                        Flags: 00000000 - No Flags Set
                     Priority: 0
                InitialFrames: 0
                    TimeScale: 66666
                     DataRate: 1000000
                    StartTime: 0
                   DataLength: 127
          SuggestedBufferSize: 7472
                      Quality: 10000
                   SampleSize: 0
                        Frame: { Top: 0, Left: 0, Bottom: 240, Right: 320 }
      }
      AVI 'strf' Stream Format for 'vids' (Location=0x000000A4 length=0x000028)
      {
            offset=0xac
                header_size: 40
                image_width: 320
               image_height: 240
           number_of_planes: 1
             bits_per_pixel: 24
           compression_type: cvid - Cinepak by Supermac
        image_size_in_bytes: 29700
           x_pels_per_meter: 0

           * * *

```
---




## History

I needed to write an AVI library in C that would handle files bigger than 2GB. I found several C libraries, but none were ODML or AVI 2.0 compatible. So in writing my own AVI library, I tried to use ReadAVI.exe written by Michael Kohn to display the internal AVI file structures. Unfortunately, while ReadAVI works on regular AVI files, it does not work at all on AVI 2.0 files.

So I got sucked into a rabbit hole modifying Mr. Kohn's code to make it work with the AVI 2.0 extensions. I say rabbit hole because it didn't take long for me to realize that I wasn't modifying his code, but rather rewriting it altogether. This is why the copyright says that this program, RdAvi2.exe, was inspired by Mr. Kohn's code rather than a modification.

Either way, the version of ReadAVI that I started with was licensed under a BSD license, so I have preserved that here with my RdAvi2 program.

---

## BSD License

RdAvi2.exe - Copyright (c) 2024-2026 by Dennis Hawkins <n4mwd@yahoo.com>. All rights reserved.  
Inspired by: ReadAvi.exe by Michael Kohn (http://www.mikekohn.net/) <mike@mikekohn.net>

Redistribution and use in source and binary forms are permitted provided that the above copyright notice and this paragraph are duplicated in all such forms and that any documentation, advertising materials, and other materials related to such distribution and use acknowledge that the software was developed by the copyright holder. The name of the copyright holder may not be used to endorse or promote products derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED 'AS IS' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

