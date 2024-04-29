// PerformanceCounter.cpp: implementation of the CPerformanceCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "mbu.h"
#include "PerformanceCounter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/////////////////////////////// PUBLIC ///////////////////////////////////////

BOOL CPerformanceCounter::CounterEnabled()
{

	return m_bCounterEnabled;
}



//============================= LIFECYCLE ====================================

CPerformanceCounter::CPerformanceCounter()
{
 m_LargeIntFrequency.LowPart=0;
 m_LargeIntFrequency.HighPart=0;
 m_bCounterEnabled=::QueryPerformanceFrequency(&m_LargeIntFrequency);
 
}// CPerformanceCounter


CPerformanceCounter::~CPerformanceCounter()
{
}// ~CPerformanceCounter


//============================= OPERATORS ====================================

CPerformanceCounter& CPerformanceCounter::operator=(CPerformanceCounter&)
{
   return *this;

}

//============================= OPERATIONS ===================================
//============================= ACESS      ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////



__int64 CPerformanceCounter::LargeInteger2Int64(LARGE_INTEGER* pLargeInteger)
{
       
	return (((__int64)(pLargeInteger->HighPart))<<32)|pLargeInteger->LowPart;
}

void CPerformanceCounter::Start()
{
	::QueryPerformanceCounter(&m_LargeIntStart);
}

void CPerformanceCounter::End()
{
	::QueryPerformanceCounter(&m_LargeIntEnd);
}

__int64 CPerformanceCounter::DiffCounts()
{
	return (LargeInteger2Int64(&m_LargeIntEnd)-LargeInteger2Int64(&m_LargeIntStart));
}

__int64 CPerformanceCounter::DiffCountsToNow() {
	
	End();
    return DiffCounts();
}

__int64 CPerformanceCounter::MiliSecToCount(float fMiliSecs)
{

 return ((__int64)(fMiliSecs*(float)LargeInteger2Int64(&m_LargeIntFrequency)))/1000;
}

float CPerformanceCounter::CountToMiliSec(__int64 iCount)
{
    return (float)(iCount*1000)/(float)(LargeInteger2Int64(&m_LargeIntFrequency));
}


