#!/usr/bin/env python3
"""Build the ATX-100 master architecture manual."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_INDEX = ROOT / "docs" / "architecture" / "ATX-100" / "index.md"
DEFAULT_OUTPUT = ROOT / "build" / "ATX-100-MASTER.md"

REQUIRED_MASTER_KEYS = {"document", "title", "type", "status", "lifecycle", "version", "canonical"}
REQUIRED_CHAPTER_KEYS = {"document", "title", "type", "status", "lifecycle", "version", "canonical_owner"}
CHAPTER_RE = re.compile(r"^-\s+(?:\[)?Chapter\s+(\d+):\s+([^\]\n(]+)(?:\]\(([^)]+)\))?", re.IGNORECASE)


def parse_front_matter(text: str) -> tuple[dict[str, Any], str]:
    if not text.startswith("---\n"):
        return {}, text
    end = text.find("\n---", 4)
    if end == -1:
        raise ValueError("front matter starts with --- but has no closing delimiter")
    block = text[4:end]
    body = text[end + len("\n---") :].strip()
    metadata: dict[str, Any] = {}
    active_key: str | None = None
    for raw_line in block.splitlines():
        line = raw_line.rstrip()
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if line.startswith("  - ") and active_key:
            metadata.setdefault(active_key, [])
            if not isinstance(metadata[active_key], list):
                metadata[active_key] = [str(metadata[active_key])]
            metadata[active_key].append(line[4:].strip().strip('"'))
            continue
        if ":" not in line:
            active_key = None
            continue
        key, value = line.split(":", 1)
        key = key.strip()
        value = value.strip().strip('"')
        if value:
            metadata[key] = value
            active_key = None
        else:
            metadata[key] = []
            active_key = key
    return metadata, body


def missing_keys(metadata: dict[str, Any], keys: set[str], label: str) -> list[str]:
    return [f"{label}: missing required front matter key: {key}" for key in sorted(keys - set(metadata))]


def discover_chapters(index_body: str) -> list[tuple[int, str, str | None]]:
    chapters: list[tuple[int, str, str | None]] = []
    for line in index_body.splitlines():
        match = CHAPTER_RE.match(line.strip())
        if match:
            chapters.append((int(match.group(1)), match.group(2).strip(), match.group(3)))
    return chapters


def build_manual(index_path: Path, output_path: Path, require_all_chapters: bool) -> int:
    failures: list[str] = []
    warnings: list[str] = []

    if not index_path.is_file():
        print(f"FAIL ATX-100 index missing: {index_path}")
        return 1

    index_meta, index_body = parse_front_matter(index_path.read_text(encoding="utf-8"))
    failures.extend(missing_keys(index_meta, REQUIRED_MASTER_KEYS, str(index_path.relative_to(ROOT))))
    if index_meta.get("document") != "ATX-100":
        failures.append("ATX-100 index front matter document must be ATX-100")
    if str(index_meta.get("canonical", "")).lower() != "true":
        failures.append("ATX-100 index must declare canonical: true")

    chapters = discover_chapters(index_body)
    if len(chapters) != 22:
        failures.append(f"ATX-100 index must enumerate 22 chapters; found {len(chapters)}")
    numbers = [item[0] for item in chapters]
    if numbers != list(range(1, len(chapters) + 1)):
        failures.append(f"ATX-100 chapter numbers are not contiguous: {numbers}")

    compiled = ["# ATX-100: CANONICAL ATARIX SYSTEM ARCHITECTURE\n", f"Generated via AEMS / Version: {index_meta.get('version', 'unknown')}\n", "---\n"]
    seen_documents: set[str] = set()

    for number, title, link in chapters:
        if not link:
            message = f"Chapter {number} is planned but not linked yet: {title}"
            if require_all_chapters:
                failures.append(message)
            else:
                warnings.append(message)
            continue
        chapter_file = (index_path.parent / link).resolve()
        rel = chapter_file.relative_to(ROOT)
        if not chapter_file.is_file():
            failures.append(f"Chapter {number} source missing: {rel}")
            continue
        chapter_meta, chapter_body = parse_front_matter(chapter_file.read_text(encoding="utf-8"))
        failures.extend(missing_keys(chapter_meta, REQUIRED_CHAPTER_KEYS, str(rel)))
        expected_doc = f"ATX-100-CH{number:02d}"
        actual_doc = str(chapter_meta.get("document", ""))
        if actual_doc != expected_doc:
            failures.append(f"{rel}: expected document {expected_doc}, found {actual_doc or 'missing'}")
        if chapter_meta.get("canonical_owner") != "ATX-100":
            failures.append(f"{rel}: canonical_owner must be ATX-100")
        if actual_doc in seen_documents:
            failures.append(f"duplicate chapter document id: {actual_doc}")
        seen_documents.add(actual_doc)
        print(f"PASS ATX-100 chapter source: {number:02d} {rel}")
        compiled.append(chapter_body)
        compiled.append("\n---\n")

    for warning in warnings:
        print(f"WARN {warning}")
    for failure in failures:
        print(f"FAIL {failure}")
    if failures:
        print(f"AEMS ATX-100 BUILD RESULT: FAIL failures={len(failures)} warnings={len(warnings)}")
        return 1

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(compiled).rstrip() + "\n", encoding="utf-8")
    print(f"PASS wrote ATX-100 master manual: {output_path.relative_to(ROOT)}")
    print(f"AEMS ATX-100 BUILD RESULT: PASS chapters={len(seen_documents)} warnings={len(warnings)}")
    return 0


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Build the ATX-100 master architecture manual")
    parser.add_argument("--index", type=Path, default=DEFAULT_INDEX)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--require-all-chapters", action="store_true")
    args = parser.parse_args(argv)
    return build_manual(args.index, args.output, args.require_all_chapters)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
