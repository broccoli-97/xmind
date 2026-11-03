"""Regression checks for parsing real Mach-O load-command output."""

import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location(
    "verify_bundle", Path(__file__).resolve().parents[1] / "packaging/macos/verify_bundle.py")
bundle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bundle)


class DeploymentTargetTests(unittest.TestCase):
    def test_modern_command_ignores_linker_and_source_versions(self):
        commands = """QtCore:
Load command 8
      cmd LC_BUILD_VERSION
  cmdsize 32
 platform 1
    minos 11.0
      sdk 14.0
   ntools 1
     tool 3
  version 820.1
Load command 9
      cmd LC_SOURCE_VERSION
  cmdsize 16
  version 999.0
"""
        self.assertEqual(bundle.deployment_targets(commands), ["11.0"])

    def test_legacy_command_uses_version(self):
        commands = """Load command 5
      cmd LC_VERSION_MIN_MACOSX
  cmdsize 16
  version 11.0
      sdk 14.0
"""
        self.assertEqual(bundle.deployment_targets(commands), ["11.0"])

    def test_genuinely_higher_target_is_preserved(self):
        commands = """Load command 5
      cmd LC_BUILD_VERSION
  cmdsize 32
 platform 1
    minos 13.3
      sdk 14.0
   ntools 1
     tool 3
  version 820.1
"""
        targets = bundle.deployment_targets(commands)
        self.assertEqual(targets, ["13.3"])
        self.assertGreater(bundle.version(targets[0]), bundle.version("11.0"))

    def test_missing_target_cannot_be_replaced_by_other_versions(self):
        commands = """Load command 5
      cmd LC_SOURCE_VERSION
  cmdsize 16
  version 11.0
"""
        self.assertEqual(bundle.deployment_targets(commands), [])


if __name__ == "__main__":
    unittest.main()
