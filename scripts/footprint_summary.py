#!/usr/bin/env python3
# twister の出力フォルダにある各ビルドの build.log から、リンカが出すメモリの使用量
# （Memory region の表）を拾い、Markdown の表にする。
# GitHub Actions では、出力を $GITHUB_STEP_SUMMARY に足すと、実行結果のページに表が出る。
#
#   python3 scripts/footprint_summary.py twister-out
#
# SPDX-License-Identifier: Apache-2.0

import pathlib
import re
import sys

# 表に出すメモリの領域（ESP32-S3 のリンカスクリプトの名前）
REGIONS = ["FLASH", "iram0_0_seg", "dram0_0_seg"]
UNITS = {"B": 1, "KB": 1024, "MB": 1024 ** 2, "GB": 1024 ** 3}
ROW = re.compile(r"^\s*(\S+):\s+(\d+)\s+(B|KB|MB|GB)\s+(\d+)\s+(B|KB|MB|GB)")


def parse_build_log(path):
    """build.log の最後の Memory region の表を {領域: 使用バイト数} で返す"""
    used = {}
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("Memory region"):
            used = {}  # 最後の表だけを使う
            continue
        m = ROW.match(line)
        if m:
            used[m.group(1)] = int(m.group(2)) * UNITS[m.group(3)]
    return used


def main(outdir):
    rows = []
    for log in sorted(pathlib.Path(outdir).rglob("build.log")):
        if "twister_links" in log.parts:  # Windows の --short-build-path が作る別名
            continue
        used = parse_build_log(log)
        if used:
            rows.append((log.parent.name, used))

    print("| シナリオ | " + " | ".join(f"{r} (B)" for r in REGIONS) + " |")
    print("| --- |" + " ---: |" * len(REGIONS))
    for name, used in rows:
        cells = [f"{used[r]:,}" if r in used else "-" for r in REGIONS]
        print(f"| {name} | " + " | ".join(cells) + " |")


if __name__ == "__main__":
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    main(sys.argv[1] if len(sys.argv) > 1 else "twister-out")
