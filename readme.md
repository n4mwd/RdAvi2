# **RdAvi2 -- Read AVI 2.0**

#### AVI File Format Display Program - Written by Dennis Hawkins April 18, 2024.

This program is a command line utility that will read an AVI file,
including those that are over 2GB in size, and display the internal AVI
file structure contents. Its purpose is for debugging and educational
purposes. It works for most AVI files, so generally speaking, if the
file extension is '.AVI' then it will probably work fine.

This program was developed in C using Borland C++ v5.02 running under
Wine on a Linux development system. So it runs under windows, from Win95
through Windows 11, or under Linux using Wine.

To run it from a windows command prompt, enter the following:

C:\\\>  **RdAvi2 YourAviFile.avi**

To run under Linux, wine must be installed, and then enter:

user@mx: \~\$ **wine RdAvi2.exe YourAviFile.avi**

There are currently no command line switches. Upon pressing enter, your
file will be loaded and then displayed in the command line console.
Because the output can sometimes be overwhelming and larger than the
console's look back buffer, it is recommended to redirect the output to
a file. Such as:

C:\\\> **RdAvi2 YourAviFile.avi  > output.txt**

## Sample Output:
    ================================

    Display the contents and file structure of an AVI file.
    This program will work on most AVI files including Open-DML
    files that are bigger than 4GB.

    RdAvi2 - RIFF AVI 2.0 Format Reader (April 18, 2024) By Dennis Hawkins
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

Borland C++ compiles ANSI C syntax so most other 32 bit ANSI C compilers
will likely work fine with little to no code modification. One thing
that might not work right in other compilers is the way that I handle
FourCC codes and tags.

When I read a FourCC tag into memory, I read it as a 4 character string
buffer, then cast that to a DWORD for storage. Then, later in the
program when I need to compare that to a known tag value, I overstuff
the character quotes to form a multi-character literal. So if I have the
tag from the file in dwTag, I can simply compare it to say 'RIFF',
like:

``` 
if (dwTag == ‘RIFF’) <do something>
```
I have seen other people's code where they painstakingly manually
reverse the buffer characters after reading, and then set up some sort
of macro to reverse multi-character literals so that the order matches.
I skipped all the reversing stuff and everything worked out just fine
with the Borland compiler. I mention this because I don't think the way
Borland handles multi-character literals is a standard so other
compilers may fail with this code. In that case, a reversing macro may
be necessary for multi-character literals used in this program.

### BSD License

RdAvi2.exe - Copyright (c) 2024 by Dennis Hawkins. All rights reserved.<br />
Inspired by: ReadAvi.exe by Michael Kohn\<mike@mikekohn.net\>(http://www.mikekohn.net/)<br />
Although not required, attribution is requested for any source code used
by others.

Redistribution and use in source and binary forms are permitted provided
that the above copyright notice and this paragraph are duplicated in all
such forms and that any documentation, advertising materials, and other
materials related to such distribution and use acknowledge that the
software was developed by the copyright holder. The name of the
copyright holder may not be used to endorse or promote products derived
from this software without specific prior written permission. THIS
SOFTWARE IS PROVIDED \`\'AS IS? AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

