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
// ��Ŷ  �ı�.
//
// Parameters: ����.
// Return: ����.
//////////////////////////////////////////////////////////////////////////
void	CProtoBuffer::Release(void)
{
	_idxRead = 0;
	_idxWrite = 0;
	m_iDataSize = 0;
}


//////////////////////////////////////////////////////////////////////////
// ��Ŷ û��.
//
// Parameters: ����.
// Return: ����.
//////////////////////////////////////////////////////////////////////////
void	CProtoBuffer::Clear(void)
{
	_idxRead = 0;
	_idxWrite = 0;
	m_iDataSize = 0;
}

//////////////////////////////////////////////////////////////////////////
// ���� Pos �̵�. (�����̵��� �ȵ�)
// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
//
// Parameters: (int) �̵� ������.
// Return: (int) �̵��� ������.
//////////////////////////////////////////////////////////////////////////
int		CProtoBuffer::MoveWritePos(int iSize)
{
	/////////////////////////////	���� ����ó��...
	if (iSize <= 0)
	{
		return -1;
	}
	////////////////////////////	���� ����ó��...
	int tempRemainSize = m_iBufferSize - _idxWrite - 1;
	if (tempRemainSize <= 0)
	{
		return 0;
	}
	///////////////////////////		�������...
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
	////////////////////////////	���� ����ó��...
	if (iSize <= 0)
	{
		return -1;
	}
	////////////////////////////	���� ����ó��...
	int tempCurrentUsingSize = _idxWrite - _idxRead;
	if (tempCurrentUsingSize <= 0)
	{
		return 0;
	}

	///////////////////////////		�������...
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
// ������ ���۷�����.
/* ============================================================================= */
CProtoBuffer&	CProtoBuffer::operator = (CProtoBuffer &clSrcAyaPacket)
{
	// �����͸� �����ϴ��� ���� �����ؾ��Ѵ�.
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
// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
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
// ����.	�� ���� Ÿ�Ը��� ��� ����.
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
// ����Ÿ ���.
//
// Parameters: (char *)Dest ������. (int)Size.
// Return: (int)������ ������.
//////////////////////////////////////////////////////////////////////////
int		CProtoBuffer::Enqueue(char *chpDest, int iSize)
{
	////////////////////////////////	����ó��...
	if (chpDest == NULL || iSize <= 0)
	{
		return -1;
	}
	///////////////////////////////////////		����ó��....
	int tempRemainSize = m_iBufferSize - _idxWrite - 1;
	if (tempRemainSize <= 0)
	{
		return 0;
	}

	///////////////////////////		�������...
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
// ����Ÿ ����.
//
// Parameters: (char *)Src ������. (int)SrcSize.
// Return: (int)������ ������.
//////////////////////////////////////////////////////////////////////////
int		CProtoBuffer::Dequeue(char *chpSrc, int iSrcSize)
{
	////////////////////////////////////////	����ó��....
	if (chpSrc == NULL || iSrcSize <= 0)
	{
		return -1;
	}
	///////////////////////////////////////		����ó��....
	int tempCurrentUsingSize = _idxWrite - _idxRead;
	if (tempCurrentUsingSize <= 0)
	{
		return 0;
	}

	///////////////////////////		�������...
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


