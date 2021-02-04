#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <registryFunction.h>
#include <aSubRecord.h>
#include <errlog.h>

#include <epicsExport.h>

#include "lakeshore340excitations.h"

// Get the enum value from the given enumAsString as an int or -1 if no matching enum value
epicsEnum16 getEnumFromString(const char * enumAsString) {
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
int tempExcitationPairValid(thresholdTempExcitationPair pair) {
    return pair.excitation >= 0 && pair.excitation < NUM_OF_EXCITATION_PAIRS && pair.temp > DBL_MIN;
}

// Check if the line is too long i.e. does not contain a new line character
static int lineIsTooLong(const char *line) {
    int doesNotContainCarriageReturn = strchr(line, '\r') == NULL;
    int doesNotContainNewLine = strchr(line, '\n') == NULL;
    return doesNotContainCarriageReturn && doesNotContainNewLine;
}

thresholdTempExcitationPair getInvalidThresholdTempExcitationPair(void) {
    thresholdTempExcitationPair invalidPair = {DBL_MIN, -1};
    return invalidPair;
}

// Build a thresholdTempExcitationPair from the line e.g. "30,10 uA"
thresholdTempExcitationPair getThresholdTempExcitationPairFromLine(const char *line) {
    if (lineIsTooLong(line)) {
        errlogSevPrintf(errlogMajor, "Line too long: ");
        errlogSevPrintf(errlogMajor, line);
        return getInvalidThresholdTempExcitationPair();
    }
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
    free(lineDup);
    return newPair;
}

// Get the first threshold temperature and excitation where the threshold temperature is less than or equal to the tempSp
// Or if none match that condition the last threshold temperature and excitation in the file, or an invalid pair if there's nothing in the file
thresholdTempExcitationPair getLargestTempExcitationPairFromFileThatIsLessThanTempSp(FILE *thresholdsFile, epicsFloat64 tempSp) {
    // Initialise values
    thresholdTempExcitationPair lastTempExcitationPair = getInvalidThresholdTempExcitationPair();
    char line[LINE_LENGTH];
    // Loop through lines to find the first temp threshold that the setpoint is higher than, set the newExcitation accordingly
    while (fgets(line, LINE_LENGTH, thresholdsFile) != NULL) {
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
static long setThresholdPVs(aSubRecord *prec, thresholdTempExcitationPair thresholdPair) {
    if (tempExcitationPairValid(thresholdPair)) {
        // Set excitation and temp PVs
        *(epicsEnum16*)prec->vala = thresholdPair.excitation;
        *(epicsFloat64*)prec->valb = thresholdPair.temp;
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

static int containsInvalidLines(FILE *thresholdsFile) {
    // Initialise values
    thresholdTempExcitationPair lastTempExcitationPair = getInvalidThresholdTempExcitationPair(); 
    int invalidLines = 0;
    char line[LINE_LENGTH];
    // Loop through lines to find the first temp threshold that the setpoint is higher than, set the newExcitation accordingly
    while (fgets(line, LINE_LENGTH, thresholdsFile) != NULL) {
        lastTempExcitationPair = getThresholdTempExcitationPairFromLine(line);
        // Return the first valid pair that is less than or equal to the temperature setpoint
        if (!tempExcitationPairValid(lastTempExcitationPair)) {
            errlogSevPrintf(errlogMajor, "Line Invalid:");
            errlogSevPrintf(errlogMajor, line);
            invalidLines = 1;
        }
    }
    // Reset file read to start
    rewind(thresholdsFile);
    // Return the last found pair or the original invalid pair
    return invalidLines;
}

static void setThresholdsToOldValuesAndSetError(aSubRecord *prec, epicsEnum16 error) {
    // Set New values to old values
    *(epicsEnum16*)prec->vala = *(epicsEnum16*)prec->c;
    *(epicsFloat64*)prec->valb = *(epicsFloat64*)prec->d;
    // Set Error 
    *(epicsEnum16*)prec->valc = error;
    // Set that we should not delay the change to wait for the temperature
    *(epicsEnum16*)prec->vald = (epicsEnum16)0;
}

static long calculateNewExcitationFromThresholds(aSubRecord *prec) {
    // Check if we are set to use excitation thresholds
    const int notUsingExcitationsFile = *(epicsEnum16*)prec->e == 0;
    if (notUsingExcitationsFile) {
        // We are not set to use excitation thresholds, set no error
        setThresholdsToOldValuesAndSetError(prec, (epicsEnum16)0);
        return 0;
    }
    // Open file and check it has opened
    FILE *thresholdsFile = fopen(prec->a, "r");
    if (thresholdsFile == NULL) {
        errlogSevPrintf(errlogMajor, "Failed to open thresholds file");
        errlogSevPrintf(errlogMajor, prec->a);
        // Set Error to File Not Found
        setThresholdsToOldValuesAndSetError(prec, (epicsEnum16)1);
        return 0;
    }
    if (containsInvalidLines(thresholdsFile)) {
        fclose(thresholdsFile);
        errlogSevPrintf(errlogMajor, "File contains invalid lines");
        errlogSevPrintf(errlogMajor, prec->a);
        // Set Error to File Invalid
        setThresholdsToOldValuesAndSetError(prec, (epicsEnum16)2);
        return 0;
    }
    // Calculate excitations from temperature setpoint and file
    epicsFloat64 tempSp = *(epicsFloat64*)prec->b;
    thresholdTempExcitationPair thresholdPair = getLargestTempExcitationPairFromFileThatIsLessThanTempSp(thresholdsFile, tempSp);
    fclose(thresholdsFile);
    // Set the thresholds excitation and temp pv if in range
    long returnValue = setThresholdPVs(prec, thresholdPair);
    return returnValue;
}

epicsRegisterFunction(calculateNewExcitationFromThresholds);
