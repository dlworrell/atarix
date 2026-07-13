#!/usr/bin/env python3
"""Inventory documentation-like files across the complete ATARIX repository tree.

The census distinguishes three placement classes:

- canonical: normative and project documentation under ``docs/`` or approved root files;
- colocated: implementation notes and README files intentionally beside code or tests;
- violation: documentation whose path indicates misplaced normative or cross-project authority.

Both JSON and Markdown reports are deterministic and suitable for CI evidence.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

DOCUMENT_EXTENSIONS = {
    ".adoc", ".asciidoc", ".html", ".htm", ".md", ".markdown",
    ".odt", ".pdf", ".rst", ".rtf", ".tex", ".txt",
}

DOCUMENT_NAMES = {
    "AUTHORS", "CHANGELOG", "CODE_OF_CONDUCT", "CONTRIBUTING", "COPYING",
    "HISTORY", "LICENSE", "NOTICE", "README", "RELEASE_NOTES", "SECURITY",
}

EXCLUDED_DIRECTORIES = {
    ".git", ".hg", ".svn", ".idea", ".vscode", "__pycache__", "build",
    "dist", "node_modules", "out", "target", "third_party", "third-party", "vendor",
}

GENERATED_REPORT_PATHS = {
    "docs/engineering/reports/document-authority-census.json",
    "docs/engineering/reports/document-authority-census.md",
}

APPROVED_ROOT_DOCUMENTS = {
    "CHANGELOG.md", "CODE_OF_CONDUCT.md", "CONTRIBUTING.md", "LICENSE",
    "LICENSE.md", "NOTICE", "README", "README.md", "SECURITY.md",
}

COLOCATED_README_ROOTS = {"include", "rtl", "sim", "src", "tests"}

COLOCATED_PREFIXES = (
    "project-zero/inventory/current/",
)

FORBIDDEN_PATHS = {
    "project-zero/README.md": "Moved to docs/roadmap/atarix-project-zero-inventory.md.",
    "rtl/atarix/ATX-SPEC-020-CI.md": "Moved to docs/verification/ATX-SPEC-020-rtl-ci.md.",
    "tools/aems/README.md": "AEMS project documentation belongs in dlworrell/AEMS.",
}


@dataclass(frozen=True)
class DocumentEntry:
    path: str
    size_bytes: int
    location: str
    placement: str
    violation: bool
    canonical_owner: str
    reason: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Inventory documentation across the complete repository tree.")
    parser.add_argument("root", nargs="?", default=".", help="Repository checkout root.")
    parser.add_argument("--repo-name", default="dlworrell/atarix")
    parser.add_argument("--format", choices=("json", "markdown"), default="markdown")
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Exit non-zero when a document-authority placement violation exists.",
    )
    return parser.parse_args()


def is_excluded(relative_path: Path) -> bool:
    return any(part in EXCLUDED_DIRECTORIES for part in relative_path.parts[:-1])


def is_document(path: Path) -> bool:
    if path.suffix.lower() in DOCUMENT_EXTENSIONS:
        return True
    stem = path.name.split(".", 1)[0].upper()
    return stem in DOCUMENT_NAMES


def iter_documents(root: Path) -> Iterable[Path]:
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(root)
        if relative.as_posix() in GENERATED_REPORT_PATHS:
            continue
        if is_excluded(relative):
            continue
        if is_document(path):
            yield path


def location_for(relative: Path) -> str:
    if len(relative.parts) == 1:
        return "repository-root"
    return relative.parts[0]


def is_readme(relative: Path) -> bool:
    return relative.name.split(".", 1)[0].upper() == "README"


def classify_placement(relative: Path) -> tuple[str, bool, str, str]:
    path = relative.as_posix()

    if path in FORBIDDEN_PATHS:
        return "violation", True, "external-or-migrated", FORBIDDEN_PATHS[path]

    if len(relative.parts) == 1:
        if path in APPROVED_ROOT_DOCUMENTS:
            return "canonical", False, "Atarix", "Approved repository-root project document."
        return "violation", True, "Atarix", "Unapproved repository-root documentation file."

    root = relative.parts[0]
    if root in {"docs", ".github"}:
        return "canonical", False, "Atarix", "Approved Atarix documentation authority location."

    if any(path.startswith(prefix) for prefix in COLOCATED_PREFIXES):
        return "colocated", False, "Atarix", "Generated operational evidence retained beside its workflow output."

    if root in COLOCATED_README_ROOTS and is_readme(relative):
        return "colocated", False, "Atarix", "Component-local README retained beside the implementation it explains."

    return (
        "violation",
        True,
        "review-required",
        "Documentation outside approved authority locations is not an allowed component-local README.",
    )


def build_report(root: Path, repo_name: str) -> dict[str, object]:
    root = root.resolve()
    entries: list[DocumentEntry] = []

    for path in iter_documents(root):
        relative = path.relative_to(root)
        placement, violation, owner, reason = classify_placement(relative)
        entries.append(
            DocumentEntry(
                path=relative.as_posix(),
                size_bytes=path.stat().st_size,
                location=location_for(relative),
                placement=placement,
                violation=violation,
                canonical_owner=owner,
                reason=reason,
            )
        )

    violations = [entry for entry in entries if entry.violation]
    placements: dict[str, int] = {}
    locations: dict[str, int] = {}
    for entry in entries:
        placements[entry.placement] = placements.get(entry.placement, 0) + 1
        locations[entry.location] = locations.get(entry.location, 0) + 1

    return {
        "schema": "catalyst.atarix.document-authority-census.v2",
        "repository": repo_name,
        "root": str(root),
        "summary": {
            "document_count": len(entries),
            "violation_count": len(violations),
            "locations": dict(sorted(locations.items())),
            "placements": dict(sorted(placements.items())),
            "strict_passed": not violations,
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
        f"- Schema: `{report['schema']}`",
        f"- Repository: `{report['repository']}`",
        f"- Documents found: `{summary['document_count']}`",
        f"- Placement violations: `{summary['violation_count']}`",
        f"- Strict authority check passed: `{summary['strict_passed']}`",
        "",
        "## Documents",
        "",
        "| Path | Location | Placement | Violation | Canonical owner | Reason |",
        "|---|---|---|---:|---|---|",
    ]

    for item in documents:
        assert isinstance(item, dict)
        reason = str(item["reason"]).replace("|", "\\|")
        lines.append(
            f"| `{item['path']}` | `{item['location']}` | `{item['placement']}` | "
            f"`{item['violation']}` | `{item['canonical_owner']}` | {reason} |"
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
