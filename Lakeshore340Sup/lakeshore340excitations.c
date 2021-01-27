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
int getEnumFromString(char * enumAsString) {
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
        tempThreshold = DBL_MIN;
    }
    char *newExcitationString = strtok(NULL, ",");
    int excitation;
    if (newExcitationString != NULL) {
        // Strip any newline chars
        newExcitationString[strcspn(newExcitationString, "\r\n") ] = '\0';
        excitation = getEnumFromString(newExcitationString);
    } else {
        excitation = -1;
    }
    // Construct struct
    thresholdTempExcitationPair newPair;
    newPair.temp = tempThreshold;
    newPair.excitation = excitation;
    return newPair;
}

// If the value of tempSp is greater than or equal to the temperature threshold 
// from the line (part of the line before the comma) and the new threshold is greater than set in tempExcitationPair
// Then return a pair of the new temperature and excitation to use or return the old pair
thresholdTempExcitationPair getExcitationPairIfConditionsMatch(char *line, epicsFloat64 tempSp, thresholdTempExcitationPair tempExcitationPair) {
    thresholdTempExcitationPair newThresholdTempExcitationPair = getThresholdTempExcitationPairFromLine(line);
    errlogSevPrintf(errlogMajor, "Getting excitation from line");
    char excitationBuffer[33];
    itoa(newThresholdTempExcitationPair.excitation, excitationBuffer, 10);
    errlogSevPrintf(errlogMajor, excitationBuffer);
    char tempBuffer[33];
    gcvt(newThresholdTempExcitationPair.temp, 10, tempBuffer);
    errlogSevPrintf(errlogMajor, tempBuffer);
    char validBuffer[33];
    itoa(tempExcitationPairValid(newThresholdTempExcitationPair), validBuffer, 10);
    errlogSevPrintf(errlogMajor, validBuffer);
    char tempSpBuffer[33];
    gcvt(tempSp, 10, tempSpBuffer);
    errlogSevPrintf(errlogMajor, tempSpBuffer);
    if (tempExcitationPairValid(newThresholdTempExcitationPair) && tempSp >= newThresholdTempExcitationPair.temp && newThresholdTempExcitationPair.temp > tempExcitationPair.temp) {
        return newThresholdTempExcitationPair;
    }
    return tempExcitationPair;
}

// Get the excitation as an enum value or -1 if no excitation found
// Find the lowest temperature threshold listed in the file and 
// return an integer representing the matching excitation enum value or -1 if no matching threshold found
thresholdTempExcitationPair getLargestTempExcitationPairFromFileThatIsLessThanTempSp(FILE *thresholdsFile, epicsFloat64 tempSp) {
    // Initialise values
    thresholdTempExcitationPair newTempExcitationPair = {DBL_MIN, -1};
    char line[256];
    // Loop through lines to find the lowest temp threshold that the setpoint is higher than, set the newExcitation accordingly
    while (fgets(line, sizeof(line), thresholdsFile) != NULL) {
        newTempExcitationPair = getExcitationPairIfConditionsMatch(line, tempSp, newTempExcitationPair);
        errlogSevPrintf(errlogMajor, "Do Conditions match");
        char excitationBuffer[33];
        itoa(newTempExcitationPair.excitation, excitationBuffer, 10);
        errlogSevPrintf(errlogMajor, excitationBuffer);
        char tempBuffer[33];
        gcvt(newTempExcitationPair.temp, 10, tempBuffer);
        errlogSevPrintf(errlogMajor, tempBuffer);
    }
    return newTempExcitationPair;
}

// Set the excitation and temperature threshold pvs from the given thresholdPair
void setThresholdPVs(aSubRecord *prec, thresholdTempExcitationPair thresholdPair) {
    char excitationBuffer[33];
    itoa(thresholdPair.excitation, excitationBuffer, 10);
    errlogSevPrintf(errlogMajor, excitationBuffer);
    char tempBuffer[33];
    gcvt(thresholdPair.temp, 10, tempBuffer);
    errlogSevPrintf(errlogMajor, tempBuffer);
    if (tempExcitationPairValid(thresholdPair)) {
        *(epicsEnum16*)prec->vala = thresholdPair.excitation;
        *(epicsFloat64*)prec->valb = thresholdPair.temp;
    } else {
        errlogSevPrintf(errlogMajor, "No matching excitation found");
    }
}

static long calculateNewExcitationFromThresholds(aSubRecord *prec) {
    // Open file and check it has opened
    FILE *thresholdsFile = fopen(prec->a, "r");
    if (thresholdsFile == NULL) {
        errlogSevPrintf(errlogMajor, "Failed to open thresholds file");
        return 1;
    }
    // Calculate excitations from temperature setpoint and file
    epicsFloat64 tempSp = *(epicsFloat64*)prec->c;
    thresholdTempExcitationPair thresholdPair = getLargestTempExcitationPairFromFileThatIsLessThanTempSp(thresholdsFile, tempSp);
    fclose(thresholdsFile);
    // Set the thresholds excitation and temp pv if in range
    setThresholdPVs(prec, thresholdPair);
    return 0;
}

epicsRegisterFunction(calculateNewExcitationFromThresholds);
