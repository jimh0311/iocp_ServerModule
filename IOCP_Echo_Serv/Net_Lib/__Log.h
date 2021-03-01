#pragma once
#include <iostream>
#include <Windows.h>
using namespace std;

#define dfLOG_SYSTEM	0	//	그냥 정상적인 활동
#define dfLOG_WARNING	1	//	로직이 이상해서 클라이언트를 끊거나 해당 로직을 점프함
#define dfLOG_SHUTDOWN	2	//	서버가 꺼지는 에러
#define dfLOG_NOTPRINT	3	//
//-------------------------------------------------------
//  17.01.18
//	로그 작성용
//
//	DWORD in_dwLevel : #define 된 로그의 레벨
//	format, 가변인자 ㅗ
//
//	서버 콘솔창에 찍고, 로그 텍스트파일에 찍는다.
//-------------------------------------------------------
void __Log(DWORD in_dwLevel, const WCHAR* format, ...);