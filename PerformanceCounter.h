
// NAME: 
//   CPerformanceCounter -
//
// SYNOPSIS:
// DESCRIPTION:
// EXAMPLES:
// FILES:
// BUGS:
// SEE ALSO:
// TYPE: C++-CLASS 
///X///////////////////////X////////////////////X/////////////////////////////

#ifndef _CPerformanceCounter_h_
#define _CPerformanceCounter_h_

// SYSTEM INCLUDES
//
// PROJECT INCLUDES
//

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//


class CPerformanceCounter
{
public:
	 float CountToMiliSec(__int64 iCount);
	 __int64 MiliSecToCount(float fMiliSecs);
	 __int64 DiffCountsToNow();
	 __int64 DiffCounts();
	 void End();
	 void Start();
	 __int64 LargeInteger2Int64(LARGE_INTEGER* pLargeInteger);
	BOOL CounterEnabled();
// LIFECYCLE

   CPerformanceCounter();
   ~CPerformanceCounter();

// OPERATORS

   CPerformanceCounter& operator=(CPerformanceCounter&);       // assignment operator

// OPERATIONS                       
// ACCESS
// INQUIRY

protected:
private:
	BOOL m_bCounterEnabled;
	LARGE_INTEGER m_LargeIntFrequency;
	LARGE_INTEGER m_LargeIntEnd;
	LARGE_INTEGER m_LargeIntStart;
};

// INLINE METHODS
//

// EXTERNAL REFERENCES
//

#endif  // _CPerformanceCounter_h_