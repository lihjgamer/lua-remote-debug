#pragma once


class CCS
{
public:
	CCS()
	{
		::InitializeCriticalSectionAndSpinCount(&_cs,1000);
	}

	~CCS()
	{
		::DeleteCriticalSection(&_cs);
	}

	void Lock()
	{
		::EnterCriticalSection(&_cs);
	}

	void Unlock()
	{
		::LeaveCriticalSection(&_cs);
	}

private:
	CRITICAL_SECTION _cs;

};


class CAutoLock
{
public:
	explicit CAutoLock(CCS& m)
		:_m(m)
	{
		_m.Lock();
	}
	~CAutoLock(void)
	{
		_m.Unlock();
	}

private:
	CCS& _m;
};


template <class T>
class CAtomic
{
public:
	CAtomic()
		:_m()
	{

	}

	CAtomic(const T& t)
		:_m(t)
	{

	}


	~CAtomic()
	{

	}

	T load()
	{
		CAutoLock lock(_cs);
		return _m;
	}

	void store(T t)
	{
		CAutoLock lock(_cs);
		_m = t;
	}

// 	T operator()
// 	{
// 		CAutoLock lock(_cs);
// 		return _m;
// 	}
private:
	CCS _cs;
	T _m;
};