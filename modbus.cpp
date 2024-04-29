// modbus.cpp: implementation of the CModbus class.
//
//////////////////////////////////////////////////////////////////////

#include "mbu.h"
#include "modbus.h"
#include "ModbusRes\resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



// Class CModbusException
IMPLEMENT_DYNAMIC(CModbusException, CException)

CModbusException::CModbusException(CString& sMessage,int nError)
{
	m_strMessage = sMessage;
	m_nError = nError;
}

BOOL CModbusException::GetErrorMessage(LPTSTR lpstrError, UINT nMaxError,
		PUINT pnHelpContext /*= NULL*/)
{

	char text[200];
	if(m_nError == 0) {
		wsprintf(text, "%s error", (const char*) m_strMessage);
	}
	else {
		wsprintf(text, "%s error #%d", (const char*) m_strMessage, m_nError);
	}
	strncpy(lpstrError, text, nMaxError - 1);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( CModbus, CObject)

CModbus::CModbus()
{
	m_wRetries=1;
	m_bThrowModbusException=FALSE;
	m_dwLockTimeout=1000;
	m_dwDelay=0;
}

CModbus::~CModbus()
{

}


WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CWordArray& anRegValues) {
	 
	 CByteArray abyQuery;
	 short nReplyLength=(3 + 2 * nQtd);
	 CByteArray  abyReply;// new int[nReplyLength];
	 WORD nError;
	 short nByte;
	 short nRespByte;
	 const short QUERY_LENGHT=6;
	 int  iRetry=0;
	 
	 
	 abyQuery.SetSize(6);
	 abyReply.SetSize(nReplyLength);
	 //Query 
	 
	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=READ_OUTPUT_REGISTERS; //Function 
	 abyQuery[2]=HIBYTE(nDataStart);
	 abyQuery[3]=LOBYTE(nDataStart);
	 abyQuery[4]=HIBYTE(nQtd);
	 abyQuery[5]=LOBYTE(nQtd);
	 
	 
	 do {
		 
		 nError=TxRxMessage(abyQuery,6,abyReply, nReplyLength);
		 
		 
		 if (nError==ERR_OK) {
			 if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
				 nError=ERR_INV_RESP;
			 }
			 else {
				 nByte=0;//return array 
				 for(nRespByte=3;nRespByte<nReplyLength;nRespByte=(short)(nRespByte+2)){
					 anRegValues[nByte] = (abyReply[nRespByte]);
					 anRegValues[nByte]= ((anRegValues[nByte]<<8)|(abyReply[nRespByte+1]));
					 nByte++;
				 }
				 
			 }
			 
			 
		 }//nerror=ERR_OK
		 
		 iRetry++;

		 ErrorDelay(nError);
		 
	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	 
	 ThrowModbusException(nError);

	 return(nError);
	 
 }//end ReadOutRegisters 



 WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CShortArray& anRegValues) {
 

	 CWordArray anValues;
	 WORD nError;
	 anValues.SetSize(anRegValues.GetSize());
	 nError =ReadOutputRegisters(nAddr,nDataStart,nQtd,anValues);
	 Word2ShortArray(anValues,anRegValues);
	 return nError;

 }


 WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CWordArray& anRegValues) {
	 
	 CByteArray abyQuery;
	 short nReplyLength=(3 + 2 * nQtd);
	 CByteArray  abyReply;// new int[nReplyLength];
	 WORD nError;
	 short nByte;
	 short nRespByte;
	 const short QUERY_LENGHT=6;
	 int  iRetry=0;
	 
	 
	 abyQuery.SetSize(6);
	 abyReply.SetSize(nReplyLength);
	 //Query 
	 
	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=READ_INPUT_REGISTERS; //Function 
	 abyQuery[2]=HIBYTE(nDataStart);
	 abyQuery[3]=LOBYTE(nDataStart);
	 abyQuery[4]=HIBYTE(nQtd);
	 abyQuery[5]=LOBYTE(nQtd);
	 
	 do {
		 
		 nError=TxRxMessage(abyQuery,6,abyReply, nReplyLength);
		 
		 
		 if (nError==ERR_OK) {
			 if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
				 nError=ERR_INV_RESP;
			 }
			 else {
				 nByte=0;//return array 
				 for(nRespByte=3;nRespByte<nReplyLength;nRespByte=(short)(nRespByte+2)){
					 anRegValues[nByte] = (abyReply[nRespByte]);
					 anRegValues[nByte]= ((anRegValues[nByte]<<8)|(abyReply[nRespByte+1]));
					 nByte++;
				 }
				 
			 }
			 
			 
		 }//nerror=ERR_OK
		 
		 iRetry++;

		 ErrorDelay(nError);
		 
	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	 
	 ThrowModbusException(nError);

	 return(nError);
	 
 }//end ReadInRegisters 


 WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CShortArray& anRegValues) {
 

	 CWordArray anValues;
	 WORD nError;
	 anValues.SetSize(anRegValues.GetSize());
	 nError =ReadInputRegisters(nAddr,nDataStart,nQtd,anValues);
	 Word2ShortArray(anValues,anRegValues);
	 return nError;

 }

 
 WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CDblArray& anRegValues,WORD wRealType) {
 

	 CWordArray anValues;
	 WORD nError;
	 anValues.SetSize(4*anRegValues.GetSize());
	 nError =ReadInputRegisters(nAddr,nDataStart,4*nQtd,anValues);
	 Word2Dbl(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
	 return nError;

 }

 WORD CModbus::ReadInputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CFloatArray& anRegValues,WORD wRealType) {
 

	 CWordArray anValues;
	 WORD nError;
	 anValues.SetSize(2*anRegValues.GetSize());
	 nError =ReadInputRegisters(nAddr,nDataStart,2*nQtd,anValues);
	 Word2Float(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
	 return nError;

 }

 WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CDblArray& anRegValues,WORD wRealType) {
 

	 CWordArray anValues;
	 WORD nError;
	 anValues.SetSize(4*anRegValues.GetSize());
	 nError =ReadOutputRegisters(nAddr,nDataStart,4*nQtd,anValues);
	 Word2Dbl(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
	 return nError;

 }

 WORD CModbus::ReadOutputRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CFloatArray& anRegValues,WORD wRealType) {
 

	 CWordArray anValues;
	 WORD nError;
	 anValues.SetSize(2*anRegValues.GetSize());
	 nError =ReadOutputRegisters(nAddr,nDataStart,2*nQtd,anValues);
	 Word2Float(anValues.GetData(),anValues.GetSize(),anRegValues.GetData(),anRegValues.GetSize(),wRealType);
	 return nError;

 }

//Read output status "coils" 
 //return
 
 WORD CModbus::ReadInputStatus(WORD nAddr, //Modbus device Address 
							   WORD nDataStart , //
							   WORD nQtd , 
							   CByteArray& abCoilValues) {
	 
	 static WORD const  QUERY_LENGHT=6;
	 
	 CByteArray abyQuery;
	 WORD nReplyLength=(3 + (nQtd + 7) / 8);
	 CByteArray  abyReply;//[]  = new int[nReplyLength];
	 WORD nError;
	 WORD nRespByte;
	 WORD nCoil;
	 WORD nBit;
	 int  iRetry=0;
	 
	 //set array sizes 
	 abyQuery.SetSize(QUERY_LENGHT);
     abyReply.SetSize(nReplyLength); 
	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=READ_INPUT_STATUS; //Function 
	 abyQuery[2]=HIBYTE(nDataStart);
	 abyQuery[3]=LOBYTE(nDataStart);
	 abyQuery[4]=HIBYTE(nQtd);
	 abyQuery[5]=LOBYTE(nQtd);
	
	 do {
		 
		 nError=TxRxMessage(abyQuery,(QUERY_LENGHT),abyReply, nReplyLength);
		 
		 
		 if (nError==ERR_OK) {
			 if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
				 nError=ERR_INV_RESP;
			 }
			 else {
				 nBit=0;
				 nRespByte=3;		
				 for(nCoil=0;nCoil<nQtd;nCoil++){
					 
					 if (nBit>7){
						 nBit=0;
						 nRespByte++;
					 }
					 
					 if ((abyReply[nRespByte] & (0x01<<nBit))>0){
						 abCoilValues[nCoil]= TRUE; 
					 }
					 else {
						 abCoilValues[nCoil]= FALSE; 
					 }
					 nBit++; //next bit 
					 
				 }// end for 
				 
			 }// else 
			 
			 
		 }//nerror=ERR_OK
		
		 iRetry++;

		 ErrorDelay(nError);
		 
	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	 
	 ThrowModbusException(nError);

	 return(nError);
	 
 }//end ReadInputStatus 



//Read output status "coils" 
 //return
 
 WORD CModbus::ReadOutputStatus(WORD nAddr, //Modbus device Address 
							   WORD nDataStart , //
							   WORD nQtd , 
							   CByteArray& abCoilValues) {
	 
	 static WORD const  QUERY_LENGHT=6;
	 
	 CByteArray abyQuery;
	 WORD nReplyLength=(3 + (nQtd + 7) / 8);
	 CByteArray  abyReply;//[]  = new int[nReplyLength];
	 WORD nError;
	 WORD nRespByte;
	 WORD nCoil;
	 WORD nBit;
	 int  iRetry=0;
	 
	 //set array sizes 
	 abyQuery.SetSize(QUERY_LENGHT);
     abyReply.SetSize(nReplyLength); 
	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=READ_OUTPUT_STATUS; //Function 
	 abyQuery[2]=HIBYTE(nDataStart);
	 abyQuery[3]=LOBYTE(nDataStart);
	 abyQuery[4]=HIBYTE(nQtd);
	 abyQuery[5]=LOBYTE(nQtd);
	

	 do {
		 
		 nError=TxRxMessage(abyQuery,(QUERY_LENGHT),abyReply, nReplyLength);
		 
		 
		 if (nError==ERR_OK) {
			 if ((abyReply[0]!=abyQuery[0])||(abyReply[1]!=abyQuery[1])) {
				 nError=ERR_INV_RESP;
			 }
			 else {
				 nBit=0;
				 nRespByte=3;		
				 for(nCoil=0;nCoil<nQtd;nCoil++){
					 
					 if (nBit>7){
						 nBit=0;
						 nRespByte++;
					 }
					 
					 if ((abyReply[nRespByte] & (0x01<<nBit))>0){
						 abCoilValues[nCoil]= TRUE; 
					 }
					 else {
						 abCoilValues[nCoil]= FALSE; 
					 }
					 nBit++; //next bit 
					 
				 }// end for 
				 
			 }// else 
			 
			 
		 }//nerror=ERR_OK

		 iRetry++;

		 ErrorDelay(nError);

	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	 
	 ThrowModbusException(nError);

	 return(nError);
	 
 }//end ReadOutStatus 


 WORD CModbus::PresetMultipleRegisters(WORD nAddr,  WORD nDataStart , WORD nQtd , CWordArray& anRegValues) {
	 
	 
	 const WORD QUERY_LENGHT=(7+2*nQtd);
	 CByteArray abyQuery;//[QUERY_LENGHT];
	 short nReplyLength=(6);
	 CByteArray  abyReply;
	 short nError;
	 int	  iReg; 	
	 int  iRetry=0;
	 
	 
	 //Query 
	 abyQuery.SetSize(QUERY_LENGHT);
	 abyReply.SetSize(nReplyLength);
	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=PRESET_MULTIPLE_REGISTERS; //Function 
	 abyQuery[2]=HIBYTE(nDataStart);
	 abyQuery[3]=LOBYTE(nDataStart);
	 abyQuery[4]=HIBYTE(nQtd);
	 abyQuery[5]=LOBYTE(nQtd);
	 abyQuery[6]=(2*nQtd);
	 
	 for (iReg=0;iReg<(nQtd);iReg++){
		 abyQuery[7+iReg*2]=HIBYTE(anRegValues[iReg]);
		 abyQuery[7+iReg*2+1]=LOBYTE(anRegValues[iReg]);
	 }
	 
	 do {
		 nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);
		 
		 
		 if (nError==ERR_OK) {
			 
			 for (iReg=0;iReg<6;iReg++){
				 if (abyReply[iReg]!=abyQuery[iReg]) {
					 nError=ERR_INV_RESP;
					 break;
				 }
			 }
			 
		 }//nerror=ERR_OK
		 
		 iRetry++;

		 ErrorDelay(nError);
		 
	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	 
	 ThrowModbusException(nError);

	 return(nError);
	 
 }//end PresetMultipleRegisters 

 WORD CModbus::PresetMultipleRegisters(WORD nAddr, //address of modbus device 
							  WORD nDataStart , //start address, first address is 0 "ZERO" 
							  WORD nQtd , //number of reg . to write 
							  CShortArray& anRegValues) {
    
	 CWordArray anValues;
	 Short2WordArray(anRegValues,anValues);
	 return PresetMultipleRegisters(nAddr,nDataStart,nQtd ,anValues);
	 
 } //end PresetMultipleRegisters //CShortArray


 WORD CModbus::PresetMultipleRegisters(WORD nAddr, //address of modbus device 
							  WORD nDataStart , //start address, first address is 0 "ZERO" 
							  WORD nQtd , //number of reg . to write 
							  CDblArray& adRegValues,
							  WORD wRealType) {
    
	 CWordArray anValues;
	 anValues.SetSize(nQtd*4);
	 ASSERT(nQtd<=adRegValues.GetSize());
	 
	 Dbl2Word(adRegValues.GetData(),nQtd,anValues.GetData(),nQtd*4,wRealType);

	 return PresetMultipleRegisters(nAddr,nDataStart,nQtd*4 ,anValues);
	 
 } //end PresetMultipleRegisters //CShortArray


WORD CModbus::PresetMultipleRegisters(WORD nAddr, //address of modbus device 
							  WORD nDataStart , //start address, first address is 0 "ZERO" 
							  WORD nQtd , //number of reg . to write 
							  CFloatArray& afRegValues,
							  WORD wRealType) {
    
	 CWordArray anValues;
	 anValues.SetSize(nQtd*2);
	 ASSERT(nQtd<=afRegValues.GetSize());
	 
	 Float2Word(afRegValues.GetData(),nQtd,anValues.GetData(),nQtd*2,wRealType);

	 return PresetMultipleRegisters(nAddr,nDataStart,nQtd*2 ,anValues);
	 
 } //end PresetMultipleRegisters //CFloatArray



WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister , WORD nRegValue) {
	 
	 
	 const short QUERY_LENGHT=(6);
	 CByteArray abyQuery;
	 short nReplyLength=(6);
	 CByteArray  abyReply;
	 short nError;
	 int	  iReg; 
	 int  iRetry=0;
	 
	 
	 
	 //Query 
	 abyQuery.SetSize(QUERY_LENGHT); 
	 abyReply.SetSize(nReplyLength);
	 
	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=PRESET_SINGLE_REGISTER; //Function 
	 abyQuery[2]=HIBYTE(nRegister);
	 abyQuery[3]=LOBYTE(nRegister);
	 abyQuery[4]=HIBYTE(nRegValue);
	 abyQuery[5]=LOBYTE(nRegValue);
	 
	 do {
		 
		 nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);
		 
		 
		 if (nError==ERR_OK) {
			 
			 for (iReg=0;iReg<QUERY_LENGHT;iReg++){
				 if (abyReply[iReg]!=abyQuery[iReg]) {
					 nError=ERR_INV_RESP;
					 break;
				 }
			 }
			 
		 }//nerror=ERR_OK
		 
		 iRetry++;

		 ErrorDelay(nError);
		 
	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 

	 
	 ThrowModbusException(nError);

	 return(nError);
	 
	}//end PresetSingleRegister

 WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister , short int nRegValue) {
 
    return(PresetSingleRegister(nAddr,nRegister ,(WORD)nRegValue));
 }

 
 WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister ,float fRegValue, WORD wRealType){
 
    CWordArray anRegs;
	WORD* pRegValue= (WORD*)&fRegValue; 
	
	anRegs.SetSize(2);
	
	if (REAL_NORMAL== wRealType) { 
		
		anRegs[0]= *(pRegValue+0); 
		anRegs[1]= *(pRegValue+1); 
	
	}
	else {
	
		anRegs[0]= *(pRegValue+1); 
		anRegs[1]= *(pRegValue+0); 

	}

    return( PresetMultipleRegisters(nAddr , nRegister , 2 ,anRegs));
 
 }

 WORD CModbus::PresetSingleRegister(WORD nAddr,  WORD nRegister , double dRegValue, WORD wRealType){
 
    CWordArray anRegs;
	WORD* pRegValue= (WORD*)&dRegValue; 
	
	anRegs.SetSize(4);
	
	if (REAL_NORMAL== wRealType) { 
		
		anRegs[0]= *(pRegValue+0); 
		anRegs[1]= *(pRegValue+1); 
		anRegs[2]= *(pRegValue+2); 
		anRegs[3]= *(pRegValue+3); 
	
	}
	else {
	
		anRegs[0]= *(pRegValue+3); 
		anRegs[1]= *(pRegValue+2); 
		anRegs[2]= *(pRegValue+1); 
		anRegs[3]= *(pRegValue+0); 

	}

    return( PresetMultipleRegisters(nAddr , nRegister , 4 ,anRegs));
 
 }


 WORD CModbus::ForceSingleCoil(WORD nAddr, WORD nRegister, BOOL bCoilValue) {
			
			
			short QUERY_LENGHT=(6);
			CByteArray abyQuery;
			short nReplyLength=(6);
			CByteArray  abyReply;
			short nError;
			int	  iReg;
			int iRetry=0;
			
			
			
			//Query 
			abyQuery.SetSize(QUERY_LENGHT);
			abyReply.SetSize(nReplyLength);
			//modbus message 
			abyQuery[0]=(BYTE)nAddr; //ADDR	
			abyQuery[1]=FORCE_SINGLE_COIL; //Function 
			abyQuery[2]=HIBYTE(nRegister); 
			abyQuery[3]=LOBYTE(nRegister);
			abyQuery[4]=HIBYTE(CmodBool(bCoilValue));
			abyQuery[5]=LOBYTE(CmodBool(bCoilValue));
			
		
			do {
				nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);
				
				
				if (nError==ERR_OK) {
					
					for (iReg=0;iReg<QUERY_LENGHT;iReg++){
						if (abyReply[iReg]!=abyQuery[iReg]) {
							nError=ERR_INV_RESP;
							break;
						}
					}
					
				}//nerror=ERR_OK
				
				iRetry++;

				ErrorDelay(nError);
				
			} while ((iRetry<m_wRetries)&&(ERR_OK!=nError));

			
			ThrowModbusException(nError);

			return(nError);
			
		}//end ForceSingleCoil 

 
 WORD CModbus::ForceMultipleCoils(WORD nAddr,  WORD nDataStart , WORD nQtd , CByteArray& abCoilValues) {
	 
	 
	 short QUERY_LENGHT=(7+(nQtd + 7)/8);
	 CByteArray abyQuery;
	 short nReplyLength=(short)(6);
	 CByteArray  abyReply;
	 short nError;
	 short nQueryByte;
	 int   iCoil; 
	 int   iBit;
	 int   iReg;
	 int  iRetry=0;
	 
	 
	 
	 //Query 
     abyQuery.SetSize(QUERY_LENGHT);
	 abyReply.SetSize(nReplyLength); 

	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=FORCE_MULTIPLE_COILS; //Function 
	 abyQuery[2]=HIBYTE(nDataStart);
	 abyQuery[3]=LOBYTE(nDataStart);
	 abyQuery[4]=HIBYTE(nQtd);
	 abyQuery[5]=LOBYTE(nQtd);
	 abyQuery[6]=(BYTE)((nQtd + 7)/8);
	 nQueryByte=7;
	 iBit=0;

	 for (iCoil=0;iCoil<(nQtd);iCoil++){
		 if (iBit>7){
			 iBit=0;
			 nQueryByte++;
		 }
		 
		 if (abCoilValues[iCoil]) {
			 abyQuery[nQueryByte]=(abyQuery[nQueryByte] | (0x01<<iBit));
		 }
		 iBit++;
		 
	 }
	 
	 do {
		 nError=TxRxMessage(abyQuery,QUERY_LENGHT,abyReply, nReplyLength);
		 
		 
		 if (nError==ERR_OK) {
			 
			 for (iReg=0;iReg<6;iReg++){
				 if (abyReply[iReg]!=abyQuery[iReg]) {
					 nError=ERR_INV_RESP;
					 break;
				 }
			 }
			 
		 }//nerror=ERR_OK
		 iRetry++;

		 ErrorDelay(nError);
		 
	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError));

	 
	 ThrowModbusException(nError);

	 return(nError);
	 
 }//end ForceMultipleCoils 



WORD CModbus::LoopbackTest(WORD nAddr) {

	 static const WORD QUERY_LENGHT=(6);
	 static const WORD nReplyLength=(6);
	 CByteArray abyQuery; 
	 CByteArray abyReply;
	 WORD  nError;
	 int  iReg;
	 int  iRetry=0;
	 
	 abyQuery.SetSize(QUERY_LENGHT);
	 abyReply.SetSize(nReplyLength);
	 //modbus message 
	 abyQuery[0]=(BYTE)nAddr; //ADDR	
	 abyQuery[1]=LOOP_BACK_TEST; //Function 
	 abyQuery[2]=HIBYTE(0); //diag. code=0
	 abyQuery[3]=LOBYTE(0);
	 abyQuery[4]=0xA5;
	 abyQuery[5]=0x37;
	 
	 do {
	
		 nError=TxRxMessage(abyQuery,abyQuery.GetSize(),abyReply,nReplyLength);
		
		 if (nError==ERR_OK) {
			 
			 for (iReg=0;iReg<QUERY_LENGHT;iReg++){
				 if (abyReply[iReg]!=abyQuery[iReg]) {
					 nError=ERR_INV_RESP;
					 break;
				 }
			 }
			 
		 }

		 iRetry++;

		 ErrorDelay(nError);

	 } while ((iRetry<m_wRetries)&&(ERR_OK!=nError)); 
	 
	 ThrowModbusException(nError);

	 return(nError);
	 
 }//end LoopBackTest



CString CModbus::ErrorMessage(WORD wErrorCode){
	 
	 CString sRetValue("");
	 CString sAux;
	 
     if (wErrorCode>=ERR_EXCPTION_CODE)
	 {
		  sAux.Format(" %d",(wErrorCode-ERR_EXCPTION_CODE));
          sRetValue=MSG_MODBUS_ERR_EXCPTION_CODE;
		  sRetValue+=sAux;

	 }
	 else if(ERR_OK==wErrorCode)
	 { 
	   		sRetValue=MSG_MODBUS_ERR_OK;
	 } 
	 else if(ERR_TIMEOUT==wErrorCode) 
	 {
			sRetValue=MSG_MODBUS_ERR_TIMEOUT;
	 }
     else if (ERR_INV_RESP==wErrorCode) 
	 {
			sRetValue=MSG_MODBUS_ERR_INV_RESP;
	 }
	 else if (ERR_CRC==wErrorCode) 
	 {
			sRetValue=MSG_MODBUS_ERR_CRC;
	 }
	 else if (ERR_WR_PORT==wErrorCode) 
	 {
			sRetValue=MSG_MODBUS_ERR_WR_PORT;
	 }

	 else if (ERR_RD_PORT==wErrorCode) 
	 {
			sRetValue=MSG_MODBUS_ERR_RD_PORT;
	 }

	 else if (ERR_NOT_INT==wErrorCode) 
	 {
			sRetValue=MSG_MODBUS_ERR_NOT_INT;
	 }

	 else if (ERR_LOCK_TIME_OUT==wErrorCode) 
	 {
			sRetValue=MSG_MODBUS_ERR_LOCK_TIME_OUT;
	 }


	 return sRetValue;
	 
 }


float CModbus::ResponseTime() {
   
   float fRespTime;	 

   if(Lock()){

	fRespTime=m_PerfCounter.CountToMiliSec(m_PerfCounter.DiffCounts());
    Unlock();
   }
   return fRespTime;
 }

 void CModbus::Serialize( CArchive& archive )
  {
	  // call base class function first
	  // base class is CObject in this case
	  CObject::Serialize( archive );
	  

	  // now do the stuff for our specific class
	  if( archive.IsStoring() ){
		 archive << m_wRetries << m_bThrowModbusException << m_dwDelay;
      } 
	  else {
		 archive >> m_wRetries >> m_bThrowModbusException >> m_dwDelay;
      }

  }

 //Delay Error 
 void CModbus::ErrorDelay(WORD nError) {

	 //if (ERR_OK!=nError) {
	 //	 ::Sleep(m_dwDelay);
	 //}

 }


//lock com device
  BOOL CModbus::Lock() {
	  return(m_csComm.Lock(m_dwLockTimeout));
  }

   
  //unlock com device
  BOOL CModbus::Unlock() {
	  return m_csComm.Unlock();
  }




//private functions 

//C++ boolean to modbus boolean 
 WORD CModbus::CmodBool(BOOL bCoil) {
	 if (TRUE==bCoil) {
		 return(0xFF00);
	 }
	 else {
		 return(0x0000);
	 }
	 
 }


void CModbus::Short2WordArray(CShortArray& aiArray ,CWordArray& anArray){
    int i;

	anArray.SetSize(aiArray.GetSize());
    for (i=0;i<=aiArray.GetUpperBound();++i){
	    anArray[i]=(WORD)aiArray[i];
	   
	}


}

void CModbus::Word2ShortArray(CWordArray& anArray ,CShortArray& aiArray){
    int i;

	aiArray.SetSize(anArray.GetSize());
    for (i=0;i<=aiArray.GetUpperBound();++i){
	    aiArray[i]=(short)anArray[i];
	   
	}
}

void CModbus::Word2Dbl(const WORD* pawSrc, int iSrcSize,double* padDest, int iDestSize,WORD wRealType){
    int i;
	WORD* pwDest;

	ASSERT(iSrcSize==(4*iDestSize));

	for (i=0;i<iDestSize;++i){
		pwDest = (WORD*)(padDest+i);
		if (REAL_NORMAL==wRealType) {
			*(pwDest+0)= pawSrc[4*i+0];
			*(pwDest+1)= pawSrc[4*i+1];
			*(pwDest+2)= pawSrc[4*i+2];
			*(pwDest+3)= pawSrc[4*i+3];
		}
		else {
			*(pwDest+0)= pawSrc[4*i+3];
			*(pwDest+1)= pawSrc[4*i+2];
			*(pwDest+2)= pawSrc[4*i+1];
			*(pwDest+3)= pawSrc[4*i+0];
		}
	}
}


void CModbus::Word2Float(const WORD* pawSrc, int iSrcSize,float* padDest, int iDestSize,WORD wRealType){
    int i;
	WORD* pwDest;

	ASSERT(iSrcSize==(2*iDestSize));

	for (i=0;i<iDestSize;++i){
		pwDest = (WORD*)(padDest+i);
		if (REAL_NORMAL==wRealType) {
			*(pwDest+0)= pawSrc[2*i+0];
			*(pwDest+1)= pawSrc[2*i+1];
		}
		else {
			*(pwDest+0)= pawSrc[2*i+1];
			*(pwDest+1)= pawSrc[2*i+0];
		}
	}
}


void CModbus::Dbl2Word(const double* padSrc, int iSrcSize,WORD* pawDest, int iDestSize,WORD wRealType){

    
	WORD* pRegValue; 
	int i;

	ASSERT((4*iSrcSize)==(iDestSize));
	
	for(i=0;i<iSrcSize;i++) {
		
		pRegValue= (WORD*)(padSrc+i);
		
		if (REAL_NORMAL== wRealType) { 
			
			pawDest[4*i+0]= *(pRegValue+0); 
			pawDest[4*i+1]= *(pRegValue+1); 
			pawDest[4*i+2]= *(pRegValue+2); 
			pawDest[4*i+3]= *(pRegValue+3); 
			
		}
		else {
			
			pawDest[4*i+0]= *(pRegValue+3); 
			pawDest[4*i+1]= *(pRegValue+2); 
			pawDest[4*i+2]= *(pRegValue+1); 
			pawDest[4*i+3]= *(pRegValue+0); 
			
		}
	}
}


void CModbus::Float2Word(const float* padSrc, int iSrcSize,WORD* pawDest, int iDestSize,WORD wRealType){

    
	WORD* pRegValue; 
	int i;

	ASSERT((2*iSrcSize)==(iDestSize));
	
	for(i=0;i<iSrcSize;i++) {
		
		pRegValue= (WORD*)(padSrc+i);
		
		if (REAL_NORMAL== wRealType) { 
			
			pawDest[2*i+0]= *(pRegValue+0); 
			pawDest[2*i+1]= *(pRegValue+1); 
		}
		else {
			
			pawDest[2*i+0]= *(pRegValue+1); 
			pawDest[2*i+1]= *(pRegValue+0); 
		}
	}
}


WORD CModbus::Retries() const {
	return m_wRetries;
 }
 
 void CModbus::Retries(WORD wRetries){
	m_wRetries=wRetries;
 }

 
 DWORD  CModbus::Delay(){
	
	return m_dwDelay;

 }

 
void  CModbus::Delay(DWORD dwDelay){
	
	m_dwDelay=dwDelay;

}


void CModbus::ThrowException(BOOL bThrow)
{
	m_bThrowModbusException=bThrow;
}

BOOL CModbus::ThrowException() const
{
	return m_bThrowModbusException;
}


void CModbus::ThrowModbusException(WORD wErrorcode)
{
	if ((ERR_OK!=wErrorcode)&&(m_bThrowModbusException)) {

		throw new CModbusException(ErrorMessage(wErrorcode),wErrorcode);

	}

}


void CModbus::DoEvents() {

	MSG message;

	if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
}

//Modbus Functions 
const BYTE CModbus::READ_OUTPUT_REGISTERS=3;
const BYTE CModbus::READ_INPUT_REGISTERS=4; 
const BYTE CModbus::READ_OUTPUT_STATUS=1; 
const BYTE CModbus::READ_INPUT_STATUS=2;
const BYTE CModbus::PRESET_MULTIPLE_REGISTERS=16;
const BYTE CModbus::PRESET_SINGLE_REGISTER=6;
const BYTE CModbus::LOOP_BACK_TEST=8;
const BYTE CModbus::FORCE_SINGLE_COIL=5;
const BYTE CModbus::FORCE_MULTIPLE_COILS=15;




