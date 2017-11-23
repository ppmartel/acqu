#include "TProcessor.h"


//-----------------------------------------------------------------------------

TProcessor::TProcessor()
{

}
//-----------------------------------------------------------------------------

TProcessor::TProcessor(const Char_t* name) : TNamed(name, "Processor")
{

}

//-----------------------------------------------------------------------------

TProcessor::~TProcessor()
{

}

//-----------------------------------------------------------------------------

void TProcessor::Run()
{
  Char_t Line[1024];
  Char_t Log[256];

  Running = true;

  sprintf(Log, "Thread%XLog.txt", Number);
  sprintf(Line, "AcquRoot --batch %s >> %s 2>&1", Config, Log);
  gSystem->Exec(Line);

  Running = false;
}

//-----------------------------------------------------------------------------

ClassImp(TProcessor)
