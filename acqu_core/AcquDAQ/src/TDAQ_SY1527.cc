//--Author	JRM Annand   18th May 2008
//--Rev 	JRM Annand...
//--Rev 	JRM Annand... 3rd Jun 2008  Conditional compilation
//--Update	JRM Annand... 6th Jul 2011  gcc4.6-x86_64 warning fix
//--Update  A Neiser...   6th June 2013  Make char_t* const
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQ_SY1527
// CAEN SY1527 HV Crate
// Assumes ethernet connection
//

#include "TDAQ_SY1527.h"


enum { Esy1527_Crate=100, Esy1527_P1932, Esy1527_S1932, Esy1527_SaveHV,
       Esy1527_TurnOn, Esy1527_TurnOff };
#ifdef AD_CAENHV
static Map_t kSY1527Keys[] = {
  {"Crate:",       Esy1527_Crate},
  {"P1932:",       Esy1527_P1932},
  {"S1932:",       Esy1527_S1932},
  {"SaveHV:",      Esy1527_SaveHV},
  {"TurnOn:",      Esy1527_TurnOn},
  {"TurnOff:",     Esy1527_TurnOff},
  {NULL,           -1}
};
#endif

//-----------------------------------------------------------------------------
TDAQ_SY1527::TDAQ_SY1527(const Char_t* name, const Char_t* file, FILE* log ):
  TDAQmodule( name, file, log )
{
#ifdef AD_CAENHV
  // Basic initialisation
  //  fCtrl = new TDAQcontrol( this );       // tack on control functions
  fType = EDAQ_Ctrl;                         // controller board (has slaves)
  AddCmdList( kSY1527Keys );                 // SY1527-specific cmds
  fLink = LINKTYPE_TCPIP;                    // ethernet connection
  fCommandLine = new Char_t[ELineSize];      // input commands
  fNm = BuildName(name);                     // name of crate
  fHV_1932 = new HV_1932_t[ENSlot];          // data for plug-in modules
  fIsSlotOccupied = new Bool_t[ENSlot];      // slots occupied ?
  fIsSlotConfigured = new Bool_t[ENSlot];      // slots occupied ?
  for( Int_t i=0; i<ENSlot; i++ ){
    fIsSlotOccupied[i] = kFALSE;             // init slot not occupied
    fIsSlotConfigured[i] = kFALSE;           // init slot not occupied
    for( Int_t j=0; j<ENChannel; j++ )
      fHV_1932[i].fNm[j] = NULL;             // init channel not defined
  }
#endif
}

//-----------------------------------------------------------------------------
TDAQ_SY1527::~TDAQ_SY1527( )
{
  // Disconnect
  Disconnect();
  delete fHV_1932;
  delete fIsSlotOccupied;
}

//-----------------------------------------------------------------------------
void TDAQ_SY1527::SetConfig( Char_t* line, Int_t key )
{
#ifdef AD_CAENHV
  // Configuration from file
  Char_t ip[256];
  Char_t name[256];
  Char_t pass[256];
  Float_t v0,i0,rdwn,rup;
  Int_t slot,chan;

  switch( key ){
  case Esy1527_Crate:
    // Crate parameters
    if( sscanf(line,"%s%s%s",ip,name,pass) != 3 )
      PrintError(line,"<Crate setup>",EErrFatal);
    fIP = BuildName(ip);
    fUser = BuildName(name);
    fPasswd = BuildName(pass);
    Connect();
    break;
  case Esy1527_P1932:
    // Primary channel setup
    if( sscanf(line,"%s%d%f%f%f%f",name,&slot,&v0,&i0,&rdwn,&rup) != 6 )
      PrintError(line,"<Primary Channel Parameters>",EErrFatal);
    if( !fIsSlotOccupied[slot] ){
      PrintError(line,"<HV slot not occupied>");
      break;
    }
    fHV_1932[slot].fNm[0] = BuildName(name);
    fHV_1932[slot].fSetI0 = i0;
    fHV_1932[slot].fSetV[0] = v0;
    fHV_1932[slot].fRDWn[0] = rdwn;
    fHV_1932[slot].fRUp[0] = rup;
    fIsSlotConfigured[slot] = kTRUE;
    break;
  case Esy1527_S1932:
    // General internal register write
    // Primary channel setup
    if( sscanf(line,"%s%d%d%f%f%f",name,&slot,&chan,&v0,&rdwn,&rup) != 6 )
      PrintError(line,"<Secondary Channel Parameters>",EErrFatal);
    if( !fIsSlotOccupied[slot] ){
      PrintError(line,"<HV slot not occupied>");
      break;
    }
    fHV_1932[slot].fNm[chan] = BuildName(name);
    fHV_1932[slot].fSetV[chan] = v0;
    fHV_1932[slot].fRDWn[chan] = rdwn;
    fHV_1932[slot].fRUp[chan] = rup;
    break;
  case Esy1527_SaveHV:
    // read values from HV and save to file
    SaveValues();
    break;
  case Esy1527_TurnOn:
    // turn on
    TurnOn();
    break;
  case Esy1527_TurnOff:
    // turn off
    TurnOff();
    break;
  default:
    // try general module setup commands
    TDAQmodule::SetConfig(line, key);
    break;
  }
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_SY1527::PostInit()
{
#ifdef AD_CAENHV
  // Download parameters to HV crate
  Char_t* name;
  Float_t i0,v,rdwn,rup;
  for(Int_t i=0; i<fNSlot; i++){
    if( !fIsSlotOccupied[i] ) continue;      // module present in slot?
    name = fHV_1932[i].fNm[0];
    i0 =  fHV_1932[i].fSetI0;
    v = fHV_1932[i].fSetV[0];
    rdwn = fHV_1932[i].fRDWn[0];
    rup = fHV_1932[i].fRUp[0];
    if( (fErrorCode = WriteNm(i,name,0)) ) return;
    if( (fErrorCode = Write(i,(Char_t*)"I0Set",&i0,0)) ) return;
    if( (fErrorCode = Write(i,(Char_t*)"V0Set",&v,0)) ) return;
    if( (fErrorCode = Write(i,(Char_t*)"RDWn",&rdwn,0)) ) return;
    if( (fErrorCode = Write(i,(Char_t*)"RUp",&rup,0)) ) return;
    for( Int_t j=1; j<ENChannel; j++ ){
      name = fHV_1932[i].fNm[j];
      if( !name ) continue;                  // channel defined?
      v = fHV_1932[i].fSetV[j];
      rdwn = fHV_1932[i].fRDWn[j];
      rup = fHV_1932[i].fRUp[j];
      if( (fErrorCode = WriteNm(i,name,j)) ) return;
      if( (fErrorCode = Write(i,(Char_t*)"V0Set",&v,j)) ) return;
      if( (fErrorCode = Write(i,(Char_t*)"RDWn",&rdwn,j)) ) return;
      if( (fErrorCode = Write(i,(Char_t*)"RUp",&rup,j)) ) return;
    }
  }
  TDAQmodule::PostInit();
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_SY1527::Connect()
{
#ifdef AD_CAENHV
  // Login to SY_1527
  // and get map of modules plugged in
  //
  if( fIsInit ) return;
  fIsInit = kTRUE;
  fSoftwareRelease = BuildName(CAENHVLibSwRel());
  fprintf(fLogStream, " CAEN HV Wrapper release: %s\n", fSoftwareRelease );

  fErrorCode = CAENHVInitSystem( fNm, fLink, fIP, fUser, fPasswd);
  if( fErrorCode == CAENHV_OK )
    fprintf(fLogStream, " HV System %s logged in\n", fNm);
  else{
    Char_t errmess[256];
    sprintf(errmess,"SY 1527 Error: %s (#%d)",CAENHVGetError(fNm),fErrorCode);
    PrintError(errmess,"<HV Connect>", EErrFatal);
  }

  fErrorCode = CAENHVGetCrateMap( fNm, &fNSlot, &fNChannel, &fModelList,
				  &fDescList, &fSerial, &fRelMin, &fRelMax );
  printf(" Crate Map of HV system %s\n\n", fNm );
  Char_t* m = fModelList;
  Char_t* d = fDescList;
  Int_t i,j,n;
  Char_t (*par)[MAX_PARAM_NAME];
  for( i = 0; i < fNSlot; i++ , m += strlen(m) + 1, d += strlen(d) + 1 ){
    if( *m == '\0' )
      fIsSlotOccupied[i] = kFALSE;
    else{          
      fIsSlotOccupied[i] = kTRUE;
      fprintf(fLogStream,
	      "Board %2d: %s %s  #Channel:%d  Serial:%d  Firmware:%d.%d\n",
	      i, m, d, fNChannel[i], fSerial[i], fRelMax[i], fRelMin[i]);
      CAENHVGetChParamInfo( fNm, i, 1, &fInfo );
      par = (char (*)[MAX_PARAM_NAME])fInfo;
      fprintf(fLogStream, "Parameters: ");
      for( j = 0 ; par[j][0] ; j++ ) fprintf(fLogStream," %s",par[j]);
      fprintf(fLogStream,"\n");
      n = j;
    }
  }
  return;
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_SY1527::Disconnect( )
{
#ifdef AD_CAENHV
  // Disconnect
  if( !fIsInit ) return;
  fIsInit = kFALSE;
  fErrorCode = CAENHVDeinitSystem(fNm);
  if( fErrorCode == CAENHV_OK )
    fprintf( fLogStream, "SY-1527 HV system %s logged out\n", fNm );
  else{
    Char_t errmess[256];
    sprintf(errmess,"SY 1527 Error: %s (#%d)",
	    CAENHVGetError(fNm),fErrorCode);
    PrintError(errmess,"<HV Disconnect>");
  }
#endif
}

//-----------------------------------------------------------------------------
Int_t TDAQ_SY1527::Read(UShort_t slot, Char_t* parName, void* val,
			UShort_t list, UShort_t n )
{
#ifdef AD_CAENHV
  // Read parameter (parName) of value (val) to module at slot.
  // n channels, held in list
  Int_t error;
  error = CAENHVGetChParam(fNm, slot, parName, n, &list, val);
  if( error != CAENHV_OK )
    fprintf(fLogStream,"<SY1527 slot/chan-%d/%d:%s Read Error:%d %s>\n",
	    slot, list, parName, error, CAENHVGetError(fNm));
  return error;
#endif
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TDAQ_SY1527::Write(UShort_t slot, Char_t* parName, void* val, 
			 UShort_t list, UShort_t n )
{
#ifdef AD_CAENHV
  // Write parameter (parName) of value (val) to module at slot.
  // n channels, held in list
  Int_t error;
  error = CAENHVSetChParam(fNm, slot, parName, n, &list, val);
  usleep(1);
  if( error != CAENHV_OK )
    fprintf(fLogStream,"<SY1527 slot/chan-%d/%d:%s Write Error:%d %s>\n",
	    slot, list, parName, error, CAENHVGetError(fNm));
  return error;
#endif
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TDAQ_SY1527::ReadNm(UShort_t slot, Char_t* name,
			  UShort_t list, UShort_t n )
{
#ifdef AD_CAENHV
  // Read name of module at slot.
  // n channels, held in list
  Char_t (*par)[MAX_CH_NAME];
  par = (char (*)[MAX_CH_NAME])name;
  Int_t error;
  error = CAENHVGetChName(fNm, slot, n, &list, par);
  if( error != CAENHV_OK )
    fprintf(fLogStream,"<SY1527 slot/chan-%d/%d Name Read Error:%d %s>\n",
	    slot, list, error, CAENHVGetError(fNm));
  return error;
#endif
  return 0;
}

//-----------------------------------------------------------------------------
Int_t TDAQ_SY1527::WriteNm(UShort_t slot, Char_t* name, 
			   UShort_t list, UShort_t n )
{
#ifdef AD_CAENHV
  // Write name of module at slot.
  // n channels, held in list
  Int_t error;
  error = CAENHVSetChName(fNm, slot, n, &list, name);
  usleep(1);
  if( error != CAENHV_OK )
    fprintf(fLogStream,"<SY1527 slot/chan-%d/%d Name Write Error:%d %s>\n",
	    slot, list, error, CAENHVGetError(fNm));
  return error;
#endif
  return 0;
}

//-----------------------------------------------------------------------------
void TDAQ_SY1527::SaveValues( )
{
#ifdef AD_CAENHV
  // Save values read from crate to file
  //
  Char_t name[256];
  Float_t i0,v,rdwn,rup;
  FILE* saveHV = fopen("HVsave1527.dat","w");
  fprintf(saveHV,"## Auto-Generated SY1527 HV file\n");
  fprintf(saveHV,"Crate: %s %s %s\n",fIP,fUser,fPasswd);
  for(Int_t i=0; i<fNSlot; i++){
    if( !fIsSlotOccupied[i] ) continue;
    if( (fErrorCode = ReadNm(i,name,0)) ) return;
    if( (fErrorCode = Read(i,(Char_t*)"I0Set",&i0,0)) ) return;
    if( (fErrorCode = Read(i,(Char_t*)"V0Set",&v,0)) ) return;
    if( (fErrorCode = Read(i,(Char_t*)"RDWn",&rdwn,0)) ) return;
    if( (fErrorCode = Read(i,(Char_t*)"RUp",&rup,0)) ) return;
    fprintf(saveHV,"P1932: %s %d %f %f %f %f\n",name,i,v,i0,rdwn,rup);
    for( Int_t j=1; j<ENChannel; j++ ){
      if( (fErrorCode = ReadNm(i,name,j)) ) return;
      if( (fErrorCode = Read(i,(Char_t*)"V0Set",&v,j)) ) return;
      if( (fErrorCode = Read(i,(Char_t*)"RDWn",&rdwn,j)) ) return;
      if( (fErrorCode = Read(i,(Char_t*)"RUp",&rup,j)) ) return;
      fprintf(saveHV,"S1932: %s %d %d %f %f %f\n",name,i,j,v,rdwn,rup);
    }
    fprintf(saveHV,"SaveHV: \n");
  }
  fclose(saveHV);
  return;
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_SY1527::TurnOnOff( Bool_t pon )
{
#ifdef AD_CAENHV
  // Turn selected modules in crate ON
  //
  for(Int_t i=0; i<fNSlot; i++){
    if( !fIsSlotConfigured[i] ) continue;
    if( (fErrorCode = Write(i,(Char_t*)"Pw",&pon,0)) ) return;
    //    fprintf(saveHV,"P1932: %s %d %f %f %f %f\n",name,i,v,i0,rdwn,rup);
    for( Int_t j=1; j<ENChannel; j++ ){
      if( !fHV_1932[i].fNm[j] ) continue;
      if( (fErrorCode = Write(i,(Char_t*)"Pw",&pon,j)) ) return;
    }
  }
  printf("All HV channels turned ON\n");
  return;
#endif
}

//-----------------------------------------------------------------------------
void TDAQ_SY1527::CommandLoop()
{
#ifdef AD_CAENHV
  for(;;){
    printf("\nAcquHV> ");
    fgets(fCommandLine, ELineSize, stdin);
    CommandConfig(fCommandLine);
  } 
#endif
}

ClassImp(TDAQ_SY1527)
