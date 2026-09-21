#pragma once
#include "Params.h"
#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>

// Everything that makes noise, framework-free so it can be unit-tested.
//
// Signal flow
//   voices -> EQ -> crush -> phaser -> tape wow        (mono)
//          -> chorus -> dust shelf + noise              (stereo from here)
//          -> compressor -> delay -> reverb -> pump -> level -> soft limit
//
// Every FX module crossfades in and out over ~10 ms, so toggling never
// clicks. Delay and reverb stop taking input when switched off but let their
// tails ring out, the way hardware sends do.

namespace dustbox {

constexpr int   kMaxVoices = 16;
constexpr int   kMaxUnison = 4;
constexpr float kPi = 3.14159265358979f;

inline float midiToHz(float n) { return 440.f * std::pow(2.f, (n - 69.f) / 12.f); }
inline float dbToGain(float db) { return std::pow(10.f, db * 0.05f); }
inline float softCeil(float x) {
    const float a = std::fabs(x), knee = 0.5f;
    if (a <= knee) return x;
    const float over = (a - knee) / (1.f - knee);
    return std::copysign(knee + (1.f - knee) * std::tanh(over), x);
}

struct Rng {
    uint32_t s = 22222u;
    float uni() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (float)(s >> 8) * (1.f / 16777216.f); }
    float bip() { return uni() * 2.f - 1.f; }
};

// ---------------------------------------------------------------- oscillator
struct Osc {
    float phase = 0.f, inc = 0.f, tri = 0.f;
    void setFreq(float hz, float sr) { inc = std::min(hz / sr, 0.45f); }
    void reset(float p) { phase = p; tri = 0.f; }
    static float blep(float t, float dt) {
        if (t < dt)       { t /= dt; return t + t - t * t - 1.f; }
        if (t > 1.f - dt) { t = (t - 1.f) / dt; return t * t + t + t + 1.f; }
        return 0.f;
    }
    float next(int wave) {
        phase += inc; if (phase >= 1.f) phase -= 1.f;
        if (wave == 0) return 2.f * phase - 1.f - blep(phase, inc);
        float sq = phase < .5f ? 1.f : -1.f;
        sq += blep(phase, inc);
        float p2 = phase + .5f; if (p2 >= 1.f) p2 -= 1.f;
        sq -= blep(p2, inc);
        if (wave == 1) return sq;
        tri += 4.f * inc * sq; tri *= 0.9995f;
        return tri;
    }
};

// ----------------------------------------------------- state-variable filter
struct SVF {
    float ic1 = 0.f, ic2 = 0.f, a1 = 0.f, a2 = 0.f, a3 = 0.f;
    void set(float fc, float q, float sr) {
        fc = std::max(20.f, std::min(fc, sr * 0.49f));
        const float g = std::tan(kPi * fc / sr), k = 1.f / std::max(0.5f, q);
        a1 = 1.f / (1.f + g * (g + k)); a2 = g * a1; a3 = g * a2;
    }
    float lp(float x) {
        const float v3 = x - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.f * v1 - ic1; ic2 = 2.f * v2 - ic2; return v2;
    }
    void reset() { ic1 = ic2 = 0.f; }
};

// ---------------------------------------------------------------- envelope
struct Env {
    enum Stage { Idle, Att, Dec, Sus, Rel };
    Stage stage = Idle; float value = 0.f, sus = 0.f, aInc = 0.f, dInc = 0.f, rInc = 0.f;
    void setTimes(float a, float d, float s, float r, float sr) {
        aInc = 1.f / std::max(1.f, a * sr); dInc = 1.f / std::max(1.f, d * sr);
        rInc = 1.f / std::max(1.f, r * sr); sus = s;
    }
    void gateOn()  { stage = Att; }
    void gateOff() { if (stage != Idle) stage = Rel; }
    bool active() const { return stage != Idle; }
    float next() {
        switch (stage) {
            case Att: value += aInc; if (value >= 1.f) { value = 1.f; stage = Dec; } break;
            case Dec: value -= dInc; if (value <= sus) { value = sus; stage = Sus;
                          if (sus <= 1e-4f) { value = 0.f; stage = Idle; } } break;
            case Sus: value = sus; break;
            case Rel: value -= rInc; if (value <= 0.f) { value = 0.f; stage = Idle; } break;
            case Idle: value = 0.f; break;
        }
        return value;
    }
    void hardReset() { stage = Idle; value = 0.f; }
};

// ---------------------------------------------------------------- voice
struct Voice {
    bool active = false; int note = -1, age = 0; float vel = .8f;
    float targetHz = 440.f, currentHz = 440.f, glideCoef = 0.f;
    Osc a[kMaxUnison], b[kMaxUnison], sub;
    float aDet[kMaxUnison] = {}, bDet[kMaxUnison] = {};
    SVF f1, f2; Env ampEnv, fltEnv;
    float driftPhase = 0.f, driftInc = 0.f, driftAmt = 0.f;
    Rng rng;

    void start(int n, float v, const Patch& p, float sr, float prevHz, uint32_t seed) {
        rng.s = seed | 1u; note = n; vel = v; active = true; age = 0;
        targetHz  = midiToHz((float) n);
        currentHz = (p.v[pGlide] > 1e-3f && prevHz > 0.f) ? prevHz : targetHz;
        glideCoef = p.v[pGlide] > 1e-3f ? std::exp(-1.f / (p.v[pGlide] * sr)) : 0.f;
        const int uni = p.c[cUnison] + 1;
        for (int i = 0; i < uni; ++i) {
            const float pos = uni > 1 ? (float) i / (float)(uni - 1) * 2.f - 1.f : 0.f;
            aDet[i] = pos * p.v[pSpread] + rng.bip() * 2.f;
            bDet[i] = pos * p.v[pSpread] + p.v[pOscBSemi] * 100.f + p.v[pDetune] + rng.bip() * 2.f;
            a[i].reset(rng.uni()); b[i].reset(rng.uni());
        }
        sub.reset(rng.uni()); f1.reset(); f2.reset();
        ampEnv.hardReset(); fltEnv.hardReset();
        ampEnv.setTimes(p.v[pAmpA], p.v[pAmpD], p.v[pAmpS], p.v[pAmpR], sr);
        fltEnv.setTimes(p.v[pFA], p.v[pFD], p.v[pFS], p.v[pAmpR], sr);
        ampEnv.gateOn(); fltEnv.gateOn();
        driftInc = (0.25f + rng.uni() * 0.7f) / sr; driftPhase = rng.uni();
        driftAmt = p.v[pDust] * 9.f;
    }
    void release() { ampEnv.gateOff(); fltEnv.gateOff(); }
    void kill() { active = false; note = -1; ampEnv.hardReset(); fltEnv.hardReset(); }
};

// ---------------------------------------------------------------- utilities
struct Delay {
    std::vector<float> buf; int w = 0;
    void prepare(int n) { buf.assign((size_t) std::max(8, n), 0.f); w = 0; }
    void push(float x) { buf[(size_t) w] = x; if (++w >= (int) buf.size()) w = 0; }
    float read(float d) const {
        const int n = (int) buf.size();
        d = std::max(1.f, std::min(d, (float) n - 2.f));
        float rp = (float) w - d; if (rp < 0.f) rp += (float) n;
        const int i0 = (int) rp; const float fr = rp - (float) i0;
        return buf[(size_t) i0] * (1.f - fr) + buf[(size_t)((i0 + 1) % n)] * fr;
    }
    void clear() { std::fill(buf.begin(), buf.end(), 0.f); }
};

struct Biquad {                                          // RBJ cookbook
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    float run(float x) { const float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
    void norm(float B0, float B1, float B2, float A0, float A1, float A2) {
        b0 = B0 / A0; b1 = B1 / A0; b2 = B2 / A0; a1 = A1 / A0; a2 = A2 / A0; }
    void lowShelf(float f, float db, float sr)  { shelf(f, db, sr, true); }
    void highShelf(float f, float db, float sr) { shelf(f, db, sr, false); }
    void shelf(float f, float db, float sr, bool low) {
        const float A = std::pow(10.f, db / 40.f), w = 2.f * kPi * f / sr;
        const float c = std::cos(w), s = std::sin(w), al = s / 2.f * std::sqrt(2.f), sq = 2.f * std::sqrt(A) * al;
        if (low) norm(A*((A+1)-(A-1)*c+sq), 2*A*((A-1)-(A+1)*c), A*((A+1)-(A-1)*c-sq),
                      (A+1)+(A-1)*c+sq, -2*((A-1)+(A+1)*c), (A+1)+(A-1)*c-sq);
        else     norm(A*((A+1)+(A-1)*c+sq), -2*A*((A-1)+(A+1)*c), A*((A+1)+(A-1)*c-sq),
                      (A+1)-(A-1)*c+sq, 2*((A-1)-(A+1)*c), (A+1)-(A-1)*c-sq);
    }
    void peak(float f, float db, float q, float sr) {
        const float A = std::pow(10.f, db / 40.f), w = 2.f * kPi * f / sr, al = std::sin(w) / (2.f * q), c = std::cos(w);
        norm(1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A);
    }
    void reset() { z1 = z2 = 0.f; }
};

struct Reverb {                                          // compact Freeverb
    static constexpr int kC[8] = {1116,1188,1277,1356,1422,1491,1557,1617};
    static constexpr int kA[4] = {556,441,341,225};
    struct Comb { std::vector<float> b; int i = 0; float st = 0.f;
        float run(float x, float fb, float d) { float y = b[(size_t) i]; st = y * (1.f - d) + st * d;
            b[(size_t) i] = x + st * fb; if (++i >= (int) b.size()) i = 0; return y; } };
    struct Ap { std::vector<float> b; int i = 0;
        float run(float x) {
            const float y = b[(size_t) i];
            b[(size_t) i] = x + y * .5f;
            if (++i >= (int) b.size()) i = 0;
            return y - x;
        } };
    Comb cl[8], cr[8]; Ap al[4], ar[4];
    void prepare(float sr) {
        const float s = sr / 44100.f;
        for (int i = 0; i < 8; ++i) { cl[i].b.assign((size_t) std::max(8, (int)((float) kC[i] * s)), 0.f); cl[i].i = 0; cl[i].st = 0;
                                      cr[i].b.assign((size_t) std::max(8, (int)((float)(kC[i] + 23) * s)), 0.f); cr[i].i = 0; cr[i].st = 0; }
        for (int i = 0; i < 4; ++i) { al[i].b.assign((size_t) std::max(8, (int)((float) kA[i] * s)), 0.f); al[i].i = 0;
                                      ar[i].b.assign((size_t) std::max(8, (int)((float)(kA[i] + 23) * s)), 0.f); ar[i].i = 0; }
    }
    void process(float in, float& L, float& R, float fb, float damp) {
        const float x = in * 0.015f; float l = 0, r = 0;
        for (int i = 0; i < 8; ++i) { l += cl[i].run(x, fb, damp); r += cr[i].run(x, fb, damp); }
        for (int i = 0; i < 4; ++i) { l = al[i].run(l); r = ar[i].run(r); }
        L = l; R = r;
    }
};

// Smoothed 0..1 "is this module on" value, for click-free toggling.
struct Fade {
    float v = 0.f, coef = 0.999f;
    void prepare(float sr) { coef = std::exp(-1.f / (0.01f * sr)); }
    float next(bool on) { const float t = on ? 1.f : 0.f; v = t + (v - t) * coef; return v; }
    void snap(bool on) { v = on ? 1.f : 0.f; }
};

// ---------------------------------------------------------------- engine
class SynthEngine {
public:
    void prepare(double sampleRate, int) {
        sr = (float) sampleRate;
        for (auto& v : voices) v.kill();
        chorusA.prepare((int)(sr * .05f) + 4); chorusB.prepare((int)(sr * .05f) + 4);
        wow.prepare((int)(sr * .05f) + 4);
        delayL.prepare((int)(sr * 4.f) + 8); delayR.prepare((int)(sr * 4.f) + 8);
        reverb.prepare(sr);
        for (auto* f : { &fEq, &fCrush, &fPhaser, &fChorus, &fComp, &fPump, &fDelayIn, &fRevIn }) f->prepare(sr);
        for (auto* b : { &eqLo, &eqMid, &eqHi }) b->reset();
        for (auto& s : phaserState) s = 0.f;
        compEnv = 0.f; pumpGain = 1.f; phaserFb = 0.f; crushHold = 0.f; crushCount = 0;
        dlyLpL = dlyLpR = 0.f; noiseL = noiseR = shelfL = shelfR = 0.f;
        lfoPh = chA = chB = wowA = wowB = phaserPh = 0.f;
        delaySmooth = -1.f; ppq = 0.0; lastHz = 0.f;
        snapFades = true;
    }

    // the engine always plays the patch with the character knobs applied
    void setPatch(const Patch& p) { patch = applyMacros(p); }
    const Patch& getPatch() const { return patch; }

    // Host transport. When the host isn't playing the pump and delay keep
    // running from their own clock at the last known tempo.
    void setTransport(double bpmIn, double ppqIn, bool playing) {
        if (bpmIn > 20.0 && bpmIn < 400.0) bpm = bpmIn;
        if (playing) ppq = ppqIn;
    }

    void noteOn(int note, float velocity) {
        Voice* slot = nullptr;
        for (auto& v : voices) if (! v.active) { slot = &v; break; }
        if (! slot) { int o = 0; for (int i = 1; i < kMaxVoices; ++i) if (voices[i].age > voices[o].age) o = i; slot = &voices[o]; }
        slot->start(note, velocity, patch, sr, lastHz, ++seed * 2654435761u);
        lastHz = midiToHz((float) note);
    }
    void noteOff(int note) {
        for (auto& v : voices) if (v.active && v.note == note && v.ampEnv.stage != Env::Rel) v.release();
    }
    void allNotesOff(bool now) { for (auto& v : voices) { if (now) v.kill(); else if (v.active) v.release(); } }
    int activeVoices() const { int c = 0; for (const auto& v : voices) if (v.active) ++c; return c; }

    void process(float* outL, float* outR, int n) {
        const Patch& p = patch;
        const auto on = [&](int c) { return p.c[c] != 0; };
        if (snapFades) {
            fEq.snap(on(cEqOn)); fCrush.snap(on(cCrushOn)); fPhaser.snap(on(cPhaserOn));
            fChorus.snap(on(cChorusOn)); fComp.snap(on(cCompOn)); fPump.snap(on(cPumpOn));
            fDelayIn.snap(on(cDelayOn)); fRevIn.snap(on(cReverbOn)); snapFades = false;
        }

        // ---- per-block constants -------------------------------------------
        const int   uni = p.c[cUnison] + 1;
        const float uniGain = 1.f / std::sqrt((float) uni);
        const float driveK = p.v[pDrive] * 70.f, driveComp = 1.f / (1.f + p.v[pDrive] * 1.7f);
        const float lfoInc = p.v[pLfoRate] / sr;
        const float dust = p.v[pDust];
        const float shelfCoef = std::exp(-2.f * kPi * (6000.f - dust * 3800.f) / sr);

        eqLo.lowShelf(120.f, p.v[pEqLow], sr);
        eqMid.peak(p.v[pEqMidFreq], p.v[pEqMid], 0.9f, sr);
        eqHi.highShelf(7000.f, p.v[pEqHigh], sr);

        const float crush = p.v[pCrush];
        const float crushLevels = std::pow(2.f, 16.f - crush * 12.f);
        const int   crushHoldN = 1 + (int)(crush * crush * 20.f);

        const float compT = p.v[pCompThresh], compR = p.v[pCompRatio];
        const float compAtk = std::exp(-1.f / (p.v[pCompAttack] * 0.001f * sr));
        const float compRel = std::exp(-1.f / (p.v[pCompRelease] * 0.001f * sr));
        const float compMk  = p.v[pCompMakeup];

        const double beatsPerSample = bpm / 60.0 / (double) sr;
        const float  pumpDiv = kPumpBeats[p.c[cPumpRate]];
        const float  pumpDepth = p.v[pPumpDepth], pumpShape = p.v[pPumpRelease];
        const float  pumpSmooth = std::exp(-1.f / (0.0015f * sr));

        const float delayTarget = (float)(kDelayBeats[p.c[cDelaySync]] * 60.0 / bpm) * sr;
        if (delaySmooth < 0.f) delaySmooth = delayTarget;
        const float dlyTone = std::exp(-2.f * kPi * p.v[pDelayTone] / sr);
        const bool  pingPong = p.c[cDelayMode] == 1;

        const float revFb = 0.70f + p.v[pReverbSize] * 0.28f, revDamp = p.v[pReverbDamp] * 0.7f;
        const float outGain = p.v[pVolume] * 2.0f;

        for (int s = 0; s < n; ++s) {
            lfoPh += lfoInc; if (lfoPh >= 1.f) lfoPh -= 1.f;
            const float lfo = std::sin(2.f * kPi * lfoPh);

            // ---- voices ------------------------------------------------------
            float mono = 0.f;
            for (auto& v : voices) {
                if (! v.active) continue;
                ++v.age;
                v.currentHz = v.glideCoef > 0.f ? v.targetHz + (v.currentHz - v.targetHz) * v.glideCoef : v.targetHz;
                v.driftPhase += v.driftInc; if (v.driftPhase >= 1.f) v.driftPhase -= 1.f;
                float cents = std::sin(2.f * kPi * v.driftPhase) * v.driftAmt;
                if (p.c[cLfoDest] == 1) cents += lfo * p.v[pLfoDepth] * 55.f;
                const float baseHz = v.currentHz * std::pow(2.f, cents / 1200.f);

                float x = 0.f;
                for (int i = 0; i < uni; ++i) {
                    if (p.v[pOscALevel] > 1e-3f) { v.a[i].setFreq(baseHz * std::pow(2.f, v.aDet[i] / 1200.f), sr);
                        x += v.a[i].next(p.c[cOscAWave]) * p.v[pOscALevel] * uniGain; }
                    if (p.v[pOscBLevel] > 1e-3f) { v.b[i].setFreq(baseHz * std::pow(2.f, v.bDet[i] / 1200.f), sr);
                        x += v.b[i].next(p.c[cOscBWave]) * p.v[pOscBLevel] * uniGain; }
                }
                if (p.v[pSubLevel] > 1e-3f) {
                    v.sub.setFreq(baseHz * .5f, sr);
                    x += std::sin(2.f * kPi * v.sub.phase) * p.v[pSubLevel] * .9f;
                    v.sub.phase += v.sub.inc; if (v.sub.phase >= 1.f) v.sub.phase -= 1.f;
                }
                x = (1.f + driveK) * x / (1.f + driveK * std::fabs(x)) * driveComp;

                const float fe = v.fltEnv.next();
                float cut = p.v[pCutoff] * std::pow(2.f, p.v[pKeytrack] * (float)(v.note - 60) / 12.f)
                                         * std::pow(2.f, p.v[pFEnvAmt] * fe);
                if (p.c[cLfoDest] == 0) cut *= std::pow(2.f, lfo * p.v[pLfoDepth] * 1.5f);
                v.f1.set(cut, p.v[pResonance], sr); v.f2.set(cut, .7f, sr);
                x = v.f2.lp(v.f1.lp(x));

                float amp = v.ampEnv.next() * v.vel;
                if (p.c[cLfoDest] == 2) amp *= 1.f - p.v[pLfoDepth] * (0.5f - 0.5f * lfo);
                x *= amp;
                if (! v.ampEnv.active()) v.kill();
                mono += x;
            }
            mono *= 0.8f;

            // ---- EQ ----------------------------------------------------------
            { const float g = fEq.next(on(cEqOn));
              if (g > 1e-4f) { const float e = eqHi.run(eqMid.run(eqLo.run(mono))); mono += (e - mono) * g; } }

            // ---- crush -------------------------------------------------------
            { const float g = fCrush.next(on(cCrushOn));
              if (g > 1e-4f) {
                  if (++crushCount >= crushHoldN) { crushCount = 0; crushHold = std::round(mono * crushLevels) / crushLevels; }
                  mono += (crushHold - mono) * g; } }

            // ---- phaser: four first-order allpasses swept by a slow LFO ------
            { const float g = fPhaser.next(on(cPhaserOn));
              phaserPh += p.v[pPhaserRate] / sr; if (phaserPh >= 1.f) phaserPh -= 1.f;
              if (g > 1e-4f) {
                  const float fc = 300.f * std::pow(2.f, 1.8f + 1.8f * std::sin(2.f * kPi * phaserPh));
                  const float t = std::tan(kPi * std::min(fc, sr * .45f) / sr), a = (t - 1.f) / (t + 1.f);
                  float y = mono + phaserFb * 0.45f;
                  for (auto& st : phaserState) { const float o = a * y + st; st = y - a * o; y = o; }
                  phaserFb = y;
                  const float wet = 0.5f * (mono + y);
                  mono += (wet - mono) * g * p.v[pPhaserMix]; } }

            // ---- tape wow (part of the dust character) -------------------------
            if (dust > 1e-3f) {
                wowA += .7f / sr; if (wowA >= 1.f) wowA -= 1.f;
                wowB += .13f / sr; if (wowB >= 1.f) wowB -= 1.f;
                const float wob = .5f * (std::sin(2.f * kPi * wowA) + std::sin(2.f * kPi * wowB));
                wow.push(mono); mono = wow.read((.008f + .0014f * dust * wob) * sr);
            }

            // ---- chorus: two modulated taps, hard-panned -----------------------
            chA += .23f / sr; if (chA >= 1.f) chA -= 1.f;
            chB += .31f / sr; if (chB >= 1.f) chB -= 1.f;
            chorusA.push(mono); chorusB.push(mono);
            const float cg = fChorus.next(on(cChorusOn)) * p.v[pChorus] * .9f;
            float l = mono + chorusA.read((.012f + .0032f * std::sin(2.f * kPi * chA)) * sr) * cg;
            float r = mono + chorusB.read((.019f + .0032f * std::sin(2.f * kPi * chB)) * sr) * cg;

            // ---- dust shelf + noise floor --------------------------------------
            shelfL = l + shelfCoef * (shelfL - l); shelfR = r + shelfCoef * (shelfR - r);
            l += (shelfL - l) * dust * .35f; r += (shelfR - r) * dust * .35f;
            if (dust > 1e-3f) {
                noiseL = noiseL * .92f + rng.bip() * .08f; noiseR = noiseR * .92f + rng.bip() * .08f;
                l += noiseL * dust * .0066f; r += noiseR * dust * .0066f;
            }

            // ---- compressor: stereo-linked, soft knee --------------------------
            { const float g = fComp.next(on(cCompOn));
              const float peakDb = 20.f * std::log10(std::max(std::fabs(l), std::fabs(r)) + 1e-9f);
              const float over = peakDb - compT, knee = 6.f, slope = 1.f - 1.f / compR;
              float red = 0.f;
              if (over >= knee * .5f) red = over * slope;
              else if (over > -knee * .5f) { const float k = over + knee * .5f; red = k * k / (2.f * knee) * slope; }
              compEnv = red > compEnv ? red + (compEnv - red) * compAtk : red + (compEnv - red) * compRel;
              if (g > 1e-4f) { const float cgain = dbToGain(compMk - compEnv);
                               l += (l * cgain - l) * g; r += (r * cgain - r) * g; } }

            // ---- delay: tempo-synced, optional ping-pong -------------------------
            { const float in = fDelayIn.next(on(cDelayOn));
              delaySmooth = delayTarget + (delaySmooth - delayTarget) * 0.9995f;
              const float dl = delayL.read(delaySmooth), dr = delayR.read(delaySmooth);
              dlyLpL = dl + dlyTone * (dlyLpL - dl); dlyLpR = dr + dlyTone * (dlyLpR - dr);
              const float fb = p.v[pDelayFeedback];
              if (pingPong) { delayL.push((l + r) * .5f * in + dlyLpR * fb); delayR.push(dlyLpL * fb); }
              else          { delayL.push(l * in + dlyLpL * fb);             delayR.push(r * in + dlyLpR * fb); }
              l += dl * p.v[pDelayMix]; r += dr * p.v[pDelayMix]; }

            // ---- reverb ----------------------------------------------------------
            { const float in = fRevIn.next(on(cReverbOn));
              float wl = 0.f, wr = 0.f;
              reverb.process((l + r) * .5f * in, wl, wr, revFb, revDamp);
              l += wl * p.v[pReverb] * 1.6f; r += wr * p.v[pReverb] * 1.6f; }

            // ---- pump: tempo-locked ducking, like a sidechain from the kick ------
            { const float g = fPump.next(on(cPumpOn));
              double ph = std::fmod(ppq, (double) pumpDiv) / (double) pumpDiv;
              if (ph < 0.0) ph += 1.0;
              float target = 1.f;
              if (ph < pumpShape) { const float x = (float) ph / pumpShape; target = 1.f - pumpDepth * (1.f - x) * (1.f - x); }
              pumpGain = target + (pumpGain - target) * pumpSmooth;
              const float pg = 1.f + (pumpGain - 1.f) * g;
              l *= pg; r *= pg; }
            ppq += beatsPerSample;

            // ---- out ------------------------------------------------------------
            // transparent below ~-6 dBFS, then a smooth knee that never passes 0 dBFS
            l *= outGain; r *= outGain;
            outL[s] = softCeil(l);
            outR[s] = softCeil(r);
        }
        if (ppq > 1.0e6) ppq = std::fmod(ppq, 64.0);
    }

private:
    Patch patch = Patch::defaults();
    Voice voices[kMaxVoices];
    float sr = 44100.f, lastHz = 0.f;
    double bpm = 124.0, ppq = 0.0;
    uint32_t seed = 7u;
    Rng rng;

    Delay chorusA, chorusB, wow, delayL, delayR;
    Reverb reverb;
    Biquad eqLo, eqMid, eqHi;
    Fade fEq, fCrush, fPhaser, fChorus, fComp, fPump, fDelayIn, fRevIn;
    bool snapFades = true;

    float lfoPh = 0, chA = 0, chB = 0, wowA = 0, wowB = 0, phaserPh = 0;
    float shelfL = 0, shelfR = 0, noiseL = 0, noiseR = 0;
    float phaserState[4] = {}, phaserFb = 0;
    float crushHold = 0; int crushCount = 0;
    float compEnv = 0, pumpGain = 1;
    float dlyLpL = 0, dlyLpR = 0, delaySmooth = -1.f;
};

} // namespace dustbox
