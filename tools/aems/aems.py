#!/usr/bin/env python3
"""Atarix Engineering Management System.

AEMS is a repo-native helper for tracking architecture documents,
dependencies, issue references, engineering status, and documentation graph
health.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
INDEX_PATH = ROOT / "tools" / "aems" / "spec_index.json"
DOCUMENT_DIRS = [
    ROOT / "docs" / "architecture",
    ROOT / "docs" / "design",
    ROOT / "docs" / "engineering",
    ROOT / "docs" / "reviews",
]
SCALAR_KEYS = {
    "document",
    "title",
    "type",
    "status",
    "lifecycle",
    "version",
    "canonical_owner",
    "canonical_document",
    "canonical_section",
}
RELATION_KEYS = {
    "canonical_owner",
    "canonical_document",
    "depends_on",
    "related",
    "implements",
    "verified_by",
    "supersedes",
    "superseded_by",
    "related_reviews",
    "related_issues",
}


def load_index() -> dict[str, Any]:
    with INDEX_PATH.open("r", encoding="utf-8") as fh:
        data = json.load(fh)
    if data.get("schema") != "atarix.aems.spec_index.v1":
        raise SystemExit("unsupported AEMS spec index schema")
    if not isinstance(data.get("specs"), list):
        raise SystemExit("spec index missing specs list")
    return data


def iter_specs(data: dict[str, Any]) -> list[dict[str, Any]]:
    return sorted(data["specs"], key=lambda item: item.get("id", ""))


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def parse_front_matter(text: str) -> dict[str, Any]:
    """Parse the small YAML subset used by Atarix document metadata.

    This intentionally avoids external dependencies so AEMS can run in minimal CI
    environments. Supported forms:

    key: value
    key:
      - value
      - value
    """
    if not text.startswith("---\n"):
        return {}
    end = text.find("\n---", 4)
    if end == -1:
        return {}

    block = text[4:end]
    metadata: dict[str, Any] = {}
    active_key: str | None = None

    for raw_line in block.splitlines():
        line = raw_line.rstrip()
        if not line.strip() or line.strip().startswith("#"):
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

    return metadata


def discover_documents() -> list[dict[str, Any]]:
    docs: list[dict[str, Any]] = []
    for directory in DOCUMENT_DIRS:
        if not directory.exists():
            continue
        for path in sorted(directory.rglob("*.md")):
            rel = str(path.relative_to(ROOT))
            text = read_text(path)
            meta = parse_front_matter(text)
            doc_id = str(meta.get("document") or _fallback_id(path))
            docs.append(
                {
                    "id": doc_id,
                    "title": str(meta.get("title") or path.stem),
                    "type": str(meta.get("type") or "Unknown"),
                    "status": str(meta.get("status") or "Unknown"),
                    "lifecycle": str(meta.get("lifecycle") or "Unknown"),
                    "path": rel,
                    "metadata": meta,
                }
            )
    return sorted(docs, key=lambda item: (item["id"], item["path"]))


def _fallback_id(path: Path) -> str:
    name = path.stem
    if name == "index" and path.parent.name.startswith("ATX-"):
        return path.parent.name
    return name


def relation_values(doc: dict[str, Any], key: str) -> list[str]:
    meta = doc.get("metadata", {})
    value = meta.get(key)
    if value is None or value == "":
        return []
    if isinstance(value, list):
        return [str(item) for item in value if str(item)]
    return [str(value)]


def iter_edges(docs: list[dict[str, Any]]) -> list[tuple[str, str, str]]:
    edges: list[tuple[str, str, str]] = []
    for doc in docs:
        source = doc["id"]
        for key in sorted(RELATION_KEYS):
            for target in relation_values(doc, key):
                edges.append((source, target, key))
    return sorted(edges)


def command_list_specs(_: argparse.Namespace) -> int:
    data = load_index()
    for spec in iter_specs(data):
        print(f"{spec['id']}\t{spec.get('status', 'unknown')}\t{spec['title']}")
    return 0


def command_list_docs(_: argparse.Namespace) -> int:
    for doc in discover_documents():
        print(f"{doc['id']}\t{doc['lifecycle']}\t{doc['type']}\t{doc['path']}")
    return 0


def command_check_docs(_: argparse.Namespace) -> int:
    failures = 0
    seen: dict[str, str] = {}
    docs = discover_documents()
    known_ids = {doc["id"] for doc in docs}

    for doc in docs:
        doc_id = doc["id"]
        path = doc["path"]
        if not doc_id:
            print(f"FAIL missing document id: {path}")
            failures += 1
            continue
        if doc_id in seen:
            print(f"FAIL duplicate document id: {doc_id} in {seen[doc_id]} and {path}")
            failures += 1
        else:
            print(f"PASS document id: {doc_id} -> {path}")
            seen[doc_id] = path
        if doc["lifecycle"] == "Unknown":
            print(f"WARN missing lifecycle metadata: {path}")
        if doc["type"] == "Unknown":
            print(f"WARN missing type metadata: {path}")

    for source, target, relation in iter_edges(docs):
        if target.startswith("#") or "/" in target or target.endswith(".h") or target.endswith(".v"):
            print(f"PASS trace edge: {source} -[{relation}]-> {target}")
            continue
        if target in known_ids:
            print(f"PASS doc edge: {source} -[{relation}]-> {target}")
        else:
            print(f"WARN unresolved edge: {source} -[{relation}]-> {target}")

    if failures:
        print(f"AEMS DOC CHECK RESULT: FAIL failures={failures}")
        return 1
    print(f"AEMS DOC CHECK RESULT: PASS documents={len(docs)} edges={len(iter_edges(docs))}")
    return 0


def command_check_specs(_: argparse.Namespace) -> int:
    data = load_index()
    failures = 0
    seen: set[str] = set()

    for spec in iter_specs(data):
        spec_id = spec.get("id")
        title = spec.get("title")
        rel_path = spec.get("path")
        if not spec_id or not title or not rel_path:
            print(f"FAIL malformed spec entry: {spec!r}")
            failures += 1
            continue
        if spec_id in seen:
            print(f"FAIL duplicate spec id: {spec_id}")
            failures += 1
        seen.add(spec_id)

        path = ROOT / rel_path
        if path.is_file():
            print(f"PASS file exists: {rel_path}")
        else:
            print(f"FAIL missing file: {rel_path}")
            failures += 1

        text = path.read_text(encoding="utf-8") if path.is_file() else ""
        if spec_id in text:
            print(f"PASS id appears in file: {spec_id}")
        else:
            print(f"FAIL id missing from file: {spec_id}")
            failures += 1

    if failures:
        print(f"AEMS CHECK RESULT: FAIL failures={failures}")
        return 1
    print("AEMS CHECK RESULT: PASS")
    return 0


def command_next(_: argparse.Namespace) -> int:
    data = load_index()
    for spec in iter_specs(data):
        if spec.get("status") != "complete":
            print(f"NEXT {spec['id']}: {spec['title']}")
            print(f"PATH {spec['path']}")
            issues = spec.get("issues", [])
            if issues:
                print("ISSUES " + ", ".join(f"#{issue}" for issue in issues))
            return 0
    print("NEXT none: all tracked specs are complete")
    return 0


def command_report(_: argparse.Namespace) -> int:
    data = load_index()
    specs = iter_specs(data)
    docs = discover_documents()
    edges = iter_edges(docs)
    print("Atarix Engineering Management Report")
    print("====================================")
    print(f"Specs tracked: {len(specs)}")
    print(f"Docs discovered: {len(docs)}")
    print(f"Doc edges: {len(edges)}")
    for spec in specs:
        depends = ", ".join(spec.get("depends_on", [])) or "none"
        issues = ", ".join(f"#{issue}" for issue in spec.get("issues", [])) or "none"
        print("")
        print(f"{spec['id']}: {spec['title']}")
        print(f"  status: {spec.get('status', 'unknown')}")
        print(f"  path:   {spec['path']}")
        print(f"  deps:   {depends}")
        print(f"  issues: {issues}")
    return 0


def _dot_id(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def command_graph(_: argparse.Namespace) -> int:
    docs = discover_documents()
    print("digraph atarix_docs {")
    print('  rankdir="LR";')
    print('  node [shape="box"];')
    for doc in docs:
        label = f"{doc['id']}\\n{doc['type']}"
        print(f'  "{_dot_id(doc["id"])}" [label="{_dot_id(label)}"];')
    for source, target, relation in iter_edges(docs):
        print(f'  "{_dot_id(source)}" -> "{_dot_id(target)}" [label="{_dot_id(relation)}"];')
    print("}")
    return 0


def command_trace(_: argparse.Namespace) -> int:
    docs = discover_documents()
    edges = iter_edges(docs)
    print("Atarix Documentation Traceability")
    print("=================================")
    for source, target, relation in edges:
        print(f"{source}\t{relation}\t{target}")
    print(f"TRACE RESULT: documents={len(docs)} edges={len(edges)}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Atarix Engineering Management System")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("list-specs", help="list tracked architecture specs")
    p.set_defaults(func=command_list_specs)

    p = sub.add_parser("list-docs", help="discover and list documentation files")
    p.set_defaults(func=command_list_docs)

    p = sub.add_parser("check-docs", help="validate documentation metadata and relationships")
    p.set_defaults(func=command_check_docs)

    p = sub.add_parser("check-specs", help="validate tracked spec files")
    p.set_defaults(func=command_check_specs)

    p = sub.add_parser("next", help="show the next incomplete tracked item")
    p.set_defaults(func=command_next)

    p = sub.add_parser("report", help="print an engineering status report")
    p.set_defaults(func=command_report)

    p = sub.add_parser("graph", help="emit a Graphviz document relationship graph")
    p.set_defaults(func=command_graph)

    p = sub.add_parser("trace", help="print document traceability edges")
    p.set_defaults(func=command_trace)

    return parser


def main(argv: list[str]) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
