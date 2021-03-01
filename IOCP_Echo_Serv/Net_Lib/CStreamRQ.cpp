
#include "CStreamRQ.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>


CStreamRQ::CStreamRQ() : _iMax(CStreamRQ::enMAX_QUEUE)
{
	_arrQueue = new char[_iMax];
	_idxWrite = 0;
	_idxRead = 0;
}
CStreamRQ::CStreamRQ(int max): _iMax(max)
{
	_arrQueue = new char[_iMax];
	_idxWrite = 0;
	_idxRead = 0;
}
CStreamRQ::~CStreamRQ()
{
	delete[] _arrQueue;
}

int	CStreamRQ::GetBufferSize(void)
{
	return _iMax;
}

/////////////////////////////////////////////////////////////////////////
// ���� ������� �뷮 ���.
//
// Parameters: ����.
// Return: (int)������� �뷮.
/////////////////////////////////////////////////////////////////////////
int	CStreamRQ::GetCurrentUsingSize(void)
{
	if (_idxWrite > _idxRead) {
		return _idxWrite - _idxRead;
	}
	else if (_idxWrite < _idxRead)
	{
		return (_iMax - _idxRead) + _idxWrite;
	}
	else {
		return 0;	// EMPTY
	}
}

/////////////////////////////////////////////////////////////////////////
// ���� ���ۿ� ���� �뷮 ���.
//
// Parameters: ����.
// Return: (int)�����뷮.
/////////////////////////////////////////////////////////////////////////
int	CStreamRQ::GetFreeSize(void)
{
	return (_iMax - GetCurrentUsingSize()) - 1;
}

/////////////////////////////////////////////////////////////////////////
// ���� �����ͷ� �ܺο��� �ѹ濡 �а�, �� �� �ִ� ����.
// (������ ���� ����)
//
// Parameters: ����.
// Return: (int)��밡�� �뷮.
////////////////////////////////////////////////////////////////////////
int	CStreamRQ::GetNotBrokenGetSize(void)
{
	if (_idxRead >= _idxWrite)
	{
		int iReturn;
		iReturn = _iMax - _idxRead;
		return iReturn;
	}
	return GetCurrentUsingSize();
}
int	CStreamRQ::GetNotBrokenPutSize(void)
{
	if (_idxWrite >= _idxRead)
	{
		int iReturn;
		iReturn = _iMax - _idxWrite;
		if (_idxRead == 0)
			iReturn = iReturn - 1;

		return iReturn;
	}
	return GetFreeSize();
}

/////////////////////////////////////////////////////////////////////////
// WritePos �� ����Ÿ ����.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��. 
// Return: (int)���� ũ��.
//
// �����ڵ� -2 �� �迭�� ���̺��� iSize�� ũ�� �־��� �� ��´�.
/////////////////////////////////////////////////////////////////////////

int	CStreamRQ::Enqueue(char *chpData, int iSize)
{	
	int iFree = GetFreeSize();

	if (_idxWrite >= _idxRead)
	{
		//
		int LeftSide;
		int RightSide;
		LeftSide = _iMax - _idxWrite;
		RightSide = _idxRead - 1;
		if (RightSide == -1)
		{
			LeftSide--;
		}
		//
		if (iFree >= iSize)
		{
			if (LeftSide >= iSize)
			{
				memcpy_s(&_arrQueue[_idxWrite], LeftSide, chpData, iSize);
				_idxWrite = (_idxWrite + iSize) % _iMax;
				return iSize;
			}
			else
			{
				memcpy_s(&_arrQueue[_idxWrite], LeftSide, chpData, LeftSide);
				memcpy_s(_arrQueue, RightSide, &chpData[LeftSide], iSize - LeftSide);
				_idxWrite = (_idxWrite + iSize) % _iMax;
				return iSize;
			}
		}
		else
		{
			memcpy_s(&_arrQueue[_idxWrite], LeftSide, chpData, LeftSide);
			if (_idxRead == -1 || _idxRead == 0)
			{
				_idxWrite = (_idxWrite + LeftSide) % _iMax;
				return LeftSide;
			}
			memcpy_s(_arrQueue, RightSide, &chpData[LeftSide], RightSide);
			_idxWrite = (_idxWrite + LeftSide + RightSide) % _iMax;
			return LeftSide + RightSide;
		}
	}
	else
	{
		if (iFree >= iSize)
		{
			memcpy_s(&_arrQueue[_idxWrite], iSize, chpData, iSize);
			_idxWrite = (_idxWrite + iSize) % _iMax;
			return iSize;
		}
		else
		{
			memcpy_s(&_arrQueue[_idxWrite], iFree, chpData, iFree);
			_idxWrite = (_idxWrite + iFree) % _iMax;
			return iFree;
		}
	}

	return 0;
}




/////////////////////////////////////////////////////////////////////////
// ReadPos ���� ����Ÿ ������. ReadPos �̵�.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
/////////////////////////////////////////////////////////////////////////
int	CStreamRQ::Dequeue(char *chpDest, int iSize)
{
	int iCurrentUsingSize = GetCurrentUsingSize();
	
	//����ִ� ť�̴�. �ʻ���.
	if (iCurrentUsingSize == 0)
	{
		return 0;
	}
	// �����Ͱ� ���� ��Ȳ�Դϴٸ�?
	if (iSize > iCurrentUsingSize)
	{	
		if (_idxRead > _idxWrite)
		{
			int iLengthToEnd = _iMax - _idxRead;
			memcpy_s(chpDest, iSize, &_arrQueue[_idxRead], iLengthToEnd);
			if (_idxWrite == 0)
			{
				_idxRead = (_idxRead + iLengthToEnd) % _iMax;
				return iLengthToEnd;
			}
			else
			{
				memcpy_s(&chpDest[iLengthToEnd], iSize - iLengthToEnd, _arrQueue, _idxWrite);
				_idxRead = (_idxRead + iLengthToEnd + _idxWrite) % _iMax;
				return iLengthToEnd + _idxWrite;
			}
			
		}
		else
		{
			memcpy_s(chpDest, iSize, &_arrQueue[_idxRead], iCurrentUsingSize);
			_idxRead = (_idxRead + iCurrentUsingSize) % _iMax;

			return iCurrentUsingSize;
		}
	}
	//���������� �ش� ������ ��ŭ ���ִٰ� ������ ��...
	else if( iSize <= iCurrentUsingSize)
	{
		//©�� ���輺�� �ִ� ���.
		if (_idxRead > _idxWrite)
		{
			int iLengthToEnd = _iMax - _idxRead;
			int iLengthToWrite = _idxWrite;
			if (iLengthToEnd >= iSize)
			{
				memcpy_s(chpDest, iSize, &_arrQueue[_idxRead], iSize);
				_idxRead = (_idxRead + iSize) % _iMax;
				return iSize;
			}
			else
			{
				memcpy_s(chpDest, iSize, &_arrQueue[_idxRead], iLengthToEnd);
				//_idxRead = (_idxRead + iLengthToEnd) % _iMax;
				memcpy_s(&chpDest[iLengthToEnd], iSize - iLengthToEnd, _arrQueue, iSize - iLengthToEnd);
				_idxRead = (_idxRead + iSize) % _iMax;
				return iSize;
			}
		}
		//���� ���
		else
		{
			memcpy_s(chpDest, iSize, &_arrQueue[_idxRead], iSize);
			_idxRead = (_idxRead + iSize) % _iMax;
			return iSize;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////
// ReadPos ���� ����Ÿ �о��. ReadPos ����.
//
// Parameters: (char *)����Ÿ ������. (int)ũ��.
// Return: (int)������ ũ��.
/////////////////////////////////////////////////////////////////////////
int	CStreamRQ::Peek(char *chpDest, int iSize)
{
	int tempIdxRead = _idxRead;
	int ret = Dequeue(chpDest, iSize);
	_idxRead = tempIdxRead;
	return ret;
}


/////////////////////////////////////////////////////////////////////////
// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////
void	CStreamRQ::RemoveData(int iSize)
{
	if (iSize > GetCurrentUsingSize())
	{
		return;
	}
	_idxRead = (_idxRead + iSize) % _iMax;
}
int		CStreamRQ::MoveWritePos(int iSize)
{
	_idxWrite = (_idxWrite + iSize) % _iMax;
	return _idxWrite;
}

/////////////////////////////////////////////////////////////////////////
// ������ ��� ����Ÿ ����.
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////
void	CStreamRQ::ClearBuffer(void)
{
	_idxRead = 0;
	_idxWrite = 0;
}

/////////////////////////////////////////////////////////////////////////
// ������ ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char*	CStreamRQ::GetBufferPtr(void)
{
	return _arrQueue;
}

/////////////////////////////////////////////////////////////////////////
// ������ ReadPos ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char*	CStreamRQ::GetReadBufferPtr(void)
{
	return &_arrQueue[_idxRead];
}

/////////////////////////////////////////////////////////////////////////
// ������ WritePos ������ ����.
//
// Parameters: ����.
// Return: (char *) ���� ������.
/////////////////////////////////////////////////////////////////////////
char*	CStreamRQ::GetWriteBufferPtr(void)
{
	return &_arrQueue[_idxWrite];
}
