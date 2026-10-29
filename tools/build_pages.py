#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build the published AMARU pages from the single source of truth.

  web/install.template.html + firmware/web/{fm1pkg,fm1ota}.js -> install.html
  web/verify.template.html  + firmware/web/fm1pkg.js          -> verify.html

The update protocol is *not* re-implemented here: the pages inline the same
modules the Felucca site builder uses (firmware/web/make_site.py), so the
installer can never drift from the firmware's M-UPGRADE reader again.

  python3 tools/build_pages.py [PACKAGE]
"""
import hashlib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WEB = ROOT / "firmware" / "web"
VERSION = "0.4"
BLOCKS, BLK, KEEP = 20, 0x30, 0x2F


def strip_module(src: str) -> str:
    src = re.sub(r"^export\s+", "", src, flags=re.M)
    return re.sub(r"^import .*?;\n", "", src, flags=re.M)


def product_of(raw: bytes) -> str:
    """the package identity: one marker byte after each of the first 20 blocks"""
    if len(raw) < BLOCKS * BLK:
        return ""
    out = []
    for i in range(BLOCKS):
        m = raw[i * BLK + KEEP]
        if m != 0x7D:
            out.append(chr((m - i - 1) & 0xFF))
    return "".join(out)


def fill(template: Path, page: str, libs, meta: dict) -> str:
    html = template.read_text(encoding="utf-8")
    lib = "\n".join(strip_module((WEB / name).read_text(encoding="utf-8")).rstrip() for name in libs)
    for mark in ("/*LIB*/", "/*META*/"):
        if html.count(mark) != 1:
            raise SystemExit(f"{template}: must contain {mark} exactly once")
    html = html.replace("/*LIB*/", lib).replace("/*META*/", json.dumps(meta))
    (ROOT / page).write_text(html, encoding="utf-8")
    return page


def main(pkg: Path) -> None:
    raw = pkg.read_bytes()
    product = product_of(raw)
    meta = {
        "version": VERSION,
        "product": product,
        "size": len(raw),
        "sha256": hashlib.sha256(raw).hexdigest(),
        "pkg": pkg.relative_to(ROOT).as_posix(),
    }
    fill(ROOT / "web" / "install.template.html", "install.html",
         ("fm1pkg.js", "fm1ota.js"), meta)
    fill(ROOT / "web" / "verify.template.html", "verify.html", ("fm1pkg.js",), meta)
    print(f"install.html + verify.html: {product} {meta['size']} B {meta['sha256']}")
    print(f"package served from: {meta['pkg']}")


if __name__ == "__main__":
    main(Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "firmware" / "build" / "felucca.fwsc")
