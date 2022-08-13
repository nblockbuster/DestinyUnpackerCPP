#include "main.h"
#include "helpers.h"
#include "package.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include "Sarge/src/sarge.cpp"
#include <stdio.h>
//#include <chrono>

// Using Sarge to parse command line args: https://mayaposch.wordpress.com/2019/03/17/parsing-command-line-arguments-in-c/
// Also requires the Boost library for easiest way of capitalizing a hash

static void show_usage()
{
	std::cerr << "Usage: DestinyUnpackerCPP.exe -p [packages path] -i [package id] -o [output path] -v [version] -w -h -t -f\n"
		<< "-w converts wem audio to standard wav\n"
		<< "-h names the audio with hexadecimal, to make it easier to read\n"
		<< "-f extracts from all the packages in the packages path\n"
		<< "-v [version] changes the version of the game to unpack from (Default post-bl, valid options: prebl, d1)\n"
		<< "-m extracts only music audio files\n"
		<< "-t extracts foobar2000 & vgmstream compatible .txtp files\n"
		<< "-b extracts ONLY .bnk files\n"
		<< "-s [hash/ginsorid] extracts a single file, given it's hash/ginsorid\n"
		<< "-l [backup package id] for use with -s, if you already know the package its in\n"
		<< "-g converts to ogg"
		<< std::endl;
}

int main(int argc, char** argv)
{
	Sarge sarge;

	sarge.setArgument("i", "pkgsIds", "pkgs id", true);
	sarge.setArgument("p", "pkgspath", "pkgs path", true);
	sarge.setArgument("o", "outpath", "output path", true);
	sarge.setArgument("w", "wavconv", "wav conv", false);
	sarge.setArgument("h", "hexid", "hex id", false);
	sarge.setArgument("f", "folder", "folder packages", false);
	sarge.setArgument("v", "version", "pkg version", true);
	sarge.setArgument("m", "music_only", "only extracts audio files that are known to be music", false);
	sarge.setArgument("t", "txtpgen", "txtpgen", false);
	sarge.setArgument("b", "bnkonly", "only bnks", false);
	sarge.setArgument("s", "singlefile", "single file", true);
	sarge.setArgument("l", "pkgbackup", "backup pkg flag for single file export", true);
	sarge.setArgument("g", "oggconv", "ogg conversion", false);
	sarge.setArgument("u", "unknown_only", "only unpacks non-wem and bnk files", false);
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

	//mainly used in the json parser python script

	if (singleFileHash != "")
	{

		std::string pkgid;
		Entry audioEntry;
		bool bFound = false;
		if ((singleFileHash.substr(6, 2) == "80" || singleFileHash.substr(6, 2) == "81") && backupId == "")
			bFound = true;
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
					{
						continue;
					}
				}
			}
			for (int o = 0; o < existingPkgIDS.size(); o++)
			{
				std::cout << "Searching for " + singleFileHash + " in " + pkgf[o] << "\n";

				Package Pkg(pkgf[o], packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));
				Pkg.options.d1 = boost::iequals(version, "d1");
				Pkg.options.preBL = boost::iequals(version, "prebl");
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
						int wemType, wemSubType, bnkType, bnkSubType, bnkSubType2;
						if (Pkg.options.preBL)
						{
							wemType = 26;
							wemSubType = 6;
							bnkType = 26;
							bnkSubType = 5;
							bnkSubType2 = 5;
						}
						else if (Pkg.options.d1)
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
						unsigned char* data = nullptr;
						std::string name = "";
						FILE* oFile = nullptr;
						if (outputPath == "")
							outputPath = uint16ToHexStr(Pkg.header.pkgID);
						std::filesystem::create_directories(outputPath);
						std::string arg0 = argv[0];
						std::string prepath;
						if (entry.numType == wemType && entry.numSubType == wemSubType)
						{
							if (sarge.exists("hexid"))
							{
								name = outputPath + "/" + singleFileHash + ".wem";
								if (sarge.exists("wavconv"))
								{
									oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
									fwrite(data, entry.fileSize, 1, oFile);
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
									fwrite(data, entry.fileSize, 1, oFile);
									fclose(oFile);
								}
							}
							else
							{
								name = outputPath + "/" + std::to_string(hexStrToUint32(singleFileHash)) + ".wem";
								if (sarge.exists("wavconv"))
								{
									oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
									fwrite(data, entry.fileSize, 1, oFile);
									fclose(oFile);
									std::string vgmstring = "res\\vgmstream\\vgmstream-cli.exe \"" + name + "\" -o \"" + outputPath + "/" + std::to_string(hexStrToUint32(singleFileHash)) + ".wav\"";
									std::cout << vgmstring << "\n";
									system(vgmstring.c_str());
									remove(name.c_str());
								}
								else
								{
									oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
									fwrite(data, entry.fileSize, 1, oFile);
									fclose(oFile);
								}
							}
						}
						else if (entry.numType == bnkType && (entry.numSubType == bnkSubType || entry.numSubType == bnkSubType2))
						{
							name = outputPath + "/" + getFileFromHash(singleFileHash) + ".bnk";
							oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
							fwrite(data, entry.fileSize, 1, oFile);
							fclose(oFile);
						}
						else
						{
							name = outputPath + "/" + singleFileHash + ".bin";
							oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
							fwrite(data, entry.fileSize, 1, oFile);
							fclose(oFile);
						}


						exit(0);
					}
				}
				if (pkgid != "")
					break;
			}
		}

		if (!bFound)
		{
			std::cout << "Could not find audio.\n";
			return 5;
		}

		if ((singleFileHash.substr(6, 2) == "80" || singleFileHash.substr(6, 2) == "81") && backupId == "")
			pkgid = getPkgID(singleFileHash);

		Package pkg(pkgid, packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));
		pkg.options.d1 = boost::iequals(version, "d1");
		pkg.options.preBL = boost::iequals(version, "prebl");
		pkg.readHeader();
		pkg.getEntryTable();

		int wemType, wemSubType, bnkType, bnkSubType, bnkSubType2;
		if (pkg.options.preBL)
		{
			wemType = 26;
			wemSubType = 6;
			bnkType = 26;
			bnkSubType = 5;
			bnkSubType2 = 5;
		}
		else if (pkg.options.d1)
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
		uint8_t t = 0, st = 0;
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
		else
		{
			name = outputPath + "/" + singleFileHash + ".bin";
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
				if (dir_entry.path().string().find("tower") == std::string::npos)
					continue;
				/*
				if (dir_entry.path().string().find("audio") != std::string::npos || boost::iequals(version, "d1"))
				{
					//skip probably unwanted dialogue
					if (dir_entry.path().string().find("_de_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_en_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_fr_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_it_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_jpn_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_mx_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_po_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_pt_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_ru_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_sp_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_cs_") != std::string::npos)
						continue;
					if (dir_entry.path().string().find("_ct_") != std::string::npos)
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
					continue;
				*/
				if ((sarge.exists("music_only") || sarge.exists("bnkonly"))
					&& dir_entry.path().string().find("audio") == std::string::npos)
				{
					continue;
				}
				std::string pkgidf = pkgidfolder;
				std::string tt;
				tt = dir_entry.path().string().substr(0, dir_entry.path().string().size() - 6);
				std::replace(tt.begin(), tt.end(), '\\', '/');
				pkgidf = tt.substr(tt.find_last_of('/')).substr(1);
				pkgf.push_back(pkgidf);
				existingPkgIDS.insert(pkgidfolder);
			}
		}
		for (int o = 0; o < pkgf.size(); o++)
		{
			std::cout << pkgf[o] << "\n";

			Package Pkg(pkgf[o], packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));

			PackageOptions options;

			options.txtpgen = sarge.exists("txtpgen");
			options.hexid = sarge.exists("hexid");
			options.wavconv = sarge.exists("wavconv");
			options.oggconv = sarge.exists("oggconv");
			options.outPathBase = outputPath;
			options.d1 = boost::iequals(version, "d1");
			options.preBL = boost::iequals(version, "prebl");
			options.bnkonly = sarge.exists("bnkonly");
			options.musiconly = sarge.exists("music_only");
			options.unknown_only = sarge.exists("unknown_only");
			
			Pkg.options = options;
				
			Pkg.Unpack();
			//if (Pkg.pkgFile != nullptr)
				//fclose(Pkg.pkgFile);
		}
	}

	else
	{
		Package Pkg(pkgId, packagesPath, (boost::iequals(version, "prebl") || boost::iequals(version, "d1")));

		PackageOptions options;
		
		options.txtpgen = sarge.exists("txtpgen");
		options.hexid = sarge.exists("hexid");
		options.wavconv = sarge.exists("wavconv");
		options.oggconv = sarge.exists("oggconv");
		options.outPathBase = outputPath;
		options.d1 = boost::iequals(version, "d1");
		options.preBL = boost::iequals(version, "prebl");
		options.bnkonly = sarge.exists("bnkonly");
		options.musiconly = sarge.exists("music_only");
		options.unknown_only = sarge.exists("unknown_only");

		Pkg.options = options;

		Pkg.Unpack();
		
		/*
		if (sarge.exists("gen_wem_hashmap"))
		{
			FILE* wem_hashes_file = nullptr;
			wem_hashes_file = _fsopen("wem_hashes", "wb", _SH_DENYNO);
			if (wem_hashes_file == nullptr) {
				std::perror("buh");
				exit(1);
			}
			uint32_t wemsize = Pkg.WemHashMap.size();
			fwrite(&wemsize, 1, 4, wem_hashes_file);
			for (auto& element : Pkg.WemHashMap)
			{
				std::cout << "Wem with GinsorID " + element.first + " has XXH64 hash value " + std::to_string(element.second) << "\n";
				uint32_t ginsid = hexStrToUint32(element.first);
				fwrite(&ginsid, 1, 4, wem_hashes_file);
				fwrite((char*)&element.second, 1, 8, wem_hashes_file);
			}
			fclose(wem_hashes_file);
		}
		*/
	}
	return 0;
}