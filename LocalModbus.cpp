// LocalModbus.cpp: implementation of the CLocalModbus class.
//
//////////////////////////////////////////////////////////////////////

#include "mbu.h"
#include "LocalModbus.h"
#include "eurotherm.h"

IMPLEMENT_DYNAMIC(CLocalModbus,CModbus)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocalModbus::CLocalModbus()
{
   m_hComHandle=INVALID_HANDLE_VALUE;
   m_lTimeOut=1000;
   m_ComPort=2;
   m_byParity=NOPARITY; 
   m_BaudRate=9600;
   m_dwSilentInterval=0;
   FlowControl(FC_NONE);
   StopBits(ONESTOPBIT);
   m_wByteSize=8;
   m_wTranmitionMode=MODE_RTU;
   m_abyBuffer.SetSize(RESP_BUFFER*2);
   m_Latencyms = 100;

   m_fLog = FALSE;
   

}

CLocalModbus::~CLocalModbus()
{
	CloseCommPort();

	if (m_fLog)
		LogFile.Close();
}

//properties 

 BOOL CLocalModbus::SetTimeouts(COMMTIMEOUTS* timeouts_p )
 {
	 ASSERT( m_hComHandle!=INVALID_HANDLE_VALUE );
	 
	 // Choose to live
	 
	 if ( m_hComHandle ==INVALID_HANDLE_VALUE )
	 {
		 return( FALSE );
	 }
	 
	 BOOL return_value = FALSE;
	 
	 COMMTIMEOUTS communications_timeouts;
	 
	 ::ZeroMemory( &communications_timeouts, sizeof( communications_timeouts ) );
	 
	 if ( timeouts_p != NULL )
	 {
		 // We were passed a pointer, do not trust it
		 
		 try
		 {
			 communications_timeouts.ReadIntervalTimeout         = timeouts_p->ReadIntervalTimeout;
			 communications_timeouts.ReadTotalTimeoutMultiplier  = timeouts_p->ReadTotalTimeoutMultiplier;
			 communications_timeouts.ReadTotalTimeoutConstant    = timeouts_p->ReadTotalTimeoutConstant;
			 communications_timeouts.WriteTotalTimeoutMultiplier = timeouts_p->WriteTotalTimeoutMultiplier;
			 communications_timeouts.WriteTotalTimeoutConstant   = timeouts_p->WriteTotalTimeoutConstant;
		 }
		 catch( ... )
		 {
			 communications_timeouts.ReadIntervalTimeout         = MAXDWORD;
			 communications_timeouts.ReadTotalTimeoutMultiplier  = 0;
			 communications_timeouts.ReadTotalTimeoutConstant    = 0;
			 communications_timeouts.WriteTotalTimeoutMultiplier = 0;
			 communications_timeouts.WriteTotalTimeoutConstant   = 0;
		 }
	 }
	 else
	 {
		 // Use our defaults
		 communications_timeouts.ReadIntervalTimeout         = MAXDWORD;
		 communications_timeouts.ReadTotalTimeoutMultiplier  = 0;
		 communications_timeouts.ReadTotalTimeoutConstant    = 0;
		 communications_timeouts.WriteTotalTimeoutMultiplier = 0;
		 communications_timeouts.WriteTotalTimeoutConstant   = 0;
	 }
	 
	 return_value = ::SetCommTimeouts( (HANDLE) m_hComHandle, &communications_timeouts );
	 
	 return( return_value );
 }
 
 BOOL CLocalModbus::GetTimeouts( COMMTIMEOUTS& timeouts )
 {
     ASSERT( m_hComHandle != INVALID_HANDLE_VALUE );
	 
	 // Choose to live
	 
	 if ( m_hComHandle ==INVALID_HANDLE_VALUE )
	 {
		 ::ZeroMemory( &timeouts, sizeof( COMMTIMEOUTS ) );
		  return( FALSE );
	 }
	 
	 BOOL return_value = FALSE;
	 
	 return_value = ::GetCommTimeouts(m_hComHandle, &timeouts );
	 
	 return( return_value );
 }


 DWORD CLocalModbus::Timeout() const {
    return m_lTimeOut;
 }

 
 void CLocalModbus::Timeout(DWORD dwTimeout){
    m_lTimeOut=dwTimeout;
 }


 DWORD CLocalModbus::SilentInterval() const {
    return m_dwSilentInterval;
 }

 void CLocalModbus::SilentInterval(DWORD dwSilentInterval ){
    if (dwSilentInterval>0){
      m_dwSilentInterval=dwSilentInterval;
    }
 }

 BYTE CLocalModbus::ComPort()
 {
	 return m_ComPort;  
		 
 }


 void CLocalModbus::ComPort(BYTE byComPort)
 {
	 m_ComPort=byComPort;  
		 
 }

 void CLocalModbus::DevAddr(BYTE byDevAddr)
 {
	 m_DevAddr=byDevAddr;  		 
 }
 
 BYTE CLocalModbus::FlowControl() {
 
	 return  m_byFlowControl;
 
 }

 void CLocalModbus::FlowControl(BYTE byFlowControl){
 
	 m_byFlowControl= byFlowControl;
 
 }

 BYTE CLocalModbus::StopBits() {
 
	 return  m_byStopBits;
 
 }

 void CLocalModbus::StopBits(BYTE byStopBits){
 
	 m_byStopBits= byStopBits;
 
 }
 
 
 DWORD CLocalModbus::BaudRate()
 {
	 
	 return m_BaudRate;
 }
 
void CLocalModbus::BaudRate(DWORD dwBaudRate)
 {
	 
	 m_BaudRate=dwBaudRate;
 }

void CLocalModbus::Latency(int Latency)
 {
	 
	 m_Latencyms = Latency;
 }

 BYTE CLocalModbus::Parity()
 {
	 return m_byParity;
	 
 }

 void  CLocalModbus::Parity(BYTE byParity) {

	 m_byParity = byParity;
 }

//close serial port 
BOOL CLocalModbus::CloseCommPort()
{
	if (m_hComHandle!=INVALID_HANDLE_VALUE)
	{
		if (::CloseHandle(m_hComHandle)) 
		{
			m_hComHandle=INVALID_HANDLE_VALUE;
			Log("Comm port closed");
			return TRUE;
		}
		else 
		{
			return FALSE;
		}
	}
	else 
	{
		return (TRUE);
	}
}

DWORD CLocalModbus::UpdateSerialConfig(BYTE byComPort,  //com port
									DWORD dwBaudRate, // Baudrate 9600,4800 actual values  
									BYTE byParity) //Parity EVENPARITY or ODDPARITY or NOPARITY DCB definitions
{     				  		 
	 // set timeouts
	 CString sPort;
	 COMMTIMEOUTS cto;
	 DCB dcb;
	 DWORD dwSilentInterval;
	 BYTE bSet;
	 DWORD error = 0;
	 CString s;
	 
	 Lock();

	 s.Format("Update port %d at %d", byComPort, dwBaudRate);
	 Log(s);
	 
	 if (!CloseCommPort()){
		 Log("Can't close comm port");
		 error = 100000;
		 goto OpenCommError;
	 }
	 
	 if (0==byComPort)
	 {
		byComPort=m_ComPort;
	 }

	 if ( (byComPort>=1) && (byComPort<=MAX_COM_PORTS) )
	 {
		 m_ComPort=byComPort;
	 }
     else 
	 {
		 error = 110000;
		 goto OpenCommError;
	 }

	 sPort.Format(_T("COM%d"),m_ComPort);
	 
     m_hComHandle = ::CreateFile((LPCTSTR)sPort,
		 GENERIC_READ|GENERIC_WRITE,
		 0,
		 NULL,
		 OPEN_EXISTING,
		 0,
		 NULL);
	 
	 if(m_hComHandle == INVALID_HANDLE_VALUE) {

		 Log("CreateFile Failed");

		 error = GetLastError();
		 error += 120000;
		 
		 /*sPort.Format(_T("\\\\.\\COM%d"),m_ComPort); //Try NT Format 
		 
		 m_hComHandle = ::CreateFile((LPCTSTR)sPort,
			 GENERIC_READ|GENERIC_WRITE,
			 0,
			 NULL,
			 OPEN_EXISTING,
			 0,
			 NULL);
		 
		 if(m_hComHandle == INVALID_HANDLE_VALUE) {
			 Log("CreateFile Failed \n");*/

			 goto OpenCommError;
		  
	 }
	 
     if(!::SetupComm(
					m_hComHandle,	// handle of communications device
					INPUT_BUFFER_SIZE,	// size of input buffer
					OUTPUT_BUFFER_SIZE	// size of output buffer
					))
	 {
		 Log("SetupComm failed");
		 error = GetLastError();
		 error += 130000;
		 goto OpenCommError;
     }
	 
	 //missinig verify baudrate and pairty *****
	 
	 // set DCB
	 if (0==dwBaudRate)
	 {
		//m_byParity=dcb.Parity; 
		dwBaudRate =m_BaudRate;
	 }

	 if (INVALID_PARITY==byParity) 
	 {
		 byParity=m_byParity;
	 }

	 ::memset(&dcb,0,sizeof(dcb));
	 dcb.DCBlength	 = sizeof(dcb);
	 dcb.BaudRate	 = dwBaudRate;
	 dcb.fBinary	 = TRUE;
	 dcb.StopBits	 = StopBits();
	 dcb.Parity		 = byParity;
	 dcb.ByteSize	 = BYTE(m_wByteSize);
	 
	 bSet			 =(BYTE) ((FlowControl()& FC_DTRDSR)!=0);
	 dcb.fOutxDsrFlow = bSet ;
	 
	 if (bSet)
		 dcb.fDtrControl = DTR_CONTROL_HANDSHAKE ;
	 else
		 dcb.fDtrControl = DTR_CONTROL_ENABLE ;
	 
	 bSet = (BYTE) ((FlowControl()& FC_RTSCTS) != 0) ;
	 dcb.fOutxCtsFlow = bSet ;
	 
	 if (bSet)
		 dcb.fRtsControl = RTS_CONTROL_HANDSHAKE ;
	 else
		 dcb.fRtsControl = RTS_CONTROL_ENABLE ;
	 
	 // setup software flow control
	 
	 bSet = (BYTE) ((FlowControl()&FC_XONXOFF) != 0) ;
	 
	 dcb.fInX = dcb.fOutX = bSet ;
	 dcb.XonChar = ASCII_XON ;
	 dcb.XoffChar = ASCII_XOFF ;
	 dcb.XonLim = 100 ;
	 dcb.XoffLim = 100 ;
	 	 
	 dwSilentInterval= (DWORD)((dcb.ByteSize+4)*(4)*1000L)/(dcb.BaudRate);
     m_byParity=dcb.Parity; 
	 m_BaudRate=dcb.BaudRate;
     
	 if (dwSilentInterval>m_dwSilentInterval) {
		 SilentInterval(dwSilentInterval);
	 }
	 
	 cto.ReadIntervalTimeout=SilentInterval();//Max Char Interval 
	 cto.ReadTotalTimeoutMultiplier=(DWORD)((dcb.ByteSize+4)*(1)*1000L)/(dcb.BaudRate);
	 
	 if (cto.ReadTotalTimeoutMultiplier<=0){ 
		 cto.ReadTotalTimeoutMultiplier=1;
	 }

	 cto.ReadTotalTimeoutConstant=m_lTimeOut;
	 cto.WriteTotalTimeoutMultiplier=SilentInterval();
	 cto.WriteTotalTimeoutConstant=m_lTimeOut;
	 

	 if (MODE_RTU==m_wTranmitionMode)
	 {
		 if(!SetTimeouts(&cto))
		 {
			 Log("SetTimeouts failed ");
			 error = 140000;
			 goto OpenCommError;
		 }
	 }
	 else 
	 {	
		 if(!SetTimeouts(NULL))
		 {
			 Log("SetTimeouts failed ");
			 error = 150000;
			 goto OpenCommError;
		 }
	 }
	 
	 
	 if(!::SetCommState(m_hComHandle,&dcb))
	 {
		 Log("SetCommState failed \n");
		 error = GetLastError();
		 error += 160000;
		 goto OpenCommError;
	 }
	 
OpenCommError:
	 Unlock();
	 s.Format("baud=%d bin=%d sb=%d pa=%d bytesz=%d lat=%d",
			 dcb.BaudRate,
			 dcb.fBinary,
			 dcb.StopBits,
			 dcb.Parity,
			 dcb.ByteSize,
			 m_Latencyms);
	 Log(s);
	 Log("Port configured");
	 return (error);	 
}

BOOL CLocalModbus::IsActive() 
{
	 if (m_hComHandle!=INVALID_HANDLE_VALUE) {
		 return TRUE;
	 }
	 else {
		 return FALSE;
	 }
 }
 

  void CLocalModbus::Serialize( CArchive& archive )
  {
	  // call base class function first
	  // base class is CObject in this case
	  CModbus::Serialize( archive );
	  
	  BOOL bIsActive;	
	  // now do the stuff for our specific class
	  if( archive.IsStoring() ){
		 archive << m_dwSilentInterval << m_lTimeOut;
		 archive <<m_ComPort<<m_byParity<<m_BaudRate<<IsActive();
         archive <<m_byStopBits<<m_byFlowControl<<m_wTranmitionMode<<m_wByteSize;
		 
      } 
	  else {

		 archive >> m_dwSilentInterval >> m_lTimeOut >> m_ComPort >> m_byParity >> m_BaudRate >> bIsActive;
		 archive >>m_byStopBits >> m_byFlowControl>>m_wTranmitionMode>>m_wByteSize;

		 if (bIsActive) {
			 UpdateSerialConfig(m_ComPort,  //com port
								  m_BaudRate, // Baudrate 9600,4800 actual values  
								  m_byParity);
		 
		 }
		
      }

  }

WORD CLocalModbus::TxRxMessage(CByteArray& abyQuery, //modbus query without CRC
								 WORD wLengthQuery, //QueryLength without CRC
								 CByteArray& abyResponse, //modbus Response without CRC
								 WORD wLengthResponse , //wLengthResponse Response without CRC
								 WORD* pwNumOfBytesRead)
{ 
	 
	 
	 WORD wcrc ;
	 DWORD  dwNumberOfBytesWritten;
	 
	 //CByteArray m_abyBuffer;
	 DWORD dwNumberOfBytesRead;
	 WORD wRespSize;	
	 WORD wError;
	 CString s;


	 
	 //Fill response 
	 if (abyResponse.GetSize()<wLengthResponse) {
		 abyResponse.SetSize(wLengthResponse);
	 }
	 
	 //lock com device  
	 if (!Lock()) {
		 Log("Lock Failed");
		 return(ERR_LOCK_TIME_OUT);
	 }
	 
	 if (m_hComHandle==INVALID_HANDLE_VALUE){
		 Log("Port Not opened"); 
		 
		 wError=(ERR_NOT_INT);
		 goto TxRxError;
	 }



	 if (m_fLog)
	 {
		 if (wLengthQuery > 5)
		 {
			 s.Format("TxRx:[%d]Dev=%d Func=%d addr=%X data1=%X",
				 wLengthQuery,
				 abyQuery[0],
				 abyQuery[1],
				 MAKEWORD(abyQuery[3], abyQuery[2]),
				 MAKEWORD(abyQuery[5], abyQuery[4]) );
		 }
		 else
			s.Format("TxRx:[%d]Dev=%d Func=%d",
				 wLengthQuery,
				 abyQuery[0],
				 abyQuery[1]);
				 

		 Log(s);
	 }
	 

	 if (MODE_ASCII==m_wTranmitionMode) {
		 wError=TxRxMessageAscii(abyQuery,wLengthQuery,abyResponse,wLengthResponse,pwNumOfBytesRead);
		 Unlock();
		 return wError;
	 }

	 //Add CRC16 to query 
	 wcrc=m_crc.CalcCrcFast(abyQuery,wLengthQuery);
	
	 if (abyQuery.GetSize() < (wLengthQuery + 2) ) 
	 {
		 abyQuery.SetSize(wLengthQuery + 2);
	 }
	 
	 abyQuery[wLengthQuery]=LOBYTE(wcrc); 
	 abyQuery[wLengthQuery+1]=HIBYTE(wcrc);

	 if (m_fLog)
	 {
		 s.Format("TxRx:CRC=%X (%X-%X) dec=%d", wcrc, LOBYTE(wcrc), HIBYTE(wcrc),
			 wcrc );
		 Log(s);
	 }
	 
	 
	 if(!ClearBuffers()){
		 Log("ClearBuffers Error");
	 };
	 
	 //write query 
	 if (!WriteQuery(abyQuery, wLengthQuery + 2, dwNumberOfBytesWritten))
	 {
		 Log("Write Query Error");
		 wError = (ERR_WR_PORT);
		 goto TxRxError;
	 }
	 else
	 {
		 if (m_fLog)
		 {
			 CString a, b;
			 for (int ii=0; ii < (int)dwNumberOfBytesWritten; ii++)
			 {
				 a.Format("%X", abyQuery[ii]);
				 b += a;
			 }
			 s.Format("WRITE:[%d]>%s<", dwNumberOfBytesWritten, b);
			 Log(s);
		 }
	 }

	 

	 s.Format("SLEEP %d ms", m_Latencyms);
	 Log(s);
	 Sleep(m_Latencyms);
	 	 
	 //read response 
	 m_abyBuffer.SetSize(RESP_BUFFER); //fill temp buffer 

	 ::ZeroMemory(m_abyBuffer.GetData(), m_abyBuffer.GetSize());
	 
	 //if (!ReadResponse(m_abyBuffer,dwNumberOfBytesRead,wLengthResponse+2)) //using COMMTIMEOUTS
	 if (!ReadResponseRTU(m_abyBuffer,dwNumberOfBytesRead)) 
	 {
		 Log("ReadResponse Failed"); 
		 wError=(ERR_RD_PORT);
		 goto TxRxError;

	 }
	  
	 if (dwNumberOfBytesRead == 0)
	 {
		 m_PerfCounter.End();
		 wError=(ERR_TIMEOUT);
		 goto TxRxError;
	 }
	 else
	 {
		 if (m_fLog)
		 {
			 CString a, b;
			 for (int ii=0; ii < (int)dwNumberOfBytesRead; ii++)
			 {
				 a.Format("%X", m_abyBuffer[ii]);
				 b += a;
			 }
			 s.Format("READ:[%d]>%s<", dwNumberOfBytesRead, b);
			 Log(s);
		 }
	 }
	 
	 
	 //Verify if  modbus exception ocurred
	 if (m_abyBuffer[1] > 0x80) 
	 { 
		 //slave message error 
		 if (!VerifyRespCRC(m_abyBuffer, 3))	//length modbus error message=3
		 {  
			 wError=(ERR_CRC);
			 goto TxRxError;
		 }   
		 else 
		 {
			 wError= (ERR_EXCPTION_CODE+m_abyBuffer[2]);
			 goto TxRxError;
			 
		 }
	 }
	 
	 //ResponseLength not defined 
	 
	 if (0 == wLengthResponse) 
	 {
		 wRespSize=WORD(dwNumberOfBytesRead);
	 }
	 else 
	 {
		 wRespSize=wLengthResponse;
	 }
	 
	 //verify if response has a valid CRC 
	 
	 if (!VerifyRespCRC(m_abyBuffer,wRespSize))	//length modbus error message=3 
	 { 
		 wError = (ERR_CRC);
		 goto TxRxError;
	 }   
	 	 
	 if (0 == wLengthResponse) 
	 {
		 if  (wRespSize > 2) 
		 {
			 wLengthResponse = (wRespSize - 2);
		 }
	 }
	 
	 //Discard possible garbage read from buffer 
	 
	 m_abyBuffer.SetSize(wLengthResponse);
	 //abyQuery.SetSize(wLengthQuery);
	 
	 abyResponse.Copy(m_abyBuffer);
	 
	 if (pwNumOfBytesRead != NULL) 
	 {
		 *pwNumOfBytesRead = wLengthResponse;
	 }
	 
	 Unlock();

	 return(ERR_OK);

TxRxError:
	 
	 if ( Delay() > 0) 
	 {
		::Sleep( Delay() );
	 }
	 
	 Unlock();
	 return(wError);
}

 CString CLocalModbus::ErrorMessage(WORD wErrorCode){
	return CModbus::ErrorMessage(wErrorCode);
 }

 WORD CLocalModbus::TxRxMessageAscii(CByteArray& abyQuery, //modbus query without CRC
				  WORD wLengthQuery, //QueryLength without CRC
				  CByteArray& abyResponse, //modbus Response without CRC
				  WORD wLengthResponse, //wLengthResponse Response without CRC
				  WORD* pwNumOfBytesRead){

	 int iBufferSize;
	 int i;
	 int iDataSize;
	 BYTE byLRC;
	 DWORD dwNumberOfBytesWritten;
	 DWORD dwNumberOfBytesRead;
	 WORD wError;

	 m_abyBuffer.SetSize(wLengthQuery*2+5);
	 
	 iBufferSize=m_abyBuffer.GetSize();
	 ::ZeroMemory(m_abyBuffer.GetData(),m_abyBuffer.GetSize());
	 
	 RTU2ASCII(abyQuery.GetData(),wLengthQuery,&m_abyBuffer[1]);


	 //byLRC=CRC16::LRC(&m_abyBuffer[1],wLengthQuery*2);
	 byLRC=CRC16::LRC(abyQuery.GetData(),wLengthQuery);
	 m_abyBuffer[0]=':';
	 m_abyBuffer[wLengthQuery*2+1]=Num2Ascii(HI4BITS(byLRC));
	 m_abyBuffer[wLengthQuery*2+2]=Num2Ascii(LO4BITS(byLRC));
	 m_abyBuffer[wLengthQuery*2+3]=C_R;
	 m_abyBuffer[wLengthQuery*2+4]=L_F;
	 
	 if(!ClearBuffers()){
		 Log("ClearBuffers Error");
	 };
	 
	 //write query 
	 if (!WriteQuery(m_abyBuffer,wLengthQuery*2+5,dwNumberOfBytesWritten)){
		 Log("Write Query Error");
		 wError=(ERR_WR_PORT);
		 goto TxRxErrorAscii;
	 } 

	 m_abyBuffer.SetSize(RESP_BUFFER*2);
	 ::ZeroMemory(m_abyBuffer.GetData(),m_abyBuffer.GetSize());

	 
	 if (!ReadResponseASCII(m_abyBuffer,dwNumberOfBytesRead)) //not using COMMTIMEOUTS
	 {
		 Log("ReadResponse Failed"); 
		 wError=(ERR_RD_PORT);
		 goto TxRxErrorAscii;

	 }
	 
	 
	 if (dwNumberOfBytesRead == 0){
		 m_PerfCounter.End();
		 wError=(ERR_TIMEOUT);
		 goto TxRxErrorAscii;
	 }


	 if (!VerifyRespLRC(m_abyBuffer,dwNumberOfBytesRead)){

		 wError=(ERR_CRC);
		 goto TxRxErrorAscii;

	 }
	 
	 if (HiLo4BitsToByte(Ascii2Num(m_abyBuffer[3]),Ascii2Num(m_abyBuffer[4]))>0x80) { 
		 
		 wError= (ERR_EXCPTION_CODE+HiLo4BitsToByte(Ascii2Num(m_abyBuffer[5]),Ascii2Num(m_abyBuffer[6]))
				 );
		 goto TxRxErrorAscii;
		 
	 }

	 iDataSize = int((dwNumberOfBytesRead-5)/2);

	 iDataSize= min(iDataSize,m_abyBuffer.GetSize());
	 
	 for (i=0;i<iDataSize;i++){
		abyResponse[i]=HiLo4BitsToByte(Ascii2Num(m_abyBuffer[1+i*2]),
									   Ascii2Num(m_abyBuffer[1+i*2+1]));
	 }
		 
	 if (pwNumOfBytesRead!=NULL) {
		 *pwNumOfBytesRead=WORD((dwNumberOfBytesRead-5)/2);
	 }
	 
	 return(ERR_OK);
	 
TxRxErrorAscii:
	 
	 if (Delay()>0) {
		::Sleep(Delay());
	 }

	 return(wError);
 }


 
 //Transmit Raw Data to serial port 
 WORD CLocalModbus::TxRxRawData(const BYTE* pDataWrite,
	 WORD wLengthData,
	 BYTE* pRespBuffer,
	 WORD  wMaxBufferSize,
	 WORD& wNumOfBytesRead,
	 DWORD dwTimeOut
	 ){ 
	 
	 COMMTIMEOUTS cto;
	 WORD wError;
	 DWORD  dwTime_Out;
	 
	 CString s;

	 
	 
	 Lock();
	 
	 wNumOfBytesRead=0;
	 
	 if (m_hComHandle==INVALID_HANDLE_VALUE){
		 Log("Port Not opened\n"); 
		 
		 wError=(ERR_NOT_INT);
		 goto TxRxErrorRawData;
	 }
	 
	 if (dwTimeOut!=0) {
		 
		 if (!GetTimeouts(cto)){
			 wError=(ERR_NOT_INT);
			 goto TxRxErrorRawData;
			 
		 }
		 dwTime_Out= dwTimeOut;
		 SetTimeouts(NULL);
	 }
	 else {
		 
		 dwTime_Out=(DWORD)m_lTimeOut;
	 }
	 
	 
	 if(!ClearBuffers()){
		 Log("ClearBuffers Error");
	 }
	 
	 //Write Data 
	 DWORD  dwNumberOfBytesWritten;
	 DWORD  dwErrors;
	 DWORD  dwStart;
	 COMSTAT cstStatus;
	 
	 if( !::WriteFile(m_hComHandle,pDataWrite,wLengthData,&dwNumberOfBytesWritten,NULL) ) {
		 if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus))
		 {
			 //Verify Returned error 
			 s.Format("Writefile Error %d\n",dwErrors);
			 Log(s);
		 }
		 else{
			 Log("ClearCommError Failed writing\n");
		 }
		 wError=(ERR_WR_PORT);
		 goto TxRxErrorRawData;
	 }
	 
	 
	 
	 ::ClearCommError(m_hComHandle,&dwErrors,&cstStatus);
	 
	 dwStart = ::timeGetTime(); 
	 
	 while (((dwStart+dwTime_Out) >::timeGetTime())&&(cstStatus.cbOutQue>0)){
		 ::ClearCommError(m_hComHandle,&dwErrors,&cstStatus);
	 }
	 
	 
	 //Read Data
     DWORD dwNumberOfBytesRead;
	 DWORD dwRead;
	 
	 dwRead=0;
	 dwStart = ::timeGetTime(); 
	 
	 do {
		 if( !::ReadFile(m_hComHandle,(pRespBuffer+dwRead),1,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
			 if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus))
			 {
				 s.Format("ReadFile Errors 0x=%xd",dwErrors);
				 Log(s);
			 }
			 else{
				 Log(" ClearCommError Failed reading");
			 }
			 wError=(ERR_RD_PORT);
			 goto TxRxErrorRawData;
		 }
		 dwRead+=dwNumberOfBytesRead;
	 }
	 while (((dwStart+dwTime_Out) >::timeGetTime())&&(dwRead<wMaxBufferSize));
	 
	 wNumOfBytesRead=(WORD)dwRead;
	 
	 //set timeout back to default values
	 if (dwTimeOut!=0) {
		 
		 if (!SetTimeouts(&cto)){
			 wError=(ERR_NOT_INT);
			 goto TxRxErrorRawData;
			 
		 }
		 
	 }
	 
	 
	 Unlock();
	 return(ERR_OK);
	 
TxRxErrorRawData:
	 
	 if (Delay()>0) {
		::Sleep(Delay());
	 }

	 Unlock();
	 return(wError);
 }

 // private functions 
 
 //verify modbus response
 //return true if ok 
 //abyResp Modbus response 
 //dwLength Length of response without CRC 

BOOL CLocalModbus::VerifyRespCRC(CByteArray& abyResp, WORD wLength)
{ 
	 WORD wcrc; 
	 wcrc = m_crc.CalcCrcFast(abyResp,wLength);
	 return( (abyResp[wLength] == LOBYTE(wcrc)) && (abyResp[wLength+1] == HIBYTE(wcrc)) );
}

BOOL CLocalModbus::WriteQuery(CByteArray& abyQuery, DWORD dwQuerySize, DWORD& dwNumberOfBytesWritten) {
 
  DWORD  dwTimeOut;  
  DWORD  dwErrors;
  COMSTAT cstStatus;
  CString s;

  //Write Query 

  s.Format("Attempting to write %d bytes", dwQuerySize);
  Log(s);

  if( !::WriteFile(m_hComHandle, &abyQuery[0], dwQuerySize, &dwNumberOfBytesWritten, NULL) ) 
  {
	  if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus))
	  {
		  //Verify Returned error 
		  s.Format("Writefile Error %d",dwErrors);
		  Log(s);
	  }
	  else{
		  Log("ClearCommError Failed writing");
	  }
	  return (FALSE);
  }

 

    //count time after send message to device
  m_PerfCounter.Start();

  ::ClearCommError(m_hComHandle,&dwErrors,&cstStatus);
  
  dwTimeOut = ::timeGetTime() + m_lTimeOut;
  
  while ( (dwTimeOut < ::timeGetTime()) && (cstStatus.cbOutQue > 0) )
  {
	  ::ClearCommError(m_hComHandle, &dwErrors, &cstStatus);
  }

  s.Format("%d bytes written", dwNumberOfBytesWritten);
  Log(s);

  return (cstStatus.cbOutQue == 0);


 }

 //read device response 
 //return true if Read OK 
 BOOL CLocalModbus::ReadResponseASCII(CByteArray& abyResponse,//bytes read 
							DWORD& dwRead) {    //num of bytes read 

   
   DWORD dwNumberOfBytesRead=0;
   DWORD dwTimeOut = m_lTimeOut;
   DWORD bFirstNull=FALSE;
   DWORD dwFirstNullTime=0;
   DWORD dwNullTime=0;
   DWORD  dwErrors;
   COMSTAT cstStatus;
   int iRespMaxSize;
   BOOL bAsciiCompleted;
   CString s;

   iRespMaxSize = abyResponse.GetSize();
   
   dwRead=0;
   
   bAsciiCompleted=FALSE;
   
   do {
	   
	   if (dwRead>DWORD(iRespMaxSize-1)) {
		   return FALSE;
	   }

       if( !::ReadFile(m_hComHandle,&abyResponse[dwRead],1,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
		   if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus))
		   {
			   s.Format("ReadFile Errors 0x=%xd",dwErrors);
			   Log(s);
		   }
		   else{
			   Log(" ClearCommError Failed reading");
		   }
		   
		   return (FALSE);
       }
	   
	   
	   if (dwNumberOfBytesRead == 1){
		   dwRead += 1;
		   if (1==dwRead) {
			   m_PerfCounter.End();
		   }
		   dwTimeOut=m_dwSilentInterval;
           bFirstNull=FALSE;
		   dwNullTime=0;
		   if (dwRead>(DWORD)abyResponse.GetSize()) {
				return FALSE;
		   }
	   }
	   
	   else{
		   
		   if (!bFirstNull){
               bFirstNull=TRUE;
               dwFirstNullTime=::timeGetTime();
		   }
		   else {
			   dwNullTime=::timeGetTime();
		   }
		    //DoEvents();
		   ::Sleep(1);
		   
	   }
	   
	   
	   
	   if (!::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
		   Log("ReadResponse ClearCommError Failed reading");
	   }
	   
	   if (m_wTranmitionMode==MODE_ASCII){
		   if (dwRead>5){//Read at least  <:>  <Address> <Functioncode> 
			   if ((C_R==abyResponse[dwRead-2])&&(L_F==abyResponse[dwRead-1])){
				   bAsciiCompleted=TRUE;
			   }
		   }
		   
	   }
	   
	   
   } while	( ( (cstStatus.cbInQue>0)
		        ||(dwNullTime<(dwTimeOut+dwFirstNullTime))
			  )
			  &&(!bAsciiCompleted)
			);

   return(TRUE);
 
 }

 //read device response 
 //return true if Read OK 
 BOOL CLocalModbus::ReadResponseRTU(CByteArray& abyResponse,//bytes read 
							DWORD& dwRead) {    //num of bytes read 

   
   DWORD dwNumberOfBytesRead=0;
   DWORD  dwErrors;
   COMSTAT cstStatus;
   int iRespMaxSize;
   CString s;
  

   iRespMaxSize = abyResponse.GetSize();
   
   dwRead=0;
   
      
   if( !::ReadFile(m_hComHandle,&abyResponse[dwRead],INITIAL_READ,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
	   if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus))
	   {
		   s.Format("ReadFile Errors 0x=%xd",dwErrors);
		   Log(s);
	   }
	   else{
		   Log(" ClearCommError Failed reading");
	   }
	   
	   return (FALSE);
   }

   s.Format("RResp: bytes=%d", dwNumberOfBytesRead);
   Log(s);

   dwRead=dwRead+dwNumberOfBytesRead;

   if (INITIAL_READ==dwNumberOfBytesRead) {
	   
	   // do {
	   
	   if (dwRead>DWORD(iRespMaxSize-INITIAL_READ)) {
		   return FALSE;
	   }
	   
	   if( !::ReadFile(m_hComHandle,&abyResponse[dwRead],(iRespMaxSize-INITIAL_READ),&dwNumberOfBytesRead,NULL) ) { //read one charcter 
		   if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus))
		   {
			   s.Format("ReadFile Errors 0x=%xd",dwErrors);
			   Log(s);
		   }
		   else{
			   Log(" ClearCommError Failed reading");
		   }
		   
		   return (FALSE);
	   }
	   
	   dwRead=dwRead+dwNumberOfBytesRead;
	   
	   
	   //if (!::ClearCommError(m_hComHandle,&dwErrors,&cstStatus)){
	//	   Log("ReadResponse ClearCommError Failed reading");
	  // }
	   
	   
	   //} while	(READ_SIZE==dwNumberOfBytesRead);   
	   //} while	((cstStatus.cbInQue>0)||(READ_SIZE==dwNumberOfBytesRead));
	   
   }

   return(TRUE);
 
 }

 BOOL CLocalModbus::ReadResponse(CByteArray& abyResponse,DWORD& dwRead,DWORD dwBytesToRead) {
	 
	 DWORD dwErrors;
	 COMSTAT cstStatus;
     DWORD dwNumberOfBytesRead;
	 COMMTIMEOUTS timeouts;
	 DWORD dwCurrentReadTotalTimeoutConstant;
	 CString s;
	 	 
	 //Get Current Timeouts 
	 if(GetTimeouts(timeouts)) {
		 
		 dwCurrentReadTotalTimeoutConstant=timeouts.ReadTotalTimeoutConstant;				 
		 
	 }
	 else {
		 return FALSE;
	 }
	 
	 dwRead=0;

	 do {
		 if( !::ReadFile(m_hComHandle,&abyResponse[dwRead],1,&dwNumberOfBytesRead,NULL) ) { //read one charcter 
			 if (::ClearCommError(m_hComHandle,&dwErrors,&cstStatus))
			 {
				 s.Format("ReadFile Errors 0x=%xd",dwErrors);
				 Log(s);
			 }
			 else{
				 Log(" ClearCommError Failed reading");
			 }
			 
			 return (FALSE);
		 }
		 
		 if (dwNumberOfBytesRead == 1){
			 dwRead++;
			 if (1==dwRead) {
				 m_PerfCounter.End();
				 timeouts.ReadTotalTimeoutConstant=SilentInterval();
				 if (!SetTimeouts(&timeouts)){
					 Log("SetTimeouts Failed");
					 break;
				 }
			 }
		 }
		 
	 } while (dwNumberOfBytesRead>0);
	 
	 if (dwRead>0) {
		 timeouts.ReadTotalTimeoutConstant=dwCurrentReadTotalTimeoutConstant;
		 if (!SetTimeouts(&timeouts)){
			 Log("SetTimeouts Failed\n");
		 }
	 }

	 return (TRUE);
	 
 }
 

 BOOL CLocalModbus::ClearBuffers(){
   //Clear RxTx Buffers 
    return(::PurgeComm(m_hComHandle,(PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR)));
 }



void CLocalModbus::TransmissionMode(WORD wMode)
{

	m_wTranmitionMode=wMode;
}

WORD CLocalModbus::TransmissionMode()
{
	return m_wTranmitionMode;
}

void CLocalModbus::RTU2ASCII(BYTE* aRtu, int iSize , BYTE* aAscii)
{

	int i=0;

	//ASSERT((iSize*2)<aAscii.GetSize());

	for (i=0;i<iSize;i++){
		aAscii[i*2]=Num2Ascii(HI4BITS(aRtu[i]));
		aAscii[i*2+1]=Num2Ascii(LO4BITS(aRtu[i]));
	}

}

BYTE CLocalModbus::Num2Ascii(BYTE byNum)
{
	if (byNum<=9){
		return byNum+'0';
	}
	else if ((byNum>=0xA)&&(byNum<=0xF)){
		return ((byNum-0xA)+'A');
	}
	else {
		ASSERT(FALSE);
		return '0';
	}
}

BOOL CLocalModbus::VerifyRespLRC(CByteArray& aResp, DWORD dwLength)
{

	BYTE byLRC;
	BYTE byMsgLRC;

	if (dwLength<5) {
		return FALSE;
	}

	byLRC=LRCASCII(aResp.GetData(),WORD(dwLength)); //calc crc 

	byMsgLRC=HiLo4BitsToByte(Ascii2Num(aResp[dwLength-4]),
									   Ascii2Num(aResp[dwLength-3]));

	return (byLRC==byMsgLRC);
}

BYTE CLocalModbus::Ascii2Num(BYTE byChar)
{
	if ((byChar>='0')&&(byChar<='9')){
		return (byChar-'0');
	}
	else if ((byChar>='A')&&(byChar<='F')){
		return (byChar-'A')+0xA;
	}
	else {
     //ASSERT(FALSE);
		return 0; 

	}
}

BYTE CLocalModbus::HiLo4BitsToByte(BYTE byHi, BYTE byLo)
{

	return ( ( ((0xF)&byHi) << 4) | ( (0xF)&byLo)  );
	 
}


BYTE CLocalModbus::LRCASCII(BYTE *auchMsgASCII, WORD usDataLen) {

	BYTE uchLRC = 0 ;	/* LRC char initialized   */
	BYTE byTemp =0;
	int i;

	int iDataSize = int((usDataLen-5)/2);
	 
	for (i=0;i<iDataSize;i++){
		byTemp=HiLo4BitsToByte(Ascii2Num(auchMsgASCII[1+i*2]),
									   Ascii2Num(auchMsgASCII[1+i*2+1]));
		uchLRC +=byTemp;
	}

	//uchLRC = uchLRC ^ 0xffff;
	//uchLRC += 1;
										
	return BYTE(-uchLRC);


}

void CLocalModbus::ByteSize(WORD wSize)
{
  m_wByteSize= wSize;
}

WORD CLocalModbus::ByteSize()
{
	return m_wByteSize;
}

BOOL CLocalModbus::InitController(BOOL log)
{

	if (log)
	{
		char* pFileName = "comms.log";

		if ( !LogFile.Open( pFileName, CFile::modeCreate | CFile::modeWrite| CFile::typeText) )
		{
			return(FALSE);
		}

		m_fLog = TRUE;
	}

	Log("Controller dll initiated ok");

	return(TRUE);

}

void CLocalModbus::InitProgram()
{

	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = 0;
	CString s;

	Log("PP= Init Program");

	// set program to 1

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PNUMBER);	
		abyQuery[3] = LOBYTE(ETA_PNUMBER);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(1);	// set program number to 1 
		abyQuery[8] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	// set rate unit to minute

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PRATEUNIT);	
		abyQuery[3] = LOBYTE(ETA_PRATEUNIT);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(1);	// set rate to degs per minute 
		abyQuery[8] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	// set end action to reset

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PENDACTION);	
		abyQuery[3] = LOBYTE(ETA_PENDACTION);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(1);	// set end action to reset 
		abyQuery[8] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	// set holdback to 0 (whatever that does....)

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PHOLDBACK);	
		abyQuery[3] = LOBYTE(ETA_PHOLDBACK);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(0);	// set to 0 
		abyQuery[8] = LOBYTE(0);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	// set live program to 1

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PRUNNO);	
		abyQuery[3] = LOBYTE(ETA_PRUNNO);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(1);	// prog 1 
		abyQuery[8] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	// set fast run off

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PFASTRUN);	
		abyQuery[3] = LOBYTE(ETA_PFASTRUN);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(0);	// prog 1 
		abyQuery[8] = LOBYTE(0);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	ResetProgram();

}

void  CLocalModbus::ResetProgram()
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = 0;
	CString s;

	Log("PP= Reset Program");

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSTATUS);	
		abyQuery[3] = LOBYTE(ETA_PSTATUS);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(1);	// reset program 
		abyQuery[8] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}
}

void  CLocalModbus::RunProgram()
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = 0;
	CString s;

	Log("PP= Run Program");

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSTATUS);	
		abyQuery[3] = LOBYTE(ETA_PSTATUS);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(2);	// run program 
		abyQuery[8] = LOBYTE(2);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}
}

void  CLocalModbus::SetEndSeg(int segno)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = 0;
	CString s;

	s.Format("PP= SetEndSeg %d", segno);
	Log(s);

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE( ETA_PSEGTYPE + (32 * segno));	
		abyQuery[3] = LOBYTE( ETA_PSEGTYPE + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(1);	// end type segment 
		abyQuery[8] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE( ETA_PSEGTARGET + (32 * segno));	
		abyQuery[3] = LOBYTE( ETA_PSEGTARGET + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(0);	 
		abyQuery[8] = LOBYTE(0);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}
}

void CLocalModbus::SetLoop(int segno, int LoopTo, int Cycles)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = 0;
	CString s;

	s.Format("PP= SetLoop at %d to %d %d times", segno, LoopTo, Cycles);
	Log(s);

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGNO + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGNO + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(segno + 1);	// this segment's number 
		abyQuery[8] = LOBYTE(segno + 1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGTYPE + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGTYPE + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(2);	// this segment's type (2=go back) 
		abyQuery[8] = LOBYTE(2);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE( ETA_PDWELLTIME + (32 * segno));	
		abyQuery[3] = LOBYTE( ETA_PDWELLTIME + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(0);	 
		abyQuery[8] = LOBYTE(0);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE( ETA_PSEGLOOPTO + (32 * segno));	
		abyQuery[3] = LOBYTE( ETA_PSEGLOOPTO + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(LoopTo);	 
		abyQuery[8] = LOBYTE(LoopTo);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE( ETA_PCYCLES + (32 * segno));	
		abyQuery[3] = LOBYTE( ETA_PCYCLES + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(Cycles);	 
		abyQuery[8] = LOBYTE(Cycles);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}
}

void  CLocalModbus::SetRamp(int segno, float rate, float target)
{

	// segno is 0-based

	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = (int)(rate * 100);	// convert for Eurotherm
	CString s;

	s.Format("PP= Set ramp seg %d at %4.1f to %4.1f", segno, rate, target);
	Log(s);

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGNO + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGNO + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(segno + 1);	// this segment's number 
		abyQuery[8] = LOBYTE(segno + 1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGTYPE + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGTYPE + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(0);	// this segment's type (0=profile) 
		abyQuery[8] = LOBYTE(0);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGPSPTYPE + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGPSPTYPE + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(3);	// this segment's psptype (1 step,2 dwell, 3 ramp) 
		abyQuery[8] = LOBYTE(3);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGRATE + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGRATE + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(val);	// rate 
		abyQuery[8] = LOBYTE(val);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	val = (int)target;		// convert for Eurotherm

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGTARGET + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGTARGET + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(val);	// target 
		abyQuery[8] = LOBYTE(val);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}
}

void  CLocalModbus::SetDwell(int segno, int period, float target)
{

	// segno is 0-based

	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = (int)target;
	CString s;

	s.Format("PP= Set dwell seg %d at %4.1f for %d secs", segno, target, period);
	Log(s);

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGNO + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGNO + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(segno + 1);	// this segment's number 
		abyQuery[8] = LOBYTE(segno + 1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGTYPE + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGTYPE + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(0);	// this segment's type (0=profile) 
		abyQuery[8] = LOBYTE(0);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGPSPTYPE + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGPSPTYPE + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(2);	// this segment's psptype (1 step,2 dwell, 3 ramp) 
		abyQuery[8] = LOBYTE(2);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PSEGTARGET + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PSEGTARGET + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(val);	// target 
		abyQuery[8] = LOBYTE(val);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_PDWELLTIME + (32 * segno));	
		abyQuery[3] = LOBYTE(ETA_PDWELLTIME + (32 * segno));
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(period / 6);	// dwell period 
		abyQuery[8] = LOBYTE(period / 6);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}
}

// Shutdown is basically SetSetPoint without throw

void  CLocalModbus::ShutDown()
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	int val = 0;
	CString s;

	Log("Shutdown");

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_SET_POINT);	//SP
		abyQuery[3] = LOBYTE(ETA_SET_POINT);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(val);	// the value
		abyQuery[8] = LOBYTE(val);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		Log(s);
	}

}



BOOL CLocalModbus::TestComms()
{
	if ( LoopbackTest(m_DevAddr) != ERR_OK)
	{
		Log("Loopback test failed");
		return FALSE;
	}
	else
		Log("Loopback test OK");

	return TRUE;
}

float CLocalModbus::GetSetPoint() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	float val;
	CString s;

	Log("GetSetPoint");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_SET_POINT);	//SP
		abyQuery[3] = LOBYTE(ETA_SET_POINT);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			val = (float)( (float)res / (float)10 );
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}


}
int CLocalModbus::SetSetPoint(const float setpoint) throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	int val = (int)(setpoint * 10);
	CString s;


	s.Format("SetSetPoint %f %d", setpoint, val);
	Log(s);

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_SET_POINT);	//SP
		abyQuery[3] = LOBYTE(ETA_SET_POINT);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(val);	// the value
		abyQuery[8] = LOBYTE(val);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,6); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			if ( (abyReply[2] == abyQuery[2]) &&
				 (abyReply[3] == abyQuery[3]) &&
				 (abyReply[4] == abyQuery[4]) &&
				 (abyReply[5] == abyQuery[5]) )
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}
}

 
int CLocalModbus::GetProgramStatus()
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	int val;
	CString s;

	Log("GetProgramStatus");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_PSTATUS);	//SP
		abyQuery[3] = LOBYTE(ETA_PSTATUS);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			val = MAKEWORD(abyReply[4], abyReply[3]);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}

}

int CLocalModbus::GetTimeLeft()
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	int val;
	CString s;

	Log("GetTimeLeft");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_PTIMELEFT);	//SP
		abyQuery[3] = LOBYTE(ETA_PTIMELEFT);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			val = MAKEWORD(abyReply[4], abyReply[3]);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}

}

int CLocalModbus::GetCurSeg()
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	int val;
	CString s;

	Log("GetCurSeg");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_PCURSEG);	//SP
		abyQuery[3] = LOBYTE(ETA_PCURSEG);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			val = MAKEWORD(abyReply[4], abyReply[3]);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}

}

float CLocalModbus::GetSampleTemp() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	float val;
	CString s;

	Log("GetSampleTemp");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_SAMPLE_TEMP);	//SP
		abyQuery[3] = LOBYTE(ETA_SAMPLE_TEMP);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			val = (float)( (float)res / (float)10 );
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}

}

float CLocalModbus::GetFurnTemp() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	float val;
	CString s;

	Log("GetFurnTemp");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_FURN_TEMP);	//SP
		abyQuery[3] = LOBYTE(ETA_FURN_TEMP);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			val = (float)( (float)res / (float)10 );
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}

}
float CLocalModbus::GetBalance() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	float val;
	CString s;

	Log("GetBalance");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_BALANCE_HR);	//SP
		abyQuery[3] = LOBYTE(ETA_BALANCE_HR);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			val = (float)res;
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}

}

int CLocalModbus::GetPower() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	int val;
	CString s;

	Log("GetPower");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_POWER);	//SP
		abyQuery[3] = LOBYTE(ETA_POWER);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			val = MAKEWORD(abyReply[4], abyReply[3]);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}
}

int CLocalModbus::SetLogic1(int value) throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	int val;
	CString s;

	s.Format("SetLogic1 - %d", value);
	Log(s);

	do 
	{
		abyQuery.SetSize(9);
		abyReply.SetSize(6);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 16;			//Function write
		abyQuery[2] = HIBYTE(ETA_SET_LOGIC1);
		abyQuery[3] = LOBYTE(ETA_SET_LOGIC1);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);
		abyQuery[6] = 2;			// bytes to write
		abyQuery[7] = HIBYTE(value);	// the value
		abyQuery[8] = LOBYTE(value);


		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			val = MAKEWORD(abyReply[4], abyReply[3]);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( value );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}
}

float CLocalModbus::GetValvePosn() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	float val;
	CString s;

	Log("GetValvePosn");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_VALVE_POSN);	//SP
		abyQuery[3] = LOBYTE(ETA_VALVE_POSN);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			val = (float)( (float)res / (float)10 );
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError == CLocalModbus::ERR_OK)
		return( val );
	else
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);
		return(0);
	}
}

CString CLocalModbus::GetSerial() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	CString s;

	Log("GetSerial");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_SERIAL);	//SP
		abyQuery[3] = LOBYTE(ETA_SERIAL);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			s.Format("%d", res);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);	
	}

	return(s);
}

CString CLocalModbus::GetMID() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	CString s;

	Log("GetMID");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_MID);	//SP
		abyQuery[3] = LOBYTE(ETA_MID);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			s.Format("%X", res);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);	
	}

	return(s);
}

CString CLocalModbus::GetVersion() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	CString s;

	Log("GetVersion");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_VERSION);	//SP
		abyQuery[3] = LOBYTE(ETA_VERSION);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			s.Format("%X", res);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);	
	}

	return(s);
}

CString CLocalModbus::GetType() throw(CommsFailed)
{
	CByteArray abyQuery; 
	CByteArray abyReply;
	WORD nError;
	int iRetry=0;
	iRetry = 0;
	CString s;

	Log("GetType");

	do 
	{
		abyQuery.SetSize(6);
		abyReply.SetSize(5);

		abyQuery[0] = m_DevAddr;	//Dev ADDR 
		abyQuery[1] = 3;			//Function read
		abyQuery[2] = HIBYTE(ETA_TYPE);	//SP
		abyQuery[3] = LOBYTE(ETA_TYPE);
		abyQuery[4] = HIBYTE(1);	// no of words
		abyQuery[5] = LOBYTE(1);

		nError = TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,5); 

		if (nError==CLocalModbus::ERR_OK) 
		{	//interpret response	
			int res = MAKEWORD(abyReply[4], abyReply[3]);
			s.Format("%X", res);
			break;
		}
		iRetry++;
	} while ((iRetry < 5) && (CLocalModbus::ERR_OK != nError)); 

	if (nError != CLocalModbus::ERR_OK)
	{
		s.Format("error %d", nError);
		throw CommsFailed(s);	
	}

	return(s);
}

void CLocalModbus::Log(const char* buf)
{

	if (!m_fLog) return;

	TRACE(buf);
	TRACE("\n");
	LogFile.WriteString(buf);
	LogFile.WriteString("\n");
	LogFile.Flush();

}

void CLocalModbus::Log(CString str)
{

	if (!m_fLog) return;

	TRACE(str);
	TRACE("\n");
	LogFile.WriteString(str);
	LogFile.WriteString("\n");
	LogFile.Flush();
}

