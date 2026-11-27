# PICT 사용법 (시험 조합 생성)

PICT(Pairwise Independent Combinatorial Testing)는 입력 매개변수 조합을 pairwise로 줄여 준다.
설치: https://github.com/microsoft/pict (Windows 릴리스 바이너리 `pict.exe`, Linux/macOS는 소스 빌드).

## 모델 파일 예 (`sensor.pict`)

```
# 매개변수: 값 목록 (동치 클래스 대표값 + 경계값)
Ready:   true, false
Raw:     0, 1, 4094, 4095, 4096
Mode:    NORMAL, CALIB, FAULT
Unit:    mV, mA

# 제약: 불가능한 조합 제외
IF [Mode] = "FAULT" THEN [Ready] = "false";
```

## 실행

```bash
pict sensor.pict > sensor_cases.tsv          # 기본 pairwise (2-way)
pict sensor.pict /o:3 > sensor_cases_3.tsv   # 3-way 조합이 필요할 때
```

## 사용 규칙
- 매개변수 값은 **동치 분할의 대표값과 경계값(min-1, min, max, max+1 등)** 으로 정한다.
- 불가능하거나 의미 없는 조합은 제약(`IF ... THEN`)으로 제외한다.
- 생성된 각 행은 시험 케이스 하나다. **기대 결과(예상 출력)는 행마다 설계 문서에서 사람이 판단해 적는다.** PICT는 입력만 만든다.
- 모델 파일(`*.pict`)은 시험 코드와 같이 `test/model/`에 커밋해 재현 가능하게 한다.
- 입력이 하나뿐이거나 조합이 의미 없으면 PICT 대신 경계값/동치 분할만 쓴다.
