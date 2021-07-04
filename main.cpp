#include "main.h"
#include "helpers.h"
#include "package.h"
#include <iostream>
#include "Sarge/src/sarge.cpp"
//#include "ww2ogg/src/ww2ogg.cpp"


// Using Sarge to parse command line args: https://mayaposch.wordpress.com/2019/03/17/parsing-command-line-arguments-in-c//

static void show_usage()
{
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path]"
		<< std::endl;
}



int main(int argc, char** argv)
{
	
	Sarge sarge;

	sarge.setArgument("p", "pkgspath", "pkgs path", true);
	//sarge.setArgument("o", "outputpath", "output path", true);
	sarge.setDescription("Destiny 2 C++ Unpacker by Monteven. Modified to export .WEM files by nblock with help from Philip");
	sarge.setUsage("DestinyUnpackerCPP");

	if (!sarge.parseArguments(argc, argv))
	{
		std::cerr << "Couldn't parse arguments..." << std::endl;
		show_usage();
		return 1;
	}
	std::string packagesPath;
	//std::string outPath;
	
	sarge.getFlag("pkgspath", packagesPath);
	//sarge.getFlag("outputpath", outPath);

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

	std::string pkgid;
	std::cout << "Package ID: ";
	std::cin >> pkgid;

	Package Pkg(pkgid, packagesPath);
	Pkg.Unpack();

	//TODO: Implement ww2ogg for converting to ogg

	return 0;
}