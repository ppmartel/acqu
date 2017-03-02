#ifndef __HV_1932_t_h__
#define __HV_1932_t_h__

enum { ENChannel = 49 };
class HV_1932_t {
 public:
  Float_t fSetI0;
  Float_t fMonI0;
  Float_t fSetV[ENChannel];
  Float_t fMonV[ENChannel];
  Float_t fRDWn[ENChannel];
  Float_t fRUp[ENChannel];
  Char_t* fNm[ENChannel];  
};

#endif

