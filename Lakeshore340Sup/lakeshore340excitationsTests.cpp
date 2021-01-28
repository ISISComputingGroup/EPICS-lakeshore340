#include "gtest/gtest.h"
#include "lakeshore340excitations.h"
#include <stdint.h>

namespace {

    TEST(LSK340ExctationTests, test_when_excitation_val_in_range_then_pair_is_valid){
        thresholdTempExcitationPair pair1 = {120.0, 0};
        ASSERT_TRUE(tempExcitationPairValid(pair1));
        thresholdTempExcitationPair pair2 = {120.0, 12};
        ASSERT_TRUE(tempExcitationPairValid(pair2));
    }

    TEST(LSK340ExctationTests, test_when_excitation_val_not_in_range_then_pair_is_valid){
        thresholdTempExcitationPair pair1 = {120.0, -1};
        ASSERT_FALSE(tempExcitationPairValid(pair1));
        thresholdTempExcitationPair pair2 = {120.0, 13};
        ASSERT_FALSE(tempExcitationPairValid(pair2));
    }

    TEST(LSK340ExctationTests, test_when_temp_greater_than_int_min_then_pair_is_valid){
        thresholdTempExcitationPair pair1 = {DBL_MIN+1, 3};
        ASSERT_TRUE(tempExcitationPairValid(pair1));
        thresholdTempExcitationPair pair2 = {FLT_MAX, 3};
        ASSERT_TRUE(tempExcitationPairValid(pair2));
    }

    TEST(LSK340ExctationTests, test_when_temp_is_int_min_then_pair_is_invalid){
        thresholdTempExcitationPair pair1 = {DBL_MIN, 3};
        ASSERT_FALSE(tempExcitationPairValid(pair1));
    }

    TEST(LSK340ExctationTests, test_when_excitation_string_valid_then_excitation_value_returned){
        int i;
        for(i = 0; i < NUM_OF_EXCITATION_PAIRS; i++) {
            excitationStringValuePair pair = excitation_string_to_val_map[i];
            ASSERT_EQ(getEnumFromString(pair.excitationString), pair.excitationValue);
        }
    }

    TEST(LSK340ExctationTests, test_when_excitation_string_not_valid_then_minus_1_returned){
        ASSERT_EQ(getEnumFromString("Invalid String"), -1);
    }

    TEST(LSK340ExctationTests, test_when_line_valid_with_newline_then_function_returns_valid_struct){
        char *line = "120.2,30 nA\r\n";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_EQ(pair.excitation, 1);
        ASSERT_EQ(pair.temp, 120.2);
        ASSERT_TRUE(tempExcitationPairValid(pair));
    }

    TEST(LSK340ExctationTests, test_when_line_without_newline_valid_then_function_returns_valid_struct){
        char *line = "120.2,30 nA";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_EQ(pair.excitation, 1);
        ASSERT_EQ(pair.temp, 120.2);
        ASSERT_TRUE(tempExcitationPairValid(pair));
    }

    TEST(LSK340ExctationTests, test_when_line_temp_not_valid_then_function_returns_invalid_struct){
        char *line = ",30 nA\r\n";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_FALSE(tempExcitationPairValid(pair));
    }

    TEST(LSK340ExctationTests, test_when_line_excitation_not_valid_then_function_returns_invalid_struct){
        char *line = "120.2,invalid\r\n";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_FALSE(tempExcitationPairValid(pair));
    }
}
