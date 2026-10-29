#!/usr/bin/env python3
"""Turns a render-test TSV into tools/levels.json (per-preset volume trims)."""
import json, math, os, sys, re
HERE = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(HERE, '..', 'Source', 'Presets.h')).read()
rows = [l.rstrip('\n').split('\t') for l in open(sys.argv[1]) if l.strip()]
TARGET = {'Bass': -14.0, 'UKG Bass': -14.0, 'Acid': -14.5, 'Pads': -17.0, 'FX': -17.0, 'Chords': -16.0}
out = {}
for idx, cat, name, peak, rms, bad, tail in rows:
    peak, rms = float(peak), float(rms)
    m = re.search(r'\{ "%s", "%s", "([^"]*)" \}' % (re.escape(name), re.escape(cat)), src)
    data = dict(kv.split('=') for kv in m.group(1).split(';') if kv) if m else {}
    cur = float(data.get('vol', 0))
    slow = float(data.get('aa', 0.003)) > 1.0 or float(data.get('fa', 0.003)) > 2.0
    trim = TARGET.get(cat, -15.5) - rms
    trim = min(trim, 20 * math.log10(0.8 / max(peak, 1e-6)))   # keep peaks under about -2 dBFS
    trim = max(-15.0, min(3.0 if slow else 10.0, trim))
    out[cat + '/' + name] = round(cur + trim, 2)
json.dump(out, open(os.path.join(HERE, 'levels.json'), 'w'), indent=0)
print(len(out), 'trims; range', min(out.values()), max(out.values()))
