#include "main.h"
#include "helpers.h"
#include "package.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include "Sarge/src/sarge.cpp"
#include <stdio.h>

namespace fs = std::filesystem;

// Using Sarge to parse command line args: https://mayaposch.wordpress.com/2019/03/17/parsing-command-line-arguments-in-c/
// Also requires the Boost library for easiest way of capitalizing a hash

static void show_usage()
{
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path] -i [package id] -w (Exports as normal RIFF WAVE) -o (Converts wems to ogg) -h (outputs wem ids in hex) -t (generates foobar usable .txtps)"
		<< std::endl;
}

int main(int argc, char** argv)
{
	Sarge sarge;

	sarge.setArgument("i", "pkgsIds", "pkgs id", true);
	sarge.setArgument("p", "pkgspath", "pkgs path", true);
	sarge.setArgument("w", "wavconv", "wav conv", false);
	sarge.setArgument("o", "oggconv", "ogg conv", false);
	sarge.setArgument("t", "txtpgen", "txtpgen", false);
	sarge.setArgument("h", "hexid", "hex id", false);
	sarge.setDescription("Destiny 2 C++ Unpacker by Monteven. Modified to export wems and txtp files by nblock with help from Philip and HighRTT.");
	sarge.setUsage("DestinyUnpackerCPP");

	if (!sarge.parseArguments(argc, argv))
	{
		std::cerr << "Couldn't parse arguments..." << std::endl;
		show_usage();
		return 1;
	}
	std::string packagesPath;
	std::string pkgId;

	
	sarge.getFlag("pkgspath", packagesPath);
	sarge.getFlag("pkgsIds", pkgId);

	if (packagesPath == "")
	{
		std::cerr << "Invalid parameters, potentially backslashes in paths or paths not given.\n";
		show_usage();
		return 1;
	}
	else if (!fs::exists(packagesPath))
	{
		std::cerr << "Packages path does not exist. Check they exist and try again.\n";
		show_usage();
		return 1;
	}

	if (packagesPath.find('\\') != std::string::npos)
	{
		printf("\nBackslashes in paths detected, please change to forward slashes (/).\n");
		return 1;
	}
	Package Pkg(pkgId, packagesPath);

	Pkg.txtpgen = sarge.exists("txtpgen");
	Pkg.hexid = sarge.exists("hexid");
	Pkg.wavconv = sarge.exists("wavconv");
	Pkg.oggconv = sarge.exists("oggconv");

	Pkg.Unpack();
	return 0;
}

