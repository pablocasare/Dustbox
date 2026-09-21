# Dust Box 2

![character page](docs/character-page.png)

A house synthesizer that builds patches from plain English: 69 sounds sorted by
type, 14 character knobs that reshape any of them, and a full FX rack. Builds as **VST3**, **AU** (Mac) and a
**standalone app**.

## What's in it

**Synth** — two band-limited oscillators with up to 4-voice unison, a sub, a
24 dB lowpass with drive into it, amp and filter envelopes, an LFO to cutoff /
pitch / tremolo, glide, and a "dust" control (tape wow, top-end roll-off, noise
floor, per-voice pitch drift).

**FX rack** — every module has its own power switch and is saved with the preset:

| module     | controls                                   | notes |
|------------|--------------------------------------------|-------|
| EQ         | low shelf, mid (sweepable), high shelf     | |
| Crush      | amount                                     | bit depth and sample-rate reduction together |
| Phaser     | rate, mix                                  | four-stage |
| Chorus     | mix                                        | two taps, hard-panned |
| Pump       | rate (¼ ⅛ ½), depth, shape                 | tempo-locked ducking — sidechain feel without routing a kick |
| Compressor | threshold, ratio, attack, release, makeup  | stereo-linked, soft knee |
| Delay      | time (synced), mode, feedback, mix, tone   | ping-pong or mono; follows host tempo |
| Reverb     | mix, size, damp                            | |

Every module crossfades in and out, so switching never clicks. Delay and reverb
let their tails ring out when switched off.

**69 sounds in 11 categories**: Bass, Sub, Acid, Chords, Stabs, Keys & Organs,
Strings, Pads, Plucks & Bleeps, Leads, Textures & FX. Loudness-matched, and
nothing exceeds 0 dBFS.

**Character page**: 14 knobs that reshape whatever sound is loaded.

| group   | knobs |
|---------|-------|
| tone    | dark, bright, deep, dirty, clean, wide |
| length  | short, long |
| effects | pumped, dub, space, crushed, phased, glued |

At 0 each knob leaves the sound exactly as designed; turning it up blends
toward that flavour. They sit on top of the preset rather than changing it, so
they stay put while you browse. Set dub to 60% and audition every sound through
it. "reset character" puts them all back to 0. The effects knobs switch their FX
module on if it's off, and fade its amount in from zero.

**Vibe box** — type what you want. It finds the closest presets, blends them
within a category, then applies modifier words. It understands FX words too:
*pumping*, *sidechain*, *dub*, *echo*, *crushed*, *phased*, *spacious*, *dark*,
*compressed*. Those words turn up the matching character knob, so you can see
what it did and dial it back.

**Patches** — save and load as JSON. The browser prototype's patch files load too.

## About the "UK groove" presets

Presets tagged `ukgroove` were designed from research into the current London
house scene around Max Dean, Luke Dean and Omar+: rolling sidechained
basslines, bouncy swung plucks, string stabs, R&B-leaning keys and '90s
Chicago touches. Type *"max dean"* or *"omar+"* into the vibe box and it'll
steer toward that part of the bank.

To be clear about what that means: these were designed from how the music is
described in interviews, press and production breakdowns — nobody's recordings
were analysed or sampled, and the presets aren't named after or claimed to be
anyone's actual sounds. Use your ears and adjust.

---

## Building

You need a compiler and CMake. JUCE downloads automatically on first build.

### macOS
```bash
xcode-select --install
brew install cmake
./build-mac.sh
```
First build: 5–15 minutes (compiling JUCE). After that, under a minute.
Installs to `~/Library/Audio/Plug-Ins/VST3/` and `~/Library/Audio/Plug-Ins/Components/`.

Logic and GarageBand only see the AU. Ableton, Bitwig, FL and Studio One use
the VST3. If Logic doesn't pick it up: `killall -9 AudioComponentRegistrar`,
then reopen.

### Windows
Install Visual Studio Community ("Desktop development with C++") and CMake, then:
```bat
build-windows.bat
```
Installs to `C:\Program Files\Common Files\VST3\`. If the copy step fails, run
as administrator or copy `build\DustBox_artefacts\Release\VST3\Dust Box.vst3`
there yourself.

### Linux
```bash
sudo apt install build-essential cmake pkg-config libasound2-dev libfreetype-dev \
  libfontconfig1-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
  libxcomposite-dev libxext-dev libgl1-mesa-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
```

### Before sharing it
Change `PLUGIN_MANUFACTURER_CODE`, `PLUGIN_CODE` and `COMPANY_NAME` in
`CMakeLists.txt`. Colliding codes make hosts load the wrong plugin.

---

## Adding sounds

Everything lives in `tools/make_presets.py`:

- `CAT`: one template per category, FX included
- `arch(...)` lines: the 69 sounds; each lists only what differs from its category

After editing, rebuild the bank and re-match loudness:
```bash
python3 tools/make_presets.py
g++ -std=c++17 -O2 -I. tools/calibrate_levels.cpp -o calib && ./calib
python3 tools/make_presets.py
```

The character knobs are `applyMacros()` in `Source/Params.h`; each is a few
lines. New vibe words go in `kModWords` or `kMacroWords` (Source/Matcher.h);
synonyms in `kAliases`.

## Layout
```
Source/Params.h          parameter contract: ids, ranges, 0..1 mapping
Source/Presets.h         generated bank (don't hand-edit)
Source/Matcher.h         vibe -> patch, STL only
Source/SynthEngine.h     all DSP + FX, framework-free
Source/PluginProcessor   MIDI, host tempo, state, patch files
Source/PluginEditor      UI
tools/make_presets.py    preset generator
tools/calibrate_levels   loudness matcher
```
