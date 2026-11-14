#!/usr/bin/env python3
# twister の出力フォルダにある各ビルドの build.log から、GCC の静的解析（-fanalyzer）の
# 指摘（[-Wanalyzer-...]）を集め、Markdown で出す。
# 指摘のうち、このリポジトリのファイル（自分のコード）にあるものが1件でもあれば、
# 終了コード 1 で終わる（CI のジョブを失敗にする）。Zephyr 本体などの指摘は数えて出すだけ。
#
#   python3 scripts/sca_report.py twister-out [自分のコードのフォルダ]
#
# SPDX-License-Identifier: Apache-2.0

import pathlib
import re
import sys

# 例: /path/to/apps/foo/src/main.c:14:17: warning: dereference of ... [-Wanalyzer-...]
DIAG = re.compile(r"^(?P<file>.+?):(?P<line>\d+):(?P<col>\d+): (?:warning|error): "
                  r"(?P<msg>.*\[-W(?:error=)?analyzer-[^\]]+\])\s*$")


def norm(p):
    return str(p).replace("\\", "/").lower()


def main(outdir, own_root):
    own_prefix = norm(pathlib.Path(own_root).resolve()) + "/"
    found = {}  # (file, line, msg) -> 自分のコードか
    for log in pathlib.Path(outdir).rglob("build.log"):
        if "twister_links" in log.parts:  # Windows の --short-build-path が作る別名
            continue
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            m = DIAG.match(line)
            if not m:
                continue
            f = m.group("file")
            path = norm(pathlib.Path(f).resolve()) if pathlib.Path(f).is_absolute() else norm(f)
            found[(f, int(m.group("line")), m.group("msg"))] = path.startswith(own_prefix)

    own = sorted(k for k, v in found.items() if v)
    other = sorted(k for k, v in found.items() if not v)

    print(f"- 自分のコードの指摘: {len(own)} 件")
    print(f"- それ以外（Zephyr 本体など）の指摘: {len(other)} 件")
    for title, items in (("自分のコード", own), ("それ以外", other)):
        if items:
            print(f"\n### {title}\n")
            print("```text")
            for f, ln, msg in items:
                print(f"{f}:{ln}: {msg}")
            print("```")

    return 1 if own else 0


if __name__ == "__main__":
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    outdir = sys.argv[1] if len(sys.argv) > 1 else "twister-out"
    own_root = sys.argv[2] if len(sys.argv) > 2 else "."
    sys.exit(main(outdir, own_root))
