#!/usr/bin/python3

import ctypes
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import time
from contextlib import contextmanager
from pathlib import Path

COMPILER_RESULT_DIRS = ("clang", "gcc", "msvc", "msvc-pgo")

BENCHMARK_RUNS = (
  ("clang", "x64-windows-clang-bench"),
  ("gcc", "x64-windows-mingw-gcc-bench"),
  ("msvc", "x64-windows-msvc-bench"),
  ("msvc-pgo", "x64-windows-msvc-bench-pgo"),
)

# Hash-map N sweep uses RANGE_MIN=512 in benchmarks/cpp/containers/hash_map_shared.hpp.
BENCHMARK_ARGS = [
  "--benchmark_min_time=2.0s",
  "--benchmark_min_warmup_time=0.5s",
  "--benchmark_repetitions=5",
  "--benchmark_report_aggregates_only=true",
  "--benchmark_enable_random_interleaving=true",
  "--benchmark_counters_tabular=true",
  "--benchmark_out=bench_results.json",
  "--benchmark_out_format=json",
]

HIGH_PERFORMANCE_POWER_PLAN = "8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c"
DEFAULT_BENCH_AFFINITY_CORE = 2
BENCH_COOLDOWN_SECONDS = 5.0

HIGH_PRIORITY_CLASS = 0x00000080


def setup_results_tree(benchmark_dir: Path, root_dir: Path) -> tuple[Path, str]:
  results_dir = benchmark_dir / "results"
  results_dir.mkdir(exist_ok=True)

  result = subprocess.run(
    ["git", "rev-parse", "HEAD"],
    cwd=root_dir,
    capture_output=True,
    text=True,
    check=True,
  )
  commit_id = result.stdout.strip()[:7]

  commit_dir = results_dir / commit_id
  if commit_dir.is_dir():
    shutil.rmtree(commit_dir)
  commit_dir.mkdir()

  for name in COMPILER_RESULT_DIRS:
    (commit_dir / name).mkdir()

  return commit_dir, commit_id


def _run_capture(command: list[str], cwd: Path | None = None) -> str:
  try:
    result = subprocess.run(
      command,
      cwd=cwd,
      capture_output=True,
      text=True,
      check=False,
    )
  except FileNotFoundError:
    return ""
  if result.returncode != 0:
    return (result.stdout + result.stderr).strip()
  return result.stdout.strip()


def _cpu_brand() -> str:
  if sys.platform == "win32":
    output = _run_capture(
      [
        "powershell",
        "-NoProfile",
        "-Command",
        "(Get-CimInstance Win32_Processor | Select-Object -First 1 -ExpandProperty Name)",
      ]
    )
    if output:
      return output
  return platform.processor() or "unknown"


def _logical_cores() -> int:
  count = os.cpu_count()
  return count if count is not None else 0


def _compiler_versions(root_dir: Path, vcvars64: Path | None) -> dict[str, str]:
  versions = {
    "clang": _run_capture(["clang", "--version"]),
    "gcc": _run_capture(["g++-15", "--version"]),
    "msvc": "",
  }
  if vcvars64 is not None:
    versions["msvc"] = _run_capture(
      ["cmd", "/c", "call", str(vcvars64), "&&", "cl"],
      cwd=root_dir,
    )
  return versions


def write_meta_json(
  commit_dir: Path,
  commit_id: str,
  root_dir: Path,
  vcvars64: Path | None,
  powerplan_before: str,
  powerplan_during: str,
) -> None:
  meta = {
    "commit": commit_id,
    "host_name": platform.node(),
    "os": platform.platform(),
    "cpu_brand": _cpu_brand(),
    "physical_cores": _logical_cores(),
    "logical_cores": _logical_cores(),
    "python_version": platform.python_version(),
    "powerplan_before": powerplan_before,
    "powerplan_during": powerplan_during,
    "compilers": _compiler_versions(root_dir, vcvars64),
    "benchmark_args": BENCHMARK_ARGS,
    "bench_affinity_core": DEFAULT_BENCH_AFFINITY_CORE,
    "bench_cooldown_seconds": BENCH_COOLDOWN_SECONDS,
  }
  (commit_dir / "meta.json").write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")


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


def auxid_bench_exe(root_dir: Path, preset_name: str) -> Path:
  return root_dir / "out" / "build" / preset_name / "bin" / "Release" / "AuxidBench.exe"


def _get_active_power_plan_guid() -> str:
  output = _run_capture(["powercfg", "/getactivescheme"])
  match = re.search(r"([0-9a-fA-F-]{36})", output)
  return match.group(1) if match else ""


@contextmanager
def set_windows_high_perf_power_plan():
  previous = _get_active_power_plan_guid()
  active_during = HIGH_PERFORMANCE_POWER_PLAN
  try:
    subprocess.run(
      ["powercfg", "/setactive", HIGH_PERFORMANCE_POWER_PLAN],
      check=True,
    )
    yield previous, active_during
  finally:
    if previous:
      subprocess.run(["powercfg", "/setactive", previous], check=False)


def _set_process_affinity(process: subprocess.Popen[bytes], core: int) -> None:
  if sys.platform != "win32":
    return
  if process.pid is None:
    return
  kernel32 = ctypes.windll.kernel32
  handle = kernel32.OpenProcess(0x0200, False, process.pid)
  if not handle:
    return
  try:
    affinity_mask = 1 << core
    if kernel32.SetProcessAffinityMask(handle, affinity_mask) == 0:
      raise OSError(f"SetProcessAffinityMask failed for core {core}")
  finally:
    kernel32.CloseHandle(handle)


def launch_bench_with_priority_and_affinity(
  exe: Path,
  args: list[str],
  cwd: Path,
  affinity_core: int = DEFAULT_BENCH_AFFINITY_CORE,
) -> None:
  if sys.platform == "win32":
    process = subprocess.Popen(
      [str(exe), *args],
      cwd=cwd,
      creationflags=HIGH_PRIORITY_CLASS,
    )
    _set_process_affinity(process, affinity_core)
    return_code = process.wait()
    if return_code != 0:
      raise subprocess.CalledProcessError(return_code, [str(exe), *args])
    return

  subprocess.run([str(exe), *args], cwd=cwd, check=True)


def run_benchmarks(root_dir: Path, commit_dir: Path) -> None:
  with set_windows_high_perf_power_plan():
    for index, (result_name, preset_name) in enumerate(BENCHMARK_RUNS):
      results_cwd = commit_dir / result_name
      exe = auxid_bench_exe(root_dir, preset_name)
      if not exe.is_file():
        raise FileNotFoundError(f"AuxidBench not found: {exe}")
      launch_bench_with_priority_and_affinity(exe, BENCHMARK_ARGS, results_cwd)
      if index + 1 < len(BENCHMARK_RUNS):
        time.sleep(BENCH_COOLDOWN_SECONDS)


def main(args: list[str]):
  benchmark_dir = Path(__file__).resolve().parent
  root_dir = benchmark_dir.parent

  os.chdir(root_dir)

  commit_dir, commit_id = setup_results_tree(benchmark_dir, root_dir)

  vcvars64: Path | None = None
  try:
    vcvars64 = find_vcvars64()
  except FileNotFoundError:
    vcvars64 = None

  powerplan_before = _get_active_power_plan_guid()
  powerplan_during = HIGH_PERFORMANCE_POWER_PLAN
  write_meta_json(
    commit_dir,
    commit_id,
    root_dir,
    vcvars64,
    powerplan_before,
    powerplan_during,
  )

  build_benchmarks(root_dir)

  run_benchmarks(root_dir, commit_dir)


if __name__ == "__main__":
  main(sys.argv)
