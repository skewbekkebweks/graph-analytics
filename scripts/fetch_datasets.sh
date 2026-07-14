#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIR="$ROOT/data/snap"
mkdir -p "$DIR"

url_for() {
    case "$1" in
        web-Google)       echo "https://snap.stanford.edu/data/web-Google.txt.gz" ;;
        soc-LiveJournal1) echo "https://snap.stanford.edu/data/soc-LiveJournal1.txt.gz" ;;
        wiki-Talk)        echo "https://snap.stanford.edu/data/wiki-Talk.txt.gz" ;;
        cit-Patents)      echo "https://snap.stanford.edu/data/cit-Patents.txt.gz" ;;
        *)                echo "" ;;
    esac
}

download() {
    if command -v curl >/dev/null 2>&1; then
        curl -fL "$1" -o "$2"
    else
        wget -O "$2" "$1"
    fi
}

fetch() {
    local name="$1"
    local url
    url="$(url_for "$name")"
    if [ -z "$url" ]; then
        echo "unknown dataset: $name" >&2
        exit 1
    fi
    local gz="$DIR/$name.txt.gz"
    local csv="$DIR/$name.csv"
    if [ ! -f "$gz" ]; then
        echo "downloading $name ..."
        download "$url" "$gz"
    fi
    echo "converting $name -> $csv"
    gzip -dc "$gz" | grep -v '^#' | awk 'BEGIN { print "from,to" } { print $1 "," $2 }' > "$csv"
    echo "done: $csv"
}

if [ "$#" -eq 0 ]; then
    set -- web-Google
fi
for d in "$@"; do
    fetch "$d"
done
