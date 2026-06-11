#!/usr/bin/env python3
"""Check that repository documentation is discoverable from README.md.

This script validates that Markdown documents under docs/ are linked from README.md,
with a few intentionally ignored cases for ADR indexes and archived/internal docs.

It complements check-md-links.py:

- check-md-links.py verifies that links are not broken.
- check-doc-index.py verifies that docs are discoverable from the main README.
"""

from __future__ import annotations

from pathlib import Path
import re
import sys
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
README = ROOT / "README.md"
DOCS = ROOT / "docs"

LINK_RE = re.compile(r"(?<!!)\[[^\]]+\]\(([^)]+)\)")

# These paths are intentionally allowed to be absent from the top-level README
# index because they are sub-indexes, generated later, or local-support files.
IGNORED_DOC_PATTERNS = (
    "docs/adr/",
)


def normalize_link(link: str) -> str:
    link = link.strip()
    link = link.split("#", 1)[0]
    link = link.split("?", 1)[0]
    return unquote(link).strip()


def is_local_doc_link(link: str) -> bool:
    return link.startswith("docs/") and link.endswith(".md")


def readme_doc_links() -> set[str]:
    text = README.read_text(encoding="utf-8")
    links: set[str] = set()

    for match in LINK_RE.finditer(text):
        link = normalize_link(match.group(1))
        if is_local_doc_link(link):
            links.add(link)

    return links


def all_docs() -> set[str]:
    docs: set[str] = set()

    for path in sorted(DOCS.rglob("*.md")):
        rel = path.relative_to(ROOT).as_posix()
        if any(rel.startswith(pattern) for pattern in IGNORED_DOC_PATTERNS):
            continue
        docs.add(rel)

    return docs


def main() -> int:
    if not README.exists():
        print("README.md not found.")
        return 1

    if not DOCS.exists():
        print("docs/ directory not found.")
        return 1

    indexed = readme_doc_links()
    docs = all_docs()

    missing_from_index = sorted(docs - indexed)
    stale_index_links = sorted(indexed - docs)

    failed = False

    if missing_from_index:
        failed = True
        print("Docs missing from README documentation index:\n")
        for path in missing_from_index:
            print(f"- {path}")
        print()

    if stale_index_links:
        failed = True
        print("README documentation index links that do not correspond to docs/*.md:\n")
        for path in stale_index_links:
            print(f"- {path}")
        print()

    if failed:
        return 1

    print("README documentation index is consistent with docs/*.md.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
