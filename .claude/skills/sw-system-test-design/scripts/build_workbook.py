"""Validate system test cases against the test basis and write an Excel workbook.

Usage:
    python build_workbook.py CASES.json -o OUT.xlsx [--template TEMPLATE.xlsx] [--template-sheet NAME]

Errors (coverage < 100 %, broken traceability, duplicate IDs, missing fields,
equivalence partitioning / boundary value analysis unused) stop the build with
exit code 1. Warnings are printed but the workbook is still written.
See ../references/json_schema.md for the input format.
"""

import argparse
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Alignment, Border, Font, PatternFill, Side
from openpyxl.utils import get_column_letter

CATEGORIES = {"기능": "기능 테스트", "비기능": "비기능 테스트"}
REQUIRED = ["id", "category", "basis_ids", "purpose", "inputs", "procedure", "expected", "techniques"]
ECP = "동치 클래스 분할"
BVA = "경계값 분석"
WOW = "WoW 검증"
PAIRWISE = "페어와이즈(PICT)"

DEFAULT_COLUMNS = [
    ("id", "테스트 케이스 ID", 16),
    ("basis_ids", "테스트 베이시스 ID", 18),
    ("purpose", "테스트 목적", 36),
    ("preconditions", "사전조건", 28),
    ("inputs", "입력값", 30),
    ("procedure", "절차", 44),
    ("expected", "예상 출력값", 40),
    ("techniques", "사용한 테스트 기법", 22),
    ("suggested", "제안", 8),
    ("notes", "비고", 28),
]

# header aliases for template mapping (normalized: lowercase, no spaces/punctuation)
ALIASES = {
    "id": ["테스트케이스id", "tcid", "testcaseid", "케이스id", "tc번호", "테스트케이스번호"],
    "basis_ids": ["테스트베이시스id", "베이시스id", "요구사항id", "reqid", "requirementid", "요구사항번호", "testbasisid"],
    "condition_ids": ["조건id", "conditionid", "테스트조건id"],
    "purpose": ["테스트목적", "목적", "objective", "purpose", "테스트설명", "description"],
    "preconditions": ["사전조건", "전제조건", "precondition", "preconditions"],
    "inputs": ["입력값", "입력", "input", "inputs", "testdata", "테스트데이터"],
    "procedure": ["절차", "테스트절차", "수행절차", "steps", "procedure", "teststeps"],
    "expected": ["예상출력값", "기대결과", "예상결과", "기대값", "expected", "expectedresult", "expectedoutput"],
    "techniques": ["사용한테스트기법", "테스트기법", "기법", "technique", "techniques", "testtechnique"],
    "suggested": ["제안", "구분", "suggested"],
    "environment": ["테스트환경", "환경", "environment", "testenvironment"],
    "notes": ["비고", "remarks", "notes", "note", "comment"],
    "category": ["분류", "기능비기능", "category", "테스트유형"],
}

VAGUE = ["정상적으로", "정상 동작", "적절히", "적절한", "올바르게", "올바른", "제대로", "문제없", "문제 없", "등등",
         "works", "correctly", "properly", "as expected"]

HEADER_FILL = PatternFill("solid", fgColor="1F4E78")
HEADER_FONT = Font(bold=True, color="FFFFFF")
SUGGEST_FILL = PatternFill("solid", fgColor="FFF2CC")
MISS_FILL = PatternFill("solid", fgColor="F8CBAD")
THIN = Side(style="thin", color="BFBFBF")
BORDER = Border(left=THIN, right=THIN, top=THIN, bottom=THIN)
WRAP = Alignment(wrap_text=True, vertical="top")


def norm(s) -> str:
    return re.sub(r"[\s_\-/().·:]+", "", str(s or "")).lower()


def as_list(v):
    if v is None:
        return []
    if isinstance(v, list):
        return [str(x).strip() for x in v if str(x).strip()]
    return [x.strip() for x in re.split(r"[,\n]", str(v)) if x.strip()]


def cell_value(tc, field):
    v = tc.get(field)
    if field == "procedure":
        steps = as_list(v) if isinstance(v, list) else None
        if steps:
            return "\n".join(f"{i}. {s}" for i, s in enumerate(steps, 1))
        return str(v or "")
    if field in ("basis_ids", "condition_ids", "techniques"):
        return ", ".join(as_list(v))
    if field == "suggested":
        return "제안" if v else ""
    return "" if v is None else str(v)


# ---------------------------------------------------------------- validation
def validate(data):
    errors, warnings = [], []
    basis = data.get("basis") or []
    cases = data.get("test_cases") or []
    if not basis:
        errors.append("basis 가 비어 있습니다.")
    if not cases:
        errors.append("test_cases 가 비어 있습니다.")

    basis_ids, cond_owner = [], {}
    for b in basis:
        bid = str(b.get("id", "")).strip()
        if not bid:
            errors.append(f"id 없는 베이시스 항목: {b}")
            continue
        if bid in basis_ids:
            errors.append(f"베이시스 ID 중복: {bid}")
        basis_ids.append(bid)
        for c in b.get("conditions") or []:
            cid = str(c.get("id", "")).strip()
            if not cid:
                errors.append(f"{bid}: id 없는 조건")
            elif cid in cond_owner:
                errors.append(f"조건 ID 중복: {cid}")
            else:
                cond_owner[cid] = bid
    basis_set = set(basis_ids)

    seen, dup_key = set(), defaultdict(list)
    for tc in cases:
        tid = str(tc.get("id", "")).strip() or "<id 없음>"
        for f in REQUIRED:
            if not cell_value(tc, f).strip():
                errors.append(f"{tid}: 필수 항목 누락 '{f}'")
        if tid in seen:
            errors.append(f"테스트 케이스 ID 중복: {tid}")
        seen.add(tid)
        if tc.get("category") not in CATEGORIES:
            errors.append(f"{tid}: category 는 '기능' 또는 '비기능' 이어야 합니다 (현재 {tc.get('category')!r})")
        tb = as_list(tc.get("basis_ids"))
        for b in tb:
            if b not in basis_set:
                errors.append(f"{tid}: 존재하지 않는 베이시스 ID 참조 '{b}'")
        for c in as_list(tc.get("condition_ids")):
            if c not in cond_owner:
                errors.append(f"{tid}: 존재하지 않는 조건 ID 참조 '{c}'")
            elif cond_owner[c] not in tb:
                errors.append(f"{tid}: 조건 '{c}' 의 베이시스 {cond_owner[c]} 가 basis_ids 에 없습니다")
        exp = str(tc.get("expected") or "")
        hits = [w for w in VAGUE if w in exp]
        if hits:
            warnings.append(f"{tid}: 예상 출력값이 모호할 수 있음 ({', '.join(hits)}) - 관찰 가능한 값/상태로 쓰세요")
        key = (norm(tc.get("preconditions")), norm(tc.get("inputs")), norm(cell_value(tc, "procedure")))
        dup_key[key].append(tid)
    for ids in dup_key.values():
        if len(ids) > 1:
            warnings.append(f"사전조건·입력·절차가 같은 중복 의심 케이스: {', '.join(ids)}")

    # coverage
    cov_basis = defaultdict(list)
    cov_cond = defaultdict(list)
    tech_by_basis = defaultdict(set)
    all_tech = set()
    for tc in cases:
        techs = as_list(tc.get("techniques"))
        all_tech.update(techs)
        for b in as_list(tc.get("basis_ids")):
            cov_basis[b].append(tc.get("id"))
            tech_by_basis[b].update(techs)
        for c in as_list(tc.get("condition_ids")):
            cov_cond[c].append(tc.get("id"))
    for b in basis_ids:
        if not cov_basis.get(b):
            errors.append(f"커버되지 않은 베이시스: {b}")
        elif not ({ECP, BVA} & tech_by_basis[b]):
            warnings.append(f"{b}: 동치 클래스 분할/경계값 분석 케이스가 없음 - 입력 도메인이 없다면 비고에 이유를 남기세요")
    for c in cond_owner:
        if not cov_cond.get(c):
            errors.append(f"커버되지 않은 조건: {c} ({cond_owner[c]})")

    if cases and ECP not in all_tech:
        errors.append(f"'{ECP}' 기법을 사용한 케이스가 없습니다")
    if cases and BVA not in all_tech:
        errors.append(f"'{BVA}' 기법을 사용한 케이스가 없습니다")
    if cases and WOW not in all_tech:
        warnings.append(f"'{WOW}' 케이스가 없습니다 - 엣지 상황 검토 여부를 확인하세요")
    if PAIRWISE in all_tech and not data.get("pict_models"):
        errors.append(f"'{PAIRWISE}' 케이스가 있지만 pict_models (PICT 모델/결과 증적)이 없습니다")
    for m in data.get("pict_models") or []:
        if not str(m.get("output") or "").strip():
            errors.append(f"PICT 모델 '{m.get('name')}' 에 output(PICT 결과)이 없습니다")

    stats = {
        "basis_total": len(basis_ids),
        "basis_covered": sum(1 for b in basis_ids if cov_basis.get(b)),
        "cond_total": len(cond_owner),
        "cond_covered": sum(1 for c in cond_owner if cov_cond.get(c)),
        "cov_basis": cov_basis,
        "cov_cond": cov_cond,
        "cond_owner": cond_owner,
        "basis_ids": basis_ids,
    }
    return errors, warnings, stats


# ---------------------------------------------------------------- writing
def style_header(ws, row, ncols):
    for c in range(1, ncols + 1):
        cell = ws.cell(row=row, column=c)
        cell.fill, cell.font, cell.border = HEADER_FILL, HEADER_FONT, BORDER
        cell.alignment = Alignment(wrap_text=True, vertical="center", horizontal="center")


def write_case_sheet_default(ws, cases):
    for i, (_, title, width) in enumerate(DEFAULT_COLUMNS, 1):
        ws.cell(row=1, column=i, value=title)
        ws.column_dimensions[get_column_letter(i)].width = width
    style_header(ws, 1, len(DEFAULT_COLUMNS))
    ws.freeze_panes = "B2"
    for r, tc in enumerate(cases, 2):
        for c, (field, _, _) in enumerate(DEFAULT_COLUMNS, 1):
            cell = ws.cell(row=r, column=c, value=cell_value(tc, field))
            cell.alignment, cell.border = WRAP, BORDER
            if tc.get("suggested"):
                cell.fill = SUGGEST_FILL
    if cases:
        ws.auto_filter.ref = f"A1:{get_column_letter(len(DEFAULT_COLUMNS))}{len(cases) + 1}"


def find_header_row(ws):
    for r in range(1, min(ws.max_row, 15) + 1):
        vals = [ws.cell(row=r, column=c).value for c in range(1, ws.max_column + 1)]
        if sum(1 for v in vals if v not in (None, "")) >= 3:
            return r
    return 1


def map_headers(ws, header_row):
    mapping, unmapped = {}, []
    used = set()
    for c in range(1, ws.max_column + 1):
        raw = ws.cell(row=header_row, column=c).value
        if raw in (None, ""):
            continue
        h = norm(raw)
        field = None
        for f, names in ALIASES.items():  # exact match first
            if f not in used and h in names:
                field = f
                break
        if not field:
            for f, names in sorted(ALIASES.items(), key=lambda kv: -max(len(n) for n in kv[1])):
                if f not in used and any(n in h for n in names if len(n) >= 2):
                    field = f
                    break
        if field:
            mapping[c] = field
            used.add(field)
        else:
            unmapped.append(str(raw))
    return mapping, unmapped


def write_case_sheet_template(ws, cases, header_row, mapping):
    from copy import copy
    style_row = header_row + 1
    styles = {c: ws.cell(row=style_row, column=c)._style for c in range(1, ws.max_column + 1)}
    for r in range(header_row + 1, ws.max_row + 1):  # clear sample rows
        for c in range(1, ws.max_column + 1):
            ws.cell(row=r, column=c).value = None
    for i, tc in enumerate(cases):
        r = header_row + 1 + i
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            cell._style = copy(styles[c])
            if c in mapping:
                cell.value = cell_value(tc, mapping[c])
                cell.alignment = WRAP
            if tc.get("suggested"):
                cell.fill = SUGGEST_FILL


def write_trace_sheet(ws, data, stats):
    titles = {b["id"]: b.get("title") or b.get("text", "") for b in data["basis"]}
    cond_text = {c["id"]: c.get("text", "") for b in data["basis"] for c in b.get("conditions") or []}
    headers = ["테스트 베이시스 ID", "조건 ID", "내용", "테스트 케이스 ID", "케이스 수", "커버 여부"]
    widths = [18, 18, 50, 50, 10, 10]
    for i, (h, w) in enumerate(zip(headers, widths), 1):
        ws.cell(row=1, column=i, value=h)
        ws.column_dimensions[get_column_letter(i)].width = w
    style_header(ws, 1, len(headers))
    ws.freeze_panes = "A2"
    r = 2
    for b in stats["basis_ids"]:
        rows = [(b, "", titles.get(b, ""), stats["cov_basis"].get(b, []))]
        rows += [(b, c, cond_text.get(c, ""), stats["cov_cond"].get(c, []))
                 for c, owner in stats["cond_owner"].items() if owner == b]
        for bid, cid, text, tcs in rows:
            vals = [bid, cid, text, ", ".join(tcs), len(tcs), "O" if tcs else "X"]
            for c, v in enumerate(vals, 1):
                cell = ws.cell(row=r, column=c, value=v)
                cell.alignment, cell.border = WRAP, BORDER
                if not tcs:
                    cell.fill = MISS_FILL
            r += 1


def write_summary_sheet(ws, data, stats):
    cases = data["test_cases"]
    tech_count = defaultdict(int)
    for tc in cases:
        for t in as_list(tc.get("techniques")):
            tech_count[t] += 1
    pct = lambda a, b: f"{(a / b * 100):.1f}%" if b else "-"
    rows = [
        ("프로젝트", data.get("project", "")),
        ("테스트 베이시스 수", stats["basis_total"]),
        ("베이시스 커버리지", f"{stats['basis_covered']}/{stats['basis_total']} ({pct(stats['basis_covered'], stats['basis_total'])})"),
        ("테스트 조건 수", stats["cond_total"]),
        ("조건 커버리지", f"{stats['cond_covered']}/{stats['cond_total']} ({pct(stats['cond_covered'], stats['cond_total'])})"),
        ("기능 테스트 케이스", sum(1 for t in cases if t.get("category") == "기능")),
        ("비기능 테스트 케이스", sum(1 for t in cases if t.get("category") == "비기능")),
        ("제안 케이스", sum(1 for t in cases if t.get("suggested"))),
        ("", ""),
        ("기법", "케이스 수"),
    ] + sorted(tech_count.items(), key=lambda kv: -kv[1])
    ws.column_dimensions["A"].width = 26
    ws.column_dimensions["B"].width = 30
    for r, (k, v) in enumerate(rows, 1):
        ws.cell(row=r, column=1, value=k).font = Font(bold=True)
        ws.cell(row=r, column=2, value=v)


def write_pict_sheet(ws, models):
    r = 1
    ws.column_dimensions["A"].width = 24
    for m in models:
        ws.cell(row=r, column=1, value=f"PICT 모델: {m.get('name', '')}").font = Font(bold=True, size=12)
        ws.cell(row=r + 1, column=1, value="베이시스")
        ws.cell(row=r + 1, column=2, value=", ".join(as_list(m.get("basis_ids"))))
        ws.cell(row=r + 2, column=1, value="모델")
        cell = ws.cell(row=r + 2, column=2, value=m.get("model", ""))
        cell.alignment = WRAP
        r += 4
        lines = [ln.split("\t") for ln in str(m.get("output", "")).splitlines() if ln.strip()]
        for i, parts in enumerate(lines):
            ws.cell(row=r, column=1, value="행" if i == 0 else i)
            for c, v in enumerate(parts, 2):
                ws.cell(row=r, column=c, value=v).border = BORDER
            if i == 0:
                style_header(ws, r, len(parts) + 1)
            r += 1
        r += 2


def main() -> int:
    for stream in (sys.stdout, sys.stderr):
        try:
            # captured output (agents, pipes) as UTF-8; interactive console keeps its codepage
            stream.reconfigure(encoding=None if stream.isatty() else "utf-8", errors="replace")
        except AttributeError:
            pass
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cases")
    ap.add_argument("-o", "--out", required=True)
    ap.add_argument("--template")
    ap.add_argument("--template-sheet")
    args = ap.parse_args()

    data = json.loads(Path(args.cases).read_text(encoding="utf-8-sig"))
    errors, warnings, stats = validate(data)
    for w in warnings:
        print(f"[경고] {w}")
    for e in errors:
        print(f"[오류] {e}")
    print(f"베이시스 커버리지 {stats['basis_covered']}/{stats['basis_total']}, "
          f"조건 커버리지 {stats['cond_covered']}/{stats['cond_total']}, "
          f"케이스 {len(data.get('test_cases') or [])}개, 오류 {len(errors)}, 경고 {len(warnings)}")
    if errors:
        print("오류가 있어 엑셀을 생성하지 않았습니다.")
        return 1

    groups = {cat: [t for t in data["test_cases"] if t.get("category") == cat] for cat in CATEGORIES}
    if args.template:
        wb = load_workbook(args.template)
        base = wb[args.template_sheet] if args.template_sheet else wb.worksheets[0]
        header_row = find_header_row(base)
        mapping, unmapped = map_headers(base, header_row)
        print(f"양식 헤더 행 {header_row}, 매핑: " + ", ".join(
            f"{base.cell(row=header_row, column=c).value}→{f}" for c, f in mapping.items()))
        if unmapped:
            print(f"[양식] 매핑되지 않은 열(빈칸으로 둠): {', '.join(unmapped)}")
        missing = [f for f in ("id", "basis_ids", "purpose", "inputs", "procedure", "expected", "techniques")
                   if f not in mapping.values()]
        if missing:
            print(f"[양식] 양식에 없는 필수 항목(엑셀에 표시되지 않음): {', '.join(missing)}")
        for cat, title in CATEGORIES.items():
            ws = wb.copy_worksheet(base)
            ws.title = title
            write_case_sheet_template(ws, groups[cat], header_row, mapping)
        wb.remove(base)
        order = [wb[t] for t in CATEGORIES.values()]
        wb._sheets = order + [s for s in wb._sheets if s not in order]
    else:
        wb = Workbook()
        wb.remove(wb.active)
        for cat, title in CATEGORIES.items():
            write_case_sheet_default(wb.create_sheet(title), groups[cat])

    write_trace_sheet(wb.create_sheet("추적성 매트릭스"), data, stats)
    write_summary_sheet(wb.create_sheet("커버리지 요약"), data, stats)
    if data.get("pict_models"):
        write_pict_sheet(wb.create_sheet("PICT 모델"), data["pict_models"])
    wb.save(args.out)
    print(f"생성 완료: {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
