
// Discrete addresses

#define ETA_TYPE		122
#define ETA_VERSION		107
#define ETA_MID			121
#define ETA_SERIAL		10428
#define ETA_VALVE_POSN	53
#define ETA_POWER		4
#define ETA_BALANCE		5268
#define ETA_BALANCE_HR	4948		// Hi res balance
#define ETA_FURN_TEMP	1
#define ETA_SAMPLE_TEMP	4468
#define ETA_SET_POINT	24
#define ETA_SET_LOGIC1	5402

// Program addresses general

#define ETA_PNUMBER1		21136
#define ETA_PNUMBER			12288
#define ETA_PRATEUNIT		12290		// 1
#define ETA_PENDACTION		12291		// 1
#define ETA_PSEGMENTS		12293
#define ETA_PHOLDBACK		12294		// 0

// program addresses segment
#define ETA_PSEGNO			12312
#define ETA_PSEGTXT			12313		// 0
#define ETA_PSEGTYPE		12314		// 0, profile or 1, end seg
#define ETA_PSEGPSPTYPE		12316		// 1 step, 2 dwell, 3 ramp
#define ETA_PSEGTARGET		12317		//  float 1dp
#define ETA_PSEGRATE		12318		//  float 1 dp
#define ETA_PDWELLTIME		12319		// tenths of hours
#define ETA_PSEGLOOPTO		12341		// seg to jump back to
#define ETA_PCYCLES			12342		// loop back n times

#define ETA_PSEGLENGTH		32			// address units long

// Program addresses running

#define ETA_PRUNNO			5820		// number to run, 1
#define ETA_PTIMELEFT		5827
#define ETA_PFASTRUN		5789		// 0, no
#define ETA_PSTATUS			5844		// 1, reset, 2, run, 4, hold
#define ETA_PTIMEGONE		5815		
#define ETA_PTOTALSEGS		5826
#define ETA_PCURSEG			5822
#define ETA_PCURSEGTYPE		5810
#define ETA_PPENDACTION		5807		// 1







