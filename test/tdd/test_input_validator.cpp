/**
 * @file    test_input_validator.cpp
 * @brief   input_validator의 TDD 테스트 (단위 테스트와 별개). SSR-018~020, SWR-I-001~004, I-007
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include <gtest/gtest.h>

extern "C"
{
#include "input_validator.h"
}

namespace
{

cl_raw_input_t AllValidRaw()
{
    cl_raw_input_t raw = {};
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        raw.field[i].present = 1;
        raw.field[i].value = 0;
    }
    return raw;
}

class InputValidatorTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        ASSERT_EQ(CL_OK, input_validator_init(&state_));
    }
    input_validator_state_t state_ = {};
    cl_validated_t out_ = {};
};

TEST_F(InputValidatorTest, NullStateIsRejected)
{
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_init(nullptr));
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_validate(nullptr, nullptr, 0U, &out_));
}

TEST_F(InputValidatorTest, NullOutIsRejected)
{
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_validate(&state_, nullptr, 0U, nullptr));
}

TEST_F(InputValidatorTest, NoValueEverReceivedIsInvalid)
{
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 0U, &out_));
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        EXPECT_EQ(CL_INVALID, out_.validity[i]);
        EXPECT_EQ(0, out_.value[i]);
    }
}

TEST_F(InputValidatorTest, ValidSnapshotIsValidWithValues)
{
    cl_raw_input_t raw = AllValidRaw();
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 100U, &out_));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
    EXPECT_EQ(CL_CRASH_CONFIRMED, out_.value[CL_IN_CRASH]);
    EXPECT_EQ(1, out_.value[CL_IN_APPROACH_LEFT]);
}

TEST_F(InputValidatorTest, MissingFieldIsInvalid)
{
    cl_raw_input_t raw = AllValidRaw();
    raw.field[CL_IN_IGNITION].present = 0;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_IGNITION]);
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
}

TEST_F(InputValidatorTest, PresentFlagOutsideZeroOneIsInvalid)
{
    cl_raw_input_t raw = AllValidRaw();
    raw.field[CL_IN_IGNITION].present = 2;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_IGNITION]);
}

TEST_F(InputValidatorTest, OutOfRangeValuesAreInvalid)
{
    cl_raw_input_t raw = AllValidRaw();
    raw.field[CL_IN_CRASH].value = 3;
    raw.field[CL_IN_APPROACH_LEFT].value = 2;
    raw.field[CL_IN_APPROACH_RIGHT].value = -1;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_CRASH]);
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_APPROACH_LEFT]);
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_APPROACH_RIGHT]);
}

TEST_F(InputValidatorTest, CrashAcceptsAllThreeValuesAndBooleanRejectsTwo)
{
    cl_raw_input_t raw = AllValidRaw();
    raw.field[CL_IN_CRASH].value = CL_CRASH_PENDING;
    raw.field[CL_IN_SENSOR_FAULT].value = 1;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_SENSOR_FAULT]);
}

TEST_F(InputValidatorTest, BadValueAfterValidStaysInvalidWithoutNewSnapshot)
{
    cl_raw_input_t good = AllValidRaw();
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &good, 0U, &out_));
    cl_raw_input_t bad = AllValidRaw();
    bad.field[CL_IN_IGNITION].value = 7;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &bad, 10U, &out_));
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 20U, &out_));
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_IGNITION]);
}

TEST_F(InputValidatorTest, NextValidValueRecoversFromInvalid)
{
    cl_raw_input_t bad = AllValidRaw();
    bad.field[CL_IN_IGNITION].value = 7;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &bad, 0U, &out_));
    cl_raw_input_t good = AllValidRaw();
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &good, 10U, &out_));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_IGNITION]);
}

TEST_F(InputValidatorTest, AgeBoundaryAt200And201Milliseconds)
{
    cl_raw_input_t raw = AllValidRaw();
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 1000U, &out_));
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 1199U, &out_));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 1200U, &out_));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 1201U, &out_));
    EXPECT_EQ(CL_STALE, out_.validity[CL_IN_CRASH]);
    EXPECT_EQ(0, out_.value[CL_IN_CRASH]);
}

TEST_F(InputValidatorTest, ClockGoingBackwardIsNotStale)
{
    cl_raw_input_t raw = AllValidRaw();
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 5000U, &out_));
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 100U, &out_));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
}

TEST_F(InputValidatorTest, AgeUsesReceiptTimeOfLastValidUpdate)
{
    cl_raw_input_t raw = AllValidRaw();
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 150U, &out_));
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 340U, &out_));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 351U, &out_));
    EXPECT_EQ(CL_STALE, out_.validity[CL_IN_CRASH]);
}

TEST_F(InputValidatorTest, NoRequestWhenNotPresent)
{
    cl_raw_input_t raw = AllValidRaw();
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    EXPECT_EQ(0, out_.request_valid);
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 10U, &out_));
    EXPECT_EQ(0, out_.request_valid);
}

TEST_F(InputValidatorTest, ValidRequestIsDecoded)
{
    cl_raw_input_t raw = AllValidRaw();
    raw.request_present = 1;
    raw.request_side = CL_SIDE_ALL;
    raw.request_action = CL_ACTION_UNLOCK;
    raw.request_source = 3;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    EXPECT_EQ(1, out_.request_valid);
    EXPECT_EQ(CL_SIDE_ALL, out_.request_side);
    EXPECT_EQ(CL_ACTION_UNLOCK, out_.request_action);
}

TEST_F(InputValidatorTest, UnregisteredRequestEnumsAreRejected)
{
    cl_raw_input_t raw = AllValidRaw();
    raw.request_present = 1;
    raw.request_side = 0;
    raw.request_action = 1;
    raw.request_source = 0;
    int32_t bad_side = 3;
    int32_t bad_action = 2;
    int32_t bad_source = 4;
    raw.request_side = bad_side;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 0U, &out_));
    EXPECT_EQ(0, out_.request_valid);
    raw.request_side = 0;
    raw.request_action = bad_action;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 10U, &out_));
    EXPECT_EQ(0, out_.request_valid);
    raw.request_action = 1;
    raw.request_source = bad_source;
    ASSERT_EQ(CL_OK, input_validator_validate(&state_, &raw, 20U, &out_));
    EXPECT_EQ(0, out_.request_valid);
}

} // namespace
