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

static void show_usage()
{
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path] -i [package id] Not Implemented Yet: -w (Exports as normal RIFF WAVE)"
		<< std::endl;
}

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
	Sarge sarge;

	sarge.setArgument("p", "pkgspath", "pkgs path", true);
	sarge.setArgument("w", "wavconv", "wav conv", false);
	sarge.setArgument("d", "deletewems", "delete wems", false);
	//sarge.setArgument("o", "oggconv", "ogg conv", false);
	sarge.setArgument("i", "pkgsIds", "pkgs id", true);
	sarge.setDescription("Destiny 2 C++ Unpacker by Monteven. Modified to export .WEM and .WAV files by nblock with help from Philip and HighRTT.");
	sarge.setUsage("DestinyUnpackerCPP");

	if (!sarge.parseArguments(argc, argv))
	{
		std::cerr << "Couldn't parse arguments..." << std::endl;
		show_usage();
		return 1;
	}
	std::string packagesPath;
	std::string pkgId;
	bool bWavconv = false;
	bool bDeletewems = false;

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

	Package Pkg(pkgId, packagesPath);
	Pkg.Unpack();
	
	std::string outputPath = "output" + pkgId + "/";
	std::string outPath2 = "output" + pkgId + "\\";
	std::string wempath;
	std::string cmdstr;

	//probably way overcomplicated but it works for now, thats all that matters

	if (sarge.exists("wavconv")) {

		for (const auto& entry : fs::directory_iterator(outputPath)) {
			std::wstring dwide = entry.path();
			std::transform(dwide.begin(), dwide.end(), std::back_inserter(wempath), [](wchar_t c) {
				return (char)c; });
			wempath.insert(0, "\"");
			wempath.append("\"");
			cmdstr = std::string("test.exe") + std::string(" ") + wempath;
			system(cmdstr.c_str());
			std::cout << "Converted " + wempath << std::endl;
			wempath.clear();
		}
		std::string delWemPaths = "del " + outPath2 + "*.wem";
		system(delWemPaths.c_str());
		std::cout << "Purged .WEMS leftover from conversion";
	}
	if (sarge.exists("deletewems")) {
		
		for (const auto& entry : fs::directory_iterator(outputPath)) {
			std::wstring dwide = entry.path();
			std::transform(dwide.begin(), dwide.end(), std::back_inserter(wempath), [](wchar_t c) {
				return (char)c; });
			wempath.insert(0, "\"");
			wempath.append("\"");
			//std::cout << wempath << std::endl;
			std::string delWemPaths = "del " + outPath2 + "*.wem";
			system(delWemPaths.c_str());
			wempath.clear();
		}
	
	}

return 0;
}