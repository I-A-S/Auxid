#!/usr/bin/python3
"""Aggregate Google Benchmark JSON results into a single CSV summary."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

# Must match directory names under benchmarks/results/<commit>/ in run.py.
COMPILERS: tuple[tuple[str, str], ...] = (
    ("msvc", "MSVC"),
    ("msvc-pgo", "MSVC-PGO"),
    ("clang", "Clang"),
    ("gcc", "GCC"),
)

AGGREGATES: tuple[str, ...] = ("median", "mean", "stddev", "cv")

TIME_AGGREGATES: frozenset[str] = frozenset({"median", "mean", "stddev"})


def _format_time_ns(cpu_time_ns: float) -> str:
    """Convert nanoseconds to microseconds, rounded to 2 decimal places."""
    return f"{cpu_time_ns / 1000.0:.2f}"


def _format_cv(cpu_time: float) -> str:
    """Convert CV ratio to percentage, rounded to 2 decimal places."""
    return f"{cpu_time * 100.0:.2f}"


def _metric_value(entry: dict) -> float:
    """Use cpu_time; fall back to real_time when cpu_time is zero."""
    cpu_time = entry.get("cpu_time", 0.0)
    if cpu_time != 0.0:
        return float(cpu_time)
    return float(entry.get("real_time", 0.0))


def load_bench_results(json_path: Path) -> dict[str, dict[str, float]]:
    """Return run_name -> aggregate_name -> metric value for one results file."""
    with json_path.open(encoding="utf-8") as handle:
        payload = json.load(handle)

    by_run: dict[str, dict[str, float]] = {}
    for bench in payload.get("benchmarks", []):
        if bench.get("run_type") != "aggregate":
            continue
        aggregate = bench.get("aggregate_name")
        if aggregate not in AGGREGATES:
            continue
        run_name = bench.get("run_name")
        if not run_name:
            continue
        by_run.setdefault(run_name, {})[aggregate] = _metric_value(bench)
    return by_run


def find_bench_result_files(results_dir: Path) -> list[tuple[str, Path]]:
    """Return (compiler_dir_name, path) for each bench_results.json under results_dir."""
    found: list[tuple[str, Path]] = []
    for json_path in sorted(results_dir.rglob("bench_results.json")):
        compiler_key = json_path.parent.name.lower()
        found.append((compiler_key, json_path))
    return found


def build_table(
    results_dir: Path,
) -> tuple[list[str], dict[str, dict[str, dict[str, float]]]]:
    """
    Load all compiler JSON files and merge into:
      run_name -> compiler_key -> aggregate_name -> value
    """
    compiler_keys = {key for key, _ in COMPILERS}
    table: dict[str, dict[str, dict[str, float]]] = {}

    for compiler_key, json_path in find_bench_result_files(results_dir):
        if compiler_key not in compiler_keys:
            continue
        for run_name, aggregates in load_bench_results(json_path).items():
            table.setdefault(run_name, {})[compiler_key] = aggregates

    run_names = sorted(table.keys())
    return run_names, table


def column_headers() -> list[str]:
    headers = ["Benchmark"]
    for _, label in COMPILERS:
        headers.extend(
            [
                f"{label} Median (us)",
                f"{label} Mean (us)",
                f"{label} StdDev (us)",
                f"{label} CV (%)",
            ]
        )
    return headers


def format_cell(compiler_data: dict[str, float] | None, aggregate: str) -> str:
    if not compiler_data or aggregate not in compiler_data:
        return ""
    value = compiler_data[aggregate]
    if aggregate in TIME_AGGREGATES:
        return _format_time_ns(value)
    return _format_cv(value)


def write_csv(
    output_path: Path,
    run_names: list[str],
    table: dict[str, dict[str, dict[str, float]]],
) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(column_headers())
        for run_name in run_names:
            row = [run_name]
            compilers = table.get(run_name, {})
            for compiler_key, _ in COMPILERS:
                data = compilers.get(compiler_key)
                for aggregate in AGGREGATES:
                    row.append(format_cell(data, aggregate))
            writer.writerow(row)


def main() -> int:
    benchmark_dir = Path(__file__).resolve().parent
    default_results = benchmark_dir / "results"
    default_output = default_results / "summary.csv"

    parser = argparse.ArgumentParser(
        description="Generate a CSV summary from benchmark JSON results.",
    )
    parser.add_argument(
        "--results-dir",
        type=Path,
        default=default_results,
        help=f"Directory containing benchmark results (default: {default_results})",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=default_output,
        help=f"Output CSV path (default: {default_output})",
    )
    args = parser.parse_args()

    results_dir: Path = args.results_dir
    if not results_dir.is_dir():
        print(f"Results directory not found: {results_dir}", file=__import__("sys").stderr)
        return 1

    run_names, table = build_table(results_dir)
    if not run_names:
        print(f"No benchmark data found under {results_dir}", file=__import__("sys").stderr)
        return 1

    write_csv(args.output, run_names, table)
    print(f"Wrote {len(run_names)} rows to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
