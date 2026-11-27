/**
 * @file    unit_helpers.hpp
 * @brief   단위 테스트 공용 도우미. 제품 코드는 포함하지 않고 입력 구조체 생성만 돕는다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef UNIT_HELPERS_HPP
#define UNIT_HELPERS_HPP

#include <gtest/gtest.h>

extern "C"
{
#include "cl_types.h"
}

namespace unit
{

/** 모든 입력이 VALID이고 값은 0(점화만 1=ON)인 검증 결과. */
inline cl_validated_t MakeValidated()
{
    cl_validated_t in = {};
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        in.validity[i] = CL_VALID;
        in.value[i] = 0;
    }
    in.value[CL_IN_IGNITION] = 1;
    return in;
}

/** 필수 입력이 모두 있고 값은 0(점화만 1=ON)인 원시 스냅샷. 운전자 요청은 없다. */
inline cl_raw_input_t MakeRaw()
{
    cl_raw_input_t raw = {};
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        raw.field[i].present = 1;
        raw.field[i].value = 0;
    }
    raw.field[CL_IN_IGNITION].value = 1;
    return raw;
}

} // namespace unit

#endif /* UNIT_HELPERS_HPP */
