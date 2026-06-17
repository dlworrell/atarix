#!/usr/bin/env python3
"""Atarix Engineering Management System.

AEMS is a small repo-native helper for tracking architecture specs,
dependencies, issue references, and engineering status.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
INDEX_PATH = ROOT / "tools" / "aems" / "spec_index.json"


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


def command_list_specs(_: argparse.Namespace) -> int:
    data = load_index()
    for spec in iter_specs(data):
        print(f"{spec['id']}\t{spec.get('status', 'unknown')}\t{spec['title']}")
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
    print("Atarix Engineering Management Report")
    print("====================================")
    print(f"Specs tracked: {len(specs)}")
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


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Atarix Engineering Management System")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("list-specs", help="list tracked architecture specs")
    p.set_defaults(func=command_list_specs)

    p = sub.add_parser("check-specs", help="validate tracked spec files")
    p.set_defaults(func=command_check_specs)

    p = sub.add_parser("next", help="show the next incomplete tracked item")
    p.set_defaults(func=command_next)

    p = sub.add_parser("report", help="print an engineering status report")
    p.set_defaults(func=command_report)

    return parser


def main(argv: list[str]) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
