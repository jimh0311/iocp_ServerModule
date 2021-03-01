#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "CProtoBuffer.h"


CProtoBuffer::CProtoBuffer()
{
	m_iBufferSize = eBUFFER_DEFAULT;
	m_chpReadPos = m_chaBufferDefault;
	m_chpWritePos = m_chaBufferDefault;

	_idxRead = 0;
	_idxWrite = 0;
	m_iDataSize = 0;
}

CProtoBuffer::CProtoBuffer(int iSize)
{
	m_iBufferSize = iSize;
	m_chpReadPos = m_chaBufferDefault;
	m_chpWritePos = m_chaBufferDefault;

	_idxRead = 0;
	_idxWrite = 0;
	m_iDataSize = 0;
}


//////////////////////////////////////////////////////////////////////////
// 패킷  파괴.
//
// Parameters: 없음.
// Return: 없음.
//////////////////////////////////////////////////////////////////////////
void	CProtoBuffer::Release(void)
{
	_idxRead = 0;
	_idxWrite = 0;
	m_iDataSize = 0;
}


//////////////////////////////////////////////////////////////////////////
// 패킷 청소.
//
// Parameters: 없음.
// Return: 없음.
//////////////////////////////////////////////////////////////////////////
void	CProtoBuffer::Clear(void)
{
	_idxRead = 0;
	_idxWrite = 0;
	m_iDataSize = 0;
}

//////////////////////////////////////////////////////////////////////////
// 버퍼 Pos 이동. (음수이동은 안됨)
// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
//
// Parameters: (int) 이동 사이즈.
// Return: (int) 이동된 사이즈.
//////////////////////////////////////////////////////////////////////////
int		CProtoBuffer::MoveWritePos(int iSize)
{
	/////////////////////////////	음수 예외처리...
	if (iSize <= 0)
	{
		return -1;
	}
	////////////////////////////	음수 예외처리...
	int tempRemainSize = m_iBufferSize - _idxWrite - 1;
	if (tempRemainSize <= 0)
	{
		return 0;
	}
	///////////////////////////		정상로직...
	if (iSize > tempRemainSize)
	{
		m_iDataSize += tempRemainSize;
		_idxWrite += tempRemainSize;
		return tempRemainSize;
	}
	else
	{
		m_iDataSize += iSize;
		_idxWrite += iSize;
		return iSize;
	}
	return 0;
}
int		CProtoBuffer::MoveReadPos(int iSize)
{
	////////////////////////////	음수 예외처리...
	if (iSize <= 0)
	{
		return -1;
	}
	////////////////////////////	음수 예외처리...
	int tempCurrentUsingSize = _idxWrite - _idxRead;
	if (tempCurrentUsingSize <= 0)
	{
		return 0;
	}

	///////////////////////////		정상로직...
	if (iSize > tempCurrentUsingSize)
	{
		m_iDataSize -= tempCurrentUsingSize;
		_idxRead += tempCurrentUsingSize;
		return tempCurrentUsingSize;
	}
	else
	{
		_idxRead += iSize;
		m_iDataSize -= iSize;
		return iSize;
	}
	return 0;
}






/* ============================================================================= */
// 연산자 오퍼레이터.
/* ============================================================================= */
CProtoBuffer&	CProtoBuffer::operator = (CProtoBuffer &clSrcAyaPacket)
{
	// 데이터만 복사하더라도 모든걸 복사해야한다.
	if (m_iBufferSize == clSrcAyaPacket.m_iBufferSize)
	{
		memcpy_s(m_chaBufferDefault, m_iBufferSize, clSrcAyaPacket.m_chaBufferDefault, clSrcAyaPacket.m_iBufferSize);
		_idxRead = clSrcAyaPacket._idxRead;
		_idxWrite = clSrcAyaPacket._idxWrite;
		m_iDataSize = clSrcAyaPacket.m_iDataSize;
	}
	return *this;
}


//////////////////////////////////////////////////////////////////////////
// 넣기.	각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
CProtoBuffer&	CProtoBuffer::operator << (BYTE byValue)
{
	Enqueue((char*)&byValue, sizeof(BYTE));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator << (char chValue)
{
	Enqueue(&chValue, sizeof(char));
	return *this;
}

CProtoBuffer&	CProtoBuffer::operator << (short shValue)
{
	Enqueue((char*)&shValue, sizeof(short));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator << (WORD wValue)
{
	Enqueue((char*)&wValue, sizeof(WORD));
	return *this;
}

CProtoBuffer&	CProtoBuffer::operator << (int iValue)
{
	Enqueue((char*)&iValue, sizeof(int));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator << (DWORD dwValue)
{
	Enqueue((char*)&dwValue, sizeof(DWORD));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator << (float fValue)
{
	Enqueue((char*)&fValue, sizeof(float));
	return *this;
}

CProtoBuffer&	CProtoBuffer::operator << (__int64 iValue)
{
	Enqueue((char*)&iValue, sizeof(__int64));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator << (double dValue)
{
	Enqueue((char*)&dValue, sizeof(double));
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 빼기.	각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
CProtoBuffer&	CProtoBuffer::operator >> (BYTE &byValue)
{
	Dequeue((char*)&byValue, sizeof(BYTE));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator >> (char &chValue)
{
	Dequeue((char*)&chValue, sizeof(char));
	return *this;
}

CProtoBuffer&	CProtoBuffer::operator >> (short &shValue)
{
	Dequeue((char*)&shValue, sizeof(short));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator >> (WORD &wValue)
{
	Dequeue((char*)&wValue, sizeof(WORD));
	return *this;
}

CProtoBuffer&	CProtoBuffer::operator >> (int &iValue)
{
	Dequeue((char*)&iValue, sizeof(int));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator >> (DWORD &dwValue)
{
	Dequeue((char*)&dwValue, sizeof(DWORD));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator >> (float &fValue)
{
	Dequeue((char*)&fValue, sizeof(float));
	return *this;
}

CProtoBuffer&	CProtoBuffer::operator >> (__int64 &iValue)
{
	Dequeue((char*)&iValue, sizeof(__int64));
	return *this;
}
CProtoBuffer&	CProtoBuffer::operator >> (double &dValue)
{
	Dequeue((char*)&dValue, sizeof(double));
	return *this;
}



//////////////////////////////////////////////////////////////////////////
// 데이타 얻기.
//
// Parameters: (char *)Dest 포인터. (int)Size.
// Return: (int)복사한 사이즈.
//////////////////////////////////////////////////////////////////////////
int		CProtoBuffer::Enqueue(char *chpDest, int iSize)
{
	////////////////////////////////	예외처리...
	if (chpDest == NULL || iSize <= 0)
	{
		return -1;
	}
	///////////////////////////////////////		예외처리....
	int tempRemainSize = m_iBufferSize - _idxWrite - 1;
	if (tempRemainSize <= 0)
	{
		return 0;
	}

	///////////////////////////		정상로직...
	if (iSize > tempRemainSize)
	{
		memcpy_s(&m_chaBufferDefault[_idxWrite], tempRemainSize, chpDest, tempRemainSize);
		m_iDataSize += tempRemainSize;
		_idxWrite += tempRemainSize;
		return tempRemainSize;
	}
	else
	{
		memcpy_s(&m_chaBufferDefault[_idxWrite], tempRemainSize, chpDest, iSize);
		m_iDataSize += iSize;
		_idxWrite += iSize;
		return iSize;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 데이타 삽입.
//
// Parameters: (char *)Src 포인터. (int)SrcSize.
// Return: (int)복사한 사이즈.
//////////////////////////////////////////////////////////////////////////
int		CProtoBuffer::Dequeue(char *chpSrc, int iSrcSize)
{
	////////////////////////////////////////	예외처리....
	if (chpSrc == NULL || iSrcSize <= 0)
	{
		return -1;
	}
	///////////////////////////////////////		예외처리....
	int tempCurrentUsingSize = _idxWrite - _idxRead;
	if (tempCurrentUsingSize <= 0)
	{
		return 0;
	}

	///////////////////////////		정상로직...
	if (iSrcSize > tempCurrentUsingSize)
	{
		memcpy_s(chpSrc, tempCurrentUsingSize, &m_chaBufferDefault[_idxRead], tempCurrentUsingSize);
		m_iDataSize -= tempCurrentUsingSize;
		_idxRead += tempCurrentUsingSize;
		return tempCurrentUsingSize;
	}
	else
	{
		memcpy_s(chpSrc, iSrcSize, &m_chaBufferDefault[_idxRead], iSrcSize);
		m_iDataSize -= iSrcSize;
		_idxRead += iSrcSize;
		return iSrcSize;
	}
}


