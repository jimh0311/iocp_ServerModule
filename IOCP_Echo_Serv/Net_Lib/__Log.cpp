#include "__Log.h"
#include <time.h>
#pragma warning (disable:4996)

// 출력할 최소 로그 레벨...
DWORD g_Level = 0;

//-------------------------------------------------------
//  17.01.18
//	로그 작성용
//
//	DWORD in_dwLevel : #define 된 로그의 레벨
//	format, 가변인자 ㅗ
//
//	서버 콘솔창에 찍고, 로그 텍스트파일에 찍는다.
//-------------------------------------------------------
void __Log(DWORD in_dwLevel, const WCHAR* format, ...)
{
	va_list ap;
	WCHAR buf[1024];
	WCHAR fileBuf[64];
	
	struct tm *newtime;
	__time64_t long_time;
	_time64(&long_time);			
	newtime = _localtime64(&long_time); 					
	// 로그파일 이름 설정...
	wsprintf(fileBuf, L"%d_%02d_%02d.txt", 1900 + newtime->tm_year, newtime->tm_mon + 1, newtime->tm_mday);
	// 출력할 buf 설정하고
	wsprintf(buf, L"__LOG [%02d:%02d:%02d] : ", newtime->tm_hour, newtime->tm_min, newtime->tm_sec);

	va_start(ap, format);
	vswprintf(buf + wcslen(buf), format, ap);

	//콘솔화면에는 print 안할 것이다.
	if(in_dwLevel != dfLOG_NOTPRINT)
		wprintf(L"%s\n",buf);

	FILE *fp;
	_wfopen_s(&fp, fileBuf, L"at");
	if (fp == NULL)
		return;
	fwprintf(fp, L"%s\n", buf);
	fclose(fp);
}