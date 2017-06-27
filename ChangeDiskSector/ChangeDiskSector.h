#ifndef __CHANGE_DISK_SECTOR_H__
#define __CHANGE_DISK_SECTOR_H__
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "aplib.h"

#pragma pack(push)
#pragma pack(1)
typedef struct _CHSS
{
	LARGE_INTEGER   lStartSector;
	USHORT          uNumberSectors;
	ULONG           ulXorValue;
}CHSS,*PCHSS;
typedef struct _LDRDRV
{
	ULONG ulSignature;
	ULONG ulOffset;
	ULONG ulLength;
	ULONG ulXor;
}LDRDRV,*PLDRDRV;
#pragma pack(pop)

#define		BK_NAME_MAGIC		(ULONG)0x44444444
#define		BK_NAME_LENGTH		200

extern int __argc;
extern char **__argv;
BOOLEAN GetFileDat(PCHAR pFileName,PCHAR* pDat,PULONG ulSize);

#endif