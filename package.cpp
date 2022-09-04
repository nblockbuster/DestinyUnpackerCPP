#include "package.h"

const static unsigned char AES_KEY_0[16] =
{
	0xD6, 0x2A, 0xB2, 0xC1, 0x0C, 0xC0, 0x1B, 0xC5, 0x35, 0xDB, 0x7B, 0x86, 0x55, 0xC7, 0xDC, 0x3B,
};

const static unsigned char AES_KEY_1[16] =
{
	0x3A, 0x4A, 0x5D, 0x36, 0x73, 0xA6, 0x60, 0x58, 0x7E, 0x63, 0xE6, 0x76, 0xE4, 0x08, 0x92, 0xB5,
};

const int BLOCK_SIZE = 0x40000;

Package::Package(std::string packageID, std::string pkgsPath, bool prebl_d1)
{
	packagesPath = pkgsPath;
	if (!std::filesystem::exists(pkgsPath))
	{
		printf("Package path given is invalid!");
		exit(1);
	}
	packagePath = getLatestPatchIDPath(packageID);
	d1prebl = prebl_d1;
}

std::string Package::getLatestPatchIDPath(std::string packageID)
{
	std::string fullPath = "";
	uint16_t patchID;
	int largestPatchID = -1;
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(packagesPath))
	{
		fullPath = entry.path().string();
		if (entry.path().stem().string().substr(0, 3) == "ps3" || entry.path().stem().string().substr(0, 3) == "360")
			ps3_x360 = true;

		if (fullPath.find(packageID) != std::string::npos)
		{
			patchID = std::stoi(fullPath.substr(fullPath.size() - 5, 1));
			if (patchID > largestPatchID) largestPatchID = patchID;
			std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
			packageName = fullPath.substr(0, fullPath.size() - 6);
			packageName = packageName.substr(packageName.find_last_of('/'));
		}
	}
	// Some strings are not covered, such as the bootstrap set so we need to do pkg checks
	if (largestPatchID == -1)
	{
		FILE* patchPkg;
		uint16_t pkgID;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(packagesPath))
		{
			fullPath = entry.path().string();

			patchPkg = _fsopen(fullPath.c_str(), "rb", _SH_DENYNO);
			if (patchPkg == nullptr) exit(67);

			uint32_t val;
			fread((char*)&val, 1, 4, patchPkg);

			if (d1prebl)
				fseek(patchPkg, 0x4, SEEK_SET);
			else
				fseek(patchPkg, 0x10, SEEK_SET);

			fread((char*)&pkgID, 1, 2, patchPkg);
			if (ps3_x360)
				pkgID = swapUInt16Endianness(pkgID);
			if (packageID == uint16ToHexStr(pkgID))
			{
				if (d1prebl)
					fseek(patchPkg, 0x20, SEEK_SET);
				else
					fseek(patchPkg, 0x30, SEEK_SET);

				fread((char*)&patchID, 1, 2, patchPkg);
				if (ps3_x360)
					patchID = swapUInt16Endianness(patchID);
				if (patchID > largestPatchID) largestPatchID = patchID;
				std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
				packageName = fullPath.substr(0, fullPath.size() - 6);
				packageName = packageName.substr(packageName.find_last_of('/'));
			}
			fclose(patchPkg);
		}
	}

	return packagesPath + "/" + packageName + "_" + std::to_string(largestPatchID) + ".pkg";
}

bool Package::readHeader()
{
	// Package data
	pkgFile = _fsopen(packagePath.c_str(), "rb", _SH_DENYNO);
	if (pkgFile == nullptr)
		return false;
	if (options.preBL || options.d1)
	{
		if (ps3_x360)
		{
			uint32_t val, magic;
			uint16_t val16;

			fread((char*)&val, 1, 4, pkgFile);
			magic = swapUInt32Endianness(val);
			//std::cout << "val: " + std::to_string(val) + " magic/swap: " + std::to_string(magic) << "\n";

			fseek(pkgFile, 0x04, SEEK_SET);
			fread((char*)&val16, 1, 2, pkgFile);
			val16 = swapUInt16Endianness(val16);
			header.pkgID = val16;

			fseek(pkgFile, 0x20, SEEK_SET);
			fread((char*)&val16, 1, 2, pkgFile);
			val16 = swapUInt16Endianness(val16);
			header.patchID = val16;


			// Entry Table
			fseek(pkgFile, 0xB4, SEEK_SET);
			fread((char*)&val, 1, 4, pkgFile);
			val = swapUInt32Endianness(val);
			header.entryTableSize = val;
			fread((char*)&val, 1, 4, pkgFile);
			val = swapUInt32Endianness(val);
			header.entryTableOffset = val;

			// Block Table
			fseek(pkgFile, 0xD0, SEEK_SET);
			fread((char*)&val, 1, 4, pkgFile);
			val = swapUInt32Endianness(val);
			header.blockTableSize = val;
			fread((char*)&val, 1, 4, pkgFile);
			val = swapUInt32Endianness(val);
			header.blockTableOffset = val;

			return true;
		}

		uint32_t buildID = 0;
		fseek(pkgFile, 0x4, SEEK_SET);
		fread((char*)&header.pkgID, 1, 2, pkgFile);
		fseek(pkgFile, 0x20, SEEK_SET);
		fread((char*)&header.patchID, 1, 2, pkgFile);
		if (options.preBL)
		{
			fseek(pkgFile, 0x18, SEEK_SET);
			fread((char*)&buildID, 1, 4, pkgFile);
		}
		if (buildID != 0 && buildID <= 0x10000 || options.d1)
		{
			fseek(pkgFile, 0xB4, SEEK_SET);
			fread((char*)&header.entryTableSize, 1, 4, pkgFile);
			fread((char*)&header.entryTableOffset, 1, 4, pkgFile);

			fseek(pkgFile, 0xD0, SEEK_SET);
			fread((char*)&header.blockTableSize, 1, 4, pkgFile);
			fread((char*)&header.blockTableOffset, 1, 4, pkgFile);
		}
		else
		{
			fseek(pkgFile, 0x110, SEEK_SET);
			fread((char*)&header.entryTableOffset, 1, 4, pkgFile);
			header.entryTableOffset += 96;

			fseek(pkgFile, 0xB4, SEEK_SET);
			fread((char*)&header.entryTableSize, 1, 4, pkgFile);

			fseek(pkgFile, 0xD0, SEEK_SET);
			fread((char*)&header.blockTableSize, 1, 4, pkgFile);

			//header.blockTableOffset = header.entryTableOffset + header.entryTableSize * 16 + 32;
			fseek(pkgFile, (header.entryTableOffset - 96 + 0x28), SEEK_SET);
			fread((char*)&header.blockTableOffset, 1, 4, pkgFile);

			header.blockTableOffset = header.entryTableOffset - 96 + 0x28 + header.blockTableOffset + 0x10;
		}
	}
	else
	{
		fseek(pkgFile, 0x10, SEEK_SET);
		fread((char*)&header.pkgID, 1, 2, pkgFile);
		fseek(pkgFile, 0x30, SEEK_SET);
		fread((char*)&header.patchID, 1, 2, pkgFile);

		// Entry Table
		fseek(pkgFile, 0x44, SEEK_SET);
		fread((char*)&header.entryTableOffset, 1, 4, pkgFile);
		fseek(pkgFile, 0x60, SEEK_SET);
		fread((char*)&header.entryTableSize, 1, 4, pkgFile);

		// Block Table
		fseek(pkgFile, 0x68, SEEK_SET);
		fread((char*)&header.blockTableSize, 1, 4, pkgFile);
		fread((char*)&header.blockTableOffset, 1, 4, pkgFile);
		// Hash64 Table
		fseek(pkgFile, 0xB8, SEEK_SET);
		fread((char*)&header.hash64TableSize, 1, 4, pkgFile);
		fread((char*)&header.hash64TableOffset, 1, 4, pkgFile);
		header.hash64TableOffset += 64; // relative offset
	}
	return true;
}

void Package::getEntryTable()
{
	for (uint32_t i = header.entryTableOffset; i < header.entryTableOffset + header.entryTableSize * 16; i += 16)
	{
		Entry entry;
		if (ps3_x360)
		{
			// EntryA
			uint32_t entryA;
			fseek(pkgFile, i, SEEK_SET);
			fread((char*)&entryA, 1, 4, pkgFile);
			entryA = swapUInt32Endianness(entryA);
			entry.reference = uint32ToHexStr(entryA);
			entry.reference2 = std::to_string(entryA);

			// EntryB
			uint16_t entryB_1, entryB_2;
			fread((char*)&entryB_1, 1, 2, pkgFile);
			fread((char*)&entryB_2, 1, 2, pkgFile);
			entryB_1 = swapUInt16Endianness(entryB_1);
			//entryB_2 = swapUInt16Endianness(entryB_2);
			//uint32_t xa, xb;
			//xa = (uint32_t)entryB_1 << 16;
			//xb = (uint32_t)entryB_2;
			//uint32_t entryB = ((uint32_t)entryB_1 << 16) | (uint32_t)entryB_2 & 0x0000FFFFuL;
			//uint32_t entryB = entryB_1;
			//entryB = entryB << 16;
			//entryB |= entryB_2;
			//uint32_t entryB = xa | xb;
			entry.numType = entryB_1;
			entry.numSubType = entryB_2;

			//entry.numType = entryB_1;
			//entry.numSubType = entryB_2;

			// EntryC/D
			uint32_t entryD;
			fread((char*)&entryD, 1, 4, pkgFile);
			entryD = swapUInt32Endianness(entryD);

			// EntryD/C
			uint32_t entryC;
			fread((char*)&entryC, 1, 4, pkgFile);
			entryC = swapUInt32Endianness(entryC);
			entry.startingBlock = entryC & 0x3FFF;
			entry.startingBlockOffset = ((entryC >> 14) & 0x3FFF) << 4;

			entry.fileSize = (entryD & 0x3FFFFFF) << 4 | (entryC >> 28) & 0xF;
			entries.push_back(entry);
		}
		else
		{
			// EntryA
			uint32_t entryA;
			fseek(pkgFile, i, SEEK_SET);
			fread((char*)&entryA, 1, 4, pkgFile);
			entry.reference = uint32ToHexStr(entryA);
			entry.reference2 = std::to_string(entryA);

			// EntryB
			uint32_t entryB;
			fread((char*)&entryB, 1, 4, pkgFile);
			if (options.d1)
			{
				entry.numType = entryB & 0xFFFF;
				entry.numSubType = entryB >> 24;
				//std::cout << (354157064 & 0xFFFF) << " " << (354157064 >> 24) << "\n";
				//std::cout << (entryB & 0xFFFF) << " " << (entryB >> 24) << "\n";
			}
			else
			{
				entry.numType = (entryB >> 9) & 0x7F;
				entry.numSubType = (entryB >> 6) & 0x7;
			}

			// EntryC
			uint32_t entryC;
			fread((char*)&entryC, 1, 4, pkgFile);
			entry.startingBlock = entryC & 0x3FFF;
			entry.startingBlockOffset = ((entryC >> 14) & 0x3FFF) << 4;

			// EntryD
			uint32_t entryD;
			fread((char*)&entryD, 1, 4, pkgFile);
			entry.fileSize = (entryD & 0x3FFFFFF) << 4 | (entryC >> 28) & 0xF;

			entries.push_back(entry);
		}
	}
}

void Package::getBlockTable()
{
	if (options.d1) //d1 blocks are smaller because it doesnt need the gcmtag for decryption (which doesnt exist for d1)
	{
		for (uint32_t i = header.blockTableOffset; i < header.blockTableOffset + header.blockTableSize * 0x20; i += 0x20)
		{
			if (ps3_x360)
			{
				uint32_t val;
				uint16_t val16;
				Block block = { 0, 0, 0, 0, 0 };
				fseek(pkgFile, i, SEEK_SET);
				fread((char*)&val, 1, 4, pkgFile);
				block.offset = swapUInt32Endianness(val);
				fread((char*)&val, 1, 4, pkgFile);
				block.size = swapUInt32Endianness(val);
				fread((char*)&val16, 1, 2, pkgFile);
				block.patchID = swapUInt32Endianness(val16);
				fread((char*)&val16, 1, 2, pkgFile);
				block.bitFlag = val16;
				fseek(pkgFile, 0x14, SEEK_CUR); // SHA-1 Hash
				blocks.push_back(block);
			}
			else
			{
				Block block = { 0, 0, 0, 0, 0 };
				fseek(pkgFile, i, SEEK_SET);
				fread((char*)&block.offset, 1, 4, pkgFile);
				fread((char*)&block.size, 1, 4, pkgFile);
				fread((char*)&block.patchID, 1, 2, pkgFile);
				fread((char*)&block.bitFlag, 1, 2, pkgFile);
				fseek(pkgFile, i + 0x14, SEEK_SET);
				blocks.push_back(block);
			}
		}
	}
	else
	{
		for (uint32_t i = header.blockTableOffset; i < header.blockTableOffset + header.blockTableSize * 48; i += 48)
		{
			Block block = { 0, 0, 0, 0, 0 };
			fseek(pkgFile, i, SEEK_SET);
			fread((char*)&block.offset, 1, 4, pkgFile);
			fread((char*)&block.size, 1, 4, pkgFile);
			fread((char*)&block.patchID, 1, 2, pkgFile);
			fread((char*)&block.bitFlag, 1, 2, pkgFile);
			if (options.preBL)
			{
				fseek(pkgFile, 0x14, SEEK_CUR); // SHA-1 Hash
				fread((char*)&block.gcmTag, 16, 1, pkgFile);
			}
			else
			{
				fseek(pkgFile, i + 0x20, SEEK_SET);
				fread((char*)&block.gcmTag, 16, 1, pkgFile);
			}
			blocks.push_back(block);
		}
	}
}

void Package::modifyNonce()
{
	// Nonce
	if (options.preBL) {
		preblnonce[0] ^= (header.pkgID >> 8) & 0xFF;
		preblnonce[1] ^= 0x26;
		preblnonce[11] ^= header.pkgID & 0xFF;
		//std::cout << "OLD NONCE\n";
	}
	else {
		nonce[0] ^= (header.pkgID >> 8) & 0xFF;
		nonce[11] ^= header.pkgID & 0xFF;
		//std::cout << "NEW NONCE\n";
	}
}

unsigned char* Package::genericExtract(std::vector<std::string> pkgPatchStreamPaths, int i)
{
	Entry entry = entries[i];
	int currentBlockID = entry.startingBlock;
	int blockCount = floor((entry.startingBlockOffset + entry.fileSize - 1) / BLOCK_SIZE);
	if (entry.fileSize == 0) blockCount = 0; // Stupid check for weird C++ floor behaviour
	int lastBlockID = currentBlockID + blockCount;
	unsigned char* fileBuffer = new unsigned char[entry.fileSize];
	int currentBufferOffset = 0;
	while (currentBlockID <= lastBlockID)
	{
		Block currentBlock = blocks[currentBlockID];
		FILE* pFile;
		pFile = _fsopen(pkgPatchStreamPaths[currentBlock.patchID].c_str(), "rb", _SH_DENYNO);
		fseek(pFile, currentBlock.offset, SEEK_SET);

		unsigned char* blockBuffer = { 0 };
		std::vector<unsigned char> realblockBuffer(currentBlock.size);
		blockBuffer = &realblockBuffer[0];

		size_t result;
		result = fread(blockBuffer, 1, currentBlock.size, pFile);
		if (result != currentBlock.size) { fputs("Reading error", stderr); exit(3); }
		unsigned char* decryptBuffer = nullptr;
		std::vector<unsigned char> realDecryptBuffer(currentBlock.size);
		decryptBuffer = &realDecryptBuffer[0];

		unsigned char* decompBuffer = nullptr;
		std::vector<unsigned char> realDecompBuffer(BLOCK_SIZE);
		decompBuffer = &realDecompBuffer[0];
		if (options.d1)
		{

			if (currentBlock.bitFlag & 0x1)
				decompressBlock(currentBlock, blockBuffer, decompBuffer);
			else
				decompBuffer = blockBuffer;
		}
		else
		{
			if (currentBlock.bitFlag & 0x2)
				decryptBlock(currentBlock, blockBuffer, decryptBuffer);
			else
			{
				decryptBuffer = blockBuffer;
				//delete[] blockBuffer;
				//blockBuffer = { 0 };
				//std::fill(&blockBuffer[0], &blockBuffer[currentBlock.size], 0);
			}
			if (currentBlock.bitFlag & 0x1)
				decompressBlock(currentBlock, decryptBuffer, decompBuffer);
			else
				decompBuffer = decryptBuffer;
		}
		if (currentBlockID == entry.startingBlock)
		{
			size_t cpySize;
			if (currentBlockID == lastBlockID)
				cpySize = entry.fileSize;
			else
				cpySize = BLOCK_SIZE - entry.startingBlockOffset;
			memcpy(fileBuffer, decompBuffer + entry.startingBlockOffset, cpySize);
			currentBufferOffset += cpySize;
		}
		else if (currentBlockID == lastBlockID)
		{
			memcpy(fileBuffer + currentBufferOffset, decompBuffer, entry.fileSize - currentBufferOffset);
		}
		else
		{
			memcpy(fileBuffer + currentBufferOffset, decompBuffer, BLOCK_SIZE);
			currentBufferOffset += BLOCK_SIZE;
		}
		fclose(pFile);
		currentBlockID++;
		std::fill(&decompBuffer[0], &decompBuffer[sizeof(decompBuffer)], 0);
	}
	return fileBuffer;
}

bool Package::getWem(int i, std::string outputPath, std::string Hambit, std::string nameID, Entry entry)
{
	unsigned char* fileBuffer = genericExtract(pkgPatchStreamPaths, i);

	if (options.xxh_hashes)
	{
		XXH64_hash_t hash = XXH3_64bits(fileBuffer, entry.fileSize);
		HashMap["wem"][Hambit] = hash;
		delete[] fileBuffer;
		return true;
	}

	std::string wavOutput = outputPath + "/wav";
	std::string oggOutput = outputPath + "/ogg";
	if (options.hexid) {
		if (options.wavconv)
		{
			std::filesystem::create_directories(wavOutput);
			FILE* oFile;
			std::string name = wavOutput + "/" + Hambit + ".wem";
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(fileBuffer, entry.fileSize, 1, oFile);
			fclose(oFile);
			std::string vgmstring = "res\\vgmstream\\vgmstream-cli.exe \"" + name + "\" -o \"" + wavOutput + "/" + Hambit + ".wav\"";
			std::cout << vgmstring << "\n";
			system(vgmstring.c_str());
			std::filesystem::remove(name);
			//HMODULE convertwem_lib = LoadLibrary(L"convert_wem.dll");
			//typedef int (*ConvertWemDef)(uint8_t* data, int length, const char* outputFolder, const char* outputName);
			//ConvertWemDef ConvertWem = (ConvertWemDef)GetProcAddress(convertwem_lib, "ConvertWem");

			//int wavFileSize;
			//ConvertWem(fileBuffer, entry.fileSize, "TEMP_TEST_FOLDER", Hambit.c_str());

			//oFile = _fsopen("temp.wav", "wb", _SH_DENYNO);
			//fwrite(wav_data, 1, sizeof(wav_data), oFile);
			//fclose(oFile);
		}
		else if (options.oggconv)
		{
			FILE* oFile;
			std::string name = wavOutput + "/" + Hambit + ".wem";
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(fileBuffer, entry.fileSize, 1, oFile);
			fclose(oFile);
			std::string oggOutput = outputPath + "\\ogg";
			std::filesystem::create_directories(oggOutput);
			std::string codebooks_filename = "res\\ww2ogg\\packed_codebooks_aoTuV_603.bin";
			std::string oggout = oggOutput + "\\" + Hambit + ".ogg";
			Wwise_RIFF_Vorbis wvorb = Wwise_RIFF_Vorbis(name, codebooks_filename, false, false, kNoForcePacketFormat);
			wvorb.print_info();
			ofstream ofp(oggout.c_str(), std::ios::binary);
			if (!ofp) throw File_open_error(oggout);
			wvorb.generate_ogg(ofp);
			ofp.close();
		}
		else
		{
			std::filesystem::create_directories(outputPath+"/wem/");
			FILE* oFile;
			std::string name = outputPath + "/wem/" + Hambit + ".wem";
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(fileBuffer, entry.fileSize, 1, oFile);
			fclose(oFile);
		}
	}
	else {
		if (options.wavconv)
		{
			std::filesystem::create_directories(wavOutput);
			FILE* oFile;
			std::string name = wavOutput + "/" + nameID + ".wem";
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(fileBuffer, entry.fileSize, 1, oFile);
			fclose(oFile);
			std::string vgmstring = "res\\vgmstream\\vgmstream-cli.exe \"" + name + "\" -o \"" + wavOutput + "/" + nameID + ".wav\"";
			std::cout << vgmstring << "\n";
			system(vgmstring.c_str());
			std::filesystem::remove(name);
		}
		else if (options.oggconv)
		{
			FILE* oFile;
			std::string name = wavOutput + "/" + nameID + ".wem";
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(fileBuffer, entry.fileSize, 1, oFile);
			fclose(oFile);
			std::string oggOutput = outputPath + "\\ogg";
			std::filesystem::create_directories(oggOutput);
			std::string codebooks_filename = "res\\ww2ogg\\packed_codebooks_aoTuV_603.bin";
			std::string oggout = oggOutput + "\\" + nameID + ".ogg";
			Wwise_RIFF_Vorbis wvorb = Wwise_RIFF_Vorbis(name, codebooks_filename, false, false, kNoForcePacketFormat);
			wvorb.print_info();
			ofstream ofp(oggout.c_str(), std::ios::binary);
			if (!ofp) throw File_open_error(oggout);
			wvorb.generate_ogg(ofp);
			ofp.close();
		}
		else {
			std::filesystem::create_directories(outputPath);
			FILE* oFile;
			std::string name = outputPath + "/" + nameID + ".wem";
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(fileBuffer, entry.fileSize, 1, oFile);
			fclose(oFile);

		}
	}

	delete[] fileBuffer;
	return true;
}

bool Package::getBnk(int i, std::string bnkOutputPath, Entry entry)
{
	unsigned char* fileBuffer = genericExtract(pkgPatchStreamPaths, i);
	std::string name = bnkOutputPath + "/" + uint16ToHexStr(header.pkgID) + "-" + uint16ToHexStr(i) + ".bnk";
	FILE* oFile;
	oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
	fwrite(fileBuffer, entry.fileSize, 1, oFile);
	fclose(oFile);

	if (options.xxh_hashes)
	{
		XXH64_hash_t hash = XXH3_64bits(fileBuffer, entry.fileSize);
		HashMap["bnk"][boost::to_upper_copy(getHashFromFile(uint16ToHexStr(header.pkgID), uint16ToHexStr(i)))] = hash;
	}

	delete[] fileBuffer;
	return true;
}

void Package::extractFiles()
{

	int wemType;
	int wemSubType;
	int bnkType;
	int bnkSubType;
	int bnkSubType2;

	if (options.preBL)
	{
		wemType = 26;
		wemSubType = 6;
		bnkType = 26;
		bnkSubType = 5;
		bnkSubType2 = 5;
	}
	else if (options.d1)
	{
		wemType = 8;
		wemSubType = 21;
		bnkType = 0;
		bnkSubType = 19;
		bnkSubType2 = 20;
		if (ps3_x360)
		{
			wemType = 8;
			wemSubType = 28;
			bnkType = 0;
			bnkSubType = 28;
		}
	}
	else
	{
		wemType = 26;
		wemSubType = 7;
		bnkType = 26;
		bnkSubType = 6;
		bnkSubType2 = 6;
	}

	if (options.outPathBase == "")
		options.outPathBase = uint16ToHexStr(header.pkgID);

	std::string bnkOutputPath = options.outPathBase + "/bnk";
	std::string out = options.outPathBase;// + uint16ToHexStr(header.pkgID);
	std::filesystem::create_directories(options.outPathBase);
	std::string outputPath = out;

	// Initialising the required file streams
	for (int i = 0; i <= header.patchID; i++)
	{
		std::string pkgPatchPath = packagePath;
		pkgPatchPath[pkgPatchPath.size() - 5] = char(i + 48);
		pkgPatchStreamPaths.push_back(pkgPatchPath);
		std::cout << pkgPatchPath << "\n";
	}

	std::vector<std::string> music_names;
	if (options.musiconly)
	{
		std::ifstream file{ "OSTs.db" };
		std::string line;

		while (std::getline(file, line))
		{
			if (line == "") continue;
			music_names.push_back(line);
		}
	}

	// Extracting each entry to a file
	for (int i = 0; i < entries.size(); i++)
	{
		Entry entry = entries[i];
		if (entry.fileSize == 0)
			continue;
		std::string Hambit = boost::to_upper_copy(entry.reference);
		std::string nameID = entry.reference2;
		if (entry.numType == wemType && entry.numSubType == wemSubType)
		{
			if (options.bnkonly || options.unknown_only)
				continue;
			if (options.musiconly)
			{
				std::vector<std::string>::iterator a = std::find(music_names.begin(), music_names.end(), Hambit);
				if (a == music_names.end())
					continue;
				else if (a != music_names.end())
					music_names.erase(a);
			}
			getWem(i, outputPath, Hambit, nameID, entry);
		}
		else if (entry.numType == bnkType && (entry.numSubType == bnkSubType || entry.numSubType == bnkSubType2))
		{
			if (options.musiconly || options.unknown_only)
				continue;
			std::filesystem::create_directories(bnkOutputPath);
			getBnk(i, bnkOutputPath, entry);
		}
		else
		{
			if (!options.unknown_only)
				continue;
			Hambit = boost::to_upper_copy(entry.reference);
			std::filesystem::create_directories(outputPath + "/" + Hambit);
			std::string name = outputPath + "/" + Hambit + "/" + boost::to_upper_copy(getHashFromFile(uint16ToHexStr(header.pkgID), uint16ToHexStr(i))) + ".bin";
			unsigned char* fileBuffer = genericExtract(pkgPatchStreamPaths, i);
			FILE* oFile;
			oFile = _fsopen(name.c_str(), "wb", _SH_DENYNO);
			fwrite(fileBuffer, entry.fileSize, 1, oFile);
			fclose(oFile);

			if (options.xxh_hashes)
			{
				XXH64_hash_t hash = XXH3_64bits(fileBuffer, entry.fileSize);
				HashMap["unk"][boost::to_upper_copy(getHashFromFile(uint16ToHexStr(header.pkgID), uint16ToHexStr(i)))] = hash;
			}

			delete[] fileBuffer;
		}
	}
	if (options.txtpgen)
	{
		std::string wwiserstr = ("py res\\wwiser\\wwiser.pyz " + bnkOutputPath + "/*.bnk -g");
		system(wwiserstr.c_str());
		std::cout << "Converted all .bnks in " + uint16ToHexStr(header.pkgID) + " to txtp" << std::endl;
	}
	if (std::filesystem::exists(".expath_temp"))
		std::filesystem::remove_all(".expath_temp");

}

// Bcrypt decryption implementation largely from Sir Kane's SourcePublic_v2.cpp, very mysterious
void Package::decryptBlock(Block block, unsigned char* blockBuffer, unsigned char*& decryptBuffer)
{
	BCRYPT_ALG_HANDLE hAesAlg;
	NTSTATUS status;
	status = BCryptOpenAlgorithmProvider(&hAesAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
	status = BCryptSetProperty(hAesAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM,
		sizeof(BCRYPT_CHAIN_MODE_GCM), 0);

	alignas(alignof(BCRYPT_KEY_DATA_BLOB_HEADER)) unsigned char keyData[sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + 16];
	BCRYPT_KEY_DATA_BLOB_HEADER* pHeader = (BCRYPT_KEY_DATA_BLOB_HEADER*)keyData;
	pHeader->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
	pHeader->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
	pHeader->cbKeyData = 16;
	memcpy(pHeader + 1, block.bitFlag & 0x4 ? AES_KEY_1 : AES_KEY_0, 16);
	BCRYPT_KEY_HANDLE hAesKey;

	status = BCryptImportKey(hAesAlg, nullptr, BCRYPT_KEY_DATA_BLOB, &hAesKey, nullptr, 0, keyData, sizeof(keyData), 0);
	ULONG decryptionResult;
	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO cipherModeInfo;

	BCRYPT_INIT_AUTH_MODE_INFO(cipherModeInfo);

	cipherModeInfo.pbTag = (PUCHAR)block.gcmTag;
	cipherModeInfo.cbTag = 0x10;
	if (options.preBL)
	{
		cipherModeInfo.pbNonce = preblnonce;
		cipherModeInfo.cbNonce = sizeof(preblnonce);
	}
	else
	{
		cipherModeInfo.pbNonce = nonce;
		cipherModeInfo.cbNonce = sizeof(nonce);
	}
	status = BCryptDecrypt(hAesKey, (PUCHAR)blockBuffer, (ULONG)block.size, &cipherModeInfo, nullptr, 0,
		(PUCHAR)decryptBuffer, (ULONG)block.size, &decryptionResult, 0);
	if (status < 0)// && status != -1073700862)
		printf("\nbcrypt decryption failed!");
	BCryptDestroyKey(hAesKey);
	BCryptCloseAlgorithmProvider(hAesAlg, 0);

	std::fill(&blockBuffer[0], &blockBuffer[block.size], 0);
}

void Package::decompressBlock(Block block, unsigned char* decryptBuffer, unsigned char*& decompBuffer)
{
	int64_t result = ((OodleLZ64_DecompressDef)OodleLZ_Decompress)(decryptBuffer, block.size, decompBuffer, BLOCK_SIZE, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 3);
	if (result <= 0)
		auto a = 0;
	std::fill(&decryptBuffer[0], &decryptBuffer[block.size], 0);
}

bool Package::initOodle()
{
	if (options.preBL || options.d1) {
		hOodleDll = LoadLibrary(L"oo2core_3_win64.dll");
	}
	else {
		hOodleDll = LoadLibrary(L"oo2core_9_win64.dll");
	}

	if (hOodleDll == nullptr) {
		return false;
	}
	OodleLZ_Decompress = (int64_t)GetProcAddress(hOodleDll, "OodleLZ_Decompress");
	if (!OodleLZ_Decompress) printf("Failed to find Oodle compress/decompress functions in DLL!");
	return true;
}

bool Package::Unpack()
{
	readHeader();
	if (!initOodle())
	{
		printf("\nFailed to initialise oodle");
		return 1;
	}
	modifyNonce();
	getEntryTable();
	getBlockTable();
	if (pkgFile != nullptr)
		fclose(pkgFile);
	extractFiles();

	entries.clear();
	blocks.clear();

	return 0;
}

// Most efficient route to getting a single entry's reference
std::string Package::getEntryReference(std::string hash)
{
	// Entry index
	uint32_t id = hexStrToUint32(hash) % 8192;

	// Entry offset
	uint32_t entryTableOffset;
	pkgFile = _fsopen(packagePath.c_str(), "rb", _SH_DENYNO);
	if (pkgFile == nullptr)
	{
		printf("\nFailed to initialise pkg file, exiting...\n");
		std::cerr << hash << " " << packagePath.c_str() << std::endl << packagePath << std::endl;
		exit(1);
	}
	if (options.preBL || options.d1)
	{
		uint32_t buildID;
		if (options.preBL)
		{
			fseek(pkgFile, 0x18, SEEK_SET);
			fread((char*)&buildID, 1, 4, pkgFile);
		}
		if (buildID != 0 && buildID <= 0x10000 || options.d1)
		{
			fseek(pkgFile, 0xB8, SEEK_SET);
			fread((char*)&entryTableOffset, 1, 4, pkgFile);
			if (ps3_x360)
				entryTableOffset = swapUInt32Endianness(entryTableOffset);
		}
		else
		{
			fseek(pkgFile, 0x110, SEEK_SET);
			fread((char*)&entryTableOffset, 1, 4, pkgFile);
			entryTableOffset += 96;
		}
	}
	else {
		fseek(pkgFile, 0x44, SEEK_SET);
		fread((char*)&entryTableOffset, 1, 4, pkgFile);
	}

	// Getting reference
	uint32_t entryA;
	fseek(pkgFile, entryTableOffset + id * 16, SEEK_SET);
	fread((char*)&entryA, 1, 4, pkgFile);
	std::string reference = uint32ToHexStr(entryA);
	fclose(pkgFile);
	return reference;
}

uint8_t Package::getEntryTypes(std::string hash, uint8_t& subType)
{
	// Entry index
	uint32_t id = hexStrToUint32(hash) % 8192;

	// Entry offset
	uint32_t entryTableOffset;
	pkgFile = _fsopen(packagePath.c_str(), "rb", _SH_DENYNO);
	if (pkgFile == nullptr)
	{
		printf("\nFailed to initialise pkg file, exiting...\n");
		std::cerr << hash << std::endl << packagePath;
		exit(1);
	}
	fseek(pkgFile, 0x44, SEEK_SET);
	fread((char*)&entryTableOffset, 1, 4, pkgFile);

	// Getting reference
	// EntryB
	uint32_t entryB;
	fseek(pkgFile, entryTableOffset + id * 16 + 4, SEEK_SET);
	fread((char*)&entryB, 1, 4, pkgFile);
	uint8_t type = (entryB >> 9) & 0x7F;
	subType = (entryB >> 6) & 0x7;
	fclose(pkgFile);
	return type;
}

// This gets the minimum required data to pull out a single file from the game
unsigned char* Package::getEntryData(std::string hash, int& fileSize)
{
	// Entry index
	uint32_t id = hexStrToUint32(hash) % 8192;

	// Header data
	if (header.pkgID == 0)
	{
		bool status = readHeader();
		if (!status) return nullptr;
	}

	if (id >= header.entryTableSize) return nullptr;

	Entry entry;

	// EntryC
	uint32_t entryC;
	fseek(pkgFile, header.entryTableOffset + id * 16 + 8, SEEK_SET);
	fread((char*)&entryC, 1, 4, pkgFile);
	entry.startingBlock = entryC & 0x3FFF;
	entry.startingBlockOffset = ((entryC >> 14) & 0x3FFF) << 4;

	// EntryD
	uint32_t entryD;
	fread((char*)&entryD, 1, 4, pkgFile);
	entry.fileSize = (entryD & 0x3FFFFFF) << 4 | (entryC >> 28) & 0xF;
	fileSize = entry.fileSize;

	// Getting data to return
	if (!initOodle())
	{
		printf("\nFailed to initialise oodle, exiting...");
		exit(1);
	}
	modifyNonce();

	for (int i = 0; i <= header.patchID; i++)
	{
		std::string pkgPatchPath = packagePath;
		pkgPatchPath[pkgPatchPath.size() - 5] = char(i + 48);
		pkgPatchStreamPaths.push_back(pkgPatchPath);
		std::cout << pkgPatchPath << "\n";
	}

	unsigned char* buffer = getBufferFromEntry(entry);
	fclose(pkgFile);
	return buffer;
}

std::unordered_map<uint64_t, uint32_t> generateH64Table(std::string packagesPath)
{
	std::set<std::string> pkgIDs;
	std::unordered_map<uint64_t, uint32_t> hash64Table;
	std::string path;
	int status;
	std::string fullPath;
	std::string reducedPath;
	uint16_t pkgIDBytes;
	std::string pkgID;
	FILE* pkgFile;
	// Getting all packages
	for (const auto& entry : std::filesystem::directory_iterator(packagesPath))
	{
		path = entry.path().string();
		status = fopen_s(&pkgFile, path.c_str(), "rb");
		if (status)
		{
			std::cerr << "FAILED GETTING PACKAGES FOR H64 ERR1515";
			exit(status);
		}
		fseek(pkgFile, 0x10, SEEK_SET);
		fread((char*)&pkgIDBytes, 1, 2, pkgFile);
		pkgID = uint16ToHexStr(pkgIDBytes);
		pkgIDs.insert(pkgID);
		fclose(pkgFile);
	}
	for (auto& pkgID : pkgIDs)
	{
		Package pkg = Package(pkgID, packagesPath, false);
		//Package pkg = Package(pkgID, packagesPath);
		status = fopen_s(&pkgFile, pkg.packagePath.c_str(), "rb");
		if (status)
		{
			std::cerr << "FAILED GETTING PACKAGES FOR H64 ERR5632";
			exit(status);
		}
		// Hash64 Table
		uint32_t hash64TableCount;
		uint32_t hash64TableOffset;
		fseek(pkgFile, 0xB8, SEEK_SET);
		fread((char*)&hash64TableCount, 1, 4, pkgFile);
		if (!hash64TableCount) continue;
		fread((char*)&hash64TableOffset, 1, 4, pkgFile);
		hash64TableOffset += 64 + 0x10;

		for (int i = hash64TableOffset; i < hash64TableOffset + hash64TableCount * 0x10; i += 0x10)
		{
			uint64_t h64Val;
			fseek(pkgFile, i, SEEK_SET);
			fread((char*)&h64Val, 1, 8, pkgFile);
			uint32_t hVal;
			fread((char*)&hVal, 1, 4, pkgFile);
			hash64Table[h64Val] = hVal;
		}
		fclose(pkgFile);
	}
	return hash64Table;
}

bool saveH64Table(std::unordered_map<uint64_t, uint32_t> hash64Table)
{
	FILE* file;
	int status = fopen_s(&file, "h64", "wb");
	if (status)
	{
		std::cerr << "FAILED WRITING H64 ERR5157";
		exit(status);
	}
	if (file == NULL) return false;
	for (auto& element : hash64Table)
	{
		fwrite(&element.first, 8, 1, file);
		fwrite(&element.second, 4, 1, file);
	}
	fclose(file);
	return true;
}

std::unordered_map<uint64_t, uint32_t> loadH64Table()
{
	std::unordered_map<uint64_t, uint32_t> hash64Table;
	FILE* file;
	int status = fopen_s(&file, "h64", "rb");
	if (status)
	{
		std::cerr << "FAILED READING H64 ERR1231";
		exit(status);
	}
	uint64_t h64Val;
	uint32_t hVal;
	fread(&h64Val, 8, 1, file);
	size_t read = fread(&hVal, 4, 1, file);
	hash64Table[h64Val] = hVal;
	while (read)
	{
		fread(&h64Val, 8, 1, file);
		read = fread(&hVal, 4, 1, file);
		hash64Table[h64Val] = hVal;
	}
	fclose(file);
	return hash64Table;
}

// For batch extraction
std::vector<std::string> Package::getAllFilesGivenRef(std::string reference)
{
	//uint32_t ref = hexStrToUint32(reference);
	std::vector<std::string> hashes;

	// Header data
	bool status = readHeader();
	if (!status) return std::vector<std::string>();

	getEntryTable();
	for (int i = 0; i < entries.size(); i++)
	{
		Entry entry = entries[i];
		if (entry.reference == reference)
		{
			uint32_t a = header.pkgID * 8192;
			uint32_t b = a + i + 2155872256;
			hashes.push_back(uint32ToHexStr(b));
		}
	}

	return hashes;
}

unsigned char* Package::getBufferFromEntry(Entry entry)
{
	if (!entry.fileSize) return nullptr;
	int blockCount = floor((entry.startingBlockOffset + entry.fileSize - 1) / BLOCK_SIZE);

	// Getting required block data
	if (options.d1) //d1 blocks are smaller because it doesnt need the gcmtag for decryption (which doesnt exist for d1)
	{
		for (uint32_t i = header.blockTableOffset + entry.startingBlock * 0x20; i <= header.blockTableOffset + entry.startingBlock * 0x20 + blockCount * 0x20; i += 0x20)
		{
			Block block = { 0, 0, 0, 0, 0 };
			fseek(pkgFile, i, SEEK_SET);
			fread((char*)&block.offset, 1, 4, pkgFile);
			fread((char*)&block.size, 1, 4, pkgFile);
			fread((char*)&block.patchID, 1, 2, pkgFile);
			fread((char*)&block.bitFlag, 1, 2, pkgFile);
			fseek(pkgFile, i + 0x20, SEEK_SET);
			blocks.push_back(block);
		}
	}
	else
	{
		for (uint32_t i = header.blockTableOffset + entry.startingBlock * 48; i <= header.blockTableOffset + entry.startingBlock * 48 + blockCount * 48; i += 48)
		{
			Block block = { 0, 0, 0, 0, 0 };
			fseek(pkgFile, i, SEEK_SET);
			fread((char*)&block.offset, 1, 4, pkgFile);
			fread((char*)&block.size, 1, 4, pkgFile);
			fread((char*)&block.patchID, 1, 2, pkgFile);
			fread((char*)&block.bitFlag, 1, 2, pkgFile);
			if (options.preBL)
			{
				fseek(pkgFile, 0x14, SEEK_CUR); // SHA-1 Hash
				fread((char*)&block.gcmTag, 16, 1, pkgFile);
			}
			else
			{
				fseek(pkgFile, i + 0x20, SEEK_SET);
				fread((char*)&block.gcmTag, 16, 1, pkgFile);
			}
			blocks.push_back(block);
		}
	}

	unsigned char* fileBuffer = new unsigned char[entry.fileSize];
	int currentBufferOffset = 0;
	int currentBlockID = 0;
	for (const Block& currentBlock : blocks) // & here is good as it captures by const reference, cheaper than by value
	{
		packagePath[packagePath.size() - 5] = currentBlock.patchID + 48;
		FILE* pFile;
		pFile = _fsopen(packagePath.c_str(), "rb", _SH_DENYNO);

		fseek(pFile, currentBlock.offset, SEEK_SET);
		unsigned char* blockBuffer = { 0 };
		std::vector<unsigned char> realblockBuffer(currentBlock.size);
		blockBuffer = &realblockBuffer[0];

		size_t result;
		result = fread(blockBuffer, 1, currentBlock.size, pFile);
		if (result != currentBlock.size) { fputs("Reading error", stderr); exit(3); }
		unsigned char* decryptBuffer = nullptr;
		std::vector<unsigned char> realDecryptBuffer(currentBlock.size);
		decryptBuffer = &realDecryptBuffer[0];

		unsigned char* decompBuffer = nullptr;
		std::vector<unsigned char> realDecompBuffer(BLOCK_SIZE);
		decompBuffer = &realDecompBuffer[0];
		if (options.d1)
		{

			if (currentBlock.bitFlag & 0x1)
				decompressBlock(currentBlock, blockBuffer, decompBuffer);
			else
				decompBuffer = blockBuffer;
		}
		else
		{
			if (currentBlock.bitFlag & 0x2)
				decryptBlock(currentBlock, blockBuffer, decryptBuffer);
			else
			{
				decryptBuffer = blockBuffer;
			}
			if (currentBlock.bitFlag & 0x1)
				decompressBlock(currentBlock, decryptBuffer, decompBuffer);
			else
				decompBuffer = decryptBuffer;
		}
		if (currentBlockID == entry.startingBlock)
		{
			size_t cpySize;
			if (currentBlockID == blockCount)
				cpySize = entry.fileSize;
			else
				cpySize = BLOCK_SIZE - entry.startingBlockOffset;
			memcpy(fileBuffer, decompBuffer + entry.startingBlockOffset, cpySize);
			currentBufferOffset += cpySize;
		}
		else if (currentBlockID == blockCount)
		{
			memcpy(fileBuffer + currentBufferOffset, decompBuffer, entry.fileSize - currentBufferOffset);
		}
		else
		{
			memcpy(fileBuffer + currentBufferOffset, decompBuffer, BLOCK_SIZE);
			currentBufferOffset += BLOCK_SIZE;
		}
		fclose(pFile);
		currentBlockID++;
		std::fill(&decompBuffer[0], &decompBuffer[sizeof(decompBuffer)], 0);
	}
	blocks.clear();
	return fileBuffer;
}
