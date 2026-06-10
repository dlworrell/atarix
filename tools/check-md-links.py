#!/usr/bin/env python3

from pathlib import Path
import re
import sys
from urllib.parse import unquote

LINK_RE = re.compile(r"\[[^\]]+\]\(([^)]+)\)")

ROOT = Path(__file__).resolve().parents[1]
CHECK_DIRS = [ROOT / "README.md", ROOT / "docs"]

def iter_markdown_files():
    for item in CHECK_DIRS:
        if item.is_file():
            yield item
        elif item.is_dir():
            yield from item.rglob("*.md")

def is_external(link: str) -> bool:
    return (
        link.startswith("http://")
        or link.startswith("https://")
        or link.startswith("mailto:")
        or link.startswith("#")
    )

def clean_link(link: str) -> str:
    link = link.split("#", 1)[0]
    link = link.split("?", 1)[0]
    return unquote(link).strip()

def main() -> int:
    broken = []

    for md_file in iter_markdown_files():
        text = md_file.read_text(encoding="utf-8")

        for match in LINK_RE.finditer(text):
            raw_link = match.group(1).strip()
            link = clean_link(raw_link)

            if not link or is_external(raw_link):
                continue

            target = (md_file.parent / link).resolve()

            try:
                target.relative_to(ROOT)
            except ValueError:
                broken.append((md_file, raw_link, "points outside repo"))
                continue

            if not target.exists():
                broken.append((md_file, raw_link, "missing file"))

    if broken:
        print("Broken Markdown links found:\n")
        for md_file, link, reason in broken:
            rel = md_file.relative_to(ROOT)
            print(f"- {rel}: {link} ({reason})")
        return 1

    print("All Markdown links are valid.")
    return 0

if __name__ == "__main__":
    sys.exit(main())