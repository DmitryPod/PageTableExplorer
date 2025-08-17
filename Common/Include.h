/*
	include.h

	Definitions shared between projects.

	Dmitry Podvigalkin

	2025
*/
#pragma once
#ifndef _KERNEL_MODE
#include <Windows.h>
#include <array>
#else
#include <fltKernel.h>
#endif
#include <stdint.h>

typedef LARGE_INTEGER PHYSICAL_ADDRESS, * PPHYSICAL_ADDRESS;

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#define TABLE_SIZE 512
#define IOCTL_CONTROL_CODE 0x4000
#define IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_CONTROL_CODE, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Offsets from virtual address:
//   								   PML4       PDP       PD        PT        RAM
//   						           47<--------39--------29--------20--------11--------0
// 0x00007FF5639BF000 =                0111111111'110101011'000111001'101111110'00000000000
#define BYTES_47_39(x) x >> 39
#define BYTES_38_30(x) (x >> 30) & ~0b0111111111000000000
#define BYTES_29_21(x) (x >> 21) & ~0b0111111111111111111000000000
#define BYTES_20_12(x) (x >> 12) & ~0b0111111111111111111111111111000000000
#define BYTES_11_0(x) x &          ~0b011111111111111111111111111111111111100000000000

struct IOCTL_RESPONSE
{
	//
	// Registers
	uint64_t regCR0;
	uint64_t regCR2;
	uint64_t regCR3;

	//
	// Final phys address
	PHYSICAL_ADDRESS physAddress;

	//
	// PT Indexes
	ULONG b47_39;
	ULONG b38_30;
	ULONG b29_21;
	ULONG b20_12;
	ULONG b11_00;

	//
	// Tables flags
	uint64_t flagsPML4;
	uint64_t flagsPDP;
	uint64_t flagsPD;
	uint64_t flagsPT;

	//
	// Arrays with table addresses pointing to the next table in hierarchy.
	PHYSICAL_ADDRESS ptr47_39[TABLE_SIZE];
	PHYSICAL_ADDRESS ptr38_30[TABLE_SIZE];
	PHYSICAL_ADDRESS ptrb29_21[TABLE_SIZE];
	PHYSICAL_ADDRESS ptrbp20_12[TABLE_SIZE];

	//
	// The data stored in a page
	std::array<uint8_t, PAGE_SIZE> Buffer;
};

struct IOCTL_DATA
{
	ULONG pid;
	PVOID address;
	IOCTL_RESPONSE response;
};
