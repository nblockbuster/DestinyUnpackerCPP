# Destiny 2 Unpacker
Put oodle dll in same directory. The oodle dll is from Destiny 2 directory.

Valid for all Destiny 2: Beyond Light packages from version 3.0.0.0 onward.

This is a C++ "version" of my Python code that uses bcrypt for the AES-GCM-128 decryption.

-----

Usage: `DestinyUnpackerCPP.exe -p "path to packages"`
It'll then ask for what package id you want to extract (eg. 02f6 or 0144)

Requires Sarge to build for command line args
Modified to export .WEM files by nblock, with major help from Philip

Default output folder is in a folder called "`output pkgid`" in the same directory as the exe. 

-----

Thanks to Philip for helping me to get it extracting .WEMs
Thanks to HighRTT for letting me use his WEM to WAV converter library

## Licensing

This work is covered under GPLv3, although if derivative work is published or this work is used in another tool please provide a credit to myself or this page.
