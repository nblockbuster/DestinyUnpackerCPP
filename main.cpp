#include "main.h"
#include "helpers.h"
#include "package.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include <sarge.cpp>
#include <stdio.h>

namespace fs = std::filesystem;

// Using Sarge to parse command line args: https://mayaposch.wordpress.com/2019/03/17/parsing-command-line-arguments-in-c/

static void show_usage()
{
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path] -i [package id] -w (Exports as normal RIFF WAVE, auto-purges wems after conversion) -g (Extracts wems with wise id as hex) -d (deletes wems)"//-o Convert wems to ogg
		<< std::endl;
}

int main(int argc, char** argv)
{
	Sarge sarge;
	sarge.setArgument("p", "pkgspath", "pkgs path", true);
	sarge.setArgument("w", "wavconv", "wav conv", false);
	sarge.setArgument("g", "hexid", "hex id", false);
	sarge.setArgument("d", "deletewems", "delete wems", false);
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

	bool hexID = false;
	hexID = sarge.exists("hexid");

	Package Pkg(pkgId, packagesPath, hexID);
	Pkg.Unpack();



	std::string outputPath = "output" + pkgId + "/wem/";
	std::string outPath2 = "output" + pkgId + "\\wem\\";
	std::string wempath;
	std::string wempath2;
	std::string cmdstr;

	if (sarge.exists("wavconv")) {
		fs::directory_iterator end_itr;
		for (const auto& entry : fs::directory_iterator(outputPath)) {
			std::wstring dwide = entry.path();
			std::transform(dwide.begin(), dwide.end(), std::back_inserter(wempath2), [](wchar_t c) {
				return (char)c; });
			wempath2.insert(0, "\"");
			wempath2.append("\"");
			fs::path path(wempath2);
			if (entry.path().extension() == ".wem")
			{
				std::wstring dwide = entry.path();
				std::transform(dwide.begin(), dwide.end(), std::back_inserter(wempath), [](wchar_t c) {
					return (char)c; });
				wempath.insert(0, "\"");
				wempath.append("\"");
				cmdstr = std::string("res\\vgmstream\\test.exe") + std::string(" ") + wempath;
				std::cout << "Converting..." << std::endl;
				system(cmdstr.c_str());
				std::cout << "Converted " + wempath << std::endl;
				wempath.clear();
				cmdstr.clear();
			}
		}
		std::string delWemPaths = "cmd.exe del " + outPath2 + "*.wem";
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
			std::string delWemPaths = "del " + outPath2 + "*.wem";
			system(delWemPaths.c_str());
			wempath.clear();
			delWemPaths.clear();
		}

	}
	return 0;
}

