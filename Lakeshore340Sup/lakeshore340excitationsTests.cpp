#include "gtest/gtest.h"
#include "lakeshore340excitations.h"
#include <stdint.h>

namespace {

    TEST(LSK340ExctationTests, test_when_excitation_val_in_range_then_pair_is_valid){
        thresholdTempExcitationPair pair1 = {120, 0};
        ASSERT_TRUE(tempExcitationPairValid(pair1));
        thresholdTempExcitationPair pair2 = {120, 12};
        ASSERT_TRUE(tempExcitationPairValid(pair2));
    }

    TEST(LSK340ExctationTests, test_when_excitation_val_not_in_range_then_pair_is_valid){
        thresholdTempExcitationPair pair1 = {120, -1};
        ASSERT_FALSE(tempExcitationPairValid(pair1));
        thresholdTempExcitationPair pair2 = {120, 13};
        ASSERT_FALSE(tempExcitationPairValid(pair2));
    }

    TEST(LSK340ExctationTests, test_when_temp_greater_than_int_min_then_pair_is_valid){
        thresholdTempExcitationPair pair1 = {INT_MIN+1, 3};
        ASSERT_TRUE(tempExcitationPairValid(pair1));
        thresholdTempExcitationPair pair2 = {INT_MAX, 3};
        ASSERT_TRUE(tempExcitationPairValid(pair2));
    }

    TEST(LSK340ExctationTests, test_when_temp_is_int_min_then_pair_is_invalid){
        thresholdTempExcitationPair pair1 = {INT_MIN, 3};
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
        char *line = "120,30 nA\r\n";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_EQ(pair.excitation, 1);
        ASSERT_EQ(pair.temp, 120);
        ASSERT_TRUE(tempExcitationPairValid(pair));
    }

    TEST(LSK340ExctationTests, test_when_line_without_newline_valid_then_function_returns_valid_struct){
        char *line = "120,30 nA";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_EQ(pair.excitation, 1);
        ASSERT_EQ(pair.temp, 120);
        ASSERT_TRUE(tempExcitationPairValid(pair));
    }

    TEST(LSK340ExctationTests, test_when_line_temp_not_valid_then_function_returns_invalid_struct){
        char *line = ",30 nA\r\n";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_FALSE(tempExcitationPairValid(pair));
    }

    TEST(LSK340ExctationTests, test_when_line_excitation_not_valid_then_function_returns_invalid_struct){
        char *line = "120,invalid\r\n";
        thresholdTempExcitationPair pair = getThresholdTempExcitationPairFromLine(line);
        ASSERT_FALSE(tempExcitationPairValid(pair));
    }

    TEST(LSK340ExctationTests, test_when_line_is_valid_and_contains_temp_less_than_temp_sp_and_greater_than_old_temp_then_line_pair_returned) {
        char *line = "120,30 nA";
        int tempSp = 125;
        thresholdTempExcitationPair oldPair = {118, 3};
        thresholdTempExcitationPair newPair = getExcitationPairIfConditionsMatch(line, tempSp, oldPair);
        ASSERT_EQ(newPair.excitation, 1);
        ASSERT_EQ(newPair.temp, 120);
    }

    TEST(LSK340ExctationTests, test_when_line_is_valid_and_contains_temp_less_than_temp_sp_and_less_than_old_temp_then_old_pair_returned) {
        char *line = "120,30 nA";
        int tempSp = 125;
        thresholdTempExcitationPair oldPair = {122, 3};
        thresholdTempExcitationPair newPair = getExcitationPairIfConditionsMatch(line, tempSp, oldPair);
        ASSERT_EQ(newPair.excitation, 3);
        ASSERT_EQ(newPair.temp, 122);
    }

    TEST(LSK340ExctationTests, test_when_line_is_valid_and_contains_temp_equal_to_temp_sp_and_greater_than_old_temp_then_line_pair_returned) {
        char *line = "125,30 nA";
        int tempSp = 125;
        thresholdTempExcitationPair oldPair = {118, 3};
        thresholdTempExcitationPair newPair = getExcitationPairIfConditionsMatch(line, tempSp, oldPair);
        ASSERT_EQ(newPair.excitation, 1);
        ASSERT_EQ(newPair.temp, 125);
    }

    TEST(LSK340ExctationTests, test_when_line_is_not_valid_and_contains_temp_less_than_temp_sp_and_greater_than_old_temp_then_old_pair_returned) {
        char *line = "120,invalid";
        int tempSp = 125;
        thresholdTempExcitationPair oldPair = {118, 3};
        thresholdTempExcitationPair newPair = getExcitationPairIfConditionsMatch(line, tempSp, oldPair);
        ASSERT_EQ(newPair.excitation, 3);
        ASSERT_EQ(newPair.temp, 118);
    }

    TEST(LSK340ExctationTests, test_when_line_is_valid_and_contains_temp_greater_than_temp_sp_and_greater_than_old_temp_then_old_pair_returned) {
        char *line = "126,30 nA";
        int tempSp = 125;
        thresholdTempExcitationPair oldPair = {118, 3};
        thresholdTempExcitationPair newPair = getExcitationPairIfConditionsMatch(line, tempSp, oldPair);
        ASSERT_EQ(newPair.excitation, 3);
        ASSERT_EQ(newPair.temp, 118);
    }
}
