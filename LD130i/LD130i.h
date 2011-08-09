// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LD130I_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LD130I_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.


//##ModelId=3D407C8F0030
class  TCriticalSection
{
  public:
	//##ModelId=3D407C900385
	TCriticalSection()
	{
		#if UNDER_CE
			::InitializeCriticalSection(&m_cs);
		#elif _WIN32_WINNT >= 0x0403
		::InitializeCriticalSectionAndSpinCount(&m_cs, 5000);

		#else
			::InitializeCriticalSection(&m_cs);
		#endif
	}

	//##ModelId=3D407C90038F
	~TCriticalSection()
	{
		::DeleteCriticalSection(&m_cs);
	}

	//##ModelId=3D407C900386
	TCriticalSection(const TCriticalSection &right)
	{
		#if UNDER_CE
			::InitializeCriticalSection(&m_cs);
		#elif _WIN32_WINNT >= 0x0403
			::InitializeCriticalSectionAndSpinCount(&m_cs, 5000);
		#else
			::InitializeCriticalSection(&m_cs);
		#endif
	}


	//##ModelId=3D407C900399
	bool lock ()
	{
		::EnterCriticalSection(&m_cs);
		return true;
	}

	//##ModelId=3E023C1001EC
	bool tryLock ()
	{
		const bool bRet = ::TryEnterCriticalSection(&m_cs) != 0;
		return bRet;
	}


	//##ModelId=3D407C90039A
	bool unlock ()
	{
		::LeaveCriticalSection(&m_cs);
		return true;
	}

	//##ModelId=3D407C9003A3
	TCriticalSection & operator=(const TCriticalSection &right)
	{
		#if UNDER_CE
			::InitializeCriticalSection(&m_cs);
		#elif _WIN32_WINNT >= 0x0403
			::InitializeCriticalSectionAndSpinCount(&m_cs, 5000);
		#else
			::InitializeCriticalSection(&m_cs);
		#endif
		return *this;
	}


	#ifdef UNDER_CE
		HANDLE getOwningThread() const { return m_cs.OwnerThread; } //@stan was OwningThread
	#else
		HANDLE getOwningThread() const { return m_cs.OwningThread; }
	#endif

	bool initCountingSema()
	{
		#ifdef A3D_TRACK_CS_USAGE
		if (m_hSemaphore == 0) {
			wchar_t buf[128] =  {0};
			_snwprintf(buf, sizeof(buf) / sizeof (buf[0]) - 1, L"ACERIS_CS_COUNTER_%d", ::GetCurrentProcessId());
			m_hSemaphore = ::CreateSemaphoreW(0, 0, 1000000, buf);
			return true;
		}
		#endif
		return false;
	}

	const CRITICAL_SECTION& getCriticalSection() const { return m_cs; }

  private:
	//##ModelId=3D407C90037B
	CRITICAL_SECTION 	m_cs;

};

//##ModelId=3D407C8F0026
class  TCritical
{
  public:
	//##ModelId=3D407C900353
	TCritical (::TCriticalSection& cs_)
		: cs(cs_)
	{
		cs.lock();
	}

	//##ModelId=3D407C90035D
	~TCritical()
	{
		cs.unlock();
	}

  protected:
  private:
	//##ModelId=3D407C90035E
	TCritical(const TCritical &right);

	//##ModelId=3D407C900368
	TCritical & operator=(const TCritical &right);

  private:
	//##ModelId=3D407C900340
	::TCriticalSection& cs;

};
