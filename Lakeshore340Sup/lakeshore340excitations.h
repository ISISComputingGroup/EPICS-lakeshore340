#include <registryFunction.h>
#include <epicsExport.h>
#include <aSubRecord.h>
#include <errlog.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *excitationString;
  epicsEnum16 excitationValue;
} excitationStringValuePair;

// excitation mbbi string and enum value pairs
// If changing the values here also change the values in the Lakeshore340.db file
static excitationStringValuePair excitation_string_to_val_map[] = {
    {"Off", 0}, {"30 nA", 1}, {"100 nA", 2}, {"300 nA", 3}, {"1 uA", 4},
    {"3 uA", 5}, {"10 uA", 6}, {"30 uA", 7}, {"100 uA", 8}, {"300 uA", 9},
    {"1 mA", 10}, {"10 mV", 11}, {"1 mV", 12}
};

#define NUM_OF_EXCITATION_PAIRS (sizeof(excitation_string_to_val_map)/sizeof(excitationStringValuePair))

typedef struct {
    epicsFloat64 temp;
    epicsEnum16 excitation;
} thresholdTempExcitationPair;

bool tempExcitationPairValid(thresholdTempExcitationPair pair);
thresholdTempExcitationPair getThresholdTempExcitationPairFromLine(char *line);
epicsEnum16 getEnumFromString(char * enumAsString);
thresholdTempExcitationPair getExcitationPairIfConditionsMatch(char *line, epicsFloat64 tempSp, thresholdTempExcitationPair tempExcitationPair);
thresholdTempExcitationPair getLargestTempExcitationPairFromFileThatIsLessThanTempSp(FILE *thresholdsFile, epicsFloat64 tempSp);

#ifdef __cplusplus
}
#endif
