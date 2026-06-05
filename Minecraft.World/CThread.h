#pragma once
#include <queue>

typedef int (CThreadStartFunc)(void* lpThreadParameter);

class Level;

#if defined(_LINUX) || defined(_LINUX)

#define CPU_CORE_MAIN_THREAD 0

#define CPU_CORE_SERVER 1

#define CPU_CORE_CHUNK_UPDATE 2
#define CPU_CORE_REMOVE_PLAYER 2

#define CPU_CORE_CHUNK_REBUILD_A 3
#define CPU_CORE_SAVE_THREAD_A 3
#define CPU_CORE_UI_SCENE 3
#define CPU_CORE_POST_PROCESSING 3
#define CPU_CORE_DQR_REALTIMESESSION 3

#define CPU_CORE_CHUNK_REBUILD_B 4
#define CPU_CORE_SAVE_THREAD_B 4
#define CPU_CORE_TILE_UPDATE 4
#define CPU_CORE_CONNECTIONS 4

#define CPU_CORE_CHUNK_REBUILD_C 5
#define CPU_CORE_SAVE_THREAD_C 5

#else

#define CPU_CORE_MAIN_THREAD 0

#define CPU_CORE_CHUNK_REBUILD_A 1
#define CPU_CORE_SAVE_THREAD_A 1
#define CPU_CORE_TILE_UPDATE 1
#define CPU_CORE_CONNECTIONS 1

#define CPU_CORE_CHUNK_UPDATE 2
#define CPU_CORE_REMOVE_PLAYER 2

#define CPU_CORE_CHUNK_REBUILD_B 3
#define CPU_CORE_SAVE_THREAD_B 3
#define CPU_CORE_UI_SCENE 3
#define CPU_CORE_POST_PROCESSING 3

#define CPU_CORE_SERVER 4

#define CPU_CORE_CHUNK_REBUILD_C 5
#define CPU_CORE_SAVE_THREAD_C 5
#define CPU_CORE_LEADERBOARDS 5 // Sony only

#endif

class CThread
{
public:

	class Event
	{
	public:
		enum EMode
		{
			e_modeAutoClear,
			e_modeManualClear
		};
		Event(EMode mode = e_modeAutoClear);	
		~Event();
		void Set();
		void Clear();
		DWORD WaitForSignal(int timeoutMs);

	private:
		EMode	m_mode;
	#ifdef _LINUX
		sys_event_flag_t m_event; 
	#elif defined _LINUX
		SceKernelEventFlag m_event;
	#elif defined _LINUX
		SceUID m_event;
	#else
		HANDLE m_event;
	#endif // _LINUX
	};

	class EventArray
	{
	public:
		enum EMode
		{
			e_modeAutoClear,
			e_modeManualClear
		};

		EventArray(int size, EMode mode = e_modeAutoClear);

		void Set(int index);
		void Clear(int index);
		void SetAll();
		void ClearAll();
		DWORD WaitForAll(int timeoutMs);
		DWORD WaitForAny(int timeoutMs);
		DWORD WaitForSingle(int index, int timeoutMs);
#ifdef _LINUX
		void Cancel();
#endif

	private:
		int		m_size;
		EMode	m_mode;
#ifdef _LINUX
		sys_event_flag_t	m_events; 
#elif defined _LINUX
		SceKernelEventFlag	m_events;
#elif defined _LINUX
		SceUID m_events;
#else
		HANDLE*				m_events;
#endif // _LINUX
	};



	class EventQueue
	{
		typedef void (UpdateFunc)(void* lpParameter);
		typedef void (ThreadInitFunc)();

		CThread*				m_thread;
		std::queue<void*>		m_queue; 
		CThread::EventArray*	m_startEvent;
		CThread::Event*		m_finishedEvent;
		CRITICAL_SECTION		m_critSect;
		UpdateFunc*				m_updateFunc;
		ThreadInitFunc*			m_threadInitFunc;
		char					m_threadName[64];
		int						m_processor;
		int						m_priority;
		void init();
		static int threadFunc(void* lpParam);
		void threadPoll();

	public:
		EventQueue(UpdateFunc* updateFunc, ThreadInitFunc threadInitFunc, const char* szThreadName);
		void setProcessor(int proc) { m_processor = proc; if(m_thread) m_thread->SetProcessor(proc); }
		void setPriority(int priority) { m_priority = priority; if(m_thread) m_thread->SetPriority(priority); }
		void sendEvent(Level* pLevel);
		void waitForFinish();
	};



	CThread(CThreadStartFunc* startFunc, void* param, const char* threadName, int stackSize = 0);
	CThread( const char* mainThreadName ); // only used for the main thread
	~CThread();

	void Run();
	bool isRunning() { return m_isRunning; }
	bool hasStarted() { return m_hasStarted; }
	void SetProcessor(int proc);
	void SetPriority(int priority);
	DWORD WaitForCompletion(int timeoutMs);
	int GetExitCode();
	char* getName() { return m_threadName; }
	static void Sleep(int millisecs);
	static CThread* getCurrentThread();
	static bool isMainThread();
	static char* getCurrentThreadName() { return getCurrentThread()->getName(); }

#ifdef _LINUX
	static void PopAffinity();
	static __thread SceKernelCpumask m_oldAffinityMask;
#endif // _LINUX

#ifdef _LINUX
	static void StaticInit();
#endif

private:
	void*				m_threadParam;
	CThreadStartFunc*	m_startFunc;
	int					m_stackSize;
	char				m_threadName[64];
	bool				m_isRunning;
	bool				m_hasStarted;
	int					m_exitCode;
	__int64				m_lastSleepTime;
	static std::vector<CThread*> ms_threadList;
	static CRITICAL_SECTION ms_threadListCS;

#ifdef _LINUX
	static CThread *m_mainThread;
#else
	static CThread	m_mainThread;
#endif

#ifdef _LINUX
	sys_ppu_thread_t m_threadID;
	Event			*m_completionFlag;
	int				m_priority;
	static void entryPoint(uint64_t);
#elif defined _LINUX
	ScePthreadAttr	m_threadAttr;
	ScePthread		m_threadID;
	Event			*m_completionFlag;
	int				m_priority;
	static void *entryPoint(void *);
#elif defined _LINUX
	SceUID		m_threadID;
	Event			*m_completionFlag;
	int				m_priority;
	static SceInt32	entryPoint(SceSize argSize, void *pArgBlock);
#else
	DWORD m_threadID;
	HANDLE m_threadHandle;
	Event			*m_completionFlag;
	static DWORD WINAPI	entryPoint(LPVOID lpParam);
#endif
};
void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName );

