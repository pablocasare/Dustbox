#!/usr/bin/env python3
"""Generates Source/Presets.h for Serpent.

Hand-built archetypes (the sound families of modern tech house / UKG / minimal)
are crossed with musical modifiers (Dub, Dusty, Pumped, Gated, Wobble, Psych...)
so every preset is a real, deliberate variation rather than random noise.

Optional: tools/levels.json (written by the render test) holds a per-preset
volume trim so every preset hits the same loudness.
"""
import json, math, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, '..', 'Source')

# ---------------------------------------------------------------- parse params
txt = open(os.path.join(SRC, 'Params.h')).read()
PAR = {}
for m in re.finditer(r'X\((\w+),\s*"([^"]*)",\s*(\w+),\s*([-\d.f]+),\s*([-\d.f]+),\s*([-\d.f]+),\s*([-\d.f]+),\s*"([^"]*)"\)', txt):
    pid, name, typ, mn, mx, de, sk, ch = m.groups()
    f = lambda s: float(s.rstrip('f'))
    PAR[pid] = dict(type=typ, mn=f(mn), mx=f(mx), de=f(de), ch=ch.split('|') if ch else [])
assert 'vol' in PAR and 'g15' in PAR, 'param parse failed'

def norm(pid, v):
    d = PAR[pid]
    if isinstance(v, str):
        v = d['ch'].index(v)
    if isinstance(v, bool):
        v = 1 if v else 0
    v = max(d['mn'], min(d['mx'], float(v)))
    if d['type'] in ('Int', 'Choice', 'Bool'):
        v = int(round(v))
    return v

# ---------------------------------------------------------------- archetypes
# each: (name, {params}); categories get default XY targets
ENV_PLUCK = dict(aa=0.002, ad=0.25, as_=0.0, ar=0.15)
def P(**kw):
    if 'as_' in kw: kw['as'] = kw.pop('as_')
    return kw

GATES = {
    'Roller':   '1011101110111011',
    'Garage':   '1101101101101101',
    'Eighths':  '1010101010101010',
    'Skip':     '1110111011101110',
    'Dotted':   '1001001001001001',
    'Stutter':  '1111011111110110',
    'Broken':   '1011011010110110',
    'Offbeat':  '0010001000100010',
}
def gate(pat, div='1/16', depth=1.0, sm=0.15):
    d = dict(gate_on=1, gate_div=div, gate_dep=depth, gate_sm=sm)
    for i, c in enumerate(GATES[pat]):
        d['g%d' % i] = int(c)
    return d

CATS = {}
def cat(name, xy, items):
    CATS[name] = dict(xy=xy, items=items)

MONO_BASS = dict(mono=1, width=1.0, spread=0.0)

cat('Bass', ('Cutoff', 'Drive'), [
    ('Rolling Sub',        P(a_wave=0.3, a_oct=-1, sub=0.5, cutoff=900, fenv=0.25, fd=0.18, fs=0.3, ad=0.3, as_=0.7, ar=0.08, glide=0.03, fkey=0.3, **MONO_BASS)),
    ('Basement Roller',    P(a_wave=2, a_oct=-1, cutoff=300, reso=0.35, fenv=0.45, fd=0.15, fs=0.1, sub=0.4, fdrive=0.3, ad=0.25, as_=0.6, ar=0.06, **MONO_BASS)),
    ('Rubber Bass',        P(a_wave=3.2, a_oct=-1, cutoff=250, reso=0.5, fenv=0.55, fd=0.12, fs=0.0, ad=0.3, as_=0.4, ar=0.05, penv=2, pdec=0.02, **MONO_BASS)),
    ('Organ Bass',         P(a_wave=5, a_oct=-1, cutoff=2500, fenv=0.1, ad=0.4, as_=0.8, ar=0.06, sub=0.3, **MONO_BASS)),
    ('Digi Bass',          P(a_wave=6, a_oct=-1, cutoff=1800, fenv=0.2, fd=0.2, fs=0.2, ad=0.3, as_=0.6, ar=0.06, sub=0.25, **MONO_BASS)),
    ('FM Thump',           P(a_wave=0, b_wave=0, b_oct=1, fm=0.3, a_oct=-1, cutoff=4000, ad=0.2, as_=0.5, ar=0.06, sub=0.3, penv=3, pdec=0.015, **MONO_BASS)),
    ('Tok Bass',           P(a_wave=0.6, a_oct=-1, penv=12, pdec=0.015, ad=0.12, as_=0.0, ar=0.04, cutoff=1200, sub=0.3, **MONO_BASS)),
    ('Drive Bass',         P(a_wave=2, b_wave=2, b_lvl=0.6, b_fine=8, a_oct=-1, b_oct=-1, cutoff=400, reso=0.55, fdrive=0.5, fenv=0.5, fd=0.2, fs=0.2, dist=0.2, ad=0.3, as_=0.7, ar=0.07, **MONO_BASS)),
    ('Wonky Bass',         P(a_wave=3, a_oct=-1, glide=0.09, cutoff=600, fenv=0.35, fd=0.2, fs=0.2, ad=0.3, as_=0.6, ar=0.06, sub=0.3, **MONO_BASS)),
    ('Blip Bass',          P(a_wave=1, a_oct=-1, cutoff=1500, ad=0.09, as_=0.0, ar=0.03, fenv=0.3, fd=0.05, sub=0.4, **MONO_BASS)),
    ('Sub Boom',           P(a_wave=0, a_oct=-1, penv=7, pdec=0.04, ad=1.5, as_=0.0, ar=0.2, dist=0.15, **MONO_BASS)),
    ('Plucky Square Bass', P(a_wave=3, a_oct=-1, cutoff=500, reso=0.25, fenv=0.4, fd=0.09, fs=0.0, ad=0.2, as_=0.3, ar=0.05, sub=0.45, **MONO_BASS)),
    ('Minimal Knock',      P(a_wave=1.4, a_oct=-1, cutoff=900, reso=0.6, fenv=0.3, fd=0.06, ad=0.14, as_=0.0, ar=0.04, penv=5, pdec=0.01, sub=0.35, **MONO_BASS)),
    ('Hollow Bass',        P(a_wave=6.4, a_oct=-1, cutoff=1200, ftype='LP 12', fenv=0.2, fd=0.2, ad=0.35, as_=0.6, ar=0.07, sub=0.4, **MONO_BASS)),
])

cat('UKG Bass', ('Cutoff', 'Drive'), [
    ('Speed Garage Reese', P(a_wave=2, b_wave=2, b_lvl=0.8, b_fine=18, uni=3, detune=0.25, a_oct=-1, b_oct=-1, cutoff=700, reso=0.3, l_dest='Cutoff', l_amt=0.25, l_div='1/8', sub=0.5, glide=0.04, as_=0.9, ar=0.08, **MONO_BASS)),
    ('Wobble Garage',      P(a_wave=2.5, b_lvl=0.6, a_oct=-1, b_oct=-1, cutoff=450, reso=0.45, l_amt=0.45, l_div='1/8T', l_dest='Cutoff', fdrive=0.3, sub=0.4, as_=0.9, ar=0.08, **MONO_BASS)),
    ('2-Step Hoover',      P(a_wave=4, uni=5, detune=0.45, b_lvl=0.5, b_oct=-2, a_oct=-1, cutoff=1500, glide=0.06, sub=0.35, as_=0.9, ar=0.1, mono=1)),
    ('Bounce Organ Bass',  P(a_wave=5.4, cutoff=3000, ad=0.25, as_=0.5, ar=0.06, a_oct=-1, sub=0.35, penv=1, pdec=0.02, **MONO_BASS)),
    ('Growl Bass',         P(a_wave=3.5, fm=0.25, b_wave=0, b_oct=-1, a_oct=-1, cutoff=900, reso=0.4, l_dest='Wave A', l_amt=0.35, l_div='1/16', dist=0.25, sub=0.4, ar=0.07, **MONO_BASS)),
    ('Dark Garage Sine',   P(a_wave=0.2, a_oct=-1, sub=0.3, dist=0.25, ad=0.5, as_=0.6, ar=0.08, glide=0.05, **MONO_BASS)),
    ('Vox Bass',           P(a_wave=7, a_oct=-1, cutoff=2000, sub=0.4, ad=0.3, as_=0.7, ar=0.07, l_dest='Wave A', l_amt=0.1, l_div='1/4', **MONO_BASS)),
    ('Skippy Bass',        P(a_wave=3, a_oct=-1, cutoff=700, fenv=0.45, fd=0.1, ad=0.15, as_=0.2, ar=0.05, sub=0.5, glide=0.025, **MONO_BASS)),
    ('Warehouse Reese',    P(a_wave=2, b_wave=2.2, b_lvl=0.9, b_fine=-22, uni=2, detune=0.3, a_oct=-1, b_oct=-1, cutoff=600, reso=0.2, fdrive=0.4, dist=0.15, sub=0.45, as_=1, ar=0.1, **MONO_BASS)),
    ('Talking Bass',       P(a_wave=6.6, a_oct=-1, ftype='Band', cutoff=700, reso=0.55, l_dest='Cutoff', l_amt=0.4, l_div='1/16', l_shape='S&H', sub=0.5, as_=0.9, ar=0.06, **MONO_BASS)),
])

cat('Organ & Stabs', ('Cutoff', 'Reverb'), [
    ('Classic House Organ', P(a_wave=5, chord='Minor 7', cutoff=6000, ad=0.35, as_=0.3, ar=0.15, rev_mix=0.15)),
    ('Digi Organ Stab',     P(a_wave=6, chord='Minor 9', ad=0.25, as_=0.15, ar=0.12, fenv=0.2, fd=0.15, cutoff=4000, rev_mix=0.2, dly_mix=0.15)),
    ('Detroit Stab',        P(a_wave=2, b_wave=3, b_lvl=0.5, chord='Minor 7', cutoff=1500, reso=0.3, fenv=0.45, fd=0.2, fs=0.05, ad=0.3, as_=0.0, ar=0.2, dly_mix=0.25, rev_mix=0.25)),
    ('Dub Chord',           P(a_wave=2.3, chord='Dub', cutoff=900, fenv=0.3, fd=0.25, fs=0.0, ad=0.4, as_=0.0, ar=0.3, dly_mix=0.4, dly_fb=0.6, dly_div='3/16', rev_mix=0.35, hicut=7000, locut=150)),
    ('Rave Stab',           P(a_wave=2, uni=5, detune=0.35, chord='Minor', ad=0.2, as_=0.0, ar=0.15, cutoff=5000, fenv=0.15, fd=0.1, dist=0.2)),
    ('Keys Stab',           P(a_wave=1.5, b_wave=6, b_lvl=0.4, ad=0.6, as_=0.0, ar=0.3, fenv=0.3, fd=0.4, cutoff=2500, chord='Major 7')),
    ('Garage Organ Chop',   P(a_wave=5.6, chord='Minor 11', ad=0.2, as_=0.6, ar=0.1, cutoff=5000, **gate('Garage'))),
    ('Plucked Fourths',     P(a_wave=3, chord='Fourths', ad=0.18, as_=0.0, ar=0.12, fenv=0.5, fd=0.1, cutoff=600, dly_mix=0.25)),
    ('Hollow Stab',         P(a_wave=6.5, chord='Sus 2', ad=0.3, as_=0.0, ar=0.15, cutoff=3500, rev_mix=0.2)),
    ('Tape Stab',           P(a_wave=2, chord='Minor 9', cho_mix=0.5, dist=0.1, hicut=6000, ad=0.4, as_=0.1, ar=0.2, cutoff=3000, fenv=0.2, fd=0.2)),
    ('Bright Organ Hit',    P(a_wave=5.2, b_wave=6, b_lvl=0.35, b_oct=1, chord='Minor', ad=0.22, as_=0.0, ar=0.1, cutoff=8000, penv=0.5, pdec=0.01)),
    ('Jazz Stab',           P(a_wave=1.2, chord='Major 9', cutoff=2200, fenv=0.25, fd=0.25, ad=0.5, as_=0.0, ar=0.25, cho_mix=0.3, rev_mix=0.2)),
    ('Minimal Chord Blip',  P(a_wave=0.8, chord='Minor 7', ad=0.08, as_=0.0, ar=0.05, cutoff=3000, dly_mix=0.35, dly_div='3/16', dly_fb=0.45)),
    ('Vox Stab',            P(a_wave=7, chord='Minor 7', ad=0.3, as_=0.1, ar=0.15, cutoff=4000, rev_mix=0.2)),
])

cat('Chords', ('Cutoff', 'Reverb'), [
    ('Lush Minor 9',      P(a_wave=2.4, b_wave=1, b_lvl=0.4, chord='Minor 9', aa=0.02, ad=0.8, as_=0.6, ar=0.5, cutoff=2200, cho_mix=0.35, rev_mix=0.3)),
    ('Deep Sus Chord',    P(a_wave=1.6, chord='Sus 4', aa=0.05, ad=1, as_=0.5, ar=0.6, cutoff=1500, rev_mix=0.35, dly_mix=0.2)),
    ('Filtered Major 7',  P(a_wave=2, chord='Major 7', cutoff=900, reso=0.35, l_dest='Cutoff', l_amt=0.25, l_div='1 bar', ad=1, as_=0.7, ar=0.4, rev_mix=0.25)),
    ('Organ Chord',       P(a_wave=5.3, chord='Minor 7', aa=0.01, as_=0.9, ar=0.2, cutoff=4500, cho_mix=0.3, rev_mix=0.2)),
    ('Supersaw Chord',    P(a_wave=2, uni=7, detune=0.4, spread=1, chord='Minor', aa=0.01, as_=0.8, ar=0.3, cutoff=5000, rev_mix=0.25)),
    ('Minor 11 Wash',     P(a_wave=1.8, chord='Minor 11', aa=0.3, ad=2, as_=0.6, ar=1.2, cutoff=2000, rev_mix=0.45, cho_mix=0.3)),
    ('Garage Chord',      P(a_wave=5.8, chord='Minor 9', ad=0.4, as_=0.2, ar=0.15, cutoff=3500, fenv=0.15, fd=0.2)),
    ('Fourths Wash',      P(a_wave=6.2, chord='Fourths', aa=0.2, ad=1.5, as_=0.5, ar=1, cutoff=3000, rev_mix=0.4, dly_mix=0.2)),
])

cat('Pads', ('Cutoff', 'Reverb'), [
    ('Warm Analog Pad',  P(a_wave=2, b_wave=2, b_lvl=0.7, b_fine=12, uni=3, aa=0.8, ad=2, as_=0.8, ar=1.5, cutoff=1500, l_dest='Cutoff', l_amt=0.15, l_div='2 bars', cho_mix=0.4, rev_mix=0.4)),
    ('Glass Pad',        P(a_wave=0.5, b_wave=6, b_lvl=0.4, b_oct=1, aa=1.2, ad=2, as_=0.8, ar=2, cutoff=6000, rev_mix=0.6)),
    ('Choir Pad',        P(a_wave=7, uni=5, detune=0.2, aa=1, as_=0.9, ar=2, cho_mix=0.3, rev_mix=0.5, l_dest='Wave A', l_amt=0.1, l_sync=0, l_rate=0.2)),
    ('Dark Drone',       P(a_wave=3, a_oct=-1, noise=0.1, cutoff=500, reso=0.4, l_amt=0.3, l_div='4 bars', l_dest='Cutoff', aa=1.5, as_=1, ar=2.5, rev_mix=0.7, rev_size=0.9)),
    ('Psych Swirl',      P(a_wave=4, l_dest='Pan', l_amt=0.7, l_div='1/4', cho_mix=0.6, rev_mix=0.5, fm=0.15, aa=0.6, ar=1.5, cutoff=3000)),
    ('Supersaw Pad',     P(a_wave=2, uni=7, detune=0.45, spread=1, aa=0.5, ar=1.2, cutoff=4000, rev_mix=0.4)),
    ('Evolving Organ',   P(a_wave=5, l_dest='Wave A', l_amt=0.25, l_div='4 bars', aa=0.6, ar=1, rev_mix=0.4, cho_mix=0.3)),
    ('Underwater',       P(a_wave=1, cutoff=600, reso=0.6, l_amt=0.35, l_dest='Cutoff', l_div='1 bar', dly_mix=0.3, rev_mix=0.6, aa=0.4, ar=1.5)),
    ('Min 9 Pad',        P(a_wave=2.5, chord='Minor 9', aa=0.4, ar=1, cutoff=2000, rev_mix=0.45, cho_mix=0.2)),
    ('Vox Shimmer',      P(a_wave=6.8, b_wave=0, b_oct=1, b_lvl=0.3, uni=3, aa=1.5, ar=2.5, rev_mix=0.65, rev_size=0.85, dly_mix=0.2)),
    ('Tape Pad',         P(a_wave=1.6, uni=2, detune=0.2, l_dest='Pitch', l_amt=0.008, l_sync=0, l_rate=0.6, hicut=6000, dist=0.08, aa=0.7, ar=1.4, rev_mix=0.35)),
    ('Breathing Pad',    P(a_wave=2.8, noise=0.08, l_dest='Volume', l_amt=0.5, l_div='1/2', aa=0.5, ar=1.2, cutoff=2500, rev_mix=0.4)),
])

cat('Keys & Bells', ('Cutoff', 'Delay'), [
    ('FM Bell',        P(a_wave=0, b_wave=0, b_oct=1, b_semi=7, fm=0.5, ad=1.2, as_=0.0, ar=1, rev_mix=0.4)),
    ('Marimba Tok',    P(a_wave=0, b_oct=2, fm=0.3, ad=0.25, as_=0.0, ar=0.2, rev_mix=0.15)),
    ('EP Keys',        P(a_wave=0, b_oct=1, fm=0.12, b_lvl=0.1, ad=1.5, as_=0.2, ar=0.4, cho_mix=0.4, l_dest='Volume', l_amt=0.15, l_div='1/8')),
    ('Kalimba',        P(a_wave=1, b_oct=2, b_semi=7, fm=0.2, ad=0.5, as_=0.0, ar=0.3, dly_mix=0.3)),
    ('Toy Piano',      P(a_wave=3.5, ad=0.4, as_=0.0, ar=0.25, cutoff=5000, fenv=0.2, fd=0.2)),
    ('Music Box',      P(a_wave=0, a_oct=1, b_oct=2, fm=0.15, ad=0.8, as_=0.0, ar=0.6, rev_mix=0.5)),
    ('Steel Drum',     P(a_wave=0, b_semi=7, fm=0.25, penv=0.5, pdec=0.02, ad=0.5, as_=0.0, ar=0.3, rev_mix=0.2)),
    ('Digi Piano',     P(a_wave=6, b_wave=1, b_lvl=0.5, ad=0.9, as_=0.0, ar=0.4, cutoff=3000, fenv=0.3, fd=0.3)),
    ('Glass Keys',     P(a_wave=0.3, b_wave=0, b_oct=2, fm=0.08, b_lvl=0.2, ad=1, as_=0.1, ar=0.5, cho_mix=0.3, rev_mix=0.3)),
    ('Dusty Rhodes',   P(a_wave=0.4, b_oct=1, fm=0.1, ad=1.2, as_=0.25, ar=0.4, l_dest='Volume', l_amt=0.12, l_div='1/8T', hicut=6000, dist=0.1)),
])

cat('Plucks', ('Cutoff', 'Delay'), [
    ('House Pluck',   P(a_wave=2, cutoff=400, reso=0.3, fenv=0.55, fd=0.12, fs=0.0, ad=0.25, as_=0.0, ar=0.15, dly_mix=0.2, rev_mix=0.2)),
    ('Minimal Blip',  P(a_wave=3, ad=0.07, as_=0.0, ar=0.05, cutoff=2500, dly_mix=0.3, dly_div='3/16')),
    ('Wood Pluck',    P(a_wave=1, penv=5, pdec=0.01, ad=0.12, as_=0.0, ar=0.08, cutoff=1500)),
    ('Glass Pluck',   P(a_wave=6, fenv=0.3, fd=0.1, cutoff=2000, ad=0.3, as_=0.0, ar=0.2, rev_mix=0.3)),
    ('Rubber Blip',   P(a_wave=0.5, penv=-7, pdec=0.03, ad=0.15, as_=0.0, ar=0.08)),
    ('Bubble Pluck',  P(a_wave=7, reso=0.6, cutoff=500, fenv=0.5, fd=0.08, ftype='Band', ad=0.2, as_=0.0, ar=0.1)),
    ('Synth Harp',    P(a_wave=1.2, uni=3, detune=0.15, ad=0.6, as_=0.0, ar=0.4, cutoff=3000, rev_mix=0.25)),
    ('Square Pluck',  P(a_wave=3, cutoff=800, fenv=0.45, fd=0.1, ad=0.2, as_=0.0, ar=0.1, dly_mix=0.25, dly_div='1/8')),
    ('Saw Pluck Wide',P(a_wave=2, uni=4, detune=0.3, spread=1, cutoff=700, fenv=0.5, fd=0.14, ad=0.25, as_=0.0, ar=0.2, rev_mix=0.2)),
    ('Clicky Organ',  P(a_wave=5, penv=12, pdec=0.004, ad=0.1, as_=0.0, ar=0.05, cutoff=5000)),
])

cat('Leads', ('Cutoff', 'FM'), [
    ('Hoover Lead',     P(a_wave=4, uni=7, detune=0.5, b_oct=-1, b_lvl=0.5, glide=0.05, cutoff=3500, cho_mix=0.3)),
    ('Square Lead',     P(a_wave=3, mono=1, glide=0.03, cutoff=3000, l_dest='Pitch', l_amt=0.015, l_sync=0, l_rate=5.5, dly_mix=0.2)),
    ('Vox Lead',        P(a_wave=7, l_dest='Wave A', l_amt=0.2, l_div='1/8', mono=1, glide=0.03, dly_mix=0.15)),
    ('Whistle',         P(a_wave=0, a_oct=1, mono=1, glide=0.05, l_dest='Pitch', l_amt=0.01, l_sync=0, l_rate=5, rev_mix=0.4, dly_mix=0.3)),
    ('Saw Lead',        P(a_wave=2, uni=3, detune=0.2, mono=1, glide=0.04, cutoff=4000, fenv=0.15)),
    ('Psy Lead',        P(a_wave=2.6, fm=0.3, l_dest='FM', l_amt=0.3, l_div='1/16', mono=1, glide=0.03, dly_mix=0.3, cutoff=5000)),
    ('Gated Supersaw',  P(a_wave=2, uni=7, detune=0.4, spread=1, cutoff=6000, as_=1, rev_mix=0.3, **gate('Stutter'))),
    ('Organ Lead',      P(a_wave=5.5, mono=1, glide=0.02, l_dest='Pitch', l_amt=0.008, l_sync=0, l_rate=6, cho_mix=0.3)),
    ('Digi Lead',       P(a_wave=6, b_wave=3, b_lvl=0.4, b_oct=1, mono=1, cutoff=5000, dly_mix=0.2)),
    ('Pulse Lead',      P(a_wave=4, l_dest='Wave A', l_amt=0.12, l_sync=0, l_rate=0.4, mono=1, glide=0.04, cutoff=3500)),
])

cat('Acid', ('Cutoff', 'Resonance'), [
    ('Acid Line',       P(a_wave=2, mono=1, glide=0.06, cutoff=350, reso=0.82, fenv=0.55, fd=0.2, fs=0.0, fdrive=0.4, dist=0.25, ad=0.3, as_=0.8, ar=0.05, a_oct=-1)),
    ('Square Acid',     P(a_wave=3, mono=1, glide=0.05, cutoff=300, reso=0.85, fenv=0.6, fd=0.15, fdrive=0.5, dist=0.3, as_=0.8, ar=0.05, a_oct=-1)),
    ('Squelch',         P(a_wave=2, mono=1, glide=0.08, cutoff=250, reso=0.9, fenv=0.7, fd=0.25, fs=0.1, fdrive=0.6, dist=0.35, dist_t='Hard', as_=0.8, ar=0.05, a_oct=-1)),
    ('Acid Wobble',     P(a_wave=2, mono=1, glide=0.05, cutoff=400, reso=0.8, l_dest='Cutoff', l_amt=0.35, l_div='1/8T', fdrive=0.4, as_=0.9, ar=0.05, a_oct=-1)),
    ('Rubber Acid',     P(a_wave=2.5, mono=1, glide=0.04, cutoff=500, reso=0.75, fenv=0.45, fd=0.1, as_=0.6, ar=0.04, a_oct=-1, dly_mix=0.2)),
    ('Psy Acid',        P(a_wave=2, fm=0.2, mono=1, glide=0.05, cutoff=450, reso=0.85, fenv=0.5, fd=0.18, fdrive=0.5, dist=0.3, dist_t='Fold', as_=0.8, ar=0.05, a_oct=-1, dly_mix=0.25, dly_div='1/8T')),
])

cat('Arps', ('Cutoff', 'Delay'), [
    ('Rolling Arp',     P(a_wave=3, arp_on=1, arp_div='1/16', arp_mode='Up', arp_oct=2, ad=0.15, as_=0.0, ar=0.1, cutoff=1500, fenv=0.3, fd=0.1, dly_mix=0.25)),
    ('Psy Arp',         P(a_wave=2, arp_on=1, arp_div='1/16', arp_mode='Random', arp_oct=3, reso=0.5, cutoff=1200, l_dest='Cutoff', l_amt=0.3, l_div='1 bar', ad=0.15, as_=0.0, ar=0.1, dly_mix=0.3)),
    ('Bell Arp',        P(a_wave=0, b_oct=1, b_semi=7, fm=0.4, arp_on=1, arp_div='1/8T', arp_mode='Up/Down', arp_oct=2, ad=0.5, as_=0.0, ar=0.3, rev_mix=0.3)),
    ('Minimal Seq',     P(a_wave=5, arp_on=1, arp_div='1/16', arp_mode='As Played', ad=0.1, as_=0.0, ar=0.05, cutoff=4000, dly_mix=0.3, dly_div='3/16')),
    ('Bass Arp',        P(a_wave=2, a_oct=-1, arp_on=1, arp_div='1/16', arp_mode='As Played', arp_oct=2, ad=0.15, as_=0.2, ar=0.05, cutoff=500, fenv=0.4, fd=0.1, sub=0.4, width=1.0)),
    ('Chord Pulse',     P(a_wave=5.5, chord='Minor 7', arp_on=1, arp_div='1/8', arp_mode='Chord', arp_gate=0.3, ad=0.2, as_=0.3, ar=0.1, cutoff=4000)),
    ('Garage Arp',      P(a_wave=6, arp_on=1, arp_div='1/16T', arp_mode='Up', arp_oct=2, ad=0.12, as_=0.0, ar=0.08, cutoff=3500, dly_mix=0.2)),
    ('Trance Arp',      P(a_wave=2, uni=5, detune=0.3, arp_on=1, arp_div='1/16', arp_mode='Up/Down', arp_oct=3, ad=0.18, as_=0.0, ar=0.1, cutoff=3000, fenv=0.2, fd=0.1, rev_mix=0.25, dly_mix=0.2)),
])

cat('FX', ('Cutoff', 'Pitch Env'), [
    ('Noise Riser',     P(a_lvl=0, noise=0.9, ftype='Band', cutoff=300, reso=0.4, fenv=0.85, fa=5, fd=1, fs=1, aa=4, as_=1, ar=1.5, rev_mix=0.5, width=1.3)),
    ('Downlifter',      P(a_wave=2, uni=5, detune=0.4, penv=-36, pdec=2, ad=3, as_=0.0, ar=1, rev_mix=0.6)),
    ('Zap',             P(a_wave=0, penv=48, pdec=0.05, ad=0.15, as_=0.0, ar=0.1)),
    ('Laser',           P(a_wave=3, penv=-48, pdec=0.2, ad=0.4, as_=0.0, ar=0.2, dly_mix=0.4, dly_fb=0.5)),
    ('Dub Siren',       P(a_wave=0.5, l_dest='Pitch', l_amt=0.5, l_div='1/4', l_shape='Triangle', dly_mix=0.5, dly_fb=0.7, as_=1, ar=0.5, rev_mix=0.3)),
    ('Noise Sweep',     P(a_lvl=0, noise=1, ftype='Band', reso=0.7, cutoff=1200, l_dest='Cutoff', l_amt=0.6, l_div='2 bars', as_=1, ar=1, rev_mix=0.4)),
    ('Alien Talk',      P(a_wave=7, l_dest='Wave A', l_shape='S&H', l_div='1/16', l_amt=0.6, reso=0.5, cutoff=2500, as_=1, ar=0.3, dly_mix=0.2)),
    ('Wind',            P(a_lvl=0, noise=0.7, ftype='Band', cutoff=1200, reso=0.3, l_dest='Cutoff', l_amt=0.3, l_div='4 bars', aa=1, as_=1, ar=2, rev_mix=0.6)),
    ('Impact Boom',     P(a_wave=0, a_oct=-2, penv=24, pdec=0.1, ad=2, as_=0.0, ar=1, rev_mix=0.6, rev_size=0.9, dist=0.3)),
    ('Psy Drip',        P(a_wave=0, penv=24, pdec=0.03, ad=0.1, as_=0.0, ar=0.08, dly_mix=0.5, dly_fb=0.6, dly_div='1/8T')),
    ('Metal Hit',       P(a_wave=0, b_semi=5, b_oct=1, fm=0.9, ad=0.4, as_=0.0, ar=0.3, rev_mix=0.5)),
    ('Gated Noise',     P(a_lvl=0, noise=0.9, cutoff=6000, as_=1, ar=0.2, locut=400, **gate('Eighths', '1/16'))),
    ('Space Bubbles',   P(a_wave=0, l_dest='Pitch', l_shape='S&H', l_div='1/16', l_amt=0.6, as_=1, ar=0.4, dly_mix=0.45, dly_fb=0.6, rev_mix=0.5)),
    ('Rising Saw',      P(a_wave=2, uni=7, detune=0.5, penv=-24, pdec=2, aa=3, as_=1, ar=1, l_dest='Cutoff', l_amt=0.2, l_div='1/16', rev_mix=0.4)),
])

# ---------------------------------------------------------------- modifiers
def mul(k, f): return ('mul', k, f)
MODS = {
    'Dark':    dict(_mul=dict(cutoff=0.5), hicut=8000),
    'Bright':  dict(_mul=dict(cutoff=1.9), _add=dict(reso=0.05)),
    'Wide':    dict(uni=4, _add=dict(detune=0.15), spread=1, width=1.4),
    'Dusty':   dict(dist=0.15, hicut=7000, cho_mix=0.2, _add=dict(noise=0.03)),
    'Dub':     dict(dly_mix=0.32, dly_fb=0.55, dly_div='3/16', _add=dict(rev_mix=0.2), locut=120),
    'Pumped':  dict(pump=0.6),
    'Short':   dict(_mul=dict(ad=0.5, ar=0.5, **{'as': 0.4})),
    'Long':    dict(_mul=dict(ar=2.5, ad=1.6), _add=dict(rev_mix=0.15)),
    'Driven':  dict(dist=0.35, _add=dict(fdrive=0.3)),
    'Crushed': dict(dist_t='Crush', dist=0.35),
    'Glide':   dict(glide=0.08, mono=1),
    'Wobble':  dict(l_dest='Cutoff', l_amt=0.3, l_div='1/8', l_sync=1),
    'Triplet Wobble': dict(l_dest='Cutoff', l_amt=0.3, l_div='1/8T', l_sync=1),
    'Psych':   dict(l_dest='Pan', l_amt=0.6, l_div='1/4', l_sync=1, _add=dict(cho_mix=0.4, rev_mix=0.3, fm=0.1)),
    'Up':      dict(_add=dict(a_oct=1, b_oct=1)),
    'Down':    dict(_add=dict(a_oct=-1, b_oct=-1)),
    'Space':   dict(rev_mix=0.5, rev_size=0.85),
    'Tape':    dict(l_dest='Pitch', l_amt=0.006, l_sync=0, l_rate=0.7, hicut=9000, dist=0.08),
    'Hollow':  dict(ftype='Band', _add=dict(reso=0.2)),
    'Chord':   dict(chord='Minor 7'),
    'Arp':     dict(arp_on=1, arp_div='1/16', arp_mode='Up', arp_oct=2),
    'FM':      dict(_add=dict(fm=0.25), b_oct=1),
    'Detuned': dict(_add=dict(b_lvl=0.5, detune=0.1), b_fine=17, uni=2),
    'Organ':   dict(a_wave=5),
    'Digi':    dict(a_wave=6),
    'Vox':     dict(a_wave=7),
    'Acid':    dict(reso=0.78, fenv=0.5, fd=0.18, fdrive=0.4, _mul=dict(cutoff=0.5)),
    'Sub':     dict(sub=0.6),
    'Plucked': dict(ad=0.22, **{'as': 0.0}, ar=0.12, fenv=0.45, fd=0.12, _mul=dict(cutoff=0.6)),
    'Ping':    dict(dly_mix=0.3, dly_fb=0.45, dly_div='1/8', dly_pp=1),
}
for g in GATES:
    MODS['Gate ' + g] = gate(g)

ALLOWED = {
    'Bass':          ['Dark', 'Bright', 'Dusty', 'Short', 'Driven', 'Crushed', 'Glide', 'Wobble', 'Acid', 'Sub', 'Organ', 'Digi', 'Vox', 'FM', 'Pumped', 'Plucked', 'Gate Roller', 'Gate Garage', 'Tape'],
    'UKG Bass':      ['Dark', 'Bright', 'Dusty', 'Driven', 'Crushed', 'Glide', 'Wobble', 'Triplet Wobble', 'Sub', 'Short', 'Gate Garage', 'Gate Broken', 'FM', 'Vox', 'Organ', 'Digi', 'Acid'],
    'Organ & Stabs': ['Dark', 'Bright', 'Dusty', 'Dub', 'Pumped', 'Short', 'Long', 'Tape', 'Wide', 'Space', 'Gate Garage', 'Gate Offbeat', 'Gate Skip', 'Up', 'Down', 'Ping', 'Crushed', 'Hollow', 'Psych'],
    'Chords':        ['Dark', 'Bright', 'Dusty', 'Dub', 'Pumped', 'Tape', 'Wide', 'Space', 'Gate Roller', 'Gate Garage', 'Gate Eighths', 'Psych', 'Wobble', 'Up', 'Ping', 'Plucked'],
    'Pads':          ['Dark', 'Bright', 'Dusty', 'Pumped', 'Tape', 'Wide', 'Space', 'Psych', 'Gate Roller', 'Gate Stutter', 'Gate Dotted', 'Up', 'Down', 'Chord', 'Vox', 'Organ', 'Crushed'],
    'Keys & Bells':  ['Dark', 'Bright', 'Dusty', 'Dub', 'Space', 'Tape', 'Chord', 'Arp', 'Up', 'Down', 'Ping', 'Pumped', 'Psych', 'Long', 'Short'],
    'Plucks':        ['Dark', 'Bright', 'Dusty', 'Dub', 'Space', 'Tape', 'Chord', 'Arp', 'Up', 'Down', 'Ping', 'Wide', 'Acid', 'Crushed', 'Psych', 'Organ', 'Digi', 'Vox', 'Long'],
    'Leads':         ['Dark', 'Bright', 'Dusty', 'Dub', 'Space', 'Driven', 'Glide', 'Wide', 'Psych', 'Up', 'Down', 'Ping', 'FM', 'Detuned', 'Crushed', 'Gate Stutter', 'Gate Garage', 'Tape'],
    'Acid':          ['Dark', 'Bright', 'Dub', 'Driven', 'Crushed', 'Psych', 'Up', 'Down', 'Ping', 'Gate Roller', 'Gate Broken', 'Pumped', 'Space', 'FM', 'Organ', 'Digi'],
    'Arps':          ['Dark', 'Bright', 'Dusty', 'Dub', 'Space', 'Wide', 'Psych', 'Up', 'Down', 'Ping', 'Acid', 'Organ', 'Digi', 'Vox', 'Pumped', 'Crushed'],
    'FX':            ['Dark', 'Bright', 'Dub', 'Space', 'Crushed', 'Psych', 'Up', 'Down', 'Wide', 'Driven', 'Tape', 'Ping', 'Gate Stutter'],
}
# pairs that make sense together
PAIRS = [('Dub', 'Dusty'), ('Dark', 'Pumped'), ('Bright', 'Wide'), ('Driven', 'Glide'), ('Tape', 'Space'), ('Psych', 'Space'),
         ('Dusty', 'Short'), ('Acid', 'Driven'), ('Dark', 'Dub'), ('Up', 'Ping'), ('Wobble', 'Driven'), ('Organ', 'Dusty'),
         ('Gate Garage', 'Dub'), ('Gate Roller', 'Pumped'), ('Crushed', 'Dark'), ('Chord', 'Dub'), ('Arp', 'Space'),
         ('Sub', 'Glide'), ('Digi', 'Short'), ('Wide', 'Space'), ('Vox', 'Psych'), ('Hollow', 'Dub'), ('FM', 'Dark')]
PER_ARCH = int(os.environ.get('SERPENT_PER_ARCH', '13'))

def apply(base, mod):
    out = dict(base)
    for k, v in mod.items():
        if k == '_mul':
            for kk, f in v.items():
                cur = out.get(kk, PAR[kk]['de'])
                out[kk] = cur * f
        elif k == '_add':
            for kk, a in v.items():
                cur = out.get(kk, PAR[kk]['de'])
                if isinstance(cur, str): cur = PAR[kk]['ch'].index(cur)
                out[kk] = cur + a
        else:
            out[k] = v
    return out

def finalize(d, catname):
    xy = CATS[catname]['xy']
    d = dict(d)
    d.setdefault('xy_xd', xy[0]); d.setdefault('xy_yd', xy[1])
    # keep the low end mono-friendly on basses
    if catname in ('Bass', 'UKG Bass', 'Acid'):
        d['locut'] = max(norm('locut', d.get('locut', 20)), 20)
        d.setdefault('width', 1.0)
    res = {}
    for k, v in d.items():
        res[k] = norm(k, v)
    return res

presets = []
seen = set()
for catname, c in CATS.items():
    allowed = ALLOWED[catname]
    for name, base in c['items']:
        variants = [(name, base)]
        for m in allowed:
            variants.append((name + ' ' + m, apply(base, MODS[m])))
        for a, b in PAIRS:
            if a in allowed and b in allowed:
                variants.append((name + ' ' + a + ' ' + b, apply(apply(base, MODS[a]), MODS[b])))
        # deterministic spread through the variant list so each archetype gets a mix
        chosen = [variants[0]]
        rest = variants[1:]
        step = max(1, len(rest) / max(1, PER_ARCH - 1))
        i = (hash(name) % 7) * 0.37 if False else 0.0
        while len(chosen) < PER_ARCH and int(i) < len(rest):
            chosen.append(rest[int(i)]); i += step
        for vn, vd in chosen:
            fd = finalize(vd, catname)
            key = json.dumps(fd, sort_keys=True)
            if key in seen: continue
            seen.add(key)
            presets.append(dict(name=vn, cat=catname, p=fd))

# ---------------------------------------------------------------- loudness trims
lv_path = os.path.join(HERE, 'levels.json')
if os.path.exists(lv_path):
    lv = json.load(open(lv_path))
    for pr in presets:
        k = pr['cat'] + '/' + pr['name']
        if k in lv:
            pr['p']['vol'] = round(max(-36.0, min(6.0, lv[k])), 2)

def fmt(v):
    if isinstance(v, int): return str(v)
    s = ('%.4f' % v).rstrip('0').rstrip('.')
    return s if s not in ('', '-0') else '0'

out = ['// generated by tools/make_presets.py - do not edit by hand', '#pragma once', '#include "PluginProcessor.h"', '',
       'static const SerpentPreset kPresets[] =', '{']
for pr in presets:
    data = ';'.join('%s=%s' % (k, fmt(v)) for k, v in sorted(pr['p'].items()) if abs(float(v) - PAR[k]['de']) > 1e-6)
    out.append('    { "%s", "%s", "%s" },' % (pr['name'], pr['cat'], data))
out += ['};', 'static const int kNumPresets = (int) (sizeof (kPresets) / sizeof (kPresets[0]));', '']
open(os.path.join(SRC, 'Presets.h'), 'w').write('\n'.join(out))
json.dump([{'name': p['name'], 'cat': p['cat']} for p in presets], open(os.path.join(HERE, 'preset_index.json'), 'w'))
from collections import Counter
cnt = Counter(p['cat'] for p in presets)
print(len(presets), 'presets'); print(dict(cnt))
