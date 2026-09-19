#!/usr/bin/env bash
# detect-sanitizers.sh — Identify ASan / UBSan / MSan from a binary's symbol table & dynamic deps.
# Usage: detect-sanitizers.sh <path-to-binary-or-shared-object>

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <binary|shared object>" >&2
  exit 1
fi

BIN="$1"
if [[ ! -r "$BIN" ]]; then
  echo "error: cannot read '$BIN'" >&2
  exit 2
fi

have() { command -v "$1" >/dev/null 2>&1; }

# Collect dynamic dependencies (DT_NEEDED) via readelf or objdump
deps=""
if have readelf; then
  deps="$(readelf -d "$BIN" 2>/dev/null | awk '/NEEDED/{print $5}' | tr -d '[]' || true)"
elif have objdump; then
  # On some BSDs, -p shows NEEDED
  deps="$(objdump -p "$BIN" 2>/dev/null | awk '/NEEDED/{print $2}' || true)"
fi

# Gather symbol names from dynamic table first, then full table, then strings fallback
symdump() {
  if have nm; then
    # dynamic symbols (works on stripped DSOs)
    nm -D -g "$BIN" 2>/dev/null | awk '{print $3}'
    # full table if available (static links or non-DSO)
    nm -a "$BIN" 2>/dev/null | awk '{print $3}'
  fi
  if have objdump; then
    objdump -t "$BIN" 2>/dev/null | awk '{print $6}'
    objdump -T "$BIN" 2>/dev/null | awk '{print $6}'
  fi
  # last resort: strings (helps for fully stripped static builds)
  if have strings; then
    strings -a "$BIN" 2>/dev/null
  fi
}

syms="$(symdump | sort -u)"

detected=()
evidence=()

# Heuristics per sanitizer
check_asan() {
  local hit=0
  # libraries
  if echo "$deps" | grep -Eiq 'libasan|asan\.dll|asan\.dylib'; then
    hit=1; evidence+=("DT_NEEDED: $(echo "$deps" | tr '\n' ' ' | sed 's/^ *//;s/ *$//')")
  fi
  # symbols (common ones across compiler-rt / glibc / *BSD)
  if echo "$syms" | grep -Eq '__asan_(init|report_|version_mismatch|register_globals|unpoison|poison)'; then
    hit=1; evidence+=("Symbol(s): $(echo "$syms" | grep -E '__asan_(init|report_|version_mismatch|register_globals|unpoison|poison)' | head -n 3 | tr '\n' ' ')")
  fi
  # coverage often appears together (not definitive, so don’t mark alone)
  if [[ $hit -eq 1 ]]; then detected+=("ASAN"); fi
}

check_ubsan() {
  local hit=0
  if echo "$deps" | grep -Eiq 'libubsan|ubsan\.dll|ubsan\.dylib'; then
    hit=1; evidence+=("DT_NEEDED: $(echo "$deps" | tr '\n' ' ' | sed 's/^ *//;s/ *$//')")
  fi
  if echo "$syms" | grep -Eq '__ubsan_(init|on_report|handle_|handle_.*_v1)'; then
    hit=1; evidence+=("Symbol(s): $(echo "$syms" | grep -E '__ubsan_(init|on_report|handle_|handle_.*_v1)' | head -n 3 | tr '\n' ' ')")
  fi
  if [[ $hit -eq 1 ]]; then detected+=("UBSAN"); fi
}

check_msan() {
  local hit=0
  if echo "$deps" | grep -Eiq 'libmsan|msan\.dll|msan\.dylib'; then
    hit=1; evidence+=("DT_NEEDED: $(echo "$deps" | tr '\n' ' ' | sed 's/^ *//;s/ *$//')")
  fi
  if echo "$syms" | grep -Eq '__msan_(init|warning|poison|unpoison|report|chain_origin|set_allocated_memory)'; then
    hit=1; evidence+=("Symbol(s): $(echo "$syms" | grep -E '__msan_(init|warning|poison|unpoison|report|chain_origin|set_allocated_memory)' | head -n 3 | tr '\n' ' ')")
  fi
  if [[ $hit -eq 1 ]]; then detected+=("MSAN"); fi
}

check_asan
check_ubsan
check_msan

# Print results
if [[ ${#detected[@]} -eq 0 ]]; then
  echo "No sanitizers confidently detected."
  echo "Notes:"
  echo "  • Fully stripped static binaries may hide symbols; try an unstripped build."
  echo "  • If you only see __sanitizer_cov_* without others, that indicates coverage, not a specific sanitizer."
  exit 0
fi

printf "Detected: %s\n" "$(IFS=,; echo "${detected[*]}")"
printf "Evidence:\n"
for e in "${evidence[@]}"; do
  printf "  - %s\n" "$e"
done

