#include "main.h"
#include "helpers.h"
#include "package.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include "Sarge/src/sarge.cpp"
#include <stdio.h>
#include <chrono>

// Using Sarge to parse command line args: https://mayaposch.wordpress.com/2019/03/17/parsing-command-line-arguments-in-c/
// Also requires the Boost library for easiest way of capitalizing a hash

static void show_usage()
{
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path] -i [package id] -o [output path] -v [version] -w -h -t -f\n"
		<< "-w converts wem audio to standard wav\n"
		<< "-h names the audio with hexadecimal, to make it easier to read\n"
		<< "-t extracts foobar2000 & vgmstream compatible .txtp files\n"
		<< "-v [version] changes the version of the game to unpack from (Default post-bl, valid options: prebl, d1)\n"
		<< "-f extracts from all the packages in the packages path\n"
		<< "-b extracts ONLY .bnk files\n"
		<< "-s [hash/ginsorid] extracts a single file, given it's hash/ginsorid"
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
	sarge.setArgument("b", "bnkonly", "only bnks", false);
	sarge.setArgument("s", "singlefile", "single file", true);
	sarge.setArgument("l", "pkgbackup", "backup pkg flag for single file export", true);
	sarge.setDescription("Destiny 2 C++ Unpacker by Monteven. Modified for D1 & Pre-BL, and to export wems and txtp files by nblock with help from Philip and HighRTT.");
	sarge.setUsage("DestinyUnpackerCPP");

	if (!sarge.parseArguments(argc, argv))
	{
		std::cerr << "Couldn't parse arguments..." << std::endl;
		show_usage();
		return 1;
	}
	std::string packagesPath, pkgId, outputPath, version, singleFileHash, backupId;
	
	sarge.getFlag("pkgspath", packagesPath);
	sarge.getFlag("pkgsIds", pkgId);
	sarge.getFlag("outpath", outputPath);
	sarge.getFlag("version", version);
	sarge.getFlag("singlefile", singleFileHash);
	sarge.getFlag("pkgbackup", backupId);
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

	if (singleFileHash != "")
	{
		std::string pkgid;
		Entry audioEntry;
		if (backupId == "")
		{
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
					/*
					if (dir_entry.path().string().find("audio") != std::string::npos)
					{
						std::string tt, pkgidf;

						if (dir_entry.path().string().find("_jpn_") != std::string::npos || dir_entry.path().string().find("_de_") != std::string::npos || dir_entry.path().string().find("_en_") != std::string::npos || dir_entry.path().string().find("_fr_") != std::string::npos || dir_entry.path().string().find("_it_") != std::string::npos || dir_entry.path().string().find("_pt_") != std::string::npos || dir_entry.path().string().find("_sp_") != std::string::npos)
						{
							tt = dir_entry.path().string().substr(0, dir_entry.path().string().size() - 6);
							std::replace(tt.begin(), tt.end(), '\\', '/');
							tt = dir_entry.path().string().substr(tt.find_last_of('/'));
							pkgidf = tt.substr(1);
						}
						else
							pkgidf = pkgidfolder;

						pkgf.push_back(pkgidf);
						existingPkgIDS.insert(pkgidf);
					}
					else if (dir_entry.path().string().find("_unp") != std::string::npos)
					{
						std::string tt, pkgidf;
						tt = dir_entry.path().string().substr(0, dir_entry.path().string().size() - 6);
						std::replace(tt.begin(), tt.end(), '\\', '/');
						tt = dir_entry.path().string().substr(tt.find_last_of('/'));
						pkgidf = tt.substr(1);

						pkgf.push_back(pkgidf);
						existingPkgIDS.insert(pkgidf);
					}
					else
						continue;
					else
					{
						pkgf.push_back(pkgidfolder);
						existingPkgIDS.insert(pkgidfolder);
					}
					*/
				}
			}
			for (int o = 0; o < existingPkgIDS.size(); o++)
			{
				std::cout << "Searching for " + singleFileHash + " in " + pkgf[o] << "\n";

				Package Pkg(pkgf[o], packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));
				Pkg.d1 = boost::iequals(version, "d1");
				Pkg.preBL = boost::iequals(version, "prebl");
				Pkg.readHeader();
				Pkg.getEntryTable();
				for (int i = 0; i < Pkg.entries.size(); i++)
				{
					Entry entry = Pkg.entries[i];
					if (boost::iequals(singleFileHash, entry.reference))
					{
						pkgid = uint16ToHexStr(Pkg.header.pkgID);
						std::cout << "Found in " + pkgid << "\n";
						audioEntry = entry;
						break;
					}
				}
				if (pkgid != "")
					break;
			}
		}

		if ((singleFileHash.substr(6, 2) == "80" || singleFileHash.substr(6, 2) == "81") && backupId == "")
			pkgid = getPkgID(singleFileHash);

		Package pkg(pkgid, packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));
		pkg.d1 = boost::iequals(version, "d1");
		pkg.preBL = boost::iequals(version, "prebl");
		pkg.readHeader();
		pkg.getEntryTable();
		int wemType, wemSubType, bnkType, bnkSubType, bnkSubType2;
		if (pkg.preBL)
		{
			wemType = 26;
			wemSubType = 6;
			bnkType = 26;
			bnkSubType = 5;
			bnkSubType2 = 5;
		}
		else if (pkg.d1)
		{
			wemType = 8;
			wemSubType = 21;
			bnkType = 0;
			bnkSubType = 19;
			bnkSubType2 = 20;
		}
		else
		{
			wemType = 26;
			wemSubType = 7;
			bnkType = 26;
			bnkSubType = 6;
			bnkSubType2 = 6;
		}
		int fileSize;
		uint8_t t, st;
		unsigned char* data = nullptr;
		if (audioEntry.reference != "")
		{
			data = pkg.getBufferFromEntry(audioEntry);
			fileSize = audioEntry.fileSize;
			t = audioEntry.numType;
			st = audioEntry.numSubType;
		}
		else
			data = pkg.getEntryData(singleFileHash, fileSize);
		FILE* oFile;
		std::string name;
		if (audioEntry.reference == "")
		{
			//t = pkg.getEntryTypes(singleFileHash, st);
			Entry bnkentry = pkg.entries[(hexStrToUint32(singleFileHash) % 8192)];
			t = bnkentry.numType;
			st = bnkentry.numSubType;
		}
		
		
		if (outputPath == "")
			outputPath = uint16ToHexStr(pkg.header.pkgID);
		std::filesystem::create_directories(outputPath);
		std::string arg0 = argv[0];
		std::string prepath;
		if (t == wemType && st == wemSubType)
		{
			if (sarge.exists("hexid"))
			{
				name = outputPath + "/" + singleFileHash + ".wem";
				if (sarge.exists("wavconv"))
				{
					oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
					fwrite(data, fileSize, 1, oFile);
					fclose(oFile);
					if (arg0.find("/") == std::string::npos)
						prepath = arg0.substr(0, arg0.find_last_of('\\'));
					else
						prepath = arg0.substr(0, arg0.find_last_of('/'));
					std::string vgmstring = prepath + "\\res\\vgmstream\\vgmstream-cli.exe \"" + name + "\" -o \"" + outputPath + "/" + singleFileHash + ".wav\"";
					std::cout << vgmstring << "\n";
					system(vgmstring.c_str());
					remove(name.c_str());
				}
				else
				{
					oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
					fwrite(data, fileSize, 1, oFile);
					fclose(oFile);
				}
			}
			else
			{
				name = outputPath + "/" + std::to_string(hexStrToUint32(singleFileHash)) + ".wem";
				if (sarge.exists("wavconv"))
				{
					oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
					fwrite(data, fileSize, 1, oFile);
					fclose(oFile);
					std::string vgmstring = "res\\vgmstream\\vgmstream-cli.exe \"" + name + "\" -o \"" + outputPath + "/" + std::to_string(hexStrToUint32(singleFileHash)) + ".wav\"";
					std::cout << vgmstring << "\n";
					system(vgmstring.c_str());
					remove(name.c_str());
				}
				else
				{
					oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
					fwrite(data, fileSize, 1, oFile);
					fclose(oFile);
				}
			}
		}
		else if (t == bnkType && (st == bnkSubType || st == bnkSubType2))
		{
			name = outputPath + "/" + getFileFromHash(singleFileHash) + ".bnk";
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(data, fileSize, 1, oFile);
			fclose(oFile);
		}
	}

	else if (sarge.exists("folder"))
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
		//std::vector<std::string> usmNames;
		for (int o = 0; o < existingPkgIDS.size(); o++)
		{
			std::cout << pkgf[o] << "\n";

			Package Pkg(pkgf[o], packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));

			Pkg.txtpgen = sarge.exists("txtpgen");
			Pkg.hexid = sarge.exists("hexid");
			Pkg.wavconv = sarge.exists("wavconv");
			Pkg.outPathBase = outputPath;
			Pkg.d1 = boost::iequals(version, "d1");
			Pkg.preBL = boost::iequals(version, "prebl");
			Pkg.bnkonly = sarge.exists("bnkonly");

			Pkg.Unpack();
		}
	}
	else
	{
		Package Pkg(pkgId, packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));

		Pkg.txtpgen = sarge.exists("txtpgen");
		Pkg.hexid = sarge.exists("hexid");
		Pkg.wavconv = sarge.exists("wavconv");
		Pkg.outPathBase = outputPath;
		Pkg.d1 = boost::iequals(version, "d1");
		Pkg.preBL = boost::iequals(version, "prebl");
		Pkg.bnkonly = sarge.exists("bnkonly");

		Pkg.Unpack();
	}
	return 0;
}

