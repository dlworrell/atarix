#!/bin/sh
set -eu

if [ "$#" -ne 2 ]; then
    echo "usage: $0 TOP_SRCDIR TOP_BUILDDIR" >&2
    exit 64
fi

top_srcdir=$1
top_builddir=$2
capture_dir="$top_srcdir/evidence/atx-bib-001"
expected_output="$top_srcdir/docs/evidence/atx-bib-001/emulator-capture-v1.txt"
decoder="$top_builddir/tools/atarix-bibdump"
tmp_dir=${TMPDIR:-/tmp}/atarix-bib-evidence.$$
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM
mkdir -p "$tmp_dir"

base64 -d "$capture_dir/emulator-capture-v1.b64" > "$tmp_dir/emulator-capture-v1.bin"
(
    cd "$tmp_dir"
    sha256sum -c "$capture_dir/emulator-capture-v1.sha256"
)
"$decoder" "$tmp_dir/emulator-capture-v1.bin" > "$tmp_dir/emulator-capture-v1.txt"
diff -u "$expected_output" "$tmp_dir/emulator-capture-v1.txt"
