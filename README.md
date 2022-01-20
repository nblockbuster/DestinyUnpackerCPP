## Download from here: [Releases](https://github.com/nblockbuster/DestinyUnpackerCPP/releases/tag/1.3-PreBL)

# Destiny 2 Unpacker
Put oodle dll in same directory. The oodle dll is from Destiny 2 directory.

Valid for all Destiny 2 packages from version 1.0.0 up to verion 3.0.0.0.

Tested and works with any packages with a build date of 7/13/2017 to 8/25/2020 (Release packages to last update before Beyond Light)

This is a C++ "version" of my Python code that uses bcrypt for the AES-GCM-128 decryption.

Requires Sarge to build for command line args

-----

Usage: `DestinyUnpackerCPP-PreBL.exe -p "path to packages" -i input package id 
-w (converts audio to wav) -o (converts audio to ogg) 
-t (generates a foobar2000 & vgmstream compatible .txtp file for playing full tracks) 
-h (outputs wem/wavs/oggs in hex`

(Package ID here refers to 023d or 0242 for example)

Modified to export .WEM files by nblock, with major help from Philip & HighRTT.

Reverted to shadowkeep package format with a ton of help from Philip & HighRTT as well.

Default output folder is in a folder called "`output pkgid`" in the same directory as the exe. 

-----

Obvious thanks to Mont for making his code public

Thanks to Philip for helping me to fix a bug that wouldnt allow it to extract files

Thanks to HighRTT for helping me with the Shadowkeep and Forsaken package structure, and making the .wem to .wav conversion library

-----

## Licensing

This work is covered under GPLv3, although if derivative work is published or this work is used in another tool please provide a credit to myself or this page.
