#if ! defined( _MBU_LIB_)
#define _MBU_LIB_


#ifndef _MAKING_MBU_LIB_
	#if defined( _DEBUG )
		//#pragma comment( lib, "DebugModbuLib.lib" )
	#else
		//#pragma comment( lib, "ReleaseModbuLib.lib" )
	#endif // _DEBUG

	//other libs 
	#pragma comment( lib, "winmm.lib") //timeGetTime

#endif //_MAKING_MBU_LIB_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400 
#endif 


//MCF Declarations 
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#include <afxsock.h>		// MFC socket extensions
#include <afxtempl.h>
#include <afxmt.h> 

//windows declarations 
#include <mmsystem.h>


//declaration files for ATL base classes
#include <atlbase.h>  
//external linkage for CComModule
//extern CComModule _Module;
//ActiveX Template Library include
//#include <atlcom.h>



//utility and modbus classes

//#include "MMTimer.h"
//#include "Modbus.h"
//#include "AutoModbus.h"

#endif //_MBU_LIB_