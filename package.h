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
#include "helpers.h"


struct PkgHeader
{
	uint16_t pkgID;
	uint16_t patchID;
	uint32_t entryTableOffset;
	uint32_t entryTableSize;
	uint32_t blockTableOffset;
	uint32_t blockTableSize;
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
};

struct Block
{
	uint32_t ID;
	uint32_t offset;
	uint32_t size;
	uint16_t patchID;
	uint16_t bitFlag;
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
	const int BLOCK_SIZE = 0x40000;
	const std::string CUSTOM_DIR = "I:/test_out/pkg/";

	FILE* pkgFile;
	PkgHeader header;
	std::vector<Entry> entries;
	std::vector<Block> blocks;

	int64_t OodleLZ_Decompress;
	HMODULE hOodleDll;

	void readHeader();
	void modifyNonce();
	void getEntryTable();
	void getBlockTable();
	void extractFiles();
	void decryptBlock(Block block, unsigned char* blockBuffer, unsigned char* &decryptBuffer);
	void decompressBlock(Block block, unsigned char* decryptBuffer, unsigned char*& decompBuffer);
	bool initOodle();
public:
	std::string packagePath;

	// Constructor
	Package(std::string packagesPath, std::string packageName);

	bool Unpack();
	std::string getEntryReference(std::string hash);
	std::string getLatestPatchIDPath(std::string packagesPath, std::string packageName);
};
