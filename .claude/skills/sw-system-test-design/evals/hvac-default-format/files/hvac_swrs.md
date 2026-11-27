# HVAC 제어 SW 요구사항 명세 (SwRS) v1.2

ECU: FATC (Full Auto Temperature Control), ASIL: QM (단, SWRS-HVAC-007 은 ASIL B)

| ID | 요구사항 |
|---|---|
| SWRS-HVAC-001 | SW는 CAN 신호 `HVAC_TgtTempReq`(0x3A0, 해상도 0.5 ℃)로 수신한 목표 온도가 16.0 ℃ 이상 32.0 ℃ 이하이면 이를 목표 온도로 설정하고, 300 ms 이내에 `HVAC_TgtTempSts`(0x3A1)로 송신해야 한다. |
| SWRS-HVAC-002 | 수신한 목표 온도가 16.0 ℃ 미만 또는 32.0 ℃ 초과이면 SW는 요청을 무시하고 직전 목표 온도를 유지해야 한다. |
| SWRS-HVAC-003 | 블로워 단수 요청 `HVAC_FanLvlReq`는 0~7 단계이며, 0은 블로워 OFF이다. 8 이상 값은 무시한다. |
| SWRS-HVAC-004 | 운전 모드는 Auto, Manual, Defrost 이다. Defrost 모드에서는 A/C 를 강제로 ON 하고 블로워 단수를 7로 설정해야 한다. |
| SWRS-HVAC-005 | IGN 상태가 ACC 일 때 블로워 단수는 최대 2단으로 제한된다(요청이 2단 초과면 2단으로 동작). IGN ON 에서는 제한이 없다. |
| SWRS-HVAC-006 | SW는 `HVAC_TgtTempSts` 를 100 ms ± 10 ms 주기로 송신해야 한다. |
| SWRS-HVAC-007 | 실내 온도 센서 값이 -40 ℃ ~ 85 ℃ 범위를 벗어나거나 1000 ms 이상 갱신되지 않으면, SW는 200 ms 이내에 센서 고장 DTC(B1A2001)를 설정하고 기본 실내 온도 22.0 ℃ 로 제어를 계속해야 한다. |
| SWRS-HVAC-008 | 전원 인가 후 SW 초기화는 500 ms 이내에 완료되어야 하며, 초기 목표 온도는 NVM 에 저장된 마지막 값이다. NVM 값이 유효하지 않으면 22.0 ℃ 를 사용한다. |
