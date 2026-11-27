"""Run Microsoft PICT (https://github.com/microsoft/pict) on a model file.

Usage:
    python pict_runner.py MODEL [--out FILE.tsv] [--json] [-- PICT_ARGS...]

PICT is located in this order: $PICT_PATH, `pict` on PATH, <skill>/bin/pict(.exe).
On Windows, if it is not found, the latest pict.exe is downloaded from the
GitHub releases into <skill>/bin/. On other platforms, build instructions are
printed instead.
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import urllib.request
from pathlib import Path

SKILL_DIR = Path(__file__).resolve().parent.parent
BIN_DIR = SKILL_DIR / "bin"
RELEASE_API = "https://api.github.com/repos/microsoft/pict/releases/latest"
FALLBACK_URL = "https://github.com/microsoft/pict/releases/download/v3.7.4/pict.exe"


def _download_windows_pict() -> Path:
    url = FALLBACK_URL
    try:
        req = urllib.request.Request(RELEASE_API, headers={"User-Agent": "pict-runner"})
        with urllib.request.urlopen(req, timeout=30) as resp:
            release = json.load(resp)
        for asset in release.get("assets", []):
            if asset.get("name", "").lower() == "pict.exe":
                url = asset["browser_download_url"]
                break
    except Exception as exc:  # network/API issues: fall back to pinned version
        print(f"[pict_runner] release lookup failed ({exc}); using {url}", file=sys.stderr)
    BIN_DIR.mkdir(parents=True, exist_ok=True)
    target = BIN_DIR / "pict.exe"
    print(f"[pict_runner] downloading {url}", file=sys.stderr)
    req = urllib.request.Request(url, headers={"User-Agent": "pict-runner"})
    with urllib.request.urlopen(req, timeout=120) as resp, open(target, "wb") as f:
        shutil.copyfileobj(resp, f)
    return target


def find_pict() -> Path:
    env = os.environ.get("PICT_PATH")
    if env and Path(env).is_file():
        return Path(env)
    on_path = shutil.which("pict")
    if on_path:
        return Path(on_path)
    for name in ("pict.exe", "pict"):
        candidate = BIN_DIR / name
        if candidate.is_file():
            return candidate
    if os.name == "nt":
        return _download_windows_pict()
    sys.exit(
        "[pict_runner] PICT not found. Build it from https://github.com/microsoft/pict:\n"
        "  git clone https://github.com/microsoft/pict.git && cd pict\n"
        "  cmake -DCMAKE_BUILD_TYPE=Release -S . -B build && cmake --build build\n"
        f"then copy build/cli/pict to {BIN_DIR}/ or set PICT_PATH."
    )


def parse_tsv(text: str):
    lines = [ln for ln in text.splitlines() if ln.strip()]
    if not lines:
        return []
    header = lines[0].split("\t")
    return [dict(zip(header, ln.split("\t"))) for ln in lines[1:]]


def main() -> int:
    for stream in (sys.stdout, sys.stderr):
        try:
            # captured output (agents, pipes) as UTF-8; interactive console keeps its codepage
            stream.reconfigure(encoding=None if stream.isatty() else "utf-8", errors="replace")
        except AttributeError:
            pass
    argv = sys.argv[1:]
    extra = []
    if "--" in argv:
        idx = argv.index("--")
        argv, extra = argv[:idx], argv[idx + 1:]
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("model", help="PICT model file")
    ap.add_argument("--out", help="write raw TSV output to this file")
    ap.add_argument("--json", action="store_true", help="print rows as JSON instead of TSV")
    args = ap.parse_args(argv)

    pict = find_pict()
    proc = subprocess.run(
        [str(pict), args.model, *extra],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr or proc.stdout)
        return proc.returncode
    if proc.stderr.strip():
        sys.stderr.write(proc.stderr)
    if args.out:
        Path(args.out).write_text(proc.stdout, encoding="utf-8")
    if args.json:
        print(json.dumps(parse_tsv(proc.stdout), ensure_ascii=False, indent=2))
    else:
        sys.stdout.write(proc.stdout)
    print(f"[pict_runner] {pict} -> {len(parse_tsv(proc.stdout))} rows", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
