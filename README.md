## Download from here: [Releases](https://github.com/nblockbuster/DestinyUnpackerCPP/releases/latest)

# Destiny 2 Unpacker

Valid for all Destiny 1 PS4/PS3 (untested X360) and Destiny 2 PC packages.

This is a C++ "version" of my Python code that uses bcrypt for the AES-GCM-128 decryption.

Requires [Sarge](https://github.com/MayaPosch/Sarge) & [Boost](https://www.boost.org/) to build.

-----
## Tutorial

1\. Open a command prompt in the folder that the exe is in by typing "cmd" in the file path at the top of the folder.

2\. Enter the two commands below as follows:

"\[exe file name] -p \[path to packages folder] -i \[package id]"  
 · For example, using the unpacker with Pre-BL packages, one might enter: `DestinyUnpackerCPP.exe -p "D:/Shadowkeep/packages" -i 0242 -v prebl`

`-p` "path" - follow this with the filepath of the packages folder within your Destiny 2 install directory.  

`-i` "input" - follow this with the package you want to unpack.  

Additional arguments:  

`-w` \- Converts wem files to wav files.  
`-h` \- hex id - Generates files named with a hex id. Much easier for sorting if you know how.  
You can combine this with other commands to generate hexID-named of a given type.  
`-t` \- Generates txtp files from the soundbanks in that package. "The way I see it it's a file referencing the actual wems, and telling vgmstream how to play them in what order."  
`-o` \- Specify an output folder.  
`-v` \- Change which version of destiny to extract from. (Valid inputs: d1, prebl)  
`-f` \- Tells the unpacker to unpack all of the packages in the packages path.  
`-m` \- Only extracts music files (Needs OSTs.db file in the same folder as exe)
`-u` \- Only extracts unknown files (anything that isnt a wem or bnk)  

-----

Obvious thanks to Mont for making his code public.

Thanks to Philip for helping me to get it extracting .WEMs.

Thanks to HighRTT for helping out a lot, and making the WEM to WAV converter library I used.  

Credit to OnscreenLoki for writing the tutorial.

-----

## Licensing

This work is covered under GPLv3, although if derivative work is published or this work is used in another tool please provide a credit to myself or this page.
