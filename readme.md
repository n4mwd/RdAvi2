# **RdAvi2 -- Read AVI 2.0**

#### AVI File Format Display Program - Written by Dennis Hawkins April 18, 2024.
#### Version 1.01 released on July 23, 2024

This program is a command line utility that will read an AVI file,
including those that are over 2GB in size, and display the internal AVI
file structure contents. Its purpose is for debugging and educational
purposes. It works for most AVI files, so generally speaking, if the
file extension is '.AVI' then it will probably work fine.

This program was developed in C using Borland C++ v5.02 running under
Wine on a Linux development system. So it runs under windows, from Win95
through Windows 11, or under Linux using Wine.

As of version 1.01, the program will also compile and run with Tiny C Compiler (TCC) 
and run directly under linux.

The TCC version does not currently support files over 4GB, but does fully support 
Open DML files less than 4GB.

It has not been compiled with GCC or CLANG, but these compilers have a similar syntax 
to TCC, so they should probably work without a lot of modification.  Consult the
rdavi2.h file for possible options.

To run it from a command prompt, enter the following:

C:\\\>  **RdAvi2 YourAviFile.avi**

To run the windows executable under Linux, wine must be installed, and then enter:

user@mx: \~\$ **wine RdAvi2.exe YourAviFile.avi**

There are currently no command line switches. Upon pressing enter, your
file will be loaded and then displayed in the command line console.
Because the output can sometimes be overwhelming and larger than the
console's look back buffer, it is recommended to redirect the output to
a file. Such as:

C:\\\> **RdAvi2 YourAviFile.avi  > output.txt**

If anyone has a suggestion for possible command line switches, let me know 
at n4mwd@yahoo.com .

## Sample Output:
The following is a snippet of an actual output:

    ================================

    Display the contents and file structure of an AVI file.
    This program will work on most AVI files including Open-DML
    files that are bigger than 4GB.

    RdAvi2 - RIFF AVI 2 Format Reader (April 18, 2024) By Dennis Hawkins
    Version 1.01 released on July 23, 2024 
    Based on readavi by Michael Kohn (http://www.mikekohn.net)
    Copyright 2024 by Dennis Hawkins, BSD License applies.

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

               *     *     *
    ====================================


### History

I needed to write an AVI library in C that would handle files bigger
than 2GB. I found several C libraries, but none were ODML or AVI 2.0
compatible. So in writing my own AVI library, I tried to use ReadAVI.exe
written by Michael Kohn to display the internal AVI file structures.
Unfortunately, while ReadAVI works on regular AVI files, it does not
work at all on AVI 2.0 files.

So I got sucked into a rabbit hole modifying Mr. Kohn's code to make it
work with the AVI 2.0 extensions. I say rabbit hole because it didn\'t
take long for me to realize that I wasn't modifying his code, but rather
rewriting it altogether. This is why the copyright says that this
program, RdAvi2.exe, was inspired by Mr. Kohn's code rather than a
modification.

Either way, the version of ReadAVI that I started with was licensed
under a BSD license, so I have preserved that here with my RdAvi2
program.

My program is brand new so there might be some bugs. I tested it with
every AVI file I could get my hands on, and they all worked. Let me know
if you find one that doesn't work right.

### Building from Sources

This project was designed to run on Windows from the command line. It
was developed on a Linux machine running Wine which runs code for
Windows. I used Borland C++ v5.02 for the compiler which is an
outstanding older compiler that is free to download and use. I have
included a compiler generated MAKE file, but basically, you just add all
the C source files to your project and then click compile. With Borland
C++ there are no other files necessary to run it, so no dotNet junk or
MsVcRt stuff to find and install. The exception is that if you are going
to run it under Linux, you will need to install WINE to enable Windows
compatibility.

Starting  with version 1.01, the program will compile into a native Linux 
executable with the Tiny C Compiler (TCC) and probably also GCC and CLANG. 
Use the following command line to compile with TCC:

    $> tcc -o rdavi2 -w codecs.c file64.c fileutil.c rdavi2.c

Borland C++ compiles ANSI C syntax so most other 32 bit ANSI C compilers
will likely work fine with little to no code modification. One thing
in version 1.0 that didn't work right in other compilers is the way that 
I handle FourCC codes and tags.

I make extensive use of the C language's support for multi-character literals.
This is where a character literal that would normally be something like 'A' 
is extended to four characters such as 'RIFF'.  This is allowed because the
CPU running the code is at least 32 bits (4 bytes).

Note that a multi-character literal is not the same thing as a C string.  A
C string would be defined with double quotes like "RIFF".  Using multi-character
literals makes the program very efficient because now simple 'if' statements 
will work as well as 'switch()' statements.  So the following construct works:

    switch (varstr)
    {
        case 'RIFF':
           // do something
           break;
           
        case 'LIST':
            // do something
            break;
    }

The only problem with multi-character literals is that for whatever reason, 
the byte ordering has not been standardized.  So a compiler like Borland orders 
the DWORD in Little Endian order, the same as the x86 processor, but other 
compilers, like TCC, order the DWORD in Big Endian order.

So the code compiled fine in Borland, but bombed in TCC.  To get around this, I 
created a macro that automatically detects which Endianess is being used.  If 
Big Endian order is detected, the macro reverses the order into Little Endian.

Another major difference in the way Borland and TCC compile is how structures 
are packed.  I found that TCC was distorting structures rather than just aligning 
them as a whole.  Borland doesn't distort structures.  To get around this, it 
was necessary to use a pragma to effectively disable packing and alignment when
using TCC.


### BSD License

RdAvi2.exe - Copyright (c) 2024 by Dennis Hawkins. <n4mwd@yahoo.com> All rights reserved.<br />
Inspired by: ReadAvi.exe by Michael Kohn <mike@mikekohn.net> (http://www.mikekohn.net/)<br />
Although not required, attribution is requested for any source code used
by others.

Redistribution and use in source and binary forms are permitted provided
that the above copyright notice and this paragraph are duplicated in all
such forms and that any documentation, advertising materials, and other
materials related to such distribution and use acknowledge that the
software was developed by the copyright holder. The name of the
copyright holder may not be used to endorse or promote products derived
from this software without specific prior written permission. THIS
SOFTWARE IS PROVIDED \`AS IS\' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE.

