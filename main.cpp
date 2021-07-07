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
	//sarge.setArgument("w", "wavconv", "wav conv", false);
	//sarge.setArgument("o", "outpath", "out path", true);
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
	//bool bWavconv = false;
	
	sarge.getFlag("pkgspath", packagesPath);
	sarge.getFlag("pkgsIds", pkgId);

	if (packagesPath == "")
	{
		std::cerr << "Invalid parameters, potentially backslashes in paths or paths not given.\n";
		show_usage();
		return 1;
	}
	else if (!std::filesystem::exists(packagesPath))
	{
		std::cerr << "Packages path does not exist. Check they exist and try again.\n";
		show_usage();
		return 1;
	}

	Package Pkg(pkgId, packagesPath);
	Pkg.Unpack();
	//TODO: Still implementing a stable wem to wav convert. Initally went with ww2ogg, switched to HighRTT's custom wem to wav converter, and now trying to implement a way to
	
	/*
	std::string outputPath = "output " + pkgId + "/";
	std::string command;
	std::string wempath;
	if (sarge.exists("wavconv")) {

		for (const auto& entry : fs::directory_iterator(outputPath)) {
			std::cout << entry.path() << std::endl;
			std::wstring dwide = entry.path();
			std::string wempath;
			std::transform(dwide.begin(), dwide.end(), std::back_inserter(wempath), [](wchar_t c) {
				return (char)c; });
			std::string command = "vgmstream/test.exe " + '../' + wempath;
			std::cout << entry.path() << std::endl;
			std::cout << "(supposed) Path to WEM: " << wempath;
		}
		std::string command = "vgmstream/test.exe " + '../' + wempath;
		for (const auto& entry : fs::directory_iterator(outputPath)) {
			FILE* popen(const char* command, const char* type);
			int pclose(FILE * stream);
			std::cout << "Test: " << wempath;
		}
		
	}
  else {
	return 0;
  }
	*/
return 0;
}