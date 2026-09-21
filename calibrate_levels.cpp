// Loudness-matches the preset bank.
//
// Renders every preset the way it's typically played (bass/sub/acid as single
// low notes, chords/stabs/strings/pads as four-note chords, everything else as
// single mid notes), measures RMS, and writes tools/levels.txt: one gain factor
// per preset. make_presets.py folds those into each preset's level knob.
//
//   g++ -std=c++17 -O2 -I. tools/calibrate_levels.cpp -o calib && ./calib
//   python3 tools/make_presets.py
#include "../Source/SynthEngine.h"
#include "../Source/Presets.h"
#include <cstdio>
#include <vector>
#include <cmath>
using namespace dustbox;

int main() {
    const float targetRms = 0.13f, peakCap = 0.9f;
    std::FILE* out = std::fopen("tools/levels.txt", "w");
    if (! out) { std::puts("run from the project root"); return 1; }
    for (int i = 0; i < kNumPresets; ++i) {
        const auto& pr = kPresets[i];
        const int cat = pr.category;
        const bool chordy = (cat == 3 || cat == 4 || cat == 6 || cat == 7);
        const int note = cat <= 2 ? 40 : 57, count = chordy ? 4 : 1;
        const int iv[4] = { 0, 3, 7, 10 };

        Patch p = pr.patch;
        const float vol = p.v[pVolume];
        p.v[pVolume] = 1.f;                        // measure at unity, then solve for the knob
        SynthEngine e; e.prepare(48000, 512); e.setPatch(p); e.setTransport(124, 0, true);
        const int N = 48000 * 2; std::vector<float> L(N), R(N);
        for (int k = 0; k < count; ++k) e.noteOn(note + iv[k], .9f);
        for (int off = 0; off < N; off += 512) {
            if (off == 48000 - 48000 % 512) for (int k = 0; k < count; ++k) e.noteOff(note + iv[k]);
            e.process(L.data() + off, R.data() + off, std::min(512, N - off));
        }
        double acc = 0; float pk = 0;
        for (int s = 0; s < 48000; ++s) {       // the held half is what you hear
            acc += 0.5 * (L[s] * L[s] + R[s] * R[s]);
            pk = std::max(pk, std::max(std::fabs(L[s]), std::fabs(R[s])));
        }
        const float rms = (float) std::sqrt(acc / 48000.0);
        float want = rms > 1e-6f ? targetRms / rms : 1.f;
        if (pk * want > peakCap) want = peakCap / pk;
        std::fprintf(out, "%s\t%.5f\n", pr.name, want / std::max(vol, 1e-3f) * vol);
    }
    std::fclose(out);
    std::puts("wrote tools/levels.txt");
    return 0;
}
