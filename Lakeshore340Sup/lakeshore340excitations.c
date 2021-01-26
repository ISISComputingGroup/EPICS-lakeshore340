#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include <aSubRecord.h>
#include <errlog.h>

char * getExcitation(FILE *thresholdsFile, epicsInt32 tempSp) {
    char *newExcitation;
    newExcitation = (char*)malloc(20*sizeof(char));
    errlogSevPrintf(errlogMajor, "Before while");
    char buffer[33];
    itoa(tempSp, buffer, 10);
    errlogSevPrintf(errlogMajor, buffer);
    char line[256];
    while (fgets(line, sizeof(line), thresholdsFile) != NULL) {
        char *lineDup = strdup(line);
        errlogSevPrintf(errlogMajor, lineDup);
        char *tempString;
        char *search = ",";
        tempString = strtok(lineDup, search);
        if (tempString != NULL) {
            errlogSevPrintf(errlogMajor, tempString);
        } else {
            errlogSevPrintf(errlogMajor, "tempString is null");
        }
        int tempThreshold = atoi(tempString);
        if (tempSp >= tempThreshold) {
            newExcitation = strtok(NULL, search);
            errlogSevPrintf(errlogMajor, newExcitation);
        }
    }
    errlogSevPrintf(errlogMajor, "New Excitation");
    errlogSevPrintf(errlogMajor, newExcitation);
    return newExcitation;
}

static long calculateNewExcitationFromThresholds(aSubRecord *prec) {
    errlogSevPrintf(errlogMajor, "Before file open");
    FILE *thresholdsFile = fopen(prec->a, "r");
    if (thresholdsFile == NULL) {
        errlogSevPrintf(errlogMajor, "Failed to open thresholds file");
        return 1;
    }
    epicsInt32 tempSp = *(epicsInt32*)prec->c;
    errlogSevPrintf(errlogMajor, "Before read");
    char *newExcitation = getExcitation(thresholdsFile, tempSp);
    fclose(thresholdsFile);
    errlogSevPrintf(errlogMajor, "Before write to vala");
    if (newExcitation != NULL) {
        errlogSevPrintf(errlogMajor, newExcitation);
        *(epicsEnum16*)prec->vala = *(epicsEnum16*)newExcitation;
    } else {
        errlogSevPrintf(errlogMajor, "New excitation is NULL");
    }
    
    return 0;
}

epicsRegisterFunction(calculateNewExcitationFromThresholds);
