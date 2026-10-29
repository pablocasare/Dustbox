// Renders every preset offline and prints level / sanity stats (tab separated).
#include "../Source/PluginProcessor.h"
#include <cstdio>

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const double sr = 48000.0; const int bs = 512;
    const int only = argc > 1 ? atoi (argv[1]) : -1;
    const juce::String wavDir = argc > 2 ? juce::String (argv[2]) : juce::String();
    SerpentProcessor p;
    p.setPlayConfigDetails (0, 2, sr, bs);
    for (int i = 0; i < SerpentProcessor::numPresets(); ++i)
    {
        if (only >= 0 && i != only) continue;
        p.setCurrentProgram (i);
        p.prepareToPlay (sr, bs);
        const auto& pr = SerpentProcessor::preset (i);
        const juce::String cat (pr.category);
        const bool bass = cat.contains ("Bass") || cat == "Acid";
        const int note = bass ? 48 : 60;
        const int total = (int) (sr * 3.0), offAt = (int) (sr * 1.6);
        juce::AudioBuffer<float> buf (2, bs), all (2, total);
        double peak = 0, sum = 0; bool bad = false;
        int pos = 0;
        while (pos < total)
        {
            const int n = juce::jmin (bs, total - pos);
            buf.setSize (2, n, false, false, true);
            juce::MidiBuffer m;
            if (pos == 0) m.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), 0);
            if (bass && pos <= (int) (sr * 0.5) && pos + n > (int) (sr * 0.5)) { m.addEvent (juce::MidiMessage::noteOff (1, note), (int) (sr * 0.5) - pos); m.addEvent (juce::MidiMessage::noteOn (1, note + 3, (juce::uint8) 100), (int) (sr * 0.5) - pos + 1); }
            if (pos <= offAt && pos + n > offAt) { m.addEvent (juce::MidiMessage::noteOff (1, note), offAt - pos); m.addEvent (juce::MidiMessage::noteOff (1, note + 3), offAt - pos); }
            p.processBlock (buf, m);
            for (int c = 0; c < 2; ++c)
            {
                all.copyFrom (c, pos, buf, c, 0, n);
                for (int s = 0; s < n; ++s)
                {
                    const float v = buf.getSample (c, s);
                    if (! std::isfinite (v)) bad = true;
                    peak = juce::jmax (peak, (double) std::abs (v));
                }
            }
            pos += n;
        }
        // loudest 100 ms window (RMS) in the held part
        const int win = (int) (sr * 0.1);
        double best = 0;
        for (int s = 0; s + win <= offAt; s += win / 2)
        {
            double acc = 0;
            for (int k = 0; k < win; ++k) { const double l = all.getSample (0, s + k), r = all.getSample (1, s + k); acc += 0.5 * (l * l + r * r); }
            best = juce::jmax (best, acc / win);
        }
        for (int k = 0; k < total; ++k) sum += all.getSample (0, k) * all.getSample (0, k);
        // stereo correlation of the lows isn't measured here; width is checked separately
        const double rmsDb = 10.0 * std::log10 (best + 1e-12);
        const double tailDb = 20.0 * std::log10 (std::abs (all.getSample (0, total - 1)) + 1e-9);
        std::printf ("%d\t%s\t%s\t%.4f\t%.2f\t%d\t%.1f\n", i, pr.category, pr.name, peak, rmsDb, bad ? 1 : 0, tailDb);
        if (wavDir.isNotEmpty())
        {
            juce::File f (wavDir + "/" + juce::String (i) + ".wav");
            f.deleteFile();
            juce::WavAudioFormat wav;
            if (auto os = f.createOutputStream())
                if (auto w = std::unique_ptr<juce::AudioFormatWriter> (wav.createWriterFor (os.get(), sr, 2, 24, {}, 0)))
                { os.release(); w->writeFromAudioSampleBuffer (all, 0, total); }
        }
    }
    return 0;
}
