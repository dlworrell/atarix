#!/usr/bin/env python3
"""Generate a non-qualifying historical engineering inventory from local Git history."""
import argparse
import datetime as dt
import hashlib
import json
import subprocess
from pathlib import Path

def git(*args):
    return subprocess.check_output(["git", *args], text=True).strip()

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--since", default="2026-06-17")
    p.add_argument("--until", default=None)
    p.add_argument("--output", default="build/aems/historical-inventory.json")
    args = p.parse_args()
    start = dt.date.fromisoformat(args.since)
    end = dt.date.fromisoformat(args.until) if args.until else dt.datetime.now(dt.timezone.utc).date()
    if end < start:
        p.error("--until must not precede --since")
    repo = git("remote", "get-url", "origin") if git("remote") else "unknown"
    head = git("rev-parse", "HEAD")
    # Committer timestamps determine the bounded inventory; preserve author dates too.
    raw = git("log", "--all", "--since=" + str(start) + "T00:00:00Z",
              "--until=" + str(end) + "T23:59:59Z",
              "--date=iso-strict", "--format=%H%x1f%cI%x1f%aI%x1f%s%x1e")
    commits = []
    for record in raw.split("\x1e"):
        parts = record.strip().split("\x1f")
        if len(parts) != 4:
            continue
        sha, committed, authored, subject = parts
        commits.append({"sha": sha, "committed_at": committed,
                        "authored_at": authored, "subject": subject,
                        "verification": "not_assessed",
                        "ci_evidence": [],
                        "release_qualified": False})
    manifest = {
        "schema": "atarix.aems.historical_inventory.v1",
        "repository_remote": repo,
        "auditor_commit": head,
        "period": {"start": str(start), "end": str(end)},
        "generated_at": dt.datetime.now(dt.timezone.utc).isoformat(),
        "commit_count": len(commits),
        "commits": commits,
        "limitations": [
            "Git history alone does not prove CI success or release qualification.",
            "CI run evidence, PR review decisions and artifact provenance require separate collection.",
            "Historical commits may not contain current AEMS or Engineering Gate tooling."
        ]
    }
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    digest = hashlib.sha256(out.read_bytes()).hexdigest()
    print(f"AEMS INVENTORY: commits={len(commits)} period={start}..{end}")
    print(f"OUTPUT: {out} SHA256={digest}")
    print("STATUS: inventory only; no release is qualified")
if __name__ == "__main__":
    main()
