#pragma once
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <CommCtrl.h>
#pragma comment(lib,"comctl32.lib")

// 高dpi支持（此法只能在Win7 SP1及以上系统调用，否则报错找不到dll）
//#include <ShellScalingAPI.h>
//#pragma comment(lib,"Shcore.lib")

#ifndef DPI_ENUMS_DECLARED

	typedef enum PROCESS_DPI_AWARENESS {
		PROCESS_DPI_UNAWARE = 0,
		PROCESS_SYSTEM_DPI_AWARE = 1,
		PROCESS_PER_MONITOR_DPI_AWARE = 2
	} PROCESS_DPI_AWARENESS;

	typedef enum MONITOR_DPI_TYPE {
		MDT_EFFECTIVE_DPI = 0,
		MDT_ANGULAR_DPI = 1,
		MDT_RAW_DPI = 2,
		MDT_DEFAULT = MDT_EFFECTIVE_DPI
	} MONITOR_DPI_TYPE;

	#define DPI_ENUMS_DECLARED
#endif

// 启用高DPI适配。若在Win7 SP1以下系统，找不到函数时，则不做处理
inline void SupportHighDPI()
{
	HMODULE hHighDpi = LoadLibrary(TEXT("SHCore.dll"));
	if (hHighDpi)
	{
		typedef HRESULT(__stdcall* FuncSetHighDpi)(PROCESS_DPI_AWARENESS);
		FuncSetHighDpi fnSetHighDpi = (FuncSetHighDpi)GetProcAddress(hHighDpi, "SetProcessDpiAwareness");
		if (fnSetHighDpi)
		{
			fnSetHighDpi(PROCESS_PER_MONITOR_DPI_AWARE);
		}
		FreeLibrary(hHighDpi);
	}
}