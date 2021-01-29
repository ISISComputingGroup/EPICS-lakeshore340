#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include <aSubRecord.h>
#include <errlog.h>

#include "lakeshore340excitations.h"

// Get the enum value from the given enumAsString as an int or -1 if no matching enum value
epicsEnum16 getEnumFromString(char * enumAsString) {
    int i;
    // Loop through string, value pairs
    for (i = 0; i < NUM_OF_EXCITATION_PAIRS; i++) {
        // Check if string matches and if so return the matching excitation int value
        excitationStringValuePair pair = excitation_string_to_val_map[i];
        if (strcmp(enumAsString, pair.excitationString) == 0) {
            return pair.excitationValue;
        }
    }
    // No matching string found so return error
    return -1;
}

// Check whether the given pair is valid
bool tempExcitationPairValid(thresholdTempExcitationPair pair) {
    return pair.excitation >= 0 && pair.excitation < NUM_OF_EXCITATION_PAIRS && pair.temp > DBL_MIN;
}

// Build a thresholdTempExcitationPair from the line e.g. "30,10 uA"
thresholdTempExcitationPair getThresholdTempExcitationPairFromLine(char *line) {
    // Duplicate line to avoid affecting line char array
    char *lineDup = strdup(line);
    // Get pair values from line
    char *tempString = strtok(lineDup, ",");
    epicsFloat64 tempThreshold;
    if (tempString != NULL) {
        tempThreshold = atof(tempString);
    } else {
        // Set to invalid value
        tempThreshold = DBL_MIN;
    }
    char *newExcitationString = strtok(NULL, ",");
    epicsEnum16 excitation;
    if (newExcitationString != NULL) {
        // Strip any newline chars
        newExcitationString[strcspn(newExcitationString, "\r\n") ] = '\0';
        excitation = getEnumFromString(newExcitationString);
    } else {
        // Set to invalid value
        excitation = -1;
    }
    // Construct struct
    thresholdTempExcitationPair newPair;
    newPair.temp = tempThreshold;
    newPair.excitation = excitation;
    return newPair;
}

// Get the first threshold temperature and excitation where the threshold temperature is less than or equal to the tempSp
// Or if none match that condition the last threshold temperature and excitation in the file, or an invalid pair if there's nothing in the file
thresholdTempExcitationPair getLargestTempExcitationPairFromFileThatIsLessThanTempSp(FILE *thresholdsFile, epicsFloat64 tempSp) {
    // Initialise values
    thresholdTempExcitationPair lastTempExcitationPair = {DBL_MIN, -1};
    char line[256];
    // Loop through lines to find the first temp threshold that the setpoint is higher than, set the newExcitation accordingly
    while (fgets(line, sizeof(line), thresholdsFile) != NULL) {
        lastTempExcitationPair = getThresholdTempExcitationPairFromLine(line);
        // Return the first valid pair that is less than or equal to the temperature setpoint
        if (tempExcitationPairValid(lastTempExcitationPair) && tempSp < lastTempExcitationPair.temp) {
            return lastTempExcitationPair;
        }
    }
    // Return the last found pair or the original invalid pair
    return lastTempExcitationPair;
}

// Set the excitation and temperature threshold pvs from the given thresholdPair
long setThresholdPVs(aSubRecord *prec, thresholdTempExcitationPair thresholdPair) {
    char excitationBuffer[33];
    itoa(thresholdPair.excitation, excitationBuffer, 10);
    errlogSevPrintf(errlogMajor, excitationBuffer);
    char tempBuffer[33];
    gcvt(thresholdPair.temp, 10, tempBuffer);
    errlogSevPrintf(errlogMajor, tempBuffer);
    if (tempExcitationPairValid(thresholdPair)) {
        // Set excitation and temp PVs
        *(epicsEnum16*)prec->vala = thresholdPair.excitation;
        *(epicsFloat64*)prec->valb = thresholdPair.temp;
        char excitationABuffer[33];
        itoa(*(epicsEnum16*)prec->vala, excitationABuffer, 10);
        errlogSevPrintf(errlogMajor, excitationABuffer);
        char tempABuffer[33];
        gcvt(*(epicsFloat64*)prec->valb, 10, tempABuffer);
        errlogSevPrintf(errlogMajor, tempABuffer);
        // Set that the file has been found
        *(epicsEnum16*)prec->valc = (epicsEnum16)0;
        // Set that we should delay the change to wait for the temperature
        *(epicsEnum16*)prec->vald = (epicsEnum16)1;
        // Process output records
        return 0;
    } else {
        errlogSevPrintf(errlogMajor, "No matching excitation found");
        // Do not process output records
        return -1;
    }
}

bool containsInvalidLines(FILE *thresholdsFile) {
    // Initialise values
    thresholdTempExcitationPair lastTempExcitationPair = {DBL_MIN, -1}; 
    bool invalidLines = false;
    char line[256];
    // Loop through lines to find the first temp threshold that the setpoint is higher than, set the newExcitation accordingly
    while (fgets(line, sizeof(line), thresholdsFile) != NULL) {
        lastTempExcitationPair = getThresholdTempExcitationPairFromLine(line);
        // Return the first valid pair that is less than or equal to the temperature setpoint
        if (!tempExcitationPairValid(lastTempExcitationPair)) {
            errlogSevPrintf(errlogMajor, "Line Invalid:");
            errlogSevPrintf(errlogMajor, line);
            invalidLines = true;
        }
    }
    // Reset file read to start
    rewind(thresholdsFile);
    // Return the last found pair or the original invalid pair
    return invalidLines;
}

static long calculateNewExcitationFromThresholds(aSubRecord *prec) {
    // Open file and check it has opened
    FILE *thresholdsFile = fopen(prec->a, "r");
    if (strstr(prec->a, "None.txt") != NULL) {
        // File is set to None.txt so don't set or check any values
        return -1;
    }
    if (thresholdsFile == NULL) {
        errlogSevPrintf(errlogMajor, "Failed to open thresholds file");
        errlogSevPrintf(errlogMajor, prec->a);
        // Set pvs to old values otherwise weird values are set
        *(epicsEnum16*)prec->vala = *(epicsEnum16*)prec->c;
        *(epicsFloat64*)prec->valb = *(epicsFloat64*)prec->d;
        // Set Error to File Not Found
        *(epicsEnum16*)prec->valc = (epicsEnum16)1;
        // Set that we should not delay the change to wait for the temperature
        *(epicsEnum16*)prec->vald = (epicsEnum16)0;
        return 0;
    }
    if (containsInvalidLines(thresholdsFile)) {
        errlogSevPrintf(errlogMajor, "File contains invalid lines");
        errlogSevPrintf(errlogMajor, prec->a);
        // Set pvs to old values otherwise weird values are set
        *(epicsEnum16*)prec->vala = *(epicsEnum16*)prec->c;
        *(epicsFloat64*)prec->valb = *(epicsFloat64*)prec->d;
        // Set Error to File Invalid
        *(epicsEnum16*)prec->valc = (epicsEnum16)2;
        // Set that we should not delay the change to wait for the temperature
        *(epicsEnum16*)prec->vald = (epicsEnum16)0;
        return 0;
    }
    // Calculate excitations from temperature setpoint and file
    epicsFloat64 tempSp = *(epicsFloat64*)prec->b;
    char tempBuffer[33];
    gcvt(tempSp, 10, tempBuffer);
    errlogSevPrintf(errlogMajor, "TEMP SP");
    errlogSevPrintf(errlogMajor, tempBuffer);
    thresholdTempExcitationPair thresholdPair = getLargestTempExcitationPairFromFileThatIsLessThanTempSp(thresholdsFile, tempSp);
    fclose(thresholdsFile);
    // Set the thresholds excitation and temp pv if in range
    long returnValue = setThresholdPVs(prec, thresholdPair);
    return returnValue;
}

epicsRegisterFunction(calculateNewExcitationFromThresholds);
