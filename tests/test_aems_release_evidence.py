#!/usr/bin/env python3
"""Black-box tests for AEMS release-evidence generation and qualification."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
TOOL = ROOT / "tools/aems/release_evidence.py"
COMMIT = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()

class ReleaseEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name)
        self.artifacts = self.base / "artifacts"
        self.artifacts.mkdir()
        self.summary = self.artifacts / "summary.txt"
        self.manifest = self.base / "manifest.json"

    def run_tool(self, *args, success=True):
        r = subprocess.run([sys.executable, str(TOOL), *map(str, args)],
                           cwd=ROOT, capture_output=True, text=True)
        if success:
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
        else:
            self.assertNotEqual(r.returncode, 0, r.stdout + r.stderr)
        return r

    def generate(self, summary="PASS=37\nWARN=0\nFAIL=0\n", completed=True):
        self.summary.write_text(summary)
        args = ["generate", "--summary", self.summary, "--artifacts",
                self.artifacts, "--output", self.manifest]
        if completed:
            args.append("--gate-completed")
        self.run_tool(*args)
        return json.loads(self.manifest.read_text())

    def validate(self, success=True, expected=COMMIT):
        return self.run_tool("validate", "--manifest", self.manifest,
                             "--expected-commit", expected, success=success)

    def test_valid_evidence(self):
        m = self.generate()
        self.assertEqual(m["commit"], COMMIT)
        self.assertTrue(m["verification"]["qualified"])
        self.validate()

    def test_warning_is_not_qualified(self):
        self.generate("PASS=37\nWARN=1\nFAIL=0\n")
        self.validate(success=False)

    def test_failure_is_not_qualified(self):
        self.generate("PASS=36\nWARN=0\nFAIL=1\n")
        self.validate(success=False)

    def test_zero_passes_is_not_qualified(self):
        self.generate("PASS=0\nWARN=0\nFAIL=0\n")
        self.validate(success=False)

    def test_incomplete_gate_is_not_qualified(self):
        self.generate(completed=False)
        self.validate(success=False)

    def test_missing_summary(self):
        self.run_tool("generate", "--summary", self.summary, "--output",
                      self.manifest, success=False)

    def test_malformed_summary(self):
        self.summary.write_text("PASS=37\nWARN=0\n")
        self.run_tool("generate", "--summary", self.summary, "--output",
                      self.manifest, success=False)

    def test_commit_mismatch(self):
        self.generate()
        self.validate(success=False, expected="0" * 40)

    def test_modified_artifact(self):
        self.generate()
        self.summary.write_text("PASS=999\nWARN=0\nFAIL=0\n")
        self.validate(success=False)

    def test_missing_artifact(self):
        self.generate()
        self.summary.unlink()
        self.validate(success=False)

    def test_manifest_tampering(self):
        m = self.generate()
        m["verification"]["qualified"] = False
        self.manifest.write_text(json.dumps(m))
        self.validate(success=False)

if __name__ == "__main__":
    unittest.main()
