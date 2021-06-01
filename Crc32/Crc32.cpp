#include "pch.h"
#include "Crc32.h"

static DWORD crc32_table[8][256];
__forceinline bool table()
{
	DWORD i, j, crc;
	for (i = 0; i < 256; i++)
	{
		crc = i;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
		crc32_table[0][i] = crc;
	}

	for (i = 0; i < 256; i++)
	{
		crc = crc32_table[0][i];
		for (j = 1; j < 8; j++)
		{
			crc = crc32_table[0][crc & 0xff] ^ (crc >> 8);
			crc32_table[j][i] = crc;
		}
	}

	return true;
}

DWORD Crc32::hash(LPCVOID buf, DWORD len, DWORD init_crc) {
	static bool once = true;
	if (once) {
		table();
		once = false;
	}

	auto next = (const char*)buf;
	DWORD64 crc = init_crc;

	if (len == 0)
		return (DWORD)crc;

	crc ^= 0xFFFFFFFF;

	while (len && ((DWORD)next & 7) != 0) {
		crc = crc32_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
		len--;
	}

	while (len >= 8) {
		crc ^= *(DWORD64*)next;

		crc = crc32_table[7][(crc >> 0) & 0xff] ^ crc32_table[6][(crc >> 8) & 0xff]
			^ crc32_table[5][(crc >> 16) & 0xff] ^ crc32_table[4][(crc >> 24) & 0xff]
			^ crc32_table[3][(crc >> 32) & 0xff] ^ crc32_table[2][(crc >> 40) & 0xff]
			^ crc32_table[1][(crc >> 48) & 0xff] ^ crc32_table[0][(crc >> 56)];

		next += 8;
		len -= 8;
	}

	while (len) {
		crc = crc32_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
		len--;
	}

	return (DWORD)(crc ^= 0xFFFFFFFF);
}