#!/usr/bin/env python3
"""Strict non-mutating validation entry point used by publish-release.yml."""
from pathlib import Path
import runpy,sys
script=Path(__file__).with_name('prepare_github_release.py')
sys.argv=[str(script),'--validate-only']
runpy.run_path(str(script),run_name='__main__')
