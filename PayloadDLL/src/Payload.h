#pragma once

#include <stdio.h>
#include <Windows.h>
#include <time.h>

__declspec(dllexport) void LogMessage(const char* message);
extern "C" __declspec(dllexport) void HookProcedure();

typedef NTSTATUS (WINAPI* pNtTerminateProcess)(

	IN	NTSTATUS	ExitStatus

);

#include "ftd2xx.h"

extern "C" {
	extern FILE* outp;
	static bool first = true;

	static FT_STATUS(WINAPI* TrueFtRead)(FT_HANDLE, LPVOID, DWORD, LPDWORD) = FT_Read;
	static FT_STATUS(WINAPI* TrueFtWrite)(FT_HANDLE, LPVOID, DWORD, LPDWORD) = FT_Write;
	static FT_STATUS(WINAPI* TrueFtIoctl)(FT_HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPOVERLAPPED) = FT_IoCtl;


	void get_time(char buf[], size_t bufsize) {
		time_t rawtime;
		struct tm timeinfo;

		time(&rawtime);
		int err = localtime_s(&timeinfo, &rawtime);
		strftime(buf, bufsize, "%H:%M:%S", &timeinfo);
	}

	FT_STATUS WINAPI Hooked_FT_Read(
		FT_HANDLE ftHandle,
		LPVOID lpBuffer,
		DWORD nBufferSize,
		LPDWORD lpBytesReturned
	) {
		FT_STATUS ret = TrueFtRead(ftHandle, lpBuffer, nBufferSize, lpBytesReturned);
		if (first) {
			int err = fopen_s(&outp, "C:\\Users\\Mit\\Desktop\\ftdlog.txt", "w");
			if (err == 0 || outp == NULL) {
				printf("Could not create the output log file.\n");
			}
			first = false;
		}
		if (outp)
		{
			char out[100] = { 0, };
			get_time(out, 100);
			fprintf(outp, "SerRead [%u/%u, %d]: ", *lpBytesReturned, nBufferSize, ret);
			for (DWORD i = 0; i < *lpBytesReturned; i++)
				fprintf(outp, "%02x ", ((char*)lpBuffer)[i] & 0xff);
			fprintf(outp, "\n\n");
			// fflush(outp);
		}
		return ret;
	}

	FT_STATUS WINAPI Hooked_FT_Write(
		FT_HANDLE ftHandle,
		LPVOID lpBuffer,
		DWORD nBufferSize,
		LPDWORD lpBytesWritten
	) {
		FT_STATUS ret = TrueFtWrite(ftHandle, lpBuffer, nBufferSize, lpBytesWritten);
		if (first) {
			int err = fopen_s(&outp, "C:\\Users\\Mit\\Desktop\\ftdlog.txt", "w");
			if (err == 0 || outp == NULL) {
				printf("Could not create the output log file.\n");
			}
			first = false;
		}
		if (outp)
		{
			char out[100] = { 0, };
			get_time(out, 100);
			fprintf(outp, "SerWrite [%u/%u, %d]: ", *lpBytesWritten, nBufferSize, ret);
			for (DWORD i = 0; i < nBufferSize; i++)
				fprintf(outp, "%02x ", ((char*)lpBuffer)[i] & 0xff);
			fprintf(outp, "\n\n");
			// fflush(outp);
		}
		return ret;
	}

	FT_STATUS WINAPI Hooked_FT_IoCtl(
		FT_HANDLE ftHandle,
		DWORD dwIoControlCode,
		LPVOID lpInBuf,
		DWORD nInBufSize,
		LPVOID lpOutBuf,
		DWORD nOutBufSize,
		LPDWORD lpBytesReturned,
		LPOVERLAPPED lpOverlapped
	) {
		char out[100] = { 0, };
		FT_STATUS ret = TrueFtIoctl(ftHandle, dwIoControlCode, lpInBuf, nInBufSize, lpOutBuf, nOutBufSize, lpBytesReturned, lpOverlapped);
		if (first) {
			int err = fopen_s(&outp, "C:\\Users\\Mit\\Desktop\\ftdlog.txt", "w");
			if (err == 0 || outp == NULL) {
				printf("Could not create the output log file.\n");
			}
			first = false;
		} 
		if (outp)
		{
			get_time(out, 100);
			fprintf(outp, "IoctlWrite [%u/-1]: ", nOutBufSize);
			for (DWORD i = 0; i < nOutBufSize; i++)
				fprintf(outp, "%02x ", ((char*)lpOutBuf)[i] & 0xff);
			fprintf(outp, "\n");
			fprintf(outp, "IoctlRead [%u/%u]: ", *lpBytesReturned, nInBufSize);
			DWORD cond = min(nInBufSize, *lpBytesReturned);
			for (DWORD i = 0; i < cond; i++)
				fprintf(outp, "%02x ", ((char*)lpInBuf)[i] & 0xff);
			fprintf(outp, "\n\n");
			// fflush(outp); */
		} 
		return ret;
	}
}
