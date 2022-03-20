#include "helpers.h"

std::string uint16ToHexStr(uint16_t num)
{
	std::stringstream stream;
	stream << std::hex << num;
	std::string hexStr = stream.str();
	if (hexStr.size() % 4 != 0)
		hexStr = std::string(4 - (hexStr.size() % 4), '0').append(hexStr);
	return hexStr;
}

std::string uint32ToHexStr(uint32_t num)
{
	std::stringstream stream;
	stream << std::hex << swapUInt32Endianness(num);
	std::string hexStr = stream.str();
	if (hexStr.size() % 8 != 0)
		hexStr = std::string(8 - (hexStr.size() % 8), '0').append(hexStr);
	return hexStr;
}

uint32_t hexStrToUint16(std::string hash)
{
	return swapUInt16Endianness(std::stoul(hash, nullptr, 16));
}

uint32_t hexStrToUint32(std::string hash)
{
	return swapUInt32Endianness(std::stoul(hash, nullptr, 16));
}

uint64_t hexStrToUint64(std::string hash)
{
	return swapUInt64Endianness(std::stoull(hash, nullptr, 16));
}

uint16_t swapUInt16Endianness(uint16_t x)
{
	x = (x << 8) + (x >> 8);
	return x;
}

uint32_t swapUInt32Endianness(uint32_t x)
{
	x = (x >> 24) |
		((x << 8) & 0x00FF0000) |
		((x >> 8) & 0x0000FF00) |
		(x << 24);
	return x;
}

uint64_t swapUInt64Endianness(uint64_t k)
{
	return ((k << 56) |
		((k & 0x000000000000FF00) << 40) |
		((k & 0x0000000000FF0000) << 24) |
		((k & 0x00000000FF000000) << 8) |
		((k & 0x000000FF00000000) >> 8) |
		((k & 0x0000FF0000000000) >> 24) |
		((k & 0x00FF000000000000) >> 40) |
		(k >> 56)
		);
}

std::string getFileFromHash(std::string hsh)
{
	uint32_t first_int = hexStrToUint32(hsh);
	//std::cout << std::to_string(first_int) << std::endl;
	uint32_t one = first_int - 2155872256;
	std::string first_hex = uint16ToHexStr(floor(one / 8192));
	std::string second_hex = uint16ToHexStr(first_int % 8192);
	//std::cout << first_hex + "-" + second_hex << std::endl;
	return(first_hex + "-" + second_hex);
}

std::string getHashFromFile(std::string pkgn, std::string i)
{
	uint16_t firsthex_int;
	uint16_t secondhex_int;
	uint32_t one;
	std::string two;
	firsthex_int = swapUInt16Endianness(hexStrToUint16(pkgn)); //firsthex is the package id, converted to uint16, then flip endianness (weird shit with endianness, blame c++ and probably myself)
	secondhex_int = swapUInt16Endianness(hexStrToUint16(i)); //secondhex is the number that just counts up, conv to uint16, flip endianness
	one = firsthex_int * 8192;
	two = uint32ToHexStr(one + secondhex_int + 2155872256); // two is full hash
	return two;
}