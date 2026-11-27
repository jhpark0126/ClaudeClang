# build_workbook.py 입력 JSON 형식

```json
{
  "project": "HVAC 제어 SW",
  "basis": [
    {
      "id": "SWRS-010",
      "title": "목표 온도 입력 처리",
      "text": "목표 온도 16.0~32.0 ℃ (0.5 ℃ 단위)를 수용한다. 범위 밖은 무시하고 이전 값을 유지한다.",
      "conditions": [
        {"id": "SWRS-010-C1", "text": "범위 내 값 수용"},
        {"id": "SWRS-010-C2", "text": "범위 밖 값 무시 및 이전 값 유지"}
      ]
    }
  ],
  "test_cases": [
    {
      "id": "STC-F-001",
      "category": "기능",
      "basis_ids": ["SWRS-010"],
      "condition_ids": ["SWRS-010-C1"],
      "purpose": "목표 온도 하한 경계값 16.0 ℃가 수용되는지 확인",
      "preconditions": "IGN ON, HVAC Auto 모드, 현재 목표 온도 24.0 ℃",
      "inputs": "HVAC_TargetTemp = 16.0 ℃",
      "procedure": ["HVAC_TargetTemp 신호에 16.0 ℃ 송신", "200 ms 대기", "HVAC_TargetTempSts 신호 측정"],
      "expected": "HVAC_TargetTempSts = 16.0 ℃ (200 ms 이내 반영)",
      "techniques": ["동치 클래스 분할", "경계값 분석"],
      "suggested": false,
      "environment": "HIL",
      "notes": ""
    }
  ],
  "pict_models": [
    {
      "name": "hvac_model",
      "basis_ids": ["SWRS-020"],
      "model": "Mode: Auto, Manual\nFanLevel: 0, 1, 7\nAC: On, Off",
      "output": "Mode\tFanLevel\tAC\nAuto\t0\tOn\n..."
    }
  ]
}
```

## 필드

`basis[]`
- `id` (필수), `title`, `text`
- `conditions[]` (선택): `id`, `text`. 있으면 각 조건이 커버리지 분모에 포함된다.

`test_cases[]`
- 필수: `id`, `category`(`기능` 또는 `비기능`), `basis_ids`(1개 이상), `purpose`, `inputs`, `procedure`(문자열 또는 문자열 배열), `expected`, `techniques`(1개 이상)
- 선택: `condition_ids`, `preconditions`, `suggested`(bool), `environment`, `notes`
- `condition_ids`의 조건은 `basis_ids` 중 하나에 속해야 한다.

`pict_models[]` (선택): `name`, `basis_ids`, `model`(PICT 모델 텍스트), `output`(PICT 결과 TSV)

## 양식(템플릿) 헤더 매핑

`--template`을 주면 첫 시트(또는 `--template-sheet`)의 헤더 행을 찾아 아래 이름과 매칭한다 (공백/대소문자 무시, 부분 일치 포함).

| 필드 | 인식하는 헤더 예 |
|---|---|
| id | 테스트 케이스 ID, TC ID, Test Case ID |
| basis_ids | 테스트 베이시스 ID, 요구사항 ID, Req ID, Requirement ID |
| purpose | 테스트 목적, 목적, Objective, Purpose |
| preconditions | 사전조건, 전제조건, Precondition |
| inputs | 입력값, 입력, Input |
| procedure | 절차, 테스트 절차, 수행 절차, Steps, Procedure |
| expected | 예상 출력값, 기대 결과, 예상 결과, Expected |
| techniques | 테스트 기법, 사용한 테스트 기법, Technique |
| suggested | 제안, 구분, Suggested |
| environment | 테스트 환경, 환경, Environment |
| notes | 비고, Remarks, Notes |
| category | 분류, 기능/비기능, Category |
| condition_ids | 조건 ID, Condition ID |

매핑되지 않은 헤더는 빈칸으로 두고 스크립트가 목록을 출력한다.
