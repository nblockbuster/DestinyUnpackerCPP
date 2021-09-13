#pragma once
#pragma comment(lib, "bcrypt.lib")
#include <string>
#include <vector>
#include <array>
#include <windows.h>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <string>
#include <filesystem>
#include <bcrypt.h>
#include <set>
#include "helpers.h"
#include <unordered_map>

std::unordered_map<uint64_t, uint32_t> loadH64Table();
std::unordered_map<uint64_t, uint32_t> generateH64Table(std::string packagesPath);
bool saveH64Table(std::unordered_map<uint64_t, uint32_t> hash64Table);

struct PkgHeader
{
	uint16_t pkgID;
	uint16_t patchID;
	uint16_t newFlag;
	uint32_t newTableOffset;
	uint32_t entryTableOffset;
	uint32_t entryTableOffsetTemp;
	char entryTableHash;
	uint32_t entryTableSize;
	uint32_t blockTableOffset;
	uint32_t blockTableOffsetTemp;
	uint16_t blockTableSize;
	char blockTableHash;
	uint32_t hash64TableOffset;
	uint32_t hash64TableSize;
};

struct Entry
{
	std::string reference;
	uint8_t numType;
	uint8_t numSubType;
	uint32_t startingBlock;
	uint32_t startingBlockOffset;
	uint32_t fileSize;
	uint32_t blockCount;
};

struct Block
{
	uint32_t ID;
	uint32_t offset;
	uint32_t size;
	uint16_t patchID;
	uint16_t bitFlag;
	uint8_t hash[20];
	uint8_t gcmTag[16];
};

typedef int64_t(*OodleLZ64_DecompressDef)(unsigned char* Buffer, int64_t BufferSize, unsigned char* OutputBuffer, int64_t OutputBufferSize, int32_t a, int32_t b, int64_t c, void* d, void* e, void* f, void* g, void* h, void* i, int32_t ThreadModule);

/*
* Handles the separation of a .pkg file into its constituent binary files.
* It will unpack the PatchID given, so the latest should be given if updates are being processed.
*/
class Package
{
private:
	unsigned char nonce[12] =
	{
		0x84, 0xDF, 0x11, 0xC0, 0xAC, 0xAB, 0xFA, 0x20, 0x33, 0x11, 0x26, 0x99,
	};
	const std::string CUSTOM_DIR = "output";

	FILE* pkgFile;
	std::vector<Block> blocks;

	int64_t OodleLZ_Decompress;
	HMODULE hOodleDll;

	uint8_t extract_entry_data();
	uint32_t block_count();
	void getBlockTable();
	void extractFiles();
	void decryptBlock(Block block, unsigned char* blockBuffer, unsigned char* &decryptBuffer);
	void decompressBlock(Block block, unsigned char* decryptBuffer, unsigned char*& decompBuffer);

public:
	std::string packagesPath = "D:/Program Files (x86)/Steam/steamapps/common/Destiny 2/packages/";
	std::string packagePath;
	std::string packageName;
	bool hexID;
	bool hexid2;
	PkgHeader header;
	std::vector<Entry> entries;

	// Constructor
	Package(std::string packageID, std::string pkgsPath, bool hexID);

	bool initOodle();
	void modifyNonce();
	bool readHeader();
	bool Unpack();
	void getEntryTable();
	std::string getEntryReference(std::string hash);
	uint8_t getEntryTypes(std::string hash, uint8_t& subType);
	std::string getLatestPatchIDPath(std::string packageName);
	unsigned char* getEntryData(std::string hash, int& fileSize);
	std::vector<std::string> getAllFilesGivenRef(std::string reference);
	unsigned char* getBufferFromEntry(Entry entry);
};