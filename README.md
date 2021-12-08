## Download from here: [Releases](https://github.com/nblockbuster/DestinyUnpackerCPP/releases/tag/v1.2)

# Destiny 2 Unpacker
Put oodle dll in same directory. The oodle dll is from Destiny 2 directory.

Valid for all Destiny 2: Beyond Light packages from version 3.0.0.0 onward.

This is a C++ "version" of my Python code that uses bcrypt for the AES-GCM-128 decryption.

Requires Sarge & Boost to build

-----

Usage: `DestinyUnpackerCPP.exe -p "path to packages" -i input package id -w (converts audio to wav) -o (converts audio to ogg) -t (generates a foobar2000 & vgmstream compatible .txtp file for playing full tracks)`

(Package ID here refers to 02f6 or 0144 for example)

Modified to export .WEM files by nblock, with major help from Philip & HighRTT

Default output folder is in a folder called "`output pkgid`" in the same directory as the exe. 

-----

Obvious thanks to Mont for making his code public

Thanks to Philip for helping me to get it extracting .WEMs

Thanks to HighRTT for helping out a lot

## Licensing

This work is covered under GPLv3, although if derivative work is published or this work is used in another tool please provide a credit to myself or this page.
