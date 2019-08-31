#!/bin/bash

set -e

prefix=$(which afl-fuzz)
prefix=${prefix/\/bin\/afl-fuzz}
dictionary="${prefix}/share/afl/html_tags.dict"
dir=$(dirname "$0")
output="${dir}/afl_output"
afl_wrapper="${dir}/../build/afl_wrapper"

inputdir="${dir}/afl"

if [[ $# = 1 && "$1" = '-r' ]]; then
  inputdir=-
fi

pid=()

cleanup() {
  for pid in "${pid[@]}"; do
    kill -SIGINT "$pid" 2>/dev/null || true
  done
}

trap "cleanup" EXIT

for i in {1..3}; do
  UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 afl-fuzz -x "${dictionary}" -i "${inputdir}" -o "${output}" -S "fuzzer${i}" -- "${afl_wrapper}" @@ >/dev/null 2>/dev/null &
  pid["$i"]=$!
done

UBSAN_OPTIONS=halt_on_error=1:abort_on_error=1 afl-fuzz -x "${dictionary}" -i "${inputdir}" -o "${output}" -M fuzzer0 -- "${afl_wrapper}" @@
