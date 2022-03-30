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
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path] -i [package id] -o [output path] -v [version] -w -h -t -f\n"
		<< "-w converts wem audio to standard wav\n"
		<< "-h names the audio with hexadecimal, to make it easier to read\n"
		<< "-t extracts foobar2000 & vgmstream compatible .txtp files\n"
		<< "-v [version] changes the version of the game to unpack from (Default post-bl, valid options: prebl, d1)\n"
		<< "-f extracts from all the packages in the packages path"
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
				if (boost::iequals(version, "d1")) //d1 doesnt have defined 'audio' pkgs other than globals
				{
					std::string pkgidf;
					//skip probably unwanted dialogue
					if (dir_entry.path().string().find("_jpn_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_de_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_en_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_fr_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_it_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_pt_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_sp_") != std::string::npos)
						continue;
					pkgidf = pkgidfolder;
					if (dir_entry.path().string().find("_unp") != std::string::npos)
					{
						std::string tt;
						tt = dir_entry.path().string().substr(0, dir_entry.path().string().size() - 6);
						std::replace(tt.begin(), tt.end(), '\\', '/');
						tt = dir_entry.path().string().substr(tt.find_last_of('/'));
						pkgidf = tt.substr(1);
					}
					pkgf.push_back(pkgidf);
					existingPkgIDS.insert(pkgidf);
				}
				else
				{
					if (dir_entry.path().string().find("audio") != std::string::npos)
					{
						std::string pkgidf;
						//skip probably unwanted dialogue
						if (dir_entry.path().string().find("_jpn_") != std::string::npos)
							continue;
						if (dir_entry.path().string().find("_de_") != std::string::npos)
							continue;
						if (dir_entry.path().string().find("_en_") != std::string::npos)
							continue;
						if (dir_entry.path().string().find("_fr_") != std::string::npos)
							continue;
						if (dir_entry.path().string().find("_it_") != std::string::npos)
							continue;
						if (dir_entry.path().string().find("_pt_") != std::string::npos)
							continue;
						if (dir_entry.path().string().find("_sp_") != std::string::npos)
							continue;
						pkgidf = pkgidfolder;
						if (dir_entry.path().string().find("_unp") != std::string::npos)
						{
							std::string tt;
							tt = dir_entry.path().string().substr(0, dir_entry.path().string().size() - 6);
							std::replace(tt.begin(), tt.end(), '\\', '/');
							tt = dir_entry.path().string().substr(tt.find_last_of('/'));
							pkgidf = tt.substr(1);
						}
						pkgf.push_back(pkgidf);
						existingPkgIDS.insert(pkgidf);
					}
					else
					{
						continue;
					}
				}
				//}
				//pkgf.push_back(pkgidfolder);
				//existingPkgIDS.insert(pkgidfolder);
			}
		}
		std::vector<std::string> mdEntries;
		for (int o = 0; o < existingPkgIDS.size(); o++)
		{
			//std::string ihatethis = "DestinyUnpackerCPP.exe -p \"" + packagesPath + "\" -i " + pkgf[o];
			
			std::string execpath(argv[0]);
			std::string ihatethis = execpath + " -p \"" + packagesPath + "\" -i " + pkgf[o];
			ihatethis += " -o \"" + outputPath +"\"";
			if (sarge.exists("txtpgen"))
				ihatethis += " -t";
			if (sarge.exists("hexid"))
				ihatethis += " -h";
			if (sarge.exists("wavconv"))
				ihatethis += " -w";
			if (boost::iequals(version, "d1"))
				ihatethis += " -v d1";
			else if (boost::iequals(version, "prebl"))
				ihatethis += " -v prebl";
			std::cout << ihatethis << "\n";
			system(ihatethis.c_str());			
			/*
			Package Pkg(pkgf[o], packagesPath);

			Pkg.txtpgen = sarge.exists("txtpgen");
			Pkg.hexid = sarge.exists("hexid");
			Pkg.wavconv = sarge.exists("wavconv");
			Pkg.outPathBase = outputPath;
			Pkg.d1 = boost::iequals(version, "d1");
			Pkg.preBL = boost::iequals(version, "prebl");

			//Pkg.Unpack();
			Pkg.readHeader();
			if (!Pkg.initOodle())
			{
				printf("\nFailed to initialise oodle");
				return 1;
			}
			Pkg.modifyNonce();
			Pkg.getEntryTable();
			for (int i = 0; i < Pkg.entries.size(); i++)
			{
				if (Pkg.entries[i].numType == 26 && Pkg.entries[i].numSubType == 6)
				{
					std::string bnkPath = "E:/DestinyMusic/TWQBnks/" + uint16ToHexStr(Pkg.header.pkgID) + "-" + uint16ToHexStr(i) + ".bnk";
					unsigned char* data = Pkg.getBufferFromEntry(Pkg.entries[i]);
					FILE* oBnkFile;
					oBnkFile = _fsopen(bnkPath.c_str(), "wb", _SH_DENYNO);
					fwrite(data, Pkg.entries[i].fileSize, 1, oBnkFile);
					fclose(oBnkFile);
					delete[] data;
				}
			}
			fclose(Pkg.pkgFile);
			*/
		}
		//std::string bnkConv = "py res\\wwiser\\wwiser.pyz \"E:/DestinyMusic/TWQBnks/*.bnk\" -g";
		//system(bnkConv.c_str());
	}
	else
	{
		Package Pkg(pkgId, packagesPath);

		Pkg.txtpgen = sarge.exists("txtpgen");
		Pkg.hexid = sarge.exists("hexid");
		Pkg.wavconv = sarge.exists("wavconv");
		Pkg.outPathBase = outputPath;
		Pkg.d1 = boost::iequals(version, "d1");
		Pkg.preBL = boost::iequals(version, "prebl");

		Pkg.Unpack();
	}
	return 0;
}

