#!/usr/bin/env python3
"""Builds the Dust Box preset bank (Source/Presets.h).

Each preset is assembled in three layers:
  1. the parameter defaults from Params.h
  2. a CATEGORY template (what a bass / pad / stab generally needs, FX included)
  3. an ARCHETYPE's overrides (what makes this particular sound itself)
Then every archetype gets a handful of VARIANTS (timbral or FX) chosen from the
variants its category allows, so a bass never gets a cathedral reverb.

Edit, then run:  python3 tools/make_presets.py
"""
import os, re

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PARAMS_H = open(os.path.join(HERE, "Source", "Params.h")).read()

# ---- read the parameter contract straight out of Params.h -----------------
float_rows = re.findall(r'\{\s*"(\w+)",\s*"[^"]*",\s*([-\d.]+)f,\s*([-\d.]+)f,\s*([-\d.]+)f,\s*(true|false),\s*(true|false)', PARAMS_H)
FLOATS = [r[0] for r in float_rows]
RANGE  = {r[0]: (float(r[1]), float(r[2])) for r in float_rows}
FDEF   = {r[0]: float(r[3]) for r in float_rows}
STEPPED= {r[0]: r[5] == "true" for r in float_rows}

choice_rows = re.findall(r'\{\s*"(\w+)",\s*"[^"]*",\s*"([^"]+)",\s*(\d+),\s*(true|false),\s*"\w+"\s*\}', PARAMS_H)
CHOICES = [r[0] for r in choice_rows]
ITEMS   = {r[0]: r[1].split("|") for r in choice_rows}
CDEF    = {r[0]: int(r[2]) for r in choice_rows}
assert len(FLOATS) == 58 and len(CHOICES) == 15, (len(FLOATS), len(CHOICES))

def choice_index(cid, value):
    if isinstance(value, bool):  return 1 if value else 0
    if cid == "unison":          return int(value) - 1
    return ITEMS[cid].index(str(value))

# ---- category templates ---------------------------------------------------
CAT_ORDER = ["Bass", "Sub", "Acid", "Chords", "Stabs", "Keys & Organs",
             "Strings", "Pads", "Plucks & Bleeps", "Leads", "Textures & FX"]

CAT = {
 "Bass": dict(tags="bass low groove",
   oscAWave="saw", oscBWave="square", oscALevel=.9, oscBLevel=.4, detune=6, spread=4, unison=1,
   subLevel=.55, cutoff=520, resonance=5, fEnvAmt=2.2, keytrack=.3, drive=.45,
   ampA=.003, ampD=.28, ampS=.35, ampR=.1, fA=.002, fD=.16, fS=.1, lfoDepth=0, dust=.2, volume=.8,
   chorusOn=False, reverbOn=False,
   compOn=True, compThresh=-18, compRatio=4, compAttack=10, compRelease=80, compMakeup=4,
   pumpOn=True, pumpDepth=.5, pumpRelease=.45, pumpRate="1/4",
   eqOn=True, eqLow=2, eqMid=-2, eqMidFreq=400, eqHigh=-3),
 "Sub": dict(tags="sub bass low deep round",
   oscAWave="tri", oscBWave="saw", oscALevel=.8, oscBLevel=0, detune=0, spread=0, unison=1,
   subLevel=.9, cutoff=300, resonance=2, fEnvAmt=.8, keytrack=.2, drive=.2,
   ampA=.004, ampD=.4, ampS=.8, ampR=.12, fA=.003, fD=.2, fS=.4, lfoDepth=0, dust=.08, volume=.85,
   chorusOn=False, reverbOn=False,
   compOn=True, compThresh=-14, compRatio=3, compAttack=15, compRelease=100, compMakeup=2,
   pumpOn=True, pumpDepth=.6, pumpRelease=.5,
   eqOn=True, eqLow=3, eqMid=0, eqHigh=-8),
 "Acid": dict(tags="acid 303 squelchy resonant line",
   oscAWave="saw", oscBWave="square", oscALevel=.9, oscBLevel=.15, unison=1, detune=4, spread=3,
   subLevel=.3, cutoff=650, resonance=16, fEnvAmt=3.2, keytrack=.5, drive=.65,
   ampA=.003, ampD=.45, ampS=.5, ampR=.12, fA=.002, fD=.2, fS=.05, glide=.06, lfoDepth=.04, dust=.25,
   chorusOn=False,
   compOn=True, compThresh=-16, compRatio=4,
   delayOn=True, delaySync="1/8d", delayMode="ping-pong", delayMix=.18, delayFeedback=.35, delayTone=2800,
   reverbOn=True, reverb=.12, reverbSize=.35),
 "Chords": dict(tags="chord chords harmony",
   oscAWave="saw", oscBWave="saw", oscALevel=.85, oscBLevel=.65, unison=3, detune=14, spread=18,
   subLevel=.15, cutoff=2000, resonance=4, fEnvAmt=1.8, keytrack=.45, drive=.35,
   ampA=.004, ampD=.5, ampS=.2, ampR=.35, fA=.003, fD=.3, fS=.15, lfoDepth=.1, lfoRate=.5, dust=.35,
   chorusOn=True, chorus=.45, reverbOn=True, reverb=.3, reverbSize=.55,
   delayOn=True, delaySync="1/8d", delayMix=.15, delayFeedback=.32,
   pumpOn=True, pumpDepth=.45, eqOn=True, eqLow=-4, eqHigh=1),
 "Stabs": dict(tags="stab stabs chord short hit",
   oscAWave="saw", oscBWave="saw", oscALevel=.9, oscBLevel=.7, unison=3, detune=14, spread=20,
   subLevel=.18, cutoff=2300, resonance=6, fEnvAmt=2.4, keytrack=.5, drive=.45,
   ampA=.003, ampD=.22, ampS=0, ampR=.15, fA=.002, fD=.12, fS=.05, lfoDepth=.06, dust=.45,
   chorusOn=True, chorus=.4, reverbOn=True, reverb=.22, reverbSize=.5,
   delayOn=True, delaySync="1/8d", delayMode="ping-pong", delayMix=.22, delayFeedback=.4,
   pumpOn=True, pumpDepth=.4, compOn=True, compThresh=-16, compRatio=3,
   eqOn=True, eqLow=-5),
 "Keys & Organs": dict(tags="keys organ piano",
   oscAWave="square", oscBWave="square", oscALevel=.8, oscBLevel=.55, oscBSemi=12, unison=2,
   detune=9, spread=10, subLevel=.3, cutoff=2200, resonance=2, fEnvAmt=.8, keytrack=.5, drive=.4,
   ampA=.01, ampD=.6, ampS=.7, ampR=.25, fA=.01, fD=.4, fS=.6, lfoDest="trem", lfoDepth=.1, lfoRate=5,
   dust=.4, chorusOn=True, chorus=.45, reverbOn=True, reverb=.28,
   compOn=True, compThresh=-12, compRatio=2.5, eqOn=True, eqLow=-3),
 "Strings": dict(tags="strings string orchestral bowed",
   oscAWave="saw", oscBWave="saw", oscALevel=.8, oscBLevel=.75, unison=4, detune=20, spread=32,
   subLevel=.1, cutoff=3200, resonance=1.5, fEnvAmt=.8, keytrack=.4, drive=.2,
   ampA=.08, ampD=.8, ampS=.7, ampR=.6, fA=.05, fD=.6, fS=.6, lfoDest="pitch", lfoDepth=.08, lfoRate=5,
   dust=.25, chorusOn=True, chorus=.6, reverbOn=True, reverb=.4, reverbSize=.7,
   compOn=True, compThresh=-14, compRatio=2.5, eqOn=True, eqLow=-5, eqHigh=2),
 "Pads": dict(tags="pad pads ambient sustained",
   oscAWave="saw", oscBWave="saw", oscALevel=.8, oscBLevel=.75, unison=4, detune=22, spread=38,
   subLevel=.12, cutoff=1300, resonance=2, fEnvAmt=1.2, keytrack=.3, drive=.2,
   ampA=.6, ampD=1.5, ampS=.75, ampR=1.8, fA=.7, fD=1.2, fS=.55, lfoDepth=.28, lfoRate=.25, dust=.4,
   chorusOn=True, chorus=.7, reverbOn=True, reverb=.6, reverbSize=.85, reverbDamp=.5,
   eqOn=True, eqLow=-6),
 "Plucks & Bleeps": dict(tags="pluck bleep blip short percussive",
   oscAWave="square", oscBWave="tri", oscALevel=.7, oscBLevel=.45, oscBSemi=12, unison=1,
   detune=5, spread=6, subLevel=.08, cutoff=3200, resonance=7, fEnvAmt=2.6, keytrack=.7, drive=.25,
   ampA=.002, ampD=.14, ampS=0, ampR=.1, fA=.001, fD=.08, fS=0, lfoDepth=.03, dust=.2,
   chorusOn=True, chorus=.2, reverbOn=True, reverb=.25,
   delayOn=True, delaySync="1/8d", delayMode="ping-pong", delayMix=.28, delayFeedback=.42,
   eqOn=True, eqLow=-6),
 "Leads": dict(tags="lead melody solo line",
   oscAWave="saw", oscBWave="tri", oscALevel=.75, oscBLevel=.4, oscBSemi=12, unison=2,
   detune=7, spread=10, subLevel=.08, cutoff=4500, resonance=4, fEnvAmt=1.2, keytrack=.8, drive=.3,
   ampA=.02, ampD=.5, ampS=.8, ampR=.3, fA=.02, fD=.3, fS=.6, lfoDest="pitch", lfoDepth=.1, lfoRate=5.5,
   glide=.06, dust=.25, chorusOn=True, chorus=.3, reverbOn=True, reverb=.35,
   delayOn=True, delaySync="1/4", delayMix=.2, delayFeedback=.35, compOn=True),
 "Textures & FX": dict(tags="texture drone atmosphere fx ambient",
   oscAWave="tri", oscBWave="saw", oscALevel=.6, oscBLevel=.5, unison=4, detune=30, spread=44,
   subLevel=.15, cutoff=2400, resonance=2, fEnvAmt=1, keytrack=.3, drive=.25,
   ampA=1.4, ampD=2, ampS=.7, ampR=2.8, fA=1.5, fD=1.8, fS=.6, lfoDepth=.4, lfoRate=.15, dust=.6,
   chorusOn=True, chorus=.7, reverbOn=True, reverb=.8, reverbSize=.95, reverbDamp=.4,
   delayOn=True, delaySync="1/4d", delayMix=.3, delayFeedback=.6, delayTone=2400,
   eqOn=True, eqLow=-8),
}

# ---- archetypes -----------------------------------------------------------
# Tags matter: every word is searchable by the vibe box. "ukgroove" marks the
# sounds built from research into the current London house scene (rolling
# sidechained basslines, bouncy swing, string stabs, R&B-leaning keys,
# '90s Chicago touches). Names describe the sound, not any artist or record.
A = []
def arch(cat, name, tags, **kw): A.append((cat, name, tags, kw))

# Bass
arch("Bass","Rolling Three","ukgroove rolling tech minimal bouncy swing driving sidechain",
     cutoff=480, resonance=6, fEnvAmt=2.4, ampD=.2, ampS=.3, drive=.5, subLevel=.5, pumpDepth=.6)
arch("Bass","Bounce Bass","ukgroove bouncy swing punchy plucky minimal",
     oscAWave="square", oscBWave="tri", cutoff=560, resonance=8, fD=.12, ampD=.16, ampS=.15, drive=.4)
arch("Bass","Donk Roller","ukgroove donk plucky rubbery bouncy",
     oscAWave="square", resonance=13, cutoff=700, fEnvAmt=3, fD=.08, ampD=.12, ampS=0, subLevel=.35)
arch("Bass","Heavy Line","ukgroove heavy thick fat driven",
     unison=2, detune=10, subLevel=.7, cutoff=900, resonance=4, drive=.7, ampS=.6)
arch("Bass","Warehouse Growl","dirty driven growl heavy raw",
     oscBWave="saw", oscBSemi=-12, oscBLevel=.6, drive=.8, cutoff=650, resonance=6, subLevel=.6,
     crushOn=True, crush=.18)
arch("Bass","Rubber Bass","rubbery round bouncy funky glide smooth",
     oscAWave="tri", oscBLevel=.35, cutoff=750, resonance=8, fEnvAmt=2.4, ampD=.26, glide=.05)
arch("Bass","Funk Bass","fat thick funky mid growl",
     oscBWave="saw", oscBSemi=-12, oscBLevel=.6, unison=2, detune=8, drive=.85, cutoff=600, resonance=6)
arch("Bass","Jack Bass","chicago jack 90s house punchy",
     oscAWave="square", cutoff=800, resonance=9, fD=.15, ampD=.2, drive=.5)

# Sub
arch("Sub","Basement Sub","clean simple minimal tight",   cutoff=420, oscBLevel=.25, oscBWave="saw", ampS=.6)
arch("Sub","Round Sub","clean pure sine smooth",           cutoff=220, drive=.05)
arch("Sub","Punch Sub","punchy short tight",               ampD=.25, ampS=.3, fEnvAmt=1.6, drive=.3)
arch("Sub","Warm Sub","warm driven saturated",             drive=.5, cutoff=380, dust=.2)
arch("Sub","Sub Swell","long held sustained smooth",       ampA=.05, ampS=.95, ampR=.4, pumpDepth=.45)

# Acid
arch("Acid","Acid Worm","rubbery moving bright aggressive")
arch("Acid","Squelch Line","squelchy screaming resonant", resonance=19, fEnvAmt=3.8, cutoff=520)
arch("Acid","Rubber Acid","rubbery bouncy square",         oscAWave="square", resonance=14, glide=.04)
arch("Acid","Night Acid","dark deep moody",                cutoff=420, resonance=15, delayMix=.28, delaySync="1/4d")
arch("Acid","Screamer","aggressive bright loud harsh",     cutoff=1100, drive=.92, resonance=17)

# Chords
arch("Chords","Deep Minor Nine","deep warm lush detroit mellow",
     oscAWave="tri", cutoff=1400, ampD=.9, ampS=.35, chorus=.6)
arch("Chords","Late Night Chord","rnb soulful smooth warm silky",
     oscAWave="tri", oscBWave="square", oscBSemi=12, oscBLevel=.35, cutoff=1700, resonance=2,
     lfoDest="trem", lfoDepth=.1, lfoRate=4.5, pumpDepth=.35)
arch("Chords","Skyway Chord","wide lush bright uplifting euphoric big",
     unison=4, detune=24, spread=34, cutoff=3000, ampD=.9, ampS=.4, chorus=.8, reverb=.45)
arch("Chords","Blue Haze","soft mellow dark melancholy sad emotional",
     oscAWave="tri", cutoff=900, ampA=.25, ampD=1.1, ampS=.6, ampR=1.2, reverb=.6)
arch("Chords","Deep Space Chord","deep wide dark spacious dreamy cosmic",
     oscBSemi=-12, unison=4, detune=20, spread=30, cutoff=1200, ampD=1.2, ampS=.45, reverb=.6)
arch("Chords","Dub Chord","dub reggae offbeat wet echo skank",
     oscAWave="square", cutoff=1900, ampD=.22, ampS=0, delaySync="1/4d", delayFeedback=.55, delayMix=.3)
arch("Chords","Melodic Deep","ukgroove melodic deep emotive",
     oscBWave="tri", cutoff=2200, resonance=3, ampD=.7, ampS=.3, reverb=.4)
arch("Chords","Broken Cassette","lofi dusty broken wobbly warped old muffled",
     oscBSemi=-12, cutoff=1100, drive=.7, lfoDest="pitch", lfoDepth=.42, lfoRate=.35, dust=.95,
     crushOn=True, crush=.2)

# Stabs
arch("Stabs","Seven Mile Stab","detroit dusty warm plucky classic raw soulful gritty")
arch("Stabs","Hard Stab","hard bright aggressive loud punchy sharp techno banging",
     oscBWave="square", oscBSemi=7, cutoff=4200, resonance=8, drive=.8)
arch("Stabs","Piano Stab","chicago 90s piano bouncy classic house",
     oscAWave="square", oscBWave="tri", oscBSemi=12, cutoff=3000, fEnvAmt=2, ampD=.35, unison=2)
arch("Stabs","String Hit","ukgroove strings orchestral stab bright",
     unison=4, detune=18, spread=26, cutoff=3800, resonance=2, fEnvAmt=1.5, ampD=.3, ampR=.25, chorus=.5)
arch("Stabs","Swing Stab","ukgroove bouncy swing minimal",
     oscAWave="square", unison=2, cutoff=1800, resonance=7, ampD=.12, delayMix=.25, pumpDepth=.5)
arch("Stabs","Gravel Stab","dirty gritty rough harsh distorted crunchy",
     oscAWave="square", oscBSemi=-5, resonance=11, drive=.95, dust=.7)
arch("Stabs","Offbeat Organ","chicago jack organ offbeat",
     oscAWave="square", oscBWave="square", oscBSemi=7, unison=2, cutoff=2600, resonance=4)

# Keys & Organs
arch("Keys & Organs","Tape Organ","warm soulful vintage wobbly gospel hazy dusty",
     lfoDest="pitch", lfoDepth=.18, lfoRate=5.2, dust=.6, ampS=.9, drive=.45)
arch("Keys & Organs","Jack Organ","chicago jack bouncy punchy raw",
     oscBSemi=7, cutoff=2600, ampD=.32, ampS=.15, drive=.55, lfoDest="cutoff", lfoDepth=.08)
arch("Keys & Organs","Rhodes Ghost","electric piano ep soft warm mellow jazzy",
     oscAWave="tri", oscBLevel=.3, cutoff=1900, fEnvAmt=1.9, ampD=.75, ampS=.1, ampR=.45)
arch("Keys & Organs","Silk Keys","rnb electric piano smooth soulful silky warm",
     oscAWave="tri", oscBWave="tri", cutoff=2000, ampD=1.1, ampS=.15, ampR=.6, lfoDepth=.12, lfoRate=4.5, chorus=.5)
arch("Keys & Organs","Vinyl Keys","lofi dusty crackly jazzy sampled vintage",
     oscAWave="tri", oscBWave="saw", oscBSemi=0, cutoff=1700, ampD=.55, ampS=.12, dust=.85,
     lfoDest="pitch", lfoDepth=.16, lfoRate=.45)
arch("Keys & Organs","Garage Organ","ukgroove garage bouncy shuffle",
     oscBWave="saw", cutoff=2600, drive=.5, ampD=.25, ampS=.3, pumpOn=True, pumpDepth=.4)
arch("Keys & Organs","Bell Tone","bell metallic ringing sparse",
     oscAWave="tri", oscBWave="tri", oscBSemi=19, cutoff=4800, ampD=1.4, ampS=.05, ampR=1.1, reverb=.5)

# Strings
arch("Strings","Velvet Strings","rnb smooth lush silky warm")
arch("Strings","Disco Strings","disco bright uplifting classic", oscBSemi=12, cutoff=4500, ampA=.03,
     delayOn=True, delaySync="1/8d", delayMix=.15)
arch("Strings","Low Strings","dark cinematic low moody",       oscBSemi=-12, cutoff=1600, reverb=.5)
arch("Strings","Tremolo Strings","moving pulsing trembling",   lfoDest="trem", lfoDepth=.5, lfoRate=6)
arch("Strings","Terrace Strings","ukgroove euphoric wide sunset", cutoff=3800, ampA=.15, reverb=.5,
     pumpOn=True, pumpDepth=.45)

# Pads
arch("Pads","Cold Room Pad","soft lush wide warm dreamy hazy")
arch("Pads","Warehouse Pad","huge dark cold reverberant moody techno", cutoff=1000, ampA=.9, reverb=.75)
arch("Pads","Velvet Pad","rnb warm soft smooth silky", oscAWave="tri", cutoff=1100, chorus=.6)
arch("Pads","Glass Pad","bright shimmering airy glassy", oscBSemi=12, cutoff=3600, resonance=3)
arch("Pads","Phase Pad","moving swirly evolving", phaserOn=True, phaserMix=.6, phaserRate=.2)
arch("Pads","Hazy Minor Pad","lofi dusty hazy melancholy", cutoff=900, dust=.8, crushOn=True, crush=.15)

# Plucks & Bleeps
arch("Plucks & Bleeps","Ghost Pluck","dry clean bright bell tight minimal tiny", oscBLevel=.5)
arch("Plucks & Bleeps","Rim Blip","tiny high tick minimal", oscBWave="square", oscBSemi=19, cutoff=6000, ampD=.07)
arch("Plucks & Bleeps","Hypnotic Loop","minimal hypnotic repetitive dry tight",
     oscBWave="saw", oscBSemi=0, cutoff=1600, resonance=9, lfoDepth=.22, lfoRate=.3, delayMix=.18)
arch("Plucks & Bleeps","Tech Bleep","ukgroove tech minimal bleep", cutoff=2800, resonance=10, ampD=.06)
arch("Plucks & Bleeps","Swing Pluck","ukgroove swing bouncy", oscAWave="saw", cutoff=2400, ampD=.18,
     pumpOn=True, pumpDepth=.4)
arch("Plucks & Bleeps","Wood Pluck","wooden marimba percussive warm", oscAWave="tri", oscBSemi=19,
     cutoff=2200, resonance=3, ampD=.22)
arch("Plucks & Bleeps","Dub Blip","dub echo wet", delaySync="1/4d", delayFeedback=.62, delayMix=.35)

# Leads
arch("Leads","Air Lead","bright thin high cutting simple clean melodic")
arch("Leads","Hoover Riff","hoover rave aggressive sweeping detuned",
     oscBWave="square", oscBSemi=-5, oscBLevel=.85, unison=4, detune=32, spread=40, cutoff=2400,
     resonance=7, drive=.7, lfoDest="cutoff", lfoDepth=.35, lfoRate=.8)
arch("Leads","Soul Whistle","rnb smooth whistle sweet", oscAWave="tri", oscBLevel=.2, cutoff=5000,
     glide=.08, lfoDepth=.14)
arch("Leads","Square Lead","retro 8bit chip square", oscAWave="square", oscBWave="square", unison=1, cutoff=6000)
arch("Leads","Detune Lead","wide big detuned supersaw", oscBWave="saw", unison=4, detune=22, spread=30)

# Textures & FX
arch("Textures & FX","Night Bus Drone","dark long evolving deep low moody cold huge",
     oscAWave="saw", oscBSemi=-12, cutoff=700, resonance=4, subLevel=.5)
arch("Textures & FX","Cold Wind","airy soft high breathy quiet", oscAWave="tri", oscALevel=.35, oscBLevel=.3,
     oscBSemi=12, cutoff=2600)
arch("Textures & FX","Dark Hum","dark drone low hum", oscBSemi=-12, cutoff=500, subLevel=.4, delayOn=False)
arch("Textures & FX","Crushed Air","crushed lofi noisy digital", crushOn=True, crush=.55, cutoff=3200)
arch("Textures & FX","Space Echo","echo dub spacious wet", delayFeedback=.8, delayMix=.45, reverb=.7)
arch("Textures & FX","Phase Wash","swirly phased moving wash", phaserOn=True, phaserMix=.7, phaserRate=.12)

# Variations (dark, dub, pumped...) used to be baked into extra presets. They
# are now the character knobs in the plugin (see applyMacros in Params.h),
# which work on any sound, so the bank holds the original sounds only.

# ---- assemble -------------------------------------------------------------
def clampf(k, v):
    lo, hi = RANGE[k]
    v = max(lo, min(hi, v))
    return round(v) if STEPPED[k] else v

def build(cat, over):
    p = dict(FDEF)
    for c in CHOICES:
        p[c] = CDEF[c]
    tmpl = {k: v for k, v in CAT[cat].items() if k != "tags"}
    for src in (tmpl, over):
        for k, v in src.items():
            if k in CHOICES: p[k] = choice_index(k, v)
            else:            p[k] = clampf(k, float(v))
    return p

presets, seen = [], set()
for i, (cat, name, tags, over) in enumerate(A):
    assert name not in seen, name
    seen.add(name)
    base = build(cat, over)
    full_tags = f"{tags} {CAT[cat]['tags']}"
    presets.append((cat, name, full_tags, base, False))

presets.sort(key=lambda t: CAT_ORDER.index(t[0]))   # stable: archetype, then its variants

# Loudness matching: tools/levels.txt holds the level-knob value that puts each
# preset at the same perceived volume. Written by tools/calibrate_levels.cpp.
LEVELS = os.path.join(HERE, "tools", "levels.txt")
if os.path.exists(LEVELS):
    lv = dict(l.rstrip("\n").split("\t") for l in open(LEVELS) if "\t" in l)
    for cat, name, tags, p, isvar in presets:
        if name in lv:
            p["volume"] = clampf("volume", float(lv[name]))
    print(f"applied loudness levels to {sum(1 for t in presets if t[1] in lv)} presets")

# ---- emit -----------------------------------------------------------------
def f(x): return f"{x:.4f}f"
L = ["// GENERATED BY tools/make_presets.py — do not edit by hand.",
     "#pragma once", '#include "Params.h"', "", "namespace dustbox {", "",
     f"inline constexpr int kNumCategories = {len(CAT_ORDER)};",
     "inline const char* const kCategories[kNumCategories] = {",
     "    " + ", ".join(f'"{c}"' for c in CAT_ORDER), "};", "",
     "struct PresetEntry { const char* name; int category; bool variant; const char* tags; Patch patch; };", "",
     f"inline constexpr int kNumPresets = {len(presets)};",
     "inline const PresetEntry kPresets[kNumPresets] = {"]
for cat, name, tags, p, isvar in presets:
    fv = ", ".join(f(p[k]) for k in FLOATS)
    cv = ", ".join(str(int(p[c])) for c in CHOICES)
    L.append(f'  {{ "{name}", {CAT_ORDER.index(cat)}, {"true" if isvar else "false"}, "{tags}",')
    L.append(f'    {{ {{ {fv} }},')
    L.append(f'      {{ {cv} }} }} }},')
L += ["};", "", "} // namespace dustbox", ""]
open(os.path.join(HERE, "Source", "Presets.h"), "w").write("\n".join(L))

counts = {c: sum(1 for p in presets if p[0] == c) for c in CAT_ORDER}
print(f"wrote {len(presets)} presets from {len(A)} archetypes")
for c in CAT_ORDER: print(f"  {c:18s} {counts[c]}")
