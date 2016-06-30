//--Author	JRM Annand   14th Oct 2003
//--Rev 	D Glazier... 24th Aug 2007 extra branches from G4A2
//--Update	JRM Annand.. 23rd Apr 2013 noticed additional EI_emwpc
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// MCBranchID
//
// GEANT3 Monte Carlo produces HBOOK ntuple file. On conversion to ROOT file
// via h2root utility, each set of variables is put on a separate branch.
// The enum specifies branch ID integer consts.
//
// PAW printout of the HBOOK ntuple created by the current (25/08/2003)
// version of the CB GEANT3.21 Monte Carlo simulation
//
// ******************************************************************
// * Ntuple ID = 12     Entries = 10000     ntuple
// ******************************************************************
// * Var numb * Type * Packing *    Range     *  Block   *  Name    *
// ******************************************************************
// *      1   * I*4  *         * [0,99]       * KINE     * npart
// *      2   * R*4  *         *              * KINE     * dircos(3,npart)
// *      3   * R*4  *         *              * KINE     * plab(npart)
// *      4   * R*4  *         *              * KINE     * elab(npart)
// *      5   * I*4  *         *              * KINE     * idpart(npart)
// *      6   * R*4  *         *              * KINE     * vertex(3)
// *      7   * R*4  *         *              * KINE     * beam(5)
// *      1   * I*4  *         * [0,672]      * E_DEP    * nhits
// *      2   * R*4  *         *              * E_DEP    * ecryst(nhits)
// *      3   * I*4  *         *              * E_DEP    * icryst(nhits)
// *      4   * R*4  *         *              * E_DEP    * eNaI
// *      5   * I*4  *         * [0,24]       * E_DEP    * vhits
// *      6   * R*4  *         *              * E_DEP    * eveto(2,vhits)
// *      7   * I*4  *         *              * E_DEP    * iveto(2,vhits)
// *      8   * R*4  *         *              * E_DEP    * etot
// *      9   * R*4  *         *              * E_DEP    * eleak
// *      1   * I*4  *         * [0,529]      * E_TAPS   * ntaps
// *      2   * I*4  *         *              * E_TAPS   * ictaps(ntaps)
// *      3   * R*4  *         *              * E_TAPS   * ectapsl(ntaps)
// *      4   * R*4  *         *              * E_TAPS   * ectapfs(ntaps)
// *      5   * R*4  *         *              * E_TAPS   * tctaps(ntaps)
// *      6   * R*4  *         *              * E_TAPS   * evtaps(ntaps)
// ******************************************************************
// *  Block   *  Entries  * Unpacked * Packed *   Packing Factor    *
// ******************************************************************
// * KINE     *  10000    * 2412     * Var.   *    Variable         *
// * E_DEP    *  10000    * 5780     * Var.   *    Variable         *
// * E_TAPS   *  10000    * 10584    * Var.   *    Variable         *
// * Total    *    ---    * 18776    * Var.   *    Variable         *
// ******************************************************************
// * Blocks = 3            Variables = 22      Max. Columns = 4694  *
// ******************************************************************

#ifndef __MCBranchID_h__
#define __MCBranchID_h__

// Indices to specifiy where info stored on fragmented branch
enum{ EI_npart, EI_dircos, EI_plab, EI_elab, EI_idpart, EI_vertex, EI_beam,
      EI_nhits, EI_ecryst, EI_icryst, EI_eNaI,
      EI_vhits, EI_eveto, EI_iveto, EI_tot, EI_leak,
      EI_ntaps, EI_ictaps, EI_ectapsl, EI_ectapfs, EI_tctaps, EI_evtaps,
      EI_nvtaps, EI_ivtaps, EI_tcryst, EI_tveto,
      EI_nmwpc, EI_imwpc, EI_mposx, EI_mposy, EI_mposz, EI_emwpc,
      EI_nscin, EI_escin, EI_tscin, EI_iscin, EI_xscin, EI_yscin, EI_zscin,
      EI_mc_evt_id,EI_mc_rnd_id
};

#endif
