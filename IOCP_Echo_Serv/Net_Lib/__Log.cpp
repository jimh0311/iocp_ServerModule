#include "__Log.h"
#include <time.h>
#pragma warning (disable:4996)

// ����� �ּ� �α� ����...
DWORD g_Level = 0;

//-------------------------------------------------------
//  17.01.18
//	�α� �ۼ���
//
//	DWORD in_dwLevel : #define �� �α��� ����
//	format, �������� ��
//
//	���� �ܼ�â�� ���, �α� �ؽ�Ʈ���Ͽ� ��´�.
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
	// �α����� �̸� ����...
	wsprintf(fileBuf, L"%d_%02d_%02d.txt", 1900 + newtime->tm_year, newtime->tm_mon + 1, newtime->tm_mday);
	// ����� buf �����ϰ�
	wsprintf(buf, L"__LOG [%02d:%02d:%02d] : ", newtime->tm_hour, newtime->tm_min, newtime->tm_sec);

	va_start(ap, format);
	vswprintf(buf + wcslen(buf), format, ap);

	//�ܼ�ȭ�鿡�� print ���� ���̴�.
	if(in_dwLevel != dfLOG_NOTPRINT)
		wprintf(L"%s\n",buf);

	FILE *fp;
	_wfopen_s(&fp, fileBuf, L"at");
	if (fp == NULL)
		return;
	fwprintf(fp, L"%s\n", buf);
	fclose(fp);
}