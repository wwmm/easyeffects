#!/usr/bin/env bash
# check-kcfg-numeric-defaults.sh
# Purpose: scan src/contents/kcfg/*.kcfg and report numeric entries whose <default>
#          is out of [<min>,<max>] (inclusive).
# Only numeric types checked: Int, UInt, Double, Int64, UInt64 (case-insensitive).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
KCFG_DIR="$REPO_ROOT/src/contents/kcfg"

if [ ! -d "$KCFG_DIR" ]; then
  echo "Error: expected directory '$KCFG_DIR' not found." >&2
  exit 2
fi

# Collect .kcfg files
mapfile -d '' files < <(find "$KCFG_DIR" -type f -name '*.kcfg' -print0)

if [ "${#files[@]}" -eq 0 ]; then
  echo "No .kcfg files found under $KCFG_DIR"
  exit 0
fi

echo "Checking ${#files[@]} .kcfg file(s) under: $KCFG_DIR"
for f in "${files[@]}"; do
  echo " - $(realpath --relative-to="$REPO_ROOT" "$f")"
done

# Embedded Python: robust XML parsing + namespace handling + type filtering
python3 - "$REPO_ROOT" "${files[@]}" <<'PY'
import sys, re
from pathlib import Path
import xml.etree.ElementTree as ET

repo_root = Path(sys.argv[1])
files = sys.argv[2:]

# allowed numeric types (lowercase)
allowed = {"int", "uint", "double", "int64", "uint64"}

problems = []
parse_errors = []

for f in files:
    path = Path(f)
    print(f"\nProcessing: {path.relative_to(repo_root)}")
    try:
        tree = ET.parse(path)
        root = tree.getroot()
    except Exception as e:
        parse_errors.append((str(path), str(e)))
        print(f"  XML parse error: {e}")
        continue

    # detect default namespace (if any)
    ns = ''
    m = re.match(r"\{(.+)\}", root.tag)
    if m:
        ns = m.group(1)
        print(f"  Detected default namespace: {ns}")

    # helper to find entries with namespace if present
    def findall_entries(r):
        if ns:
            return r.findall('.//{' + ns + '}entry')
        else:
            return r.findall('.//entry')

    def text_or_none(elem, tag):
        node = elem.find(tag)
        if node is None and ns:
            node = elem.find('{' + ns + '}' + tag)
        if node is None or node.text is None:
            return None
        return node.text.strip()

    try:
        raw_text = path.read_text(encoding='utf-8')
    except Exception:
        raw_text = path.read_text(encoding='latin-1')

    entries = findall_entries(root)
    print(f"  Entries found by parser: {len(entries)}")

    for entry in entries:
        name = entry.attrib.get('name', '<unnamed>')
        typ = entry.attrib.get('type', '')
        typ_l = typ.lower() if typ else ''
        # skip non-numeric types
        if typ_l not in allowed:
            # uncomment for very verbose debugging:
            # print(f"    Skipping entry {name!r} with non-numeric type {typ!r}")
            continue

        min_text = text_or_none(entry, 'min')
        max_text = text_or_none(entry, 'max')
        default_text = text_or_none(entry, 'default')

        print(f"    Checking numeric entry: name={name!r}, type={typ!r}, min={min_text!r}, max={max_text!r}, default={default_text!r}")

        if default_text is None:
            continue

        # snippet extraction (try to capture original block)
        snippet = None
        try:
            pat = re.compile(r'<entry\b[^>]*\bname\s*=\s*[\"\']%s[\"\'][^>]*>.*?</entry>' % re.escape(name), re.DOTALL | re.IGNORECASE)
            m = pat.search(raw_text)
            if m:
                snippet = m.group(0).strip()
            else:
                snippet = ET.tostring(entry, encoding='unicode').strip()
        except Exception as e:
            snippet = f"(snippet extraction failed: {e})"

        def to_number(s):
            try:
                return float(s)
            except Exception:
                s2 = s.replace(',', '.')
                return float(s2)

        try:
            default_val = to_number(default_text)
        except Exception:
            problems.append({'file': str(path.relative_to(repo_root)), 'name': name, 'type': typ, 'min': min_text, 'max': max_text, 'default': default_text, 'issue': 'default-not-numeric', 'snippet': snippet})
            print(f"      -> problem: default not numeric: {default_text!r}")
            continue

        if min_text is None and max_text is None:
            # nothing to check
            continue

        if min_text is None or max_text is None:
            try:
                if min_text is not None:
                    min_val = to_number(min_text)
                    if default_val < min_val:
                        problems.append({'file': str(path.relative_to(repo_root)), 'name': name, 'type': typ, 'min': min_text, 'max': max_text, 'default': default_text, 'issue': 'default-less-than-min', 'snippet': snippet})
                        print(f"      -> problem: default {default_val} < min {min_val}")
                if max_text is not None:
                    max_val = to_number(max_text)
                    if default_val > max_val:
                        problems.append({'file': str(path.relative_to(repo_root)), 'name': name, 'type': typ, 'min': min_text, 'max': max_text, 'default': default_text, 'issue': 'default-greater-than-max', 'snippet': snippet})
                        print(f"      -> problem: default {default_val} > max {max_val}")
            except Exception as e:
                problems.append({'file': str(path.relative_to(repo_root)), 'name': name, 'type': typ, 'min': min_text, 'max': max_text, 'default': default_text, 'issue': 'bound-not-numeric', 'snippet': snippet})
                print(f"      -> problem parsing bound: {e}")
            continue

        try:
            min_val = to_number(min_text)
            max_val = to_number(max_text)
        except Exception as e:
            problems.append({'file': str(path.relative_to(repo_root)), 'name': name, 'type': typ, 'min': min_text, 'max': max_text, 'default': default_text, 'issue': 'bound-not-numeric', 'snippet': snippet})
            print(f"      -> problem parsing min/max: {e}")
            continue

        if min_val > max_val:
            problems.append({'file': str(path.relative_to(repo_root)), 'name': name, 'type': typ, 'min': min_text, 'max': max_text, 'default': default_text, 'issue': 'min-greater-than-max', 'snippet': snippet})
            print(f"      -> problem: min {min_val} > max {max_val}")
            continue

        if not (min_val <= default_val <= max_val):
            problems.append({'file': str(path.relative_to(repo_root)), 'name': name, 'type': typ, 'min': min_text, 'max': max_text, 'default': default_text, 'issue': 'default-out-of-range', 'snippet': snippet})
            print(f"      -> problem: default {default_val} OUTSIDE [{min_val}, {max_val}]")

# final reporting
if parse_errors:
    print('\nPARSE ERRORS:')
    for p,e in parse_errors:
        print(f'  - {p}: {e}')

if problems:
    print('\nPROBLEMS FOUND:')
    for pr in problems:
        f=pr['file']; n=pr['name']; t=pr['type']; mn=pr['min']; mx=pr['max']; df=pr['default']; issue=pr['issue']; snippet=pr.get('snippet')
        if issue=='default-out-of-range':
            print(f"{f}: entry '{n}' (type={t}) -> default={df} is OUTSIDE [{mn}, {mx}]")
        elif issue=='default-less-than-min':
            print(f"{f}: entry '{n}' (type={t}) -> default={df} < min={mn}")
        elif issue=='default-greater-than-max':
            print(f"{f}: entry '{n}' (type={t}) -> default={df} > max={mx}")
        elif issue=='min-greater-than-max':
            print(f"{f}: entry '{n}' (type={t}) -> min={mn} > max={mx} (invalid bounds)")
        elif issue=='bound-not-numeric':
            print(f"{f}: entry '{n}' (type={t}) -> min/max not numeric (min='{mn}' max='{mx}')")
        elif issue=='default-not-numeric':
            print(f"{f}: entry '{n}' (type={t}) -> default not numeric: '{df}'")
        else:
            print(f"{f}: entry '{n}' (type={t}) -> issue: {issue}")
        if snippet:
            print('Original XML snippet:')
            print('---')
            print(snippet)
            print('---')
        print()
    print(f"Total problematic entries: {len(problems)}")
    sys.exit(1)
else:
    print('\nOK: all checked defaults are within their defined ranges (where min/max/default were present).')
    sys.exit(0)
PY

