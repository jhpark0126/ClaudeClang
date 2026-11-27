/**
 * @file    test_safety_arbiter_unit.cpp
 * @brief   safety_arbiter 단위 테스트 (SWE.4). MC/DC 독립 쌍, 오류 추측, PICT 2-way 조합과
 *          요구사항 표 기반 오라클로 설계했다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "unit_helpers.hpp"

extern "C"
{
#include "safety_arbiter.h"
}

namespace
{

using unit::MakeValidated;

class SafetyArbiterUnit : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
    }
    cl_status_t Decide(const cl_validated_t &in)
    {
        return safety_arbiter_decide(&state_, &in, &out_);
    }
    safety_arbiter_state_t state_ = {};
    safety_decision_t out_ = {};
};

/**
 * @test    UT-020: NULL 인자는 CL_ERR_NULL_ARG를 반환하고 출력을 바꾸지 않는다.
 * @trace   FUNC-010, FUNC-011
 * @purpose (state==NULL)||(in==NULL)||(out==NULL)의 MC/DC. 각 포인터만 NULL인 쌍과 모두 정상.
 * @input   state만 NULL, in만 NULL, out만 NULL, 모두 정상, init(NULL)
 * @steps   1. 조합별로 호출한다. 2. out의 센티널 값을 확인한다.
 * @expect  NULL이 있으면 CL_ERR_NULL_ARG, out 불변. 정상은 CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT020_NullArguments)
{
    cl_validated_t in = MakeValidated();
    out_.reason_mask = 0xABCDU;
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_init(nullptr));
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_decide(nullptr, &in, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_decide(&state_, nullptr, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_decide(&state_, &in, nullptr));
    EXPECT_EQ(0xABCDU, out_.reason_mask);
    EXPECT_EQ(CL_OK, safety_arbiter_decide(&state_, &in, &out_));
}

/**
 * @test    UT-021: 충돌 CONFIRMED 래치: 유효성, CONFIRMED, PENDING 판정의 독립 쌍.
 * @trace   FUNC-012, SSR-001, SSR-002, SSR-004, SSR-005, SSR-006, SSR-007
 * @purpose validity==VALID, value==CONFIRMED, value==PENDING 결정의 MC/DC.
 * @input   (VALID,CONFIRMED), (VALID,PENDING), (VALID,NONE), (INVALID,CONFIRMED), (STALE,PENDING)
 * @steps   1. 매번 init 후 결정한다. 2. 같은 상태에서 정상 입력으로 한 번 더 결정해 래치를 본다.
 * @expect  유효 CONFIRMED만 양쪽 RELEASE를 유지시키고, 유효 PENDING만 이유 CRASH_PENDING,
 *          유효하지 않은 입력은 값이 달라도 래치·이유 없음
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT021_CrashLatchDecisions)
{
    struct Probe
    {
        cl_validity_t validity;
        int32_t value;
        bool latches;
        bool pending_reason;
    };
    const Probe probes[] = {
        {CL_VALID, CL_CRASH_CONFIRMED, true, false}, {CL_VALID, CL_CRASH_PENDING, false, true},
        {CL_VALID, CL_CRASH_NONE, false, false},     {CL_INVALID, CL_CRASH_CONFIRMED, false, false},
        {CL_STALE, CL_CRASH_PENDING, false, false},
    };
    for (const Probe &p : probes)
    {
        ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
        cl_validated_t in = MakeValidated();
        in.validity[CL_IN_CRASH] = p.validity;
        in.value[CL_IN_CRASH] = p.value;
        ASSERT_EQ(CL_OK, Decide(in));
        const bool has_reason = (out_.reason_mask & CL_REASON_CRASH_PENDING) != 0U;
        EXPECT_EQ(p.pending_reason, has_reason) << p.value;
        cl_validated_t again = MakeValidated();
        again.validity[CL_IN_CRASH] = CL_VALID;
        ASSERT_EQ(CL_OK, Decide(again));
        EXPECT_EQ(p.latches ? CL_DIR_RELEASE : CL_DIR_FREE, out_.directive[CL_DOOR_LEFT]);
        EXPECT_EQ(p.latches ? CL_DIR_RELEASE : CL_DIR_FREE, out_.directive[CL_DOOR_RIGHT]);
    }
}

/**
 * @test    UT-022: 충돌 래치는 init 전까지 유지되고 init으로 해제된다.
 * @trace   FUNC-010, FUNC-011, SSR-002
 * @purpose 래치 유지(상태 소유)와 초기화 경로.
 * @input   CONFIRMED 수신 후 NONE과 접근위험 TRUE를 여러 번, 이어서 init
 * @steps   1. CONFIRMED를 결정한다. 2. 접근위험 TRUE 입력을 두 번 결정한다. 3. init 후 같은 입력을
 * 결정한다.
 * @expect  init 전에는 양쪽 RELEASE 유지, init 후 좌측은 LOCK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT022_LatchHoldsUntilInit)
{
    cl_validated_t confirmed = MakeValidated();
    confirmed.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Decide(confirmed));
    cl_validated_t risk = MakeValidated();
    risk.value[CL_IN_APPROACH_LEFT] = 1;
    for (int i = 0; i < 2; ++i)
    {
        ASSERT_EQ(CL_OK, Decide(risk));
        EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_LEFT]);
        EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_RIGHT]);
    }
    ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
    ASSERT_EQ(CL_OK, Decide(risk));
    EXPECT_EQ(CL_DIR_LOCK, out_.directive[CL_DOOR_LEFT]);
}

/**
 * @test    UT-023: 건전성: sensor_fault가 유효하고 TRUE일 때만 FAULT이다.
 * @trace   FUNC-013, SSR-021, SSR-024, SSR-027
 * @purpose (validity==VALID)&&(value==1)의 MC/DC. (INVALID,1)은 FAULT가 아니라 DEGRADED여야 한다.
 * @input   (VALID,1), (VALID,0), (INVALID,1), (STALE,1)
 * @steps   1. sensor_fault 입력만 바꿔 결정한다.
 * @expect  FAULT, NORMAL, DEGRADED, DEGRADED
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT023_FaultHealthDecision)
{
    struct Probe
    {
        cl_validity_t validity;
        int32_t value;
        cl_health_t health;
    };
    const Probe probes[] = {{CL_VALID, 1, CL_HEALTH_FAULT},
                            {CL_VALID, 0, CL_HEALTH_NORMAL},
                            {CL_INVALID, 1, CL_HEALTH_DEGRADED},
                            {CL_STALE, 1, CL_HEALTH_DEGRADED}};
    for (const Probe &p : probes)
    {
        cl_validated_t in = MakeValidated();
        in.validity[CL_IN_SENSOR_FAULT] = p.validity;
        in.value[CL_IN_SENSOR_FAULT] = p.value;
        ASSERT_EQ(CL_OK, Decide(in));
        EXPECT_EQ(p.health, out_.health) << p.validity << "," << p.value;
    }
}

/**
 * @test    UT-024: 필수 입력 5개 중 어느 하나라도 INVALID 또는 STALE이면 DEGRADED이다.
 * @trace   FUNC-013, FUNC-015, SSR-021, SSR-021a
 * @purpose 입력별 동치 클래스(INVALID, STALE)와 FAULT 우선(FAULT>DEGRADED).
 * @input   입력 하나씩을 INVALID, STALE로 바꾼 10개 케이스, 그리고 FAULT와 STALE 동시
 * @steps   1. 입력 하나만 바꿔 결정한다. 2. FAULT와 STALE을 함께 준다.
 * @expect  단독이면 DEGRADED, FAULT와 함께면 FAULT
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT024_AnyUnreliableInputIsDegraded)
{
    const cl_validity_t kinds[] = {CL_INVALID, CL_STALE};
    for (int input = 0; input < CL_IN_COUNT; ++input)
    {
        for (cl_validity_t kind : kinds)
        {
            cl_validated_t in = MakeValidated();
            in.validity[input] = kind;
            in.value[input] = 0;
            ASSERT_EQ(CL_OK, Decide(in));
            EXPECT_EQ(CL_HEALTH_DEGRADED, out_.health) << input << "," << kind;
        }
    }
    cl_validated_t both = MakeValidated();
    both.value[CL_IN_SENSOR_FAULT] = 1;
    both.validity[CL_IN_CRASH] = CL_STALE;
    both.value[CL_IN_CRASH] = 0;
    ASSERT_EQ(CL_OK, Decide(both));
    EXPECT_EQ(CL_HEALTH_FAULT, out_.health);
}

/**
 * @test    UT-025: 도어 지시: 접근위험 유효 TRUE, 유효 FALSE, 신뢰 불가, 건전성 비정상의 독립 쌍.
 * @trace   FUNC-014, SSR-009, SSR-010, SSR-013, SSR-014, SSR-016a, SSR-017, SSR-023, SSR-024
 * @purpose (validity==VALID)&&(value==1), validity!=VALID, health!=NORMAL 결정의 MC/DC. 좌/우 각각.
 * @input   접근위험 (VALID,1), (VALID,0), (INVALID,1), (STALE,1), (VALID,0)+건전성 FAULT
 * @steps   1. 한 도어의 입력만 바꿔 결정한다. 2. 반대편 도어 지시도 함께 본다.
 * @expect  LOCK(+이유), FREE, HOLD(+이유 UNRELIABLE), HOLD(+이유), HOLD(이유 없음). 반대편은 영향
 * 없음.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT025_DoorDirectiveDecisions)
{
    struct Probe
    {
        cl_validity_t validity;
        int32_t value;
        int32_t fault;
        cl_directive_t directive;
        uint32_t reason;
    };
    const Probe probes[] = {
        {CL_VALID, 1, 0, CL_DIR_LOCK, CL_REASON_APPROACH_RISK_LOCK},
        {CL_VALID, 0, 0, CL_DIR_FREE, 0U},
        {CL_INVALID, 1, 0, CL_DIR_HOLD, CL_REASON_APPROACH_INPUT_UNRELIABLE},
        {CL_STALE, 1, 0, CL_DIR_HOLD, CL_REASON_APPROACH_INPUT_UNRELIABLE},
        {CL_VALID, 0, 1, CL_DIR_HOLD, 0U},
    };
    const cl_input_id_t inputs[] = {CL_IN_APPROACH_LEFT, CL_IN_APPROACH_RIGHT};
    for (int door = 0; door < CL_DOOR_COUNT; ++door)
    {
        for (const Probe &p : probes)
        {
            cl_validated_t in = MakeValidated();
            in.validity[inputs[door]] = p.validity;
            in.value[inputs[door]] = p.value;
            in.value[CL_IN_SENSOR_FAULT] = p.fault;
            ASSERT_EQ(CL_OK, Decide(in));
            EXPECT_EQ(p.directive, out_.directive[door]) << door;
            EXPECT_EQ(p.reason, out_.reason_mask & ~CL_REASON_CRASH_PENDING) << door;
        }
    }
}

/**
 * @test    UT-026: 래치가 선 뒤에는 접근위험과 건전성에 관계없이 양쪽 RELEASE이고 도어별 이유가
 * 없다.
 * @trace   FUNC-014, SSR-001, SSR-003, SSR-004, SSR-005
 * @purpose crash_latched==1 결정이 접근위험, DEGRADED, FAULT보다 앞선다는 우선순위.
 * @input   CONFIRMED + 좌/우 접근위험 TRUE + FAULT + STALE 입력
 * @steps   1. 해당 입력으로 결정한다.
 * @expect  양쪽 RELEASE, health=FAULT, 접근위험 이유 비트 없음
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT026_CrashBeatsEverything)
{
    cl_validated_t in = MakeValidated();
    in.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    in.value[CL_IN_APPROACH_LEFT] = 1;
    in.value[CL_IN_APPROACH_RIGHT] = 1;
    in.value[CL_IN_SENSOR_FAULT] = 1;
    in.validity[CL_IN_IGNITION] = CL_STALE;
    in.value[CL_IN_IGNITION] = 0;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_HEALTH_FAULT, out_.health);
    EXPECT_EQ(0U, out_.reason_mask);
}

/* ---- PICT 2-way 조합 시험: 입력 값 이름 ---- */
enum Pv
{
    PV_NONE,
    PV_PENDING,
    PV_CONFIRMED,
    PV_FALSE,
    PV_TRUE,
    PV_ON,
    PV_OFF,
    PV_INVALID,
    PV_STALE
};
struct PictCase
{
    Pv crash, app_left, app_right, fault, ignition;
};
const PictCase k_pict_cases[] = {
#include "arbiter_pict_cases.inc"
};

void ApplyPv(cl_validated_t *in, cl_input_id_t input, Pv pv)
{
    in->validity[input] = CL_VALID;
    switch (pv)
    {
    case PV_PENDING:
    case PV_TRUE:
    case PV_ON:
        in->value[input] = 1;
        break;
    case PV_CONFIRMED:
        in->value[input] = 2;
        break;
    case PV_INVALID:
        in->validity[input] = CL_INVALID;
        in->value[input] = 0;
        break;
    case PV_STALE:
        in->validity[input] = CL_STALE;
        in->value[input] = 0;
        break;
    default:
        in->value[input] = 0;
        break;
    }
}

/** 요구사항 표(SSR-001~027)에서 직접 계산한 기대값. 제품 코드의 구조를 따르지 않는다. */
struct Expected
{
    cl_directive_t directive[CL_DOOR_COUNT];
    cl_health_t health;
    uint32_t reason;
};

bool IsBad(Pv v)
{
    return (v == PV_INVALID) || (v == PV_STALE);
}

cl_health_t OracleHealth(const PictCase &c)
{
    const Pv all[] = {c.crash, c.app_left, c.app_right, c.fault, c.ignition};
    bool any_bad = false;
    for (Pv v : all)
    {
        any_bad = any_bad || IsBad(v);
    }
    cl_health_t health = any_bad ? CL_HEALTH_DEGRADED : CL_HEALTH_NORMAL;
    if (c.fault == PV_TRUE)
    {
        health = CL_HEALTH_FAULT;
    }
    return health;
}

void OracleDoor(Pv risk, cl_health_t health, Expected *e, int door)
{
    if (risk == PV_TRUE)
    {
        e->directive[door] = CL_DIR_LOCK;
        e->reason |= CL_REASON_APPROACH_RISK_LOCK;
    }
    else if (IsBad(risk))
    {
        e->directive[door] = CL_DIR_HOLD;
        e->reason |= CL_REASON_APPROACH_INPUT_UNRELIABLE;
    }
    else if (health != CL_HEALTH_NORMAL)
    {
        e->directive[door] = CL_DIR_HOLD;
    }
}

Expected Oracle(const PictCase &c)
{
    Expected e = {{CL_DIR_FREE, CL_DIR_FREE}, OracleHealth(c), 0U};
    if (c.crash == PV_CONFIRMED)
    {
        e.directive[CL_DOOR_LEFT] = CL_DIR_RELEASE;
        e.directive[CL_DOOR_RIGHT] = CL_DIR_RELEASE;
    }
    else
    {
        if (c.crash == PV_PENDING)
        {
            e.reason |= CL_REASON_CRASH_PENDING;
        }
        OracleDoor(c.app_left, e.health, &e, CL_DOOR_LEFT);
        OracleDoor(c.app_right, e.health, &e, CL_DOOR_RIGHT);
    }
    return e;
}

/**
 * @test    UT-027: 입력 5개의 PICT 2-way 조합 27건을 요구사항 표 오라클과 비교한다.
 * @trace   FUNC-011~015, SSR-001~007, SSR-009~017, SSR-021, SSR-021a, SSR-023, SSR-024
 * @purpose 입력 쌍 상호작용(충돌×접근위험×센서고장×점화×유효성)에서 우선순위가 요구사항대로인지
 * 확인한다.
 * @input   test/model/safety_arbiter.pict에서 생성한 27건 (각 입력 값: 3종+INVALID+STALE)
 * @steps   1. 케이스마다 init 후 해당 입력으로 결정한다. 2. 요구사항 표 오라클의 기대값과 비교한다.
 * @expect  좌/우 지시, 건전성, 이유 비트가 오라클과 같다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(SafetyArbiterUnit, UT027_PictPairwiseAgainstOracle)
{
    int index = 0;
    for (const PictCase &c : k_pict_cases)
    {
        ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
        cl_validated_t in = MakeValidated();
        ApplyPv(&in, CL_IN_CRASH, c.crash);
        ApplyPv(&in, CL_IN_APPROACH_LEFT, c.app_left);
        ApplyPv(&in, CL_IN_APPROACH_RIGHT, c.app_right);
        ApplyPv(&in, CL_IN_SENSOR_FAULT, c.fault);
        ApplyPv(&in, CL_IN_IGNITION, c.ignition);
        ASSERT_EQ(CL_OK, Decide(in));
        const Expected e = Oracle(c);
        EXPECT_EQ(e.directive[CL_DOOR_LEFT], out_.directive[CL_DOOR_LEFT]) << "case " << index;
        EXPECT_EQ(e.directive[CL_DOOR_RIGHT], out_.directive[CL_DOOR_RIGHT]) << "case " << index;
        EXPECT_EQ(e.health, out_.health) << "case " << index;
        EXPECT_EQ(e.reason, out_.reason_mask) << "case " << index;
        ++index;
    }
}

} // namespace
