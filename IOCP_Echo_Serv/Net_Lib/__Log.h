#pragma once
#include <iostream>
#include <Windows.h>
using namespace std;

#define dfLOG_SYSTEM	0	//	�׳� �������� Ȱ��
#define dfLOG_WARNING	1	//	������ �̻��ؼ� Ŭ���̾�Ʈ�� ���ų� �ش� ������ ������
#define dfLOG_SHUTDOWN	2	//	������ ������ ����
#define dfLOG_NOTPRINT	3	//
//-------------------------------------------------------
//  17.01.18
//	�α� �ۼ���
//
//	DWORD in_dwLevel : #define �� �α��� ����
//	format, �������� ��
//
//	���� �ܼ�â�� ���, �α� �ؽ�Ʈ���Ͽ� ��´�.
//-------------------------------------------------------
void __Log(DWORD in_dwLevel, const WCHAR* format, ...);