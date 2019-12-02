/**
 *  \addtogroup CkLdb
 */
 
/*@{*/
 
#include "NewEnergyLB.h"
#include "ckgraph.h"
#include "stdio.h"
#include "charm++.h"
#include "lbdb.h"
#include "/home/karina/libLB/files.h"
 
#include <iostream>
#include <vector>
#include <string>
#include <errno.h>
#include <stdlib.h>
 
 
/* --------------------- Load balancer statements --------------------- */
CreateLBFunc_Def(NewEnergyLB, "Load balancing using DVFS")
 
NewEnergyLB::NewEnergyLB(const CkLBOptions &opt) : CentralLB(opt) {
    lbname = "NewEnergyLB";
    if(CkMyPe() == 0)
        CkPrintf("\nNewEnergyLB created on processor %d\n",CkMyPe());
}
 
CmiBool NewEnergyLB::QueryBalanceNow(int _step) {
    return CmiTrue;
}
 
 
 
/* --------------- Load balancer available functions --------------- */
int NewEnergyLB::getCoreWithHighestLoad(int npes, double *pe_loads) {
    int more = 0;

    for (int i = 0; i < npes; i++) {
        if (pe_loads[more] < pe_loads[i]) {
            more = i;
        }
    }
    return more;
}
 
void setMaxFrequency(int core, unsigned long maxClock) {
    char cmd[256];
 
    sprintf(cmd, "cpufreq-set -c %d -f %lu", core, maxClock);
    system(cmd);
 
    // printf("\n Changed clock to %d  at core %d. \n", maxClock, core);
}
 
 
void setFrequency(int maxLoad, double currentCoreLoad, unsigned long maxFreq, int core, unsigned long minFreq)
{
    char cmd[256];
    unsigned long newFreq;
 
 
    // Checking BIOS limits for frequency changes
    // If the calculated value is less than the allowed limit, the minimum frequency is assumed
    if ((currentCoreLoad/maxLoad)*maxFreq < minFreq) {
        sprintf(cmd, "cpufreq-set -c %d -f %lu ", core, minFreq);
        system(cmd);
        printf("Frequency below the minimum value allowed. Assuming the minimum frequency allowed by the fabricant.\n");
        return;
    }
 
    // If the calculated value is more than the allowed limit, the maximum frequency is assumed
    if ((currentCoreLoad/maxLoad)*maxFreq > maxFreq) {
        sprintf(cmd, "cpufreq-set -c %d -f %lu ", core, maxFreq);
        system(cmd);
        printf("Frequency above the maximum value allowed. Assuming the maximum frequency allowed by the fabricant.\n");
        return;
    }
 
 
    // If the value does not exceed BIOS limits, the algorithm will continue normally.
    // calculates the core frequency
    newFreq = (currentCoreLoad/maxLoad)*maxFreq;
 
    // creates the command string to change the clock
    sprintf(cmd, "cpufreq-set -c %d -f %lu ", core, newFreq);
 
    // executes the command contained in the string
    system(cmd);
 
    printf("\n Changed clock to %d  at core %d.", newFreq, core);
 
}
 
char * NewEnergyLB::getGovernor(int core) {
    FILE *fp;
    char msr_file_name[100];
    char *gov;
 
 
    sprintf(msr_file_name, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", core);
    fp = fopen(msr_file_name, "r");
 
    if (fp < 0) {
        printf("[getGovernor] Error [1], core %d !\n", core);
        CkExit();
    }
    if (fp == NULL) {
        printf("[getGovernor] Error trying to open the file scaling_governor! Core [%d].", core);
        CkExit();
    }
 
    gov = (char *) malloc(10 * sizeof(char));
    fscanf(fp, "%12s", gov);
    fclose(fp);
 
    return(gov);
}
 
unsigned long NewEnergyLB::getMaxFrequency(int core) {
    FILE *fp;
    char msr_file_name[100];
    int iclock;
 
    sprintf(msr_file_name, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", core);
    fp = fopen(msr_file_name, "r");
 
    if (fp < 0) {
        printf("[getMaxFrequency] Error [1], core %d !\n", core);
        CkExit();
    }
    if (fp == NULL) {
        printf("[getMaxFrequency]  Error trying to open the file scaling_max_freq! Core [%d].", core);
        CkExit();
    }
 
    fscanf(fp, "%d", &iclock);
    fclose(fp);
 
    return(iclock);
}
 
unsigned long NewEnergyLB::getMinFrequency(int core)
{
    FILE *fp;
    char msr_file_name[100];
    int iclock;
 
    sprintf(msr_file_name, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", core);
    fp = fopen(msr_file_name, "r");
 
    if (fp < 0) {
        printf("[getMinFrequency] Error [1], core %d\n", core);
        CkExit();
    }
    if (fp == NULL) {
        printf("[getMinFrequency]  Error trying to open the file scaling_min_freq ! Core [%d].", core);
        CkExit();
    }
 
    fscanf(fp, "%d", &iclock);
    fclose(fp);
 
    return(iclock);
}
 
 
unsigned long NewEnergyLB::getCurrentFrequency(int core)
{
 
    FILE *fp;
    char msr_file_name[100];
    int iclock;
 
    sprintf(msr_file_name, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core);
    fp = fopen(msr_file_name, "r");
 
    if (fp < 0) {
        printf("[getCurrentFrequency] Error [1] core %d\n", core);
        CkExit();
    }
    if (fp == NULL) {
        printf("[getCurrentFrequency]  Error trying to open the file scaling_cur_freq ! Core [%d].", core);
        CkExit();
    }
 
    fscanf(fp, "%d", &iclock);
    fclose(fp);
 
    return(iclock);
}
 
void NewEnergyLB::searchForPe(LDStats *stats, double *pe_loads) {
    int highestPe = getCoreWithHighestLoad(stats->nprocs(), pe_loads); // gets core with the highest load
    int minFreq[stats->nprocs()], maxFreq[stats->nprocs()], currentFreq[stats->nprocs()];
    char *scalingGovernor[stats->nprocs()];
 
    printf("\nHIGHEST PE: %d\n", highestPe);
    printf("CARGA MÁXIMA: %f\n\n", pe_loads[highestPe]);
 
    printf("DEBUG LIMITEDE DA BIOS");

   // Get the current frequency of cores, the minimum and maximum frequency that can be assigned
    for (int i = 0; i < stats->nprocs(); i++) {
        maxFreq[i] = getMaxFrequency(i);
        minFreq[i] = getMinFrequency(i);
        currentFreq[i] = getCurrentFrequency(i);
        scalingGovernor[i] = getGovernor(i);
         
        // BIOS clock limits
        printf("CORE %d", i);
        printf("Frequência máxima admitida pelo sistema: %d \n", maxFreq[i]);
        printf("Frequência mínima admitida pelo sistema: %d \n", minFreq[i]);
        printf("Governor em execução: %s \n", scalingGovernor[i]);
    }
     
    // runs through all cores
    for (int i = 0; i < stats->nprocs(); i++) {
        
        // if not the most loaded core, use the formula
        if (i != highestPe) {
            printf("PE[%d] load %f\n", i, pe_loads[i]);
 
            setFrequency(highestPe, pe_loads[i], maxFreq[i], i, minFreq[i]);
 
            currentFreq[i] = getCurrentFrequency(i);
 
            printf("Clock changed to: %d \n", currentFreq[i]);
            CkPrintf("-------------------------------------------- \n\n\n");
        }else {
            // printf("PE[%d] load %f\n", i, pe_loads[i]);
 
            setMaxFrequency(i, maxFreq[i]);
 
            currentFreq[i] = getCurrentFrequency(i);
 
            printf("\nMaximum frequency assumed. %d \n", currentFreq[i]);
            CkPrintf("-------------------------------------------- \n\n\n");
        }
 
        // rechecks which core has the most load
        highestPe = getCoreWithHighestLoad(stats->nprocs(), pe_loads);
    }
}
 
/* --------------------- Load balancer main function ---------------------*/
 
void NewEnergyLB::work(LDStats *stats) {
 
    // load cpufreq module if it has not been initialized
    char cmdCpuFreq[256];
    sprintf(cmdCpuFreq, "   ");
    system(cmdCpuFreq);
 
    /************ INICIALIZAÇÃO ************/
    /*                                      /
    *   n_pes: number of processors         *
    *   mag: overall load average           *
    *                                       *
    /***************************************/
 
    int n_pes = stats->nprocs();
    int *num_objs_pe[n_pes]; //total PEs objects amount
    double pe_loads[n_pes];
    const int obj = 1;
 
 
 
    /****************Preparing Strategy*****************/
    /* Inicializes all pe_loads at 0                   */
    /* Inicializes all PEs objects amount at 0         */
    /***************************************************/
    for (int i = 0; i < n_pes; i++)
    {
        pe_loads[i] = 0;
        num_objs_pe[i] = 0;
    }
 
    // Calculates cores loads
 
    double coreLoad = 0.0;
 
    for(int i = 0; i<stats->n_objs; i++)
    {
        LDObjData &oData = stats->objData[i]; // current core info
        int pe = stats->from_proc[i]; // current Pe
        coreLoad = oData.wallTime; // total amout of time of this object
        pe_loads[pe] += coreLoad; //calculates the total load of each Pe
        num_objs_pe[pe] += obj; // total amount of objects associated at this core
    }
 
 
    /** ================ STRATEGY ================ */
    searchForPe(stats, pe_loads);
 
 
    /**================ CLEANUP ================ */
 }
 
#include "NewEnergyLB.def.h"
/*@}*/