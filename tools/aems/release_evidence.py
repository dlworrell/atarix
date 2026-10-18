#!/usr/bin/env python3
"""AEMS release evidence manifest: generate, validate and refuse unqualified promotion."""
import argparse
import datetime as dt
import hashlib
import json
import os
import re
import subprocess
from pathlib import Path

SCHEMA = "atarix.aems.release_evidence.v1"
SHA = re.compile(r"^[0-9a-f]{40}$")
def git(*args):
    return subprocess.check_output(["git", *args], text=True).strip()
def digest(path):
    h = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()
def generate(args):
    commit = git("rev-parse", "HEAD")
    summary = Path(args.summary)
    if not summary.is_file():
        raise SystemExit("FAIL: engineering gate summary is missing")
    data = summary.read_text()
    m = re.search(r"(?m)^PASS=(\d+)\s*$", data)
    w = re.search(r"(?m)^WARN=(\d+)\s*$", data)
    f = re.search(r"(?m)^FAIL=(\d+)\s*$", data)
    if not (m and w and f):
        raise SystemExit("FAIL: malformed engineering gate summary")
    passes, warns, fails = int(m[1]), int(w[1]), int(f[1])
    artifacts = []
    root = Path(args.artifacts)
    if root.exists():
        for path in sorted(root.rglob("*")):
            if path.is_file():
                artifacts.append({"path": str(path), "bytes": path.stat().st_size,
                                  "sha256": digest(path)})
    manifest = {
        "schema": SCHEMA,
        "repository": os.getenv("GITHUB_REPOSITORY", git("remote", "get-url", "origin")),
        "commit": commit,
        "source_ref": args.source_ref,
        "generated_at": dt.datetime.now(dt.timezone.utc).isoformat(),
        "workflow_run_id": os.getenv("GITHUB_RUN_ID"),
        "verification": {"mode": "contemporaneous" if args.contemporaneous else "retrospective",
                         "pass": passes, "warn": warns, "fail": fails,
                         "gate_completed": args.gate_completed,
                         "qualified": bool(args.gate_completed and fails == 0 and warns == 0 and passes > 0)},
        "gate_summary_sha256": digest(summary),
        "artifacts": artifacts
    }
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(manifest, indent=2) + "\n")
    print("MANIFEST:", output, "qualified=", manifest["verification"]["qualified"])
def validate(args):
    data = json.loads(Path(args.manifest).read_text())
    errors = []
    if data.get("schema") != SCHEMA: errors.append("schema mismatch")
    if not SHA.fullmatch(str(data.get("commit", ""))): errors.append("invalid commit")
    if args.expected_commit and data.get("commit") != args.expected_commit: errors.append("commit mismatch")
    v = data.get("verification", {})
    if not (v.get("qualified") is True and v.get("gate_completed") is True
            and type(v.get("fail")) is int and v["fail"] == 0
            and type(v.get("warn")) is int and v["warn"] == 0
            and type(v.get("pass")) is int and v["pass"] > 0):
        errors.append("engineering evidence not qualified")
    for item in data.get("artifacts", []):
        p = Path(item.get("path", ""))
        if not p.is_file() or digest(p) != item.get("sha256") or p.stat().st_size != item.get("bytes"):
            errors.append("artifact mismatch: " + str(p))
    if errors:
        raise SystemExit("FAIL: " + "; ".join(errors))
    print("PASS: release evidence manifest matches commit", data["commit"])
def main():
    p = argparse.ArgumentParser()
    sub = p.add_subparsers(dest="cmd", required=True)
    g = sub.add_parser("generate")
    g.add_argument("--summary", default="build/engineering-gate/artifacts/summary.txt")
    g.add_argument("--artifacts", default="build/engineering-gate/artifacts")
    g.add_argument("--source-ref", default="HEAD")
    g.add_argument("--output", default="build/aems/release-evidence.json")
    g.add_argument("--gate-completed", action="store_true")
    g.add_argument("--contemporaneous", action="store_true")
    v = sub.add_parser("validate")
    v.add_argument("--manifest", default="build/aems/release-evidence.json")
    v.add_argument("--expected-commit")
    args = p.parse_args()
    (generate if args.cmd == "generate" else validate)(args)
if __name__ == "__main__":
    main()
