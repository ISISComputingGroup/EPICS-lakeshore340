#include <stdlib.h>
#include <stdio.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include <aSubRecord.h>
#include <errlog.h>

static void readFileLineByLine(FILE *thresholdsFile) {
    char line[256];
    while (fgets(line, sizeof(line), thresholdsFile) != NULL) {
        errlogSevPrintf(errlogMajor, line);
    }
}

static long calculateNewExcitationFromThresholds(aSubRecord *prec) {
    FILE *thresholdsFile = fopen(prec->a, "r");
    if (thresholdsFile == NULL) {
        errlogSevPrintf(errlogMajor, "Failed to open thresholds file");
        return 1;
    }
    readFileLineByLine(thresholdsFile);
    fclose(thresholdsFile);
    return 0; /* process output links */
}

epicsRegisterFunction(calculateNewExcitationFromThresholds);
