## Download from here: [Releases](https://github.com/nblockbuster/DestinyUnpackerCPP/releases/tag/s1.0)

# Destiny 2 Unpacker
Put oodle dll in same directory. The oodle dll is from Destiny 2 directory.

Valid for all Destiny 2 Day 1 packages from version 1.0.(1? 0?) onward.

Tested and works with any packages with a build date of 7/13/2017 at 12:29:40 PM (Beta packages I believe, need to modify encryption keys for the encryption)

This is a C++ "version" of my Python code that uses bcrypt for the AES-GCM-128 decryption.

Requires Sarge to build for command line args

-----

Usage: `DestinyUnpackerCPP-Shadowkeep.exe -p "path to packages" -i input package id -w (Converts to WAV & also deletes wems when done) -g (Extracts wems with wwise id in hex)`

(Package ID here refers to 023d or 0242 for example)

Modified to export .WEM files by nblock, with major help from Philip & HighRTT.

Reverted to shadowkeep package format with a ton of help from Philip & HighRTT as well.

Default output folder is in a folder called "`output pkgid`" in the same directory as the exe. 

-----

Obvious thanks to Mont for making his code public

Thanks to Philip for helping me to fix a bug that wouldnt allow it to extract files

Thanks to HighRTT for getting me the Shadowkeep and Forsaken package structure

## Licensing

This work is covered under GPLv3, although if derivative work is published or this work is used in another tool please provide a credit to myself or this page.
