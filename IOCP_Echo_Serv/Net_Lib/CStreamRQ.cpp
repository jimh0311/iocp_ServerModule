
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
// 현재 사용중인 용량 얻기.
//
// Parameters: 없음.
// Return: (int)사용중인 용량.
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
// 현재 버퍼에 남은 용량 얻기.
//
// Parameters: 없음.
// Return: (int)남은용량.
/////////////////////////////////////////////////////////////////////////
int	CStreamRQ::GetFreeSize(void)
{
	return (_iMax - GetCurrentUsingSize()) - 1;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
// (끊기지 않은 길이)
//
// Parameters: 없음.
// Return: (int)사용가능 용량.
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
// WritePos 에 데이타 넣음.
//
// Parameters: (char *)데이타 포인터. (int)크기. 
// Return: (int)넣은 크기.
//
// 에러코드 -2 는 배열의 길이보다 iSize를 크게 넣었을 때 뱉는다.
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
// ReadPos 에서 데이타 가져옴. ReadPos 이동.
//
// Parameters: (char *)데이타 포인터. (int)크기.
// Return: (int)가져온 크기.
/////////////////////////////////////////////////////////////////////////
int	CStreamRQ::Dequeue(char *chpDest, int iSize)
{
	int iCurrentUsingSize = GetCurrentUsingSize();
	
	//비어있는 큐이다. 십새얌.
	if (iCurrentUsingSize == 0)
	{
		return 0;
	}
	// 데이터가 작은 상황입니다만?
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
	//정상적으로 해당 데이터 만큼 들어가있다고 가정할 때...
	else if( iSize <= iCurrentUsingSize)
	{
		//짤릴 위험성이 있는 경우.
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
		//없는 경우
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
// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
//
// Parameters: (char *)데이타 포인터. (int)크기.
// Return: (int)가져온 크기.
/////////////////////////////////////////////////////////////////////////
int	CStreamRQ::Peek(char *chpDest, int iSize)
{
	int tempIdxRead = _idxRead;
	int ret = Dequeue(chpDest, iSize);
	_idxRead = tempIdxRead;
	return ret;
}


/////////////////////////////////////////////////////////////////////////
// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
//
// Parameters: 없음.
// Return: 없음.
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
// 버퍼의 모든 데이타 삭제.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////
void	CStreamRQ::ClearBuffer(void)
{
	_idxRead = 0;
	_idxWrite = 0;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼의 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char*	CStreamRQ::GetBufferPtr(void)
{
	return _arrQueue;
}

/////////////////////////////////////////////////////////////////////////
// 버퍼의 ReadPos 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char*	CStreamRQ::GetReadBufferPtr(void)
{
	return &_arrQueue[_idxRead];
}

/////////////////////////////////////////////////////////////////////////
// 버퍼의 WritePos 포인터 얻음.
//
// Parameters: 없음.
// Return: (char *) 버퍼 포인터.
/////////////////////////////////////////////////////////////////////////
char*	CStreamRQ::GetWriteBufferPtr(void)
{
	return &_arrQueue[_idxWrite];
}
