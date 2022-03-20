#include "main.h"
#include "helpers.h"
#include "package.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include "Sarge/src/sarge.cpp"
#include <stdio.h>

// Using Sarge to parse command line args: https://mayaposch.wordpress.com/2019/03/17/parsing-command-line-arguments-in-c/
// Also requires the Boost library for easiest way of capitalizing a hash

static void show_usage()
{
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path] -i [package id] -o [output path] -v [version] -w -h -t\n"
		<< "-w converts wem audio to standard wav\n"
		<< "-h names the audio with hexadecimal, to make it easier to read\n"
		<< "-t extracts foobar2000 & vgmstream compatible .txtp files\n"
		<< "-v [version] changes the version of the game to unpack from (Default post-bl, valid options: prebl, d1)"
		<< std::endl;
}

int main(int argc, char** argv)
{
	Sarge sarge;

	sarge.setArgument("i", "pkgsIds", "pkgs id", true);
	sarge.setArgument("p", "pkgspath", "pkgs path", true);
	sarge.setArgument("o", "outpath", "output path", true);
	sarge.setArgument("v", "version", "pkg version", true);
	sarge.setArgument("w", "wavconv", "wav conv", false);
	sarge.setArgument("t", "txtpgen", "txtpgen", false);
	sarge.setArgument("h", "hexid", "hex id", false);
	sarge.setArgument("f", "folder", "folder packages", false);
	sarge.setDescription("Destiny 2 C++ Unpacker by Monteven. Modified for D1 & Pre-BL, and to export wems and txtp files by nblock with help from Philip and HighRTT.");
	sarge.setUsage("DestinyUnpackerCPP");

	if (!sarge.parseArguments(argc, argv))
	{
		std::cerr << "Couldn't parse arguments..." << std::endl;
		show_usage();
		return 1;
	}
	std::string packagesPath, pkgId, outputPath, version;
	
	sarge.getFlag("pkgspath", packagesPath);
	sarge.getFlag("pkgsIds", pkgId);
	sarge.getFlag("outpath", outputPath);
	sarge.getFlag("version", version);

	if (packagesPath == "")
	{
		std::cerr << "Invalid parameters, potentially backslashes in paths or paths not given.\n";
		show_usage();
		return 1;
	}

	if (!std::filesystem::exists(packagesPath))
	{
		std::cerr << "Packages path does not exist. Check they exist and try again.\n";
		show_usage();
		return 1;
	}
	if (sarge.exists("folder"))
	{
		std::string pkgidold;
		std::filesystem::path pkgsFolder{ packagesPath };
		std::set<std::string> existingPkgIDS;
		std::vector<std::string> pkgf;
		for (auto const& dir_entry : std::filesystem::directory_iterator{ pkgsFolder })
		{
			std::string pkgidfolder = dir_entry.path().string();
			pkgidfolder = pkgidfolder.substr((pkgidfolder.size() - 10), 4);
			if (existingPkgIDS.find(pkgidfolder) == existingPkgIDS.end())
			{
				pkgf.push_back(pkgidfolder);
				existingPkgIDS.insert(pkgidfolder);
			}
		}
		std::vector<std::string> mdEntries;
		for (int o = 0; o < existingPkgIDS.size(); o++)
		{
			Package pkg(pkgf[o], packagesPath);
			pkg.txtpgen = sarge.exists("txtpgen");
			pkg.hexid = sarge.exists("hexid");
			pkg.wavconv = sarge.exists("wavconv");
			bool d1 = false;
			bool prebl = false;
			if (boost::iequals(version, "d1"))
				d1 = true;
			else if (boost::iequals(version, "prebl"))
				prebl = true;
			pkg.outPathBase = outputPath;
			pkg.d1 = d1;
			pkg.preBL = prebl;

			pkg.Unpack();
		}
	}
	else
	{

		Package Pkg(pkgId, packagesPath);

		Pkg.txtpgen = sarge.exists("txtpgen");
		Pkg.hexid = sarge.exists("hexid");
		Pkg.wavconv = sarge.exists("wavconv");
		bool d1 = false;
		bool prebl = false;
		if (boost::iequals(version, "d1"))
			d1 = true;
		else if (boost::iequals(version, "prebl"))
			prebl = true;
		Pkg.outPathBase = outputPath;
		Pkg.d1 = d1;
		Pkg.preBL = prebl;

		Pkg.Unpack();
	}
	return 0;
}

