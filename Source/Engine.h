#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <algorithm>
#include <atomic>
#include <vector>
#include <cmath>
#include "Params.h"

namespace sp
{
constexpr float kTwoPi = 6.28318530718f;

//==============================================================================
// Band-limited wavetables: 8 shapes x 11 mip levels, built once by inverse FFT.
struct Wavetables
{
    static constexpr int N = 2048, LEVELS = 11, SHAPES = 8;
    std::vector<float> data;

    static const char* shapeName (int s)
    {
        static const char* n[] = { "Sine", "Tri", "Saw", "Square", "Pulse", "Organ", "Digi", "Vox" };
        return n[juce::jlimit (0, SHAPES - 1, s)];
    }

    static const Wavetables& get() { static Wavetables w; return w; }

    const float* table (int shape, int level) const { return data.data() + (size_t) ((shape * LEVELS + level) * (N + 1)); }

    // amplitude of cos (a) and sin (b) for harmonic k of a shape
    static void coeff (int s, int k, float& a, float& b)
    {
        const float fk = (float) k;
        a = 0.0f; b = 0.0f;
        switch (s)
        {
            case 0: b = (k == 1) ? 1.0f : 0.0f; break;
            case 1: if (k & 1) b = ((((k - 1) / 2) & 1) ? -1.0f : 1.0f) / (fk * fk); break;
            case 2: b = ((k & 1) ? 1.0f : -1.0f) / fk; break;
            case 3: if (k & 1) b = 1.0f / fk; break;
            case 4: { const float d = 0.25f; b = (1.0f - std::cos (kTwoPi * fk * d)) / fk; a = -std::sin (kTwoPi * fk * d) / fk; break; }
            case 5: { // drawbar organ 16' 8' 5 1/3' 4' 2 2/3' 2'
                switch (k) { case 1: b = 1.0f; break; case 2: b = 0.8f; break; case 3: b = 0.6f; break;
                             case 4: b = 0.5f; break; case 6: b = 0.3f; break; case 8: b = 0.25f; break; default: break; }
                break; }
            case 6: { // hollow digital organ, the 90s house stab family
                static const int hs[] = { 1, 2, 3, 4, 6, 8, 10, 12, 16 };
                for (int h : hs) if (h == k) b = 1.0f / std::pow (fk, 0.7f);
                break; }
            case 7: { // vowel-ish formants
                if (k <= 48)
                {
                    auto g = [] (float x, float c, float w) { return std::exp (-0.5f * (x - c) * (x - c) / (w * w)); };
                    b = (g (fk, 3.0f, 1.3f) + 0.7f * g (fk, 9.0f, 2.5f) + 0.25f * g (fk, 16.0f, 3.0f) + 0.08f) / std::sqrt (fk);
                }
                break; }
            default: break;
        }
    }

    Wavetables()
    {
        data.assign ((size_t) (SHAPES * LEVELS * (N + 1)), 0.0f);
        juce::dsp::FFT fft (11); // 2048
        std::vector<float> buf ((size_t) N * 2);
        for (int s = 0; s < SHAPES; ++s)
        {
            float norm = 1.0f;
            for (int l = 0; l < LEVELS; ++l)
            {
                const int maxH = juce::jmax (1, 1024 >> l);
                std::fill (buf.begin(), buf.end(), 0.0f);
                for (int k = 1; k < juce::jmin (maxH + 1, N / 2); ++k)
                {
                    float a, b; coeff (s, k, a, b);
                    buf[(size_t) (2 * k)]     = a;   // real
                    buf[(size_t) (2 * k + 1)] = -b;  // imag (sin term)
                }
                fft.performRealOnlyInverseTransform (buf.data());
                float* t = const_cast<float*> (table (s, l));
                for (int i = 0; i < N; ++i) t[i] = buf[(size_t) i];
                if (l == 0)
                {
                    float pk = 0.0f;
                    for (int i = 0; i < N; ++i) pk = juce::jmax (pk, std::abs (t[i]));
                    norm = pk > 0.0f ? 1.0f / pk : 1.0f;
                }
                for (int i = 0; i < N; ++i) t[i] *= norm;
                t[N] = t[0];
            }
        }
    }

    static inline int levelFor (float phaseInc)
    {
        // allowed harmonics before nyquist; level l holds 1024 >> l harmonics
        const float maxH = 0.47f / juce::jmax (phaseInc, 1.0e-7f);
        int l = 0;
        while (l < LEVELS - 1 && (float) (1024 >> l) > maxH) ++l;
        return l;
    }

    inline float read (float wave, float phase, int level) const
    {
        const int s0 = juce::jlimit (0, SHAPES - 1, (int) wave);
        const int s1 = juce::jmin (SHAPES - 1, s0 + 1);
        const float fr = juce::jlimit (0.0f, 1.0f, wave - (float) s0);
        const float idx = phase * (float) N;
        const int i = juce::jlimit (0, N - 1, (int) idx);
        const float f = idx - (float) i;
        const float* t0 = table (s0, level);
        const float v0 = t0[i] + f * (t0[i + 1] - t0[i]);
        if (fr < 1.0e-4f || s0 == s1) return v0;
        const float* t1 = table (s1, level);
        const float v1 = t1[i] + f * (t1[i + 1] - t1[i]);
        return v0 + fr * (v1 - v0);
    }
};

//==============================================================================
struct Env
{
    enum St { Idle, Att, Dec, Sus, Rel } st = Idle;
    float v = 0.0f, aInc = 0.01f, dCoef = 0.99f, rCoef = 0.99f, sus = 1.0f;

    void set (float a, float d, float s, float r, float sr)
    {
        aInc = 1.0f / juce::jmax (1.0f, a * sr);
        dCoef = std::exp (-4.6f / juce::jmax (1.0f, d * sr));
        rCoef = std::exp (-4.6f / juce::jmax (1.0f, r * sr));
        sus = s;
    }
    void on()  { st = Att; }
    void off() { if (st != Idle) st = Rel; }
    inline float next()
    {
        switch (st)
        {
            case Att: v += aInc; if (v >= 1.0f) { v = 1.0f; st = Dec; } break;
            case Dec: v = sus + (v - sus) * dCoef; if (std::abs (v - sus) < 1.0e-4f) { v = sus; st = Sus; } break;
            case Sus: v = sus; break;
            case Rel: v *= rCoef; if (v < 1.0e-5f) { v = 0.0f; st = Idle; } break;
            case Idle: default: v = 0.0f; break;
        }
        return v;
    }
};

struct SVF
{
    float ic1 = 0.0f, ic2 = 0.0f;
    void reset() { ic1 = ic2 = 0.0f; }
    inline void tick (float x, float g, float k, float& lp, float& bp, float& hp)
    {
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1, a3 = g * a2;
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1; ic2 = 2.0f * v2 - ic2;
        lp = v2; bp = v1; hp = x - k * v1 - v2;
    }
};

// values that the engine computes once per control chunk and shares with all voices
struct Ctl
{
    float sr = 48000.0f;
    const float* p = nullptr;   // effective parameter values (after XY)
    float lfo = 0.0f;           // -1..1
    float bend = 0.0f;          // semitones
    float cut = 8000.0f;        // smoothed base cutoff
};

inline float fastTanh (float x)
{
    x = juce::jlimit (-4.5f, 4.5f, x);
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

//==============================================================================
struct Voice
{
    static constexpr int MAXU = 7;
    bool active = false, gateOpen = false;
    int note = -1; float velocity = 1.0f; uint32_t age = 0;
    float curNote = 60.0f, targetNote = 60.0f;
    float phA[MAXU] {}, phB[MAXU] {}, phSub = 0.0f;
    float pitchEnv = 0.0f;
    Env amp, fil;
    SVF s1[2], s2[2];
    juce::Random rnd { 1234 };

    void start (int n, float vel, float fromNote, bool glide, bool retriggerEnv, int uni, uint32_t a)
    {
        note = n; velocity = vel; age = a; targetNote = (float) n;
        curNote = glide ? fromNote : (float) n;
        if (retriggerEnv || ! active)
        {
            pitchEnv = 1.0f;
            const bool randomPhase = uni > 1;
            if (! active || amp.v < 0.02f)   // keep phases on a sounding voice so retriggers don't click
            {
                for (int i = 0; i < MAXU; ++i)
                {
                    phA[i] = randomPhase ? rnd.nextFloat() : 0.0f;
                    phB[i] = randomPhase ? rnd.nextFloat() : 0.0f;
                }
                phSub = 0.0f;
            }
            if (! active) { for (auto& f : s1) f.reset(); for (auto& f : s2) f.reset(); amp.v = 0.0f; fil.v = 0.0f; }
            amp.on(); fil.on();
        }
        active = true; gateOpen = true;
    }
    void release() { gateOpen = false; amp.off(); fil.off(); }

    void render (float* L, float* R, int n, const Ctl& c)
    {
        const float* p = c.p;
        const auto& wt = Wavetables::get();
        const float sr = c.sr;
        amp.set (p[aa], p[ad], p[as], p[ar], sr);
        fil.set (p[fa], p[fd], p[fs], p[fr], sr);

        // glide (per chunk)
        if (p[glide] > 0.0005f)
        {
            const float coef = std::exp (-(float) n / (p[glide] * 0.6f * sr));
            curNote = targetNote + (curNote - targetNote) * coef;
        }
        else curNote = targetNote;

        const int dest = (int) p[l_dest];
        const float lfoV = c.lfo * p[l_amt];
        const float pe = p[penv] * pitchEnv;
        pitchEnv *= std::exp (-(float) n * 4.6f / (p[pdec] * sr));

        const float baseNote = curNote + pe + c.bend + (dest == 1 ? lfoV * 12.0f : 0.0f);
        const float nA = baseNote + p[a_oct] * 12.0f + p[a_semi] + p[a_fine] * 0.01f;
        const float nB = baseNote + p[b_oct] * 12.0f + p[b_semi] + p[b_fine] * 0.01f;
        const float fA = 440.0f * std::exp2 ((nA - 69.0f) / 12.0f);
        const float fB = 440.0f * std::exp2 ((nB - 69.0f) / 12.0f);
        const float fSub = 440.0f * std::exp2 ((curNote + pe + c.bend + p[a_oct] * 12.0f - 12.0f - 69.0f) / 12.0f);

        float wA = p[a_wave] + (dest == 3 ? lfoV * 3.5f : 0.0f);
        float wB = p[b_wave] + (dest == 4 ? lfoV * 3.5f : 0.0f);
        wA = juce::jlimit (0.0f, 6.999f, wA); wB = juce::jlimit (0.0f, 6.999f, wB);
        const float fmAmt = juce::jlimit (0.0f, 1.0f, p[fm] + (dest == 6 ? lfoV : 0.0f)) * 1.2f;

        const int U = juce::jlimit (1, MAXU, (int) p[uni]);
        float incA[MAXU], incB[MAXU], panL[MAXU], panR[MAXU];
        const float det = p[detune] * p[detune] * 70.0f; // cents at the edges
        for (int i = 0; i < U; ++i)
        {
            const float pos = U == 1 ? 0.0f : -1.0f + 2.0f * (float) i / (float) (U - 1);
            const float ratio = std::exp2 (pos * det / 1200.0f);
            incA[i] = fA * ratio / sr; incB[i] = fB * ratio / sr;
            const float pan = 0.5f + 0.5f * pos * p[spread];
            panL[i] = std::cos (pan * 1.5707963f); panR[i] = std::sin (pan * 1.5707963f);
        }
        const int lvA = Wavetables::levelFor (incA[U - 1]);
        const int lvB = Wavetables::levelFor (incB[U - 1]);
        const float incSub = fSub / sr;
        const float uGain = 1.0f / std::sqrt ((float) U);
        const float la = p[a_lvl] * uGain, lb = p[b_lvl] * uGain;
        const bool needB = lb > 0.0f || fmAmt > 0.0f;

        // filter coefficients for this chunk
        const float velScale = 1.0f - p[vel] + p[vel] * velocity;
        float filtEnvNow = fil.v;
        float oct = p[fenv] * filtEnvNow * 7.0f * (0.6f + 0.4f * velScale)
                  + p[fkey] * (curNote - 60.0f) / 12.0f
                  + (dest == 0 ? lfoV * 4.0f : 0.0f);
        const float fc = juce::jlimit (20.0f, sr * 0.45f, c.cut * std::exp2 (oct));
        const float g = std::tan (juce::MathConstants<float>::pi * fc / sr);
        const float k = 2.0f - 1.94f * p[reso];
        const int ft = (int) p[ftype];
        const float drv = 1.0f + p[fdrive] * 8.0f;
        const float drvComp = 1.0f / (1.0f + p[fdrive] * 1.5f);

        float trem = 1.0f, panMod = 0.0f;
        if (dest == 2) trem = 1.0f - p[l_amt] * (0.5f + 0.5f * c.lfo);
        if (dest == 5) panMod = lfoV;
        const float vpL = std::cos ((0.5f + 0.5f * panMod) * 1.5707963f) * 1.41421f;
        const float vpR = std::sin ((0.5f + 0.5f * panMod) * 1.5707963f) * 1.41421f;
        const float gain = 0.32f * velScale * trem;

        for (int s = 0; s < n; ++s)
        {
            float l = 0.0f, r = 0.0f;
            for (int i = 0; i < U; ++i)
            {
                float b = 0.0f;
                if (needB)
                {
                    b = wt.read (wB, phB[i], lvB);
                    phB[i] += incB[i]; if (phB[i] >= 1.0f) phB[i] -= 1.0f;
                }
                float ph = phA[i] + fmAmt * b;
                ph -= std::floor (ph);
                const float a = wt.read (wA, ph, lvA);
                phA[i] += incA[i]; if (phA[i] >= 1.0f) phA[i] -= 1.0f;
                const float o = a * la + b * lb;
                l += o * panL[i]; r += o * panR[i];
            }
            const float sb = std::sin (kTwoPi * phSub) * p[sub] * 0.8f;
            phSub += incSub; if (phSub >= 1.0f) phSub -= 1.0f;
            const float nz = (rnd.nextFloat() * 2.0f - 1.0f) * p[noise] * 0.5f;
            l += (sb + nz) * 0.7071f; r += (sb + nz) * 0.7071f;

            float io[2] = { l, r };
            for (int ch = 0; ch < 2; ++ch)
            {
                float x = io[ch];
                if (p[fdrive] > 0.001f) x = fastTanh (x * drv) * drvComp;
                float lp, bp, hp;
                s1[ch].tick (x, g, k, lp, bp, hp);
                float y;
                if (ft == 0) { float lp2, bp2, hp2; s2[ch].tick (lp, g, 1.414f, lp2, bp2, hp2); y = lp2; }
                else if (ft == 1) y = lp;
                else if (ft == 2) y = bp * (0.5f + k * 0.5f);
                else y = hp;
                io[ch] = y;
            }
            fil.next();
            const float e = amp.next() * gain;
            L[s] += io[0] * e * vpL;
            R[s] += io[1] * e * vpR;
        }
        if (amp.st == Env::Idle) { active = false; gateOpen = false; note = -1; }
    }
};

//==============================================================================
struct Clock { double bpm = 124.0, ppq = 0.0; bool playing = false; };

class Engine
{
public:
    static constexpr int NV = 16, CR = 8;

    void prepare (double sampleRate, int maxBlock)
    {
        sr = (float) sampleRate;
        Wavetables::get();
        const int dlen = (int) (sr * 4.0f) + 16;
        for (auto& d : dly) { d.assign ((size_t) dlen, 0.0f); }
        dPos = 0;
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (maxBlock, 16), 2 };
        chorus.prepare (spec);
        reverb.setSampleRate (sampleRate);
        tmpL.assign ((size_t) CR, 0.0f); tmpR.assign ((size_t) CR, 0.0f);
        reset();
    }

    void reset()
    {
        for (auto& v : voices) { v.active = false; v.note = -1; v.amp.st = Env::Idle; }
        held.clear(); monoStack.clear(); arpNotes.clear();
        for (auto& d : dly) std::fill (d.begin(), d.end(), 0.0f);
        reverb.reset(); chorus.reset();
        for (auto& f : eqLo) f.reset();
        for (auto& f : eqHi) f.reset();
        gateGain = 1.0f; dlyLp[0] = dlyLp[1] = dlyHp[0] = dlyHp[1] = 0.0f;
    }

    // visual feedback for the UI
    std::atomic<int> uiGateStep { -1 }, uiActiveVoices { 0 };
    std::atomic<float> uiLfo { 0.0f }, uiPump { 1.0f };

    void process (juce::AudioBuffer<float>& buf, const juce::MidiBuffer& midi, const float* p, const Clock& clk)
    {
        const int n = buf.getNumSamples();
        buf.clear();
        if (buf.getNumChannels() < 2) return;
        float* L = buf.getWritePointer (0);
        float* R = buf.getWritePointer (1);

        bpm = clk.bpm > 20.0 ? clk.bpm : 124.0;
        if (clk.playing) { ppq = clk.ppq; hostPlaying = true; }
        else if (hostPlaying) { hostPlaying = false; }
        const double ppqPerSample = bpm / 60.0 / sr;

        Ctl c; c.sr = sr; c.p = p; c.bend = bend;

        auto it = midi.begin();
        int pos = 0;
        while (pos < n)
        {
            // events at or before this position
            while (it != midi.end() && (*it).samplePosition <= pos)
            {
                handleMidi ((*it).getMessage(), p);
                ++it;
            }
            int len = juce::jmin (CR, n - pos);
            if (it != midi.end()) len = juce::jmin (len, juce::jmax (1, (*it).samplePosition - pos));

            runArp (p);
            c.lfo = lfoValue (p, len);
            {
                const float tgt = std::log2 (p[cutoff]);
                if (cutLog < 0.0f) cutLog = tgt;
                cutLog += (tgt - cutLog) * 0.12f;
                c.cut = std::exp2 (cutLog);
            }
            c.bend = bend;

            std::fill (tmpL.begin(), tmpL.begin() + len, 0.0f);
            std::fill (tmpR.begin(), tmpR.begin() + len, 0.0f);
            int act = 0;
            for (auto& v : voices)
                if (v.active) { v.render (tmpL.data(), tmpR.data(), len, c); ++act; }
            uiActiveVoices.store (act);

            // trance gate, sample accurate
            const bool gOn = p[gate_on] > 0.5f;
            const double gStep = gateDiv ((int) p[gate_div]);
            const float smooth = std::exp (-1.0f / ((0.0008f + p[gate_sm] * 0.03f) * sr));
            for (int s = 0; s < len; ++s)
            {
                const double q = ppq + (double) s * ppqPerSample;
                float target = 1.0f;
                if (gOn)
                {
                    const int step = (int) std::fmod (std::floor (q / gStep), 16.0);
                    const bool on = p[g0 + juce::jlimit (0, 15, step)] > 0.5f;
                    target = on ? 1.0f : 1.0f - p[gate_dep];
                    if (s == 0) uiGateStep.store (step);
                }
                gateGain = target + (gateGain - target) * smooth;
                L[pos + s] = tmpL[(size_t) s] * gateGain;
                R[pos + s] = tmpR[(size_t) s] * gateGain;
            }
            if (! gOn) uiGateStep.store (-1);

            ppq += (double) len * ppqPerSample;
            pos += len;
        }
        while (it != midi.end()) { handleMidi ((*it).getMessage(), p); ++it; }

        fx (L, R, n, p, clk.playing ? clk.ppq : ppq - (double) n * ppqPerSample, ppqPerSample);
    }

    void allNotesOff()
    {
        for (auto& v : voices) v.release();
        held.clear(); monoStack.clear(); arpNotes.clear();
    }

private:
    float sr = 48000.0f;
    std::array<Voice, NV> voices;
    uint32_t ageCounter = 1;
    float cutLog = -1.0f;
    float bend = 0.0f, lastNote = 60.0f;
    double bpm = 124.0, ppq = 0.0;
    bool hostPlaying = false;

    std::vector<std::pair<int, float>> held;   // physical keys, in played order
    std::vector<int> monoStack;
    // arp
    std::vector<int> arpNotes; double arpOffAt = -1.0; long long arpLastStep = -1; int arpIdx = 0, arpDir = 1;
    float arpVel = 0.8f;
    // lfo
    float lfoPh = 0.0f, lfoOffset = 0.0f, shVal = 0.0f, lastLfoPh = 0.0f;
    juce::Random rng { 99 };
    // fx
    float gateGain = 1.0f;
    std::vector<float> tmpL, tmpR;
    std::array<std::vector<float>, 2> dly; int dPos = 0; float dTimeSm = -1.0f;
    float dlyLp[2] {}, dlyHp[2] {};
    juce::dsp::Chorus<float> chorus;
    juce::Reverb reverb;
    SVF eqLo[2], eqHi[2];
    float crushHold[2] {}; int crushCount = 0;
    float volSm = 1.0f;

    static double gateDiv (int i) { static const double d[] = { 0.5, 0.25, 0.125, 1.0 / 3.0, 1.0 / 6.0 }; return d[juce::jlimit (0, 4, i)]; }
    static double arpDiv  (int i) { static const double d[] = { 1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125 }; return d[juce::jlimit (0, 5, i)]; }
    static double lfoDiv  (int i) { static const double d[] = { 16, 8, 4, 2, 1, 0.5, 0.25, 2.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0 }; return d[juce::jlimit (0, 9, i)]; }
    static double dlyDiv  (int i) { static const double d[] = { 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 1.0 / 3.0, 2.0 / 3.0 }; return d[juce::jlimit (0, 7, i)]; }

    static void chordIntervals (int chord, std::vector<int>& out)
    {
        out.clear(); out.push_back (0);
        switch (chord)
        {
            case 1: out.push_back (12); break;
            case 2: out.push_back (7); break;
            case 3: out.insert (out.end(), { 3, 7 }); break;
            case 4: out.insert (out.end(), { 4, 7 }); break;
            case 5: out.insert (out.end(), { 3, 7, 10 }); break;
            case 6: out.insert (out.end(), { 4, 7, 11 }); break;
            case 7: out.insert (out.end(), { 3, 7, 10, 14 }); break;
            case 8: out.insert (out.end(), { 4, 7, 11, 14 }); break;
            case 9: out.insert (out.end(), { 2, 7 }); break;
            case 10: out.insert (out.end(), { 5, 7 }); break;
            case 11: out.insert (out.end(), { 3, 10, 14, 17 }); break;
            case 12: out.insert (out.end(), { 10, 15, 19 }); break;
            case 13: out.insert (out.end(), { 5, 10, 15 }); break;
            default: break;
        }
    }

    float lfoValue (const float* p, int len)
    {
        float ph;
        if (p[l_sync] > 0.5f)
        {
            const double d = lfoDiv ((int) p[l_div]);
            ph = (float) std::fmod (ppq / d, 1.0) - lfoOffset;
            ph -= std::floor (ph);
        }
        else
        {
            lfoPh += p[l_rate] * (float) len / sr;
            lfoPh -= std::floor (lfoPh);
            ph = lfoPh;
        }
        if (ph < lastLfoPh) shVal = rng.nextFloat() * 2.0f - 1.0f;
        lastLfoPh = ph;
        float v = 0.0f;
        switch ((int) p[l_shape])
        {
            case 0: v = std::sin (kTwoPi * ph); break;
            case 1: v = 1.0f - 4.0f * std::abs (ph - 0.5f); break;
            case 2: v = 2.0f * ph - 1.0f; break;
            case 3: v = 1.0f - 2.0f * ph; break;
            case 4: v = ph < 0.5f ? 1.0f : -1.0f; break;
            case 5: v = shVal; break;
            default: break;
        }
        uiLfo.store (v);
        return v;
    }

    void retrigLfo (const float* p)
    {
        if (p[l_retrig] < 0.5f) return;
        if (p[l_sync] > 0.5f) { const double d = lfoDiv ((int) p[l_div]); lfoOffset = (float) std::fmod (ppq / d, 1.0); }
        else lfoPh = 0.0f;
    }

    Voice* findVoice()
    {
        for (auto& v : voices) if (! v.active) return &v;
        Voice* best = nullptr;
        for (auto& v : voices) if (! v.gateOpen && (best == nullptr || v.age < best->age)) best = &v;
        if (best) return best;
        for (auto& v : voices) if (best == nullptr || v.age < best->age) best = &v;
        return best;
    }

    // one logical note (after arp) -> voices (after chord memory / mono)
    void noteOn (int note, float velo, const float* p)
    {
        const bool isMono = p[mono] > 0.5f && (int) p[chord] == 0;
        const int U = (int) p[uni];
        const bool doGlide = p[glide] > 0.0005f;
        retrigLfo (p);
        if (isMono)
        {
            monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), note), monoStack.end());
            monoStack.push_back (note);
            Voice& v = voices[0];
            for (size_t i = 1; i < voices.size(); ++i) if (voices[i].active) voices[i].release();
            const bool legato = v.active && v.gateOpen;
            v.start (note, velo, v.active ? v.curNote : lastNote, doGlide && (legato || v.active), ! legato, U, ageCounter++);
        }
        else
        {
            std::vector<int> iv; chordIntervals ((int) p[chord], iv);
            for (int i : iv)
            {
                const int nn = juce::jlimit (0, 127, note + i);
                Voice* v = nullptr;
                for (auto& vv : voices) if (vv.active && vv.note == nn) { v = &vv; break; } // same note: retrigger in place
                if (v == nullptr) v = findVoice();
                const bool steal = v->active;
                v->start (nn, velo, lastNote + (float) i, doGlide, true, U, ageCounter++);
                juce::ignoreUnused (steal);
            }
        }
        lastNote = (float) note;
    }

    void noteOff (int note, const float* p)
    {
        const bool isMono = p[mono] > 0.5f && (int) p[chord] == 0;
        if (isMono)
        {
            monoStack.erase (std::remove (monoStack.begin(), monoStack.end(), note), monoStack.end());
            Voice& v = voices[0];
            if (monoStack.empty()) { if (v.active && v.note == note) v.release(); }
            else if (v.note == note)
            {
                const int back = monoStack.back();
                v.start (back, v.velocity, v.curNote, p[glide] > 0.0005f, false, (int) p[uni], ageCounter++);
                lastNote = (float) back;
            }
        }
        else
        {
            std::vector<int> iv; chordIntervals ((int) p[chord], iv);
            for (int i : iv)
            {
                const int nn = note + i;
                for (auto& v : voices) if (v.active && v.gateOpen && v.note == nn) v.release();
            }
        }
    }

    void handleMidi (const juce::MidiMessage& m, const float* p)
    {
        const bool arp = p[arp_on] > 0.5f;
        if (m.isNoteOn())
        {
            const int nn = m.getNoteNumber();
            const float velo = m.getFloatVelocity();
            if (held.empty() && ! hostPlaying) { ppq = 0.0; arpLastStep = -1; }
            held.erase (std::remove_if (held.begin(), held.end(), [nn] (auto& h) { return h.first == nn; }), held.end());
            held.push_back ({ nn, velo });
            if (arp) { arpVel = velo; if (held.size() == 1) { arpIdx = 0; arpDir = 1; arpLastStep = -1; } }
            else noteOn (nn, velo, p);
        }
        else if (m.isNoteOff())
        {
            const int nn = m.getNoteNumber();
            held.erase (std::remove_if (held.begin(), held.end(), [nn] (auto& h) { return h.first == nn; }), held.end());
            if (! arp) noteOff (nn, p);
            else if (held.empty()) { for (int a : arpNotes) noteOff (a, p); arpNotes.clear(); }
        }
        else if (m.isPitchWheel()) bend = (float) (m.getPitchWheelValue() - 8192) / 8192.0f * 2.0f;
        else if (m.isAllNotesOff() || m.isAllSoundOff()) allNotesOff();
    }

    void runArp (const float* p)
    {
        if (p[arp_on] < 0.5f)
        {
            if (! arpNotes.empty()) { for (int a : arpNotes) noteOff (a, p); arpNotes.clear(); }
            return;
        }
        if (held.empty()) return;
        const double step = arpDiv ((int) p[arp_div]);
        const long long s = (long long) std::floor (ppq / step + 1.0e-6);
        if (arpOffAt >= 0.0 && ppq >= arpOffAt) { for (int a : arpNotes) noteOff (a, p); arpNotes.clear(); arpOffAt = -1.0; }
        if (s == arpLastStep) return;
        arpLastStep = s;
        for (int a : arpNotes) noteOff (a, p);
        arpNotes.clear();

        // build the pattern
        std::vector<int> base;
        for (auto& h : held) base.push_back (h.first);
        const int mode = (int) p[arp_mode];
        if (mode != 4) std::sort (base.begin(), base.end());
        const int octs = juce::jlimit (1, 4, (int) p[arp_oct]);
        std::vector<int> pat;
        for (int o = 0; o < octs; ++o) for (int b : base) pat.push_back (b + 12 * o);
        if (mode == 5) { for (int b : pat) arpNotes.push_back (b); }
        else
        {
            const int N = (int) pat.size();
            int idx = 0;
            if (mode == 0 || mode == 4) idx = arpIdx % N;
            else if (mode == 1) idx = (N - 1) - (arpIdx % N);
            else if (mode == 2)
            {
                const int cyc = juce::jmax (1, 2 * N - 2);
                const int k = arpIdx % cyc;
                idx = N == 1 ? 0 : (k < N ? k : cyc - k);
            }
            else idx = rng.nextInt (N);
            arpNotes.push_back (pat[(size_t) idx]);
            ++arpIdx;
        }
        for (int a : arpNotes) noteOn (a, arpVel, p);
        arpOffAt = ((double) s + p[arp_gate]) * step;
    }

    static inline float softClip (float x)
    {
        const float a = std::abs (x);
        if (a < 0.8f) return x;
        return std::copysign (0.8f + 0.2f * std::tanh ((a - 0.8f) / 0.2f), x);
    }

    void fx (float* L, float* R, int n, const float* p, double ppq0, double ppqPerSample)
    {
        // ---- drive
        const float d = p[dist];
        if (d > 0.001f)
        {
            const int t = (int) p[dist_t];
            const float g = 1.0f + d * 14.0f;
            if (t == 3)
            {
                const float bits = 16.0f - d * 12.0f;
                const float q = std::exp2 (bits - 1.0f);
                const int hold = 1 + (int) (d * 16.0f);
                for (int i = 0; i < n; ++i)
                {
                    if (crushCount-- <= 0) { crushCount = hold - 1; crushHold[0] = std::round (L[i] * q) / q; crushHold[1] = std::round (R[i] * q) / q; }
                    L[i] = crushHold[0]; R[i] = crushHold[1];
                }
            }
            else
            {
                const float comp = t == 0 ? 0.55f / std::tanh (g * 0.5f) : t == 1 ? 0.55f / juce::jmin (g * 0.5f, 1.0f) : 0.6f;
                for (int i = 0; i < n; ++i)
                {
                    float* ch[2] = { L + i, R + i };
                    for (auto* x : ch)
                    {
                        if (t == 0) *x = std::tanh (*x * g) * comp;
                        else if (t == 1) *x = juce::jlimit (-1.0f, 1.0f, *x * g) * comp;
                        else *x = std::sin (*x * g * 1.3f) * comp * (1.0f - d * 0.3f);
                    }
                }
            }
        }

        // ---- chorus
        if (p[cho_mix] > 0.001f)
        {
            chorus.setRate (p[cho_rate]);
            chorus.setDepth (0.15f + p[cho_dep] * 0.6f);
            chorus.setCentreDelay (9.0f);
            chorus.setFeedback (0.0f);
            chorus.setMix (p[cho_mix] * 0.5f);
            float* chans[2] = { L, R };
            juce::dsp::AudioBlock<float> blk (chans, 2, (size_t) n);
            juce::dsp::ProcessContextReplacing<float> ctx (blk);
            chorus.process (ctx);
        }

        // ---- tempo delay (ping-pong)
        const int dlen = (int) dly[0].size();
        const float targetT = juce::jlimit (16.0f, (float) dlen - 8.0f, (float) (dlyDiv ((int) p[dly_div]) * 60.0 / bpm * sr));
        if (dTimeSm < 0.0f) dTimeSm = targetT;
        const float mix = p[dly_mix], fb = p[dly_fb];
        const bool pp = p[dly_pp] > 0.5f;
        const float lpC = 1.0f - std::exp (-kTwoPi * (800.0f + p[dly_tone] * p[dly_tone] * 14000.0f) / sr);
        const float hpC = 1.0f - std::exp (-kTwoPi * 160.0f / sr);
        if (mix > 0.001f || fb > 0.0f)
        {
            for (int i = 0; i < n; ++i)
            {
                dTimeSm += (targetT - dTimeSm) * 0.0005f;
                float rp = (float) dPos - dTimeSm; while (rp < 0.0f) rp += (float) dlen;
                const int i0 = (int) rp; const float fr = rp - (float) i0; const int i1 = (i0 + 1) % dlen;
                float wet[2];
                for (size_t ch = 0; ch < 2; ++ch) wet[ch] = dly[ch][(size_t) i0] + fr * (dly[ch][(size_t) i1] - dly[ch][(size_t) i0]);
                float fbs[2];
                for (int ch = 0; ch < 2; ++ch)
                {
                    dlyLp[ch] += lpC * (wet[ch] - dlyLp[ch]);
                    dlyHp[ch] += hpC * (dlyLp[ch] - dlyHp[ch]);
                    fbs[ch] = (dlyLp[ch] - dlyHp[ch]) * fb;
                }
                const float inM = mix > 0.001f ? 1.0f : 0.0f;
                if (pp)
                {
                    dly[0][(size_t) dPos] = (L[i] + R[i]) * 0.5f * inM + fbs[1];
                    dly[1][(size_t) dPos] = fbs[0];
                }
                else
                {
                    dly[0][(size_t) dPos] = L[i] * inM + fbs[0];
                    dly[1][(size_t) dPos] = R[i] * inM + fbs[1];
                }
                const float dry = 1.0f - mix * 0.35f;
                L[i] = L[i] * dry + (dlyLp[0] - dlyHp[0]) * mix;
                R[i] = R[i] * dry + (dlyLp[1] - dlyHp[1]) * mix;
                if (++dPos >= dlen) dPos = 0;
            }
        }

        // ---- reverb
        if (p[rev_mix] > 0.001f)
        {
            juce::Reverb::Parameters rp;
            rp.roomSize = 0.3f + p[rev_size] * 0.68f;
            rp.damping = p[rev_damp];
            rp.wetLevel = p[rev_mix] * 0.55f;
            rp.dryLevel = 1.0f - p[rev_mix] * 0.45f;
            rp.width = 1.0f;
            reverb.setParameters (rp);
            reverb.processStereo (L, R, n);
        }

        // ---- pump (synced ducking)
        const float pm = p[pump];
        static const double pdv[] = { 1.0, 2.0, 0.5 };
        const double pd = pdv[juce::jlimit (0, 2, (int) p[pump_div])];
        float lastG = 1.0f;
        if (pm > 0.001f)
        {
            for (int i = 0; i < n; ++i)
            {
                const double q = ppq0 + (double) i * ppqPerSample;
                const float ph = (float) std::fmod (q / pd, 1.0);
                const float env = ph < 0.015f ? ph / 0.015f : std::exp (-(ph - 0.015f) * 7.0f);
                const float gg = 1.0f - pm * 0.92f * env;
                L[i] *= gg; R[i] *= gg; lastG = gg;
            }
        }
        uiPump.store (lastG);

        // ---- EQ
        const bool lo = p[locut] > 21.0f, hi = p[hicut] < 19900.0f;
        if (lo || hi)
        {
            const float gl = std::tan (juce::MathConstants<float>::pi * juce::jmin (p[locut], sr * 0.45f) / sr);
            const float gh = std::tan (juce::MathConstants<float>::pi * juce::jmin (p[hicut], sr * 0.45f) / sr);
            for (int i = 0; i < n; ++i)
            {
                float* ch[2] = { L + i, R + i };
                for (int c = 0; c < 2; ++c)
                {
                    float lp, bp, hp;
                    if (lo) { eqLo[c].tick (*ch[c], gl, 1.414f, lp, bp, hp); *ch[c] = hp; }
                    if (hi) { eqHi[c].tick (*ch[c], gh, 1.414f, lp, bp, hp); *ch[c] = lp; }
                }
            }
        }

        // ---- width, volume, safety clip
        const float w = p[width];
        const float vTarget = juce::Decibels::decibelsToGain (p[vol]);
        for (int i = 0; i < n; ++i)
        {
            volSm += (vTarget - volSm) * 0.002f;
            const float m = (L[i] + R[i]) * 0.5f, s = (L[i] - R[i]) * 0.5f * w;
            L[i] = softClip ((m + s) * volSm);
            R[i] = softClip ((m - s) * volSm);
            if (! std::isfinite (L[i]) || ! std::isfinite (R[i])) { L[i] = R[i] = 0.0f; }
        }
    }
};
} // namespace sp
