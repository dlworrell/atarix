#!/usr/bin/env python3
"""Check local Markdown links.

This script validates local Markdown links in README.md and docs/**/*.md.
It intentionally ignores external URLs, mailto links, bare anchors, and images.

It reports file and line numbers so CI failures are immediately actionable.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import sys
from urllib.parse import unquote

# Markdown inline links, excluding images: [text](target)
LINK_RE = re.compile(r"(?<!!)\[[^\]]+\]\(([^)]+)\)")

ROOT = Path(__file__).resolve().parents[1]
CHECK_PATHS = [ROOT / "README.md", ROOT / "docs"]


@dataclass(frozen=True)
class BrokenLink:
    source: Path
    line_no: int
    link: str
    reason: str


def iter_markdown_files():
    for item in CHECK_PATHS:
        if item.is_file():
            yield item
        elif item.is_dir():
            yield from sorted(item.rglob("*.md"))


def is_external(link: str) -> bool:
    return (
        link.startswith("http://")
        or link.startswith("https://")
        or link.startswith("mailto:")
        or link.startswith("#")
    )


def clean_link(link: str) -> str:
    link = link.strip()
    link = link.split("#", 1)[0]
    link = link.split("?", 1)[0]
    return unquote(link).strip()


def strip_code_fences(text: str) -> str:
    """Replace fenced code blocks with blank lines to preserve line numbers."""
    lines = text.splitlines()
    stripped: list[str] = []
    in_fence = False

    for line in lines:
        if line.lstrip().startswith("```"):
            in_fence = not in_fence
            stripped.append("")
            continue

        stripped.append("" if in_fence else line)

    return "\n".join(stripped)


def line_number_for_offset(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def check_file(md_file: Path) -> list[BrokenLink]:
    original_text = md_file.read_text(encoding="utf-8")
    text = strip_code_fences(original_text)
    broken: list[BrokenLink] = []

    for match in LINK_RE.finditer(text):
        raw_link = match.group(1).strip()
        link = clean_link(raw_link)

        if not link or is_external(raw_link):
            continue

        target = (md_file.parent / link).resolve()
        line_no = line_number_for_offset(text, match.start())

        try:
            target.relative_to(ROOT)
        except ValueError:
            broken.append(BrokenLink(md_file, line_no, raw_link, "points outside repo"))
            continue

        if not target.exists():
            broken.append(BrokenLink(md_file, line_no, raw_link, "missing file"))

    return broken


def main() -> int:
    broken: list[BrokenLink] = []

    for md_file in iter_markdown_files():
        broken.extend(check_file(md_file))

    if broken:
        print("Broken Markdown links found:\n")
        for item in broken:
            rel = item.source.relative_to(ROOT)
            print(f"- {rel}:{item.line_no}: {item.link} ({item.reason})")
        return 1

    print("All Markdown links are valid.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
