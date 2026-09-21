#pragma once
#include <cmath>
#include <cstddef>
#include <algorithm>

// The parameter contract: single source of truth for the DSP, the preset bank,
// the vibe matcher and the UI. Ids match the browser prototype's patch JSON for
// every parameter the prototype has; the FX parameters are new and load with
// their defaults (off) when an older patch doesn't mention them.

namespace dustbox {

enum FloatParam {
    // synth
    pOscALevel = 0, pOscBLevel, pOscBSemi, pDetune, pSpread, pSubLevel,
    pCutoff, pResonance, pFEnvAmt, pKeytrack, pDrive,
    pAmpA, pAmpD, pAmpS, pAmpR, pFA, pFD, pFS,
    pLfoRate, pLfoDepth, pGlide,
    pDust, pVolume,
    // fx
    pEqLow, pEqMid, pEqMidFreq, pEqHigh,
    pCrush,
    pPhaserRate, pPhaserMix,
    pChorus,
    pCompThresh, pCompRatio, pCompAttack, pCompRelease, pCompMakeup,
    pPumpDepth, pPumpRelease,
    pDelayFeedback, pDelayMix, pDelayTone,
    pReverb, pReverbSize, pReverbDamp,
    // character: non-destructive layers applied on top of whatever patch is loaded
    pMDark, pMBright, pMDeep, pMDirty, pMClean, pMWide,
    pMShort, pMLong,
    pMPumped, pMDub, pMSpace, pMCrushed, pMPhased, pMGlued,
    kNumFloatParams
};

enum ChoiceParam {
    cOscAWave = 0, cOscBWave, cUnison, cLfoDest,
    cEqOn, cCrushOn, cPhaserOn, cChorusOn, cCompOn, cPumpOn, cDelayOn, cReverbOn,
    cPumpRate, cDelaySync, cDelayMode,
    kNumChoiceParams
};

struct FloatSpec {
    const char* id;
    const char* label;
    float lo, hi, def;
    bool  logScale;
    bool  stepped;
    const char* unit;
    const char* group;   // osc flt env mot out | eq crush phaser chorus comp pump delay reverb
};

inline const FloatSpec kFloatSpecs[kNumFloatParams] = {
    { "oscALevel", "osc a",    0.f,    1.f,     0.85f, false, false, "",     "osc" },
    { "oscBLevel", "osc b",    0.f,    1.f,     0.55f, false, false, "",     "osc" },
    { "oscBSemi",  "b pitch", -24.f,   24.f,    0.f,   false, true,  " st",  "osc" },
    { "detune",    "detune",   0.f,    40.f,    9.f,   false, false, " ct",  "osc" },
    { "spread",    "spread",   0.f,    45.f,    14.f,  false, false, " ct",  "osc" },
    { "subLevel",  "sub",      0.f,    1.f,     0.30f, false, false, "",     "osc" },
    { "cutoff",    "cutoff",   110.f,  14000.f, 1800.f, true, false, " Hz",  "flt" },
    { "resonance", "reso",     0.5f,   20.f,    4.f,   false, false, "",     "flt" },
    { "fEnvAmt",   "env amt",  0.f,    5.f,     1.8f,  false, false, " oct", "flt" },
    { "keytrack",  "keytrack", 0.f,    1.f,     0.35f, false, false, "",     "flt" },
    { "drive",     "drive",    0.f,    1.f,     0.34f, false, false, "",     "flt" },
    { "ampA",      "attack",   0.001f, 2.f,     0.005f, true, false, " s",   "env" },
    { "ampD",      "decay",    0.01f,  3.f,     0.35f,  true, false, " s",   "env" },
    { "ampS",      "sustain",  0.f,    1.f,     0.f,   false, false, "",     "env" },
    { "ampR",      "release",  0.02f,  4.f,     0.30f,  true, false, " s",   "env" },
    { "fA",        "flt atk",  0.001f, 2.f,     0.004f, true, false, " s",   "env" },
    { "fD",        "flt dec",  0.01f,  3.f,     0.22f,  true, false, " s",   "env" },
    { "fS",        "flt sus",  0.f,    1.f,     0.18f, false, false, "",     "env" },
    { "lfoRate",   "lfo rate", 0.05f,  12.f,    0.9f,   true, false, " Hz",  "mot" },
    { "lfoDepth",  "lfo amt",  0.f,    1.f,     0.12f, false, false, "",     "mot" },
    { "glide",     "glide",    0.f,    0.4f,    0.f,   false, false, " s",   "mot" },
    { "dust",      "dust",     0.f,    1.f,     0.35f, false, false, "",     "out" },
    { "volume",    "level",    0.f,    1.f,     0.75f, false, false, "",     "out" },

    { "eqLow",     "low",     -12.f,   12.f,    0.f,   false, false, " dB",  "eq" },
    { "eqMid",     "mid",     -12.f,   12.f,    0.f,   false, false, " dB",  "eq" },
    { "eqMidFreq", "mid freq", 200.f,  5000.f,  900.f,  true, false, " Hz",  "eq" },
    { "eqHigh",    "high",    -12.f,   12.f,    0.f,   false, false, " dB",  "eq" },
    { "crush",     "crush",    0.f,    1.f,     0.25f, false, false, "",     "crush" },
    { "phaserRate","rate",     0.05f,  5.f,     0.4f,   true, false, " Hz",  "phaser" },
    { "phaserMix", "mix",      0.f,    1.f,     0.5f,  false, false, "",     "phaser" },
    { "chorus",    "mix",      0.f,    1.f,     0.40f, false, false, "",     "chorus" },
    { "compThresh","thresh",  -40.f,   0.f,    -14.f,  false, false, " dB",  "comp" },
    { "compRatio", "ratio",    1.f,    20.f,    4.f,    true, false, ":1",   "comp" },
    { "compAttack","attack",   0.1f,   100.f,   8.f,    true, false, " ms",  "comp" },
    { "compRelease","release", 10.f,   1000.f,  120.f,  true, false, " ms",  "comp" },
    { "compMakeup","makeup",   0.f,    18.f,    4.f,   false, false, " dB",  "comp" },
    { "pumpDepth", "depth",    0.f,    1.f,     0.55f, false, false, "",     "pump" },
    { "pumpRelease","shape",   0.1f,   0.95f,   0.5f,  false, false, "",     "pump" },
    { "delayFeedback","feedback",0.f,  0.92f,   0.38f, false, false, "",     "delay" },
    { "delayMix",  "mix",      0.f,    1.f,     0.22f, false, false, "",     "delay" },
    { "delayTone", "tone",     500.f,  12000.f, 3500.f, true, false, " Hz",  "delay" },
    { "reverb",    "mix",      0.f,    1.f,     0.22f, false, false, "",     "reverb" },
    { "reverbSize","size",     0.f,    1.f,     0.55f, false, false, "",     "reverb" },
    { "reverbDamp","damp",     0.f,    1.f,     0.45f, false, false, "",     "reverb" },
    { "mDark", "dark", 0.f, 1.f, 0.f, false, false, "", "mtone" },
    { "mBright", "bright", 0.f, 1.f, 0.f, false, false, "", "mtone" },
    { "mDeep", "deep", 0.f, 1.f, 0.f, false, false, "", "mtone" },
    { "mDirty", "dirty", 0.f, 1.f, 0.f, false, false, "", "mtone" },
    { "mClean", "clean", 0.f, 1.f, 0.f, false, false, "", "mtone" },
    { "mWide", "wide", 0.f, 1.f, 0.f, false, false, "", "mtone" },
    { "mShort", "short", 0.f, 1.f, 0.f, false, false, "", "mlen" },
    { "mLong", "long", 0.f, 1.f, 0.f, false, false, "", "mlen" },
    { "mPumped", "pumped", 0.f, 1.f, 0.f, false, false, "", "mfx" },
    { "mDub", "dub", 0.f, 1.f, 0.f, false, false, "", "mfx" },
    { "mSpace", "space", 0.f, 1.f, 0.f, false, false, "", "mfx" },
    { "mCrushed", "crushed", 0.f, 1.f, 0.f, false, false, "", "mfx" },
    { "mPhased", "phased", 0.f, 1.f, 0.f, false, false, "", "mfx" },
    { "mGlued", "glued", 0.f, 1.f, 0.f, false, false, "", "mfx" },
};

struct ChoiceSpec {
    const char* id; const char* label; const char* items; int def;
    bool toggle;          // true: exposed to the host as an on/off switch
    const char* group;
};
inline const ChoiceSpec kChoiceSpecs[kNumChoiceParams] = {
    { "oscAWave",  "a wave",  "saw|square|tri",    0, false, "osc" },
    { "oscBWave",  "b wave",  "saw|square|tri",    0, false, "osc" },
    { "unison",    "unison",  "1|2|3|4",           2, false, "osc" },
    { "lfoDest",   "lfo to",  "cutoff|pitch|trem", 0, false, "mot" },
    { "eqOn",      "eq",      "off|on", 0, true, "eq" },
    { "crushOn",   "crush",   "off|on", 0, true, "crush" },
    { "phaserOn",  "phaser",  "off|on", 0, true, "phaser" },
    { "chorusOn",  "chorus",  "off|on", 1, true, "chorus" },
    { "compOn",    "compressor","off|on",0, true, "comp" },
    { "pumpOn",    "pump",    "off|on", 0, true, "pump" },
    { "delayOn",   "delay",   "off|on", 0, true, "delay" },
    { "reverbOn",  "reverb",  "off|on", 1, true, "reverb" },
    { "pumpRate",  "rate",    "1/4|1/8|1/2",       0, false, "pump" },
    { "delaySync", "time",    "1/16|1/8|1/8t|1/8d|1/4|1/4d|1/2", 3, false, "delay" },
    { "delayMode", "mode",    "mono|ping-pong",    1, false, "delay" },
};

// beat lengths matching the menus above
inline const float kPumpBeats[]  = { 1.f, 0.5f, 2.f };
inline const float kDelayBeats[] = { 0.25f, 0.5f, 1.f / 3.f, 0.75f, 1.f, 1.5f, 2.f };

inline int choiceCount (int c)
{
    int n = 1;
    for (const char* s = kChoiceSpecs[c].items; *s; ++s) if (*s == '|') ++n;
    return n;
}

struct Patch {
    float v[kNumFloatParams];
    int   c[kNumChoiceParams];   // unison stored 0-based

    static Patch defaults() {
        Patch p{};
        for (int i = 0; i < kNumFloatParams; ++i)  p.v[i] = kFloatSpecs[i].def;
        for (int i = 0; i < kNumChoiceParams; ++i) p.c[i] = kChoiceSpecs[i].def;
        return p;
    }
    void clampAll() {
        for (int i = 0; i < kNumFloatParams; ++i) {
            const auto& s = kFloatSpecs[i];
            float x = v[i];
            if (std::isnan(x)) x = s.def;
            if (x < s.lo) x = s.lo;
            if (x > s.hi) x = s.hi;
            if (s.stepped) x = std::round(x);
            v[i] = x;
        }
        for (int i = 0; i < kNumChoiceParams; ++i) {
            const int n = choiceCount(i);
            if (c[i] < 0) c[i] = 0;
            if (c[i] >= n) c[i] = n - 1;
        }
    }
};

inline float toNorm(int i, float value) {
    const auto& s = kFloatSpecs[i];
    if (s.logScale) return std::log(value / s.lo) / std::log(s.hi / s.lo);
    return (value - s.lo) / (s.hi - s.lo);
}
inline float fromNorm(int i, float n) {
    const auto& s = kFloatSpecs[i];
    if (n < 0.f) n = 0.f;
    if (n > 1.f) n = 1.f;
    float v = s.logScale ? s.lo * std::pow(s.hi / s.lo, n) : s.lo + n * (s.hi - s.lo);
    return s.stepped ? std::round(v) : v;
}


// ---------------------------------------------------------------------------
// Character knobs. Each blends the loaded patch toward one flavour, from 0
// (untouched) to 1 (full). They never overwrite the patch itself: the engine
// plays applyMacros(patch), so turning a knob back to 0 always restores the
// original sound. FX flavours switch their module on and fade its amount in
// from zero if it was off, or push the existing settings further if it was on.
// The browser version (core.js) mirrors this function exactly.
inline Patch applyMacros(const Patch& in)
{
    Patch p = in;
    auto amt   = [&](int i) { return std::min(1.f, std::max(0.f, in.v[i])); };
    auto scale = [&](int i, float f, float a) { p.v[i] *= std::pow(f, a); };
    auto toward = [&](int i, float target, float a) { p.v[i] += (target - p.v[i]) * a; };
    // amount-style parameter of an FX module: fade in from zero if the module was off
    auto fxAmount = [&](int toggle, int param, float target, float a) {
        if (in.c[toggle]) toward(param, target, a);
        else { p.v[param] = target * a; p.c[toggle] = 1; }
    };
    // switch the EQ on flat the first time a tone knob needs it
    auto eqToward = [&](int band, float target, float a) {
        if (! p.c[cEqOn]) { p.c[cEqOn] = 1; p.v[pEqLow] = p.v[pEqMid] = p.v[pEqHigh] = 0.f; }
        toward(band, target, a);
    };
    const float on = 0.0005f;
    float a;
    if ((a = amt(pMDark))   > on) { scale(pCutoff, .25f, a); p.v[pDust] += .10f * a; eqToward(pEqHigh, -6.f, a); }
    if ((a = amt(pMBright)) > on) { scale(pCutoff, 4.f, a); p.v[pDrive] -= .05f * a; eqToward(pEqHigh, 5.f, a); }
    if ((a = amt(pMDeep))   > on) { scale(pCutoff, .5f, a); p.v[pSubLevel] += .4f * a; eqToward(pEqLow, 4.f, a); }
    if ((a = amt(pMDirty))  > on) { toward(pDrive, .95f, a); p.v[pDust] += .35f * a; }
    if ((a = amt(pMClean))  > on) { p.v[pDrive] *= 1.f - .7f * a; p.v[pDust] *= 1.f - .8f * a; p.v[pCrush] *= 1.f - a; }
    if ((a = amt(pMWide))   > on) { scale(pSpread, 1.8f, a); scale(pDetune, 1.6f, a); fxAmount(cChorusOn, pChorus, .75f, a); }
    if ((a = amt(pMShort))  > on) { scale(pAmpA, .2f, a); scale(pFA, .2f, a); scale(pAmpD, .35f, a); p.v[pAmpS] *= 1.f - .8f * a; scale(pAmpR, .45f, a); scale(pFD, .45f, a); }
    if ((a = amt(pMLong))   > on) { scale(pAmpD, 2.5f, a); p.v[pAmpS] += .35f * a; scale(pAmpR, 2.5f, a); scale(pFD, 2.f, a); }
    if ((a = amt(pMPumped)) > on) {
        const bool was = in.c[cPumpOn] != 0;
        fxAmount(cPumpOn, pPumpDepth, .85f, a);
        if (was) toward(pPumpRelease, .42f, a); else p.v[pPumpRelease] = .42f;
    }
    if ((a = amt(pMDub)) > on) {
        const bool was = in.c[cDelayOn] != 0;
        fxAmount(cDelayOn, pDelayMix, .35f, a);
        if (was) { toward(pDelayFeedback, .6f, a); scale(pDelayTone, 2500.f / p.v[pDelayTone], a); }
        else     { p.v[pDelayFeedback] = .6f; p.v[pDelayTone] = 2500.f; p.c[cDelaySync] = 5; p.c[cDelayMode] = 1; }
        if (in.c[cReverbOn]) p.v[pReverb] += .12f * a; else fxAmount(cReverbOn, pReverb, .22f, a);
    }
    if ((a = amt(pMSpace)) > on) {
        if (in.c[cReverbOn]) { p.v[pReverb] += .35f * a; toward(pReverbSize, .92f, a); }
        else { fxAmount(cReverbOn, pReverb, .5f, a); p.v[pReverbSize] = .92f; p.v[pReverbDamp] = .4f; }
    }
    if ((a = amt(pMCrushed)) > on) fxAmount(cCrushOn, pCrush, .82f, a);
    if ((a = amt(pMPhased)) > on) {
        const bool was = in.c[cPhaserOn] != 0;
        fxAmount(cPhaserOn, pPhaserMix, .8f, a);
        if (! was) p.v[pPhaserRate] = .3f;
    }
    if ((a = amt(pMGlued)) > on) {
        // makeup roughly matches the gain reduction, so it thickens without getting quieter
        if (in.c[cCompOn]) { toward(pCompThresh, -20.f, a); toward(pCompRatio, 5.f, a); toward(pCompMakeup, 10.f, a); }
        else { p.c[cCompOn] = 1; p.v[pCompThresh] = -20.f * a; p.v[pCompRatio] = 1.f + 4.f * a;
               p.v[pCompMakeup] = 10.f * a; p.v[pCompAttack] = 10.f; p.v[pCompRelease] = 100.f; }
    }
    p.clampAll();
    return p;
}

} // namespace dustbox
