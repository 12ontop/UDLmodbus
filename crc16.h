#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#ifndef _CRC16_h_
#define _CRC16_h_

#define HI4BITS(w)   ((BYTE) (((BYTE) (w) >> 4) & 0xF)) 
#define LO4BITS(w)   ((BYTE) ((w) & 0xF))  


class CRC16 {

protected:
  

private:
     static const UCHAR auchCRCHi[];
	 static const UCHAR auchCRCLo[];
public:
	 CRC16();
	 ~CRC16();
	 WORD calculate_crc (UCHAR *Str, WORD NumBytes);
	 WORD CalcCrc (UCHAR *Str, WORD NumBytes);
	 WORD CalcCrc (CByteArray& abyMessage);
	 WORD CalcCrcFast(UCHAR *puchMsg, WORD usDataLen);
	 WORD CalcCrcFast(CByteArray& abyMessage);
	 WORD CalcCrcFast(CByteArray& abyMessage , WORD wnLength);

	 static BYTE LRC(BYTE *auchMsg, WORD usDataLen);

};

#endif  // _CRC16_h_