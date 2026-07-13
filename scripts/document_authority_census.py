#!/usr/bin/env python3
"""Inventory documentation-like files across the entire ATARIX repository tree.

The census intentionally scans the complete checkout rather than only ``docs/``.
It emits a machine-readable JSON report and a human-readable Markdown report so
repository migration decisions are based on durable evidence.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

DOCUMENT_EXTENSIONS = {
    ".adoc",
    ".asciidoc",
    ".html",
    ".htm",
    ".md",
    ".markdown",
    ".odt",
    ".pdf",
    ".rst",
    ".rtf",
    ".tex",
    ".txt",
}

DOCUMENT_NAMES = {
    "AUTHORS",
    "CHANGELOG",
    "CODE_OF_CONDUCT",
    "CONTRIBUTING",
    "COPYING",
    "HISTORY",
    "LICENSE",
    "NOTICE",
    "README",
    "RELEASE_NOTES",
    "SECURITY",
}

EXCLUDED_DIRECTORIES = {
    ".git",
    ".hg",
    ".svn",
    ".idea",
    ".vscode",
    "__pycache__",
    "build",
    "dist",
    "node_modules",
    "out",
    "target",
    "third_party",
    "third-party",
    "vendor",
}

APPROVED_DOCUMENT_ROOTS = {
    ".github",
    "docs",
}

APPROVED_ROOT_DOCUMENTS = {
    "CHANGELOG.md",
    "CODE_OF_CONDUCT.md",
    "CONTRIBUTING.md",
    "LICENSE",
    "LICENSE.md",
    "NOTICE",
    "README",
    "README.md",
    "SECURITY.md",
}

DESTINATION_HINTS = {
    "Catylist": (
        "ecosystem",
        "governance",
        "repository taxonomy",
        "repository lifecycle",
        "program authority",
    ),
    "AES": (
        "engineering standard",
        "development principles",
        "coding standard",
        "documentation standard",
        "testing standard",
        "review discipline",
    ),
    "AEMS": (
        "compliance",
        "evidence model",
        "scanner",
        "validation rule",
        "project management",
        "engineering graph",
    ),
    "JAG": (
        "ai assistant",
        "assistant design",
        "knowledge graph",
        "prompt",
        "agent workflow",
    ),
}


@dataclass(frozen=True)
class DocumentEntry:
    path: str
    size_bytes: int
    location: str
    outside_approved_location: bool
    destination_hint: str
    reason: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Inventory documentation across the complete repository tree.")
    parser.add_argument("root", nargs="?", default=".", help="Repository checkout root.")
    parser.add_argument("--repo-name", default="dlworrell/atarix")
    parser.add_argument("--format", choices=("json", "markdown"), default="markdown")
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Exit non-zero when documentation-like files exist outside approved locations.",
    )
    return parser.parse_args()


def is_excluded(relative_path: Path) -> bool:
    return any(part in EXCLUDED_DIRECTORIES for part in relative_path.parts[:-1])


def is_document(path: Path) -> bool:
    suffix = path.suffix.lower()
    if suffix in DOCUMENT_EXTENSIONS:
        return True
    stem = path.name.split(".", 1)[0].upper()
    return stem in DOCUMENT_NAMES


def iter_documents(root: Path) -> Iterable[Path]:
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(root)
        if is_excluded(relative):
            continue
        if is_document(path):
            yield path


def location_for(relative: Path) -> str:
    if len(relative.parts) == 1:
        return "repository-root"
    return relative.parts[0]


def outside_approved_location(relative: Path) -> bool:
    if len(relative.parts) == 1:
        return relative.as_posix() not in APPROVED_ROOT_DOCUMENTS
    return relative.parts[0] not in APPROVED_DOCUMENT_ROOTS


def read_sample(path: Path, limit: int = 64_000) -> str:
    if path.suffix.lower() in {".pdf", ".odt", ".rtf"}:
        return ""
    try:
        return path.read_text(encoding="utf-8", errors="replace")[:limit].lower()
    except OSError:
        return ""


def destination_for(path: Path, sample: str) -> tuple[str, str]:
    normalized_path = path.as_posix().lower()
    haystack = f"{normalized_path}\n{sample}"

    if normalized_path.startswith("docs/engineering/atarix-"):
        return "Atarix", "Project-local profile extending an AES standard."
    if normalized_path.startswith("docs/engineering/secure-c-cxx"):
        return "Atarix", "Project-local secure-coding profile."
    if normalized_path.startswith("docs/engineering/aes-sec-001-waivers"):
        return "Atarix", "Project-local waiver register."

    for destination, phrases in DESTINATION_HINTS.items():
        if any(phrase in haystack for phrase in phrases):
            return destination, f"Contains language associated with {destination} authority."

    return "Atarix-review", "No deterministic cross-repository authority signal; requires manual review."


def build_report(root: Path, repo_name: str) -> dict[str, object]:
    root = root.resolve()
    entries: list[DocumentEntry] = []

    for path in iter_documents(root):
        relative = path.relative_to(root)
        destination, reason = destination_for(relative, read_sample(path))
        entries.append(
            DocumentEntry(
                path=relative.as_posix(),
                size_bytes=path.stat().st_size,
                location=location_for(relative),
                outside_approved_location=outside_approved_location(relative),
                destination_hint=destination,
                reason=reason,
            )
        )

    outside = [entry for entry in entries if entry.outside_approved_location]
    destinations: dict[str, int] = {}
    locations: dict[str, int] = {}
    for entry in entries:
        destinations[entry.destination_hint] = destinations.get(entry.destination_hint, 0) + 1
        locations[entry.location] = locations.get(entry.location, 0) + 1

    return {
        "schema": "catalyst.atarix.document-authority-census.v1",
        "repository": repo_name,
        "root": str(root),
        "summary": {
            "document_count": len(entries),
            "outside_approved_location_count": len(outside),
            "locations": dict(sorted(locations.items())),
            "destination_hints": dict(sorted(destinations.items())),
            "strict_passed": not outside,
        },
        "documents": [asdict(entry) for entry in entries],
    }


def markdown(report: dict[str, object]) -> str:
    summary = report["summary"]
    documents = report["documents"]
    assert isinstance(summary, dict)
    assert isinstance(documents, list)

    lines = [
        "# ATARIX Full-Tree Document Authority Census",
        "",
        f"- Repository: `{report['repository']}`",
        f"- Documents found: `{summary['document_count']}`",
        f"- Documents outside approved locations: `{summary['outside_approved_location_count']}`",
        f"- Strict location check passed: `{summary['strict_passed']}`",
        "",
        "## Documents",
        "",
        "| Path | Location | Outside approved location | Destination hint | Reason |",
        "|---|---|---:|---|---|",
    ]

    for item in documents:
        assert isinstance(item, dict)
        reason = str(item["reason"]).replace("|", "\\|")
        lines.append(
            f"| `{item['path']}` | `{item['location']}` | "
            f"`{item['outside_approved_location']}` | `{item['destination_hint']}` | {reason} |"
        )

    return "\n".join(lines) + "\n"


def main() -> int:
    args = parse_args()
    report = build_report(Path(args.root), args.repo_name)

    if args.format == "json":
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(markdown(report), end="")

    summary = report["summary"]
    assert isinstance(summary, dict)
    if args.strict and not bool(summary["strict_passed"]):
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
