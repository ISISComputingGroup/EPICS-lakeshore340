#include <stdlib.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include <aSubRecord.h>


static long calculateNewExcitationFromThresholds(aSubRecord *prec) {
    return 0; /* process output links */
}

epicsRegisterFunction(calculateNewExcitationFromThresholds);
