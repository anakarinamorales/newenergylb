/**
 *  \addtogroup CkLdb
 */
 
/*@{*/
 
#ifndef _NEWENERGYLB_H_
#define _NEWENERGYLB_H_
 
#include "CentralLB.h"
#include "NewEnergyLB.decl.h"
 
void CreateNewEnergyLB();
 
class NewEnergyLB : public CentralLB {
  public:
    NewEnergyLB(const CkLBOptions &opt);
    NewEnergyLB(CkMigrateMessage *m) : CentralLB (m) { };
    int getCoreWithHighestLoad(int npes, double *pe_loads);
    char * getGovernor(int core);
    unsigned long getMaxFrequency(int core);
    unsigned long getMinFrequency(int core);
    unsigned long getCurrentFrequency(int core);
    void searchForPe(LDStats *stats, double *pe_loads);
    void work(LDStats *stats);
    void pup(PUP::er &p) { CentralLB::pup(p); }
    void FreeProcs(double* bufs);
    void FreeProcs(int* bufs);
 
  private:
    CmiBool QueryBalanceNow(int _step);
};
 
#endif /* _GRAPHPARTLB_H_ */
 
/*@}*/