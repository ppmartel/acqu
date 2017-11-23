#include "TA2Event.h"


//-----------------------------------------------------------------------------

TA2Event::TA2Event() : TObject()
{
  nEvent = 0;
}

//-----------------------------------------------------------------------------

TA2Event::~TA2Event()
{

}

//-----------------------------------------------------------------------------

void TA2Event::Clear()
{
  for(Int_t t=0; t<MAXBEAM; t++)
  {
    Beam[t].Reset();
    Beam[t].SetPDG(NULL);
  }

  for(Int_t t=0; t<MAXPART; t++)
  {
    Particle[t].Reset();
    Particle[t].SetPDG(NULL);
  }

  nBeam = 0;
  nParticle = 0;

  PatternL1 = 0;
  PatternL2 = 0;
}

//-----------------------------------------------------------------------------

void TA2Event::AddBeam(TA2Particle pBeam)
{
  if(nBeam < MAXBEAM)
  {
    Beam[nBeam] = pBeam;
    Beam[nBeam].SetPDG(NULL);
    nBeam++;
  }
  else
    printf("TA2Event beam storage exceeded in event %d\n", nEvent);
}

//-----------------------------------------------------------------------------

void TA2Event::AddParticle(TA2Particle pParticle)
{
  if(nParticle < MAXPART)
  {
    Particle[nParticle] = pParticle;
    Particle[nParticle].SetPDG(NULL);
    nParticle++;
  }
  else
    printf("TA2Event particle storage exceeded in event %d %d\n", nEvent, nParticle);
}

//-----------------------------------------------------------------------------
ClassImp(TA2Event)
