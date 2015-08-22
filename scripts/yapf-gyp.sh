#!/usr/bin/env bash

function cleanup() {
  if [[ -n "${tmp_file:-}" && -e  "${tmp_file}" ]]; then
    rm -vf "${tmp_file}"
    unset tmp_file
  fi
}

trap "cleanup" EXIT
ROOT=`git rev-parse --show-toplevel`
for i in $ROOT/*.gyp $ROOT/gyp/*.gyp $ROOT/gyp/*.gypi; do
  tmp_file="$(mktemp -t "$(basename "$i").XXXXXXXXXX.py")"
  cp -v "$i" "$tmp_file"
  yapf "$tmp_file" > "$i"
  cleanup
done
