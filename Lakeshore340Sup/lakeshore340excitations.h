#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char *excitationString;
  int excitationValue;
} excitationStringValuePair;

// excitation mbbi string and int value pairs
static excitationStringValuePair excitation_string_to_val_map[] = {
    {"Off", 0}, {"30 nA", 1}, {"100 nA", 2}, {"300 nA", 3}, {"1 uA", 4},
    {"3 uA", 5}, {"10 uA", 6}, {"30 uA", 7}, {"100 uA", 8}, {"300 uA", 9},
    {"1 mA", 10}, {"10 mV", 11}, {"1 mV", 12}
};

#define NUM_OF_EXCITATION_PAIRS (sizeof(excitation_string_to_val_map)/sizeof(excitationStringValuePair))

typedef struct {
    int temp;
    int excitation;
} thresholdTempExcitationPair;

bool tempExcitationPairValid(thresholdTempExcitationPair pair);
thresholdTempExcitationPair getThresholdTempExcitationPairFromLine(char *line);
int getEnumFromString(char * enumAsString);

#ifdef __cplusplus
}
#endif
