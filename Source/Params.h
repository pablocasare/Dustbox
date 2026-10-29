#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Every parameter lives in this one list. The preset generator (tools/make_presets.py)
// parses this file, so ids and ranges stay in sync automatically.
//   X(id, display name, type, min, max, default, skew centre (0 = linear), choices)
#define SERPENT_PARAMS(X) \
 X(a_wave,   "Osc A Wave",     Float, 0, 7, 2, 0, "") \
 X(a_oct,    "Osc A Octave",   Int,  -3, 2, 0, 0, "") \
 X(a_semi,   "Osc A Semi",     Int, -12, 12, 0, 0, "") \
 X(a_fine,   "Osc A Fine",     Float,-100, 100, 0, 0, "") \
 X(a_lvl,    "Osc A Level",    Float, 0, 1, 0.8f, 0, "") \
 X(b_wave,   "Osc B Wave",     Float, 0, 7, 2, 0, "") \
 X(b_oct,    "Osc B Octave",   Int,  -3, 2, 0, 0, "") \
 X(b_semi,   "Osc B Semi",     Int, -12, 12, 0, 0, "") \
 X(b_fine,   "Osc B Fine",     Float,-100, 100, 7, 0, "") \
 X(b_lvl,    "Osc B Level",    Float, 0, 1, 0, 0, "") \
 X(uni,      "Unison",         Int,   1, 7, 1, 0, "") \
 X(detune,   "Detune",         Float, 0, 1, 0.2f, 0, "") \
 X(spread,   "Stereo Spread",  Float, 0, 1, 0.5f, 0, "") \
 X(sub,      "Sub",            Float, 0, 1, 0, 0, "") \
 X(noise,    "Noise",          Float, 0, 1, 0, 0, "") \
 X(fm,       "FM B>A",         Float, 0, 1, 0, 0, "") \
 X(ftype,    "Filter Type",    Choice,0, 3, 0, 0, "LP 24|LP 12|Band|High") \
 X(cutoff,   "Cutoff",         Float, 20, 20000, 8000, 1000, "") \
 X(reso,     "Resonance",      Float, 0, 1, 0.1f, 0, "") \
 X(fdrive,   "Filter Drive",   Float, 0, 1, 0, 0, "") \
 X(fenv,     "Filter Env",     Float,-1, 1, 0, 0, "") \
 X(fkey,     "Key Track",      Float, 0, 1, 0.5f, 0, "") \
 X(aa,       "Amp Attack",     Float, 0.001f, 10, 0.003f, 0.3f, "") \
 X(ad,       "Amp Decay",      Float, 0.01f, 10, 0.5f, 0.6f, "") \
 X(as,       "Amp Sustain",    Float, 0, 1, 1, 0, "") \
 X(ar,       "Amp Release",    Float, 0.005f, 10, 0.25f, 0.6f, "") \
 X(fa,       "Filt Attack",    Float, 0.001f, 10, 0.003f, 0.3f, "") \
 X(fd,       "Filt Decay",     Float, 0.01f, 10, 0.4f, 0.6f, "") \
 X(fs,       "Filt Sustain",   Float, 0, 1, 0, 0, "") \
 X(fr,       "Filt Release",   Float, 0.005f, 10, 0.3f, 0.6f, "") \
 X(penv,     "Pitch Env",      Float,-48, 48, 0, 0, "") \
 X(pdec,     "Pitch Decay",    Float, 0.002f, 2, 0.08f, 0.15f, "") \
 X(l_sync,   "LFO Sync",       Bool,  0, 1, 1, 0, "") \
 X(l_rate,   "LFO Rate",       Float, 0.05f, 20, 2, 2, "") \
 X(l_div,    "LFO Division",   Choice,0, 9, 4, 0, "4 bars|2 bars|1 bar|1/2|1/4|1/8|1/16|1/4T|1/8T|1/16T") \
 X(l_shape,  "LFO Shape",      Choice,0, 5, 0, 0, "Sine|Triangle|Saw Up|Saw Down|Square|S&H") \
 X(l_dest,   "LFO Target",     Choice,0, 6, 0, 0, "Cutoff|Pitch|Volume|Wave A|Wave B|Pan|FM") \
 X(l_amt,    "LFO Amount",     Float, 0, 1, 0, 0, "") \
 X(l_retrig, "LFO Retrigger",  Bool,  0, 1, 0, 0, "") \
 X(glide,    "Glide",          Float, 0, 1, 0, 0.15f, "") \
 X(mono,     "Mono",           Bool,  0, 1, 0, 0, "") \
 X(vel,      "Velocity",       Float, 0, 1, 0.5f, 0, "") \
 X(xy_x,     "XY X",           Float, 0, 1, 0, 0, "") \
 X(xy_y,     "XY Y",           Float, 0, 1, 0, 0, "") \
 X(xy_xd,    "XY X Target",    Choice,0, 11, 0, 0, "Cutoff|Resonance|Wave A|Wave B|FM|Drive|Reverb|Delay|Detune|Gate Depth|LFO Amount|Pitch Env") \
 X(xy_yd,    "XY Y Target",    Choice,0, 11, 10, 0, "Cutoff|Resonance|Wave A|Wave B|FM|Drive|Reverb|Delay|Detune|Gate Depth|LFO Amount|Pitch Env") \
 X(gate_on,  "Gate On",        Bool,  0, 1, 0, 0, "") \
 X(gate_div, "Gate Rate",      Choice,0, 4, 1, 0, "1/8|1/16|1/32|1/8T|1/16T") \
 X(gate_dep, "Gate Depth",     Float, 0, 1, 1, 0, "") \
 X(gate_sm,  "Gate Smooth",    Float, 0, 1, 0.2f, 0, "") \
 X(g0, "Step 1", Bool,0,1,1,0,"") X(g1, "Step 2", Bool,0,1,1,0,"") X(g2, "Step 3", Bool,0,1,1,0,"") X(g3, "Step 4", Bool,0,1,1,0,"") \
 X(g4, "Step 5", Bool,0,1,1,0,"") X(g5, "Step 6", Bool,0,1,1,0,"") X(g6, "Step 7", Bool,0,1,1,0,"") X(g7, "Step 8", Bool,0,1,1,0,"") \
 X(g8, "Step 9", Bool,0,1,1,0,"") X(g9, "Step 10",Bool,0,1,1,0,"") X(g10,"Step 11",Bool,0,1,1,0,"") X(g11,"Step 12",Bool,0,1,1,0,"") \
 X(g12,"Step 13",Bool,0,1,1,0,"") X(g13,"Step 14",Bool,0,1,1,0,"") X(g14,"Step 15",Bool,0,1,1,0,"") X(g15,"Step 16",Bool,0,1,1,0,"") \
 X(arp_on,   "Arp On",         Bool,  0, 1, 0, 0, "") \
 X(arp_mode, "Arp Mode",       Choice,0, 5, 0, 0, "Up|Down|Up/Down|Random|As Played|Chord") \
 X(arp_div,  "Arp Rate",       Choice,0, 5, 3, 0, "1/4|1/8|1/8T|1/16|1/16T|1/32") \
 X(arp_oct,  "Arp Octaves",    Int,   1, 4, 1, 0, "") \
 X(arp_gate, "Arp Gate",       Float, 0.05f, 1, 0.5f, 0, "") \
 X(chord,    "Chord",          Choice,0, 13, 0, 0, "Off|Octave|Fifth|Minor|Major|Minor 7|Major 7|Minor 9|Major 9|Sus 2|Sus 4|Minor 11|Dub|Fourths") \
 X(dist_t,   "Drive Type",     Choice,0, 3, 0, 0, "Warm|Hard|Fold|Crush") \
 X(dist,     "Drive",          Float, 0, 1, 0, 0, "") \
 X(cho_mix,  "Chorus Mix",     Float, 0, 1, 0, 0, "") \
 X(cho_rate, "Chorus Rate",    Float, 0.05f, 5, 0.6f, 0.8f, "") \
 X(cho_dep,  "Chorus Depth",   Float, 0, 1, 0.5f, 0, "") \
 X(dly_mix,  "Delay Mix",      Float, 0, 1, 0, 0, "") \
 X(dly_div,  "Delay Time",     Choice,0, 7, 2, 0, "1/16|1/8|3/16|1/4|3/8|1/2|1/8T|1/4T") \
 X(dly_fb,   "Delay Feedback", Float, 0, 0.95f, 0.35f, 0, "") \
 X(dly_tone, "Delay Tone",     Float, 0, 1, 0.5f, 0, "") \
 X(dly_pp,   "Ping Pong",      Bool,  0, 1, 1, 0, "") \
 X(rev_mix,  "Reverb Mix",     Float, 0, 1, 0, 0, "") \
 X(rev_size, "Reverb Size",    Float, 0, 1, 0.5f, 0, "") \
 X(rev_damp, "Reverb Damp",    Float, 0, 1, 0.5f, 0, "") \
 X(pump,     "Pump",           Float, 0, 1, 0, 0, "") \
 X(pump_div, "Pump Rate",      Choice,0, 2, 0, 0, "1/4|1/2|1/8") \
 X(locut,    "Low Cut",        Float, 20, 2000, 20, 200, "") \
 X(hicut,    "High Cut",       Float, 500, 20000, 20000, 5000, "") \
 X(width,    "Width",          Float, 0, 2, 1, 0, "") \
 X(vol,      "Volume",         Float,-36, 6, 0, 0, "")

namespace sp
{
enum class PT { Float, Int, Bool, Choice };

enum P : int
{
   #define SP_ENUM(id, ...) id,
    SERPENT_PARAMS (SP_ENUM)
   #undef SP_ENUM
    NUM_P
};

struct PDef { const char* id; const char* name; PT type; float mn, mx, def, skew; const char* choices; };

inline const PDef kParams[] =
{
   #define SP_DEF(id, name, type, mn, mx, def, skew, ch) { #id, name, PT::type, (float) (mn), (float) (mx), (float) (def), (float) (skew), ch },
    SERPENT_PARAMS (SP_DEF)
   #undef SP_DEF
};

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (auto& d : kParams)
    {
        const juce::ParameterID pid { d.id, 1 };
        switch (d.type)
        {
            case PT::Float:
            {
                juce::NormalisableRange<float> r (d.mn, d.mx);
                if (d.skew > 0.0f) r.setSkewForCentre (d.skew);
                layout.add (std::make_unique<juce::AudioParameterFloat> (pid, d.name, r, d.def));
                break;
            }
            case PT::Int:
                layout.add (std::make_unique<juce::AudioParameterInt> (pid, d.name, (int) d.mn, (int) d.mx, (int) d.def));
                break;
            case PT::Bool:   // an Off/On choice snaps to 0/1, so hosts can't leave it half-way (strict state tests)
                layout.add (std::make_unique<juce::AudioParameterChoice> (pid, d.name, juce::StringArray { "Off", "On" }, d.def > 0.5f ? 1 : 0));
                break;
            case PT::Choice:
                layout.add (std::make_unique<juce::AudioParameterChoice> (pid, d.name,
                              juce::StringArray::fromTokens (d.choices, "|", ""), (int) d.def));
                break;
        }
    }
    return layout;
}
} // namespace sp
