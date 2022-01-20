## Download from here: [Releases](https://github.com/nblockbuster/DestinyUnpackerCPP/releases/tag/v1.3)

# Destiny 2 Unpacker
Put oodle dll in same directory. The oodle dll is from Destiny 2 directory.

Valid for all Destiny 2: Beyond Light packages from version 3.0.0.0 onward.

This is a C++ "version" of my Python code that uses bcrypt for the AES-GCM-128 decryption.

Requires [Sarge](https://github.com/MayaPosch/Sarge), [Boost](https://www.boost.org/) & [ww2ogg](https://github.com/hcs64/ww2ogg) to build.

(May trigger a virus warning, because I use the Vorbis exe instead of the library, [here's the virustotal link.](https://www.virustotal.com/gui/file/065d95aa0681c0fb275216dc20621756f4cbe45b85c1b65b0bde9c3720cd2374) 

-----

Usage: `DestinyUnpackerCPP.exe -p "path to packages" -i input package id 
-w (converts audio to wav) -o (converts audio to ogg) 
-t (generates a foobar2000 & vgmstream compatible .txtp file for playing full tracks) 
-h (outputs wem/wavs/oggs in hex`

(Package ID here refers to 02f6 or 0144 for example)

Modified to export .WEM files by nblock, with major help from Philip & HighRTT

Default output folder is in a folder called "`output[pkgid]`" in the same directory as the exe. 

-----

Obvious thanks to Mont for making his code public.

Thanks to Philip for helping me to get it extracting .WEMs.

Thanks to HighRTT for helping out a lot, and making the WEM to WAV converter library I use.

-----

## Licensing

This work is covered under GPLv3, although if derivative work is published or this work is used in another tool please provide a credit to myself or this page.
