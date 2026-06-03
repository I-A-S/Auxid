#!/usr/bin/python3

import os
import subprocess
from pathlib import Path


def find_vcvars64() -> Path:
  program_files_x86 = os.environ.get(
    "ProgramFiles(x86)", r"C:\Program Files (x86)"
  )
  vswhere = (
    Path(program_files_x86)
    / "Microsoft Visual Studio"
    / "Installer"
    / "vswhere.exe"
  )
  if not vswhere.is_file():
    raise FileNotFoundError(f"vswhere not found: {vswhere}")

  result = subprocess.run(
    [
      str(vswhere),
      "-latest",
      "-requires",
      "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
      "-property",
      "installationPath",
    ],
    capture_output=True,
    text=True,
    check=True,
  )
  install_path = Path(result.stdout.strip())
  vcvars64 = install_path / "VC" / "Auxiliary" / "Build" / "vcvars64.bat"
  if not vcvars64.is_file():
    raise FileNotFoundError(f"vcvars64.bat not found: {vcvars64}")
  return vcvars64


def _cmd_chain_argv(command: str) -> list[str]:
  argv: list[str] = []
  segments = command.split(" && ")
  for index, segment in enumerate(segments):
    if index > 0:
      argv.append("&&")
    argv.extend(segment.split())
  return argv


def run_command(command: str, cwd: Path) -> None:
  subprocess.run(["cmd", "/c", *_cmd_chain_argv(command)], cwd=cwd, check=True)


def run_in_msvc_env(vcvars64: Path, command: str, cwd: Path) -> None:
  subprocess.run(
    ["cmd", "/c", "call", str(vcvars64), "&&", *_cmd_chain_argv(command)],
    cwd=cwd,
    check=True,
  )


def build_benchmarks(root_dir: Path) -> None:
  os.chdir(root_dir)
  vcvars64 = find_vcvars64()

  build_steps = [
    (
      True,
      "cmake --preset x64-windows-msvc-bench && cmake --build --preset x64-windows-msvc-bench",
    ),
    (
      False,
      "cmake --preset x64-windows-clang-bench && cmake --build --preset x64-windows-clang-bench",
    ),
    (
      False,
      "cmake --preset x64-windows-mingw-gcc-bench && cmake --build --preset x64-windows-mingw-gcc-bench",
    ),
    (
      True,
      (
        "cmake --preset x64-windows-msvc-bench-pgo-gen"
        " && cmake --build --preset x64-windows-msvc-bench-pgo-gen"
        " && cmake --build --preset x64-windows-msvc-bench-pgo-train"
        " && cmake --preset x64-windows-msvc-bench-pgo"
        " && cmake --build --preset x64-windows-msvc-bench-pgo"
      ),
    ),
  ]

  for use_msvc_env, command in build_steps:
    if use_msvc_env:
      run_in_msvc_env(vcvars64, command, root_dir)
    else:
      run_command(command, root_dir)


def main() -> None:
  benchmark_dir = Path(__file__).resolve().parent
  root_dir = benchmark_dir.parent
  os.chdir(root_dir)
  build_benchmarks(root_dir)


if __name__ == "__main__":
  main()
