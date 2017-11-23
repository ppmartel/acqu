//--Author	Ken Livingston    23nd Sep 2012  ROOT Wrapper for EPICs ezca function 
//                                               Get the useful functions available in a shared root lib
//                                               with dictionary.
//--Rev 	
//
//--Description
//  ROOT Wrapper for EPICs ezca function. Get the useful functions available in a shared root lib
//  with dictionary.
//  No need to make a class out of this stuff.

#include <ezcaRoot.h>

int epicsGet(char *pvname, char ezcatype,int nelem, void *data_buff){  return ezcaGet(pvname, ezcatype,nelem,data_buff);}
int epicsGetControlLimits(char *pvname,double *low, double *high){  return ezcaGetControlLimits(pvname,low,high);}
int epicsGetGraphicLimits(char *pvname,double *low, double *high){  return  ezcaGetGraphicLimits(pvname,low, high);}
int epicsGetNelem(char *pvname, int *nelem){  return ezcaGetNelem(pvname, nelem);}
int epicsGetPrecision(char *pvname,short *precision){  return ezcaGetPrecision(pvname,precision);}
int epicsGetStatus(char *pvname,TS_STAMP *timestamp, short *status, short *severity){  return ezcaGetStatus(pvname,timestamp, status, severity);}
int epicsGetUnits(char *pvname,char *units){return ezcaGetUnits(pvname,units);} /* units must be at least	EZCA_UNITS_SIZE large */
int epicsGetWithStatus(char *pvname,char ezcatype, int nelem, void *data_buff, TS_STAMP *timestamp,short *status, short *severity){
  return ezcaGetWithStatus(pvname,ezcatype, nelem, data_buff, timestamp,status, severity);
}
int epicsPut(char *pvname, char ezcatype,int nelem, void *data_buff){ return ezcaPut(pvname, ezcatype,nelem, data_buff);}
int epicsPutOldCa(char *pvname, char ezcatype,int nelem, void *data_buff){  return ezcaPutOldCa(pvname, ezcatype,nelem, data_buff);}
int epicsSetMonitor(char *pvname, char ezcatype, unsigned long count=0){  return ezcaSetMonitor(pvname, ezcatype, count);}

//These are puts and gets for specific types to make it versy simple. 
int epicsGetChar(char *pvname, int nelem, char *data_buff){return epicsGet(pvname,kEpicsBYTE,nelem,(void*)data_buff);}
int epicsGetString(char *pvname, int nelem, char *data_buff){return epicsGet(pvname,kEpicsSTRING,nelem,(void*)data_buff);}
int epicsGetShort(char *pvname, int nelem, short *data_buff){return epicsGet(pvname,kEpicsSHORT,nelem,(void*)data_buff);}
int epicsGetFloat(char *pvname, int nelem, float *data_buff){return epicsGet(pvname,kEpicsFLOAT,nelem,(void*)data_buff);}
int epicsGetDouble(char *pvname, int nelem, double *data_buff){return epicsGet(pvname,kEpicsDOUBLE,nelem,(void*)data_buff);}

int epicsPutChar(char *pvname, int nelem, char *data_buff){return epicsPut(pvname,kEpicsBYTE,nelem,(void*)data_buff);}
int epicsPutString(char *pvname, int nelem, char *data_buff){return epicsPut(pvname,kEpicsSTRING,nelem,(void*)data_buff);}
int epicsPutShort(char *pvname, int nelem, short *data_buff){return epicsPut(pvname,kEpicsSHORT,nelem,(void*)data_buff);}
int epicsPutFloat(char *pvname, int nelem, float *data_buff){return epicsPut(pvname,kEpicsFLOAT,nelem,(void*)data_buff);}
int epicsPutDouble(char *pvname, int nelem, double *data_buff){return epicsPut(pvname,kEpicsDOUBLE,nelem,(void*)data_buff);}

