"""Placement rules of scripts/document_authority_census.py, including REUSE license texts."""
import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("document_authority_census", ROOT / "scripts/document_authority_census.py")
census = importlib.util.module_from_spec(spec)
sys.modules["document_authority_census"] = census
spec.loader.exec_module(census)


def placement(path):
    return census.classify_placement(Path(path))[:2]


class PlacementTests(unittest.TestCase):
    def test_reuse_license_texts_are_canonical(self):
        for path in ("LICENSES/Apache-2.0.txt", "LICENSES/MIT.txt", "LICENSES/CC-BY-4.0.txt",
                     "LICENSES/GPL-2.0-or-later.txt", "LICENSES/LicenseRef-Proprietary.txt"):
            with self.subTest(path=path):
                self.assertEqual(placement(path), ("canonical", False))

    def test_other_files_under_licenses_stay_violations(self):
        for path in ("LICENSES/notes.md", "LICENSES/sub/MIT.txt", "LICENSES/.hidden.txt", "LICENSES/MIT.txt.bak"):
            with self.subTest(path=path):
                self.assertEqual(placement(path), ("violation", True))

    def test_existing_rules_are_unchanged(self):
        cases = {
            "README.md": ("canonical", False),
            "NOTES.md": ("violation", True),
            "docs/anything.md": ("canonical", False),
            "src/README.md": ("colocated", False),
            "src/notes.md": ("violation", True),
            "project-zero/README.md": ("violation", True),
            "project-zero/inventory/current/README.md": ("colocated", False),
        }
        for path, expected in cases.items():
            with self.subTest(path=path):
                self.assertEqual(placement(path), expected)

    def test_strict_cli_passes_with_a_license_text_and_fails_on_a_stray_document(self):
        script = ROOT / "scripts/document_authority_census.py"
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "LICENSES").mkdir()
            (root / "LICENSES/Apache-2.0.txt").write_text("Apache License\n")
            (root / "README.md").write_text("# demo\n")
            ok = subprocess.run([sys.executable, str(script), str(root), "--strict"], capture_output=True, text=True)
            self.assertEqual(ok.returncode, 0, ok.stdout + ok.stderr)
            (root / "LICENSES/notes.md").write_text("stray\n")
            bad = subprocess.run([sys.executable, str(script), str(root), "--strict"], capture_output=True, text=True)
            self.assertNotEqual(bad.returncode, 0)


if __name__ == "__main__":
    unittest.main()
