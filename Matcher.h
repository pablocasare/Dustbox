#pragma once
#include "Params.h"
#include "Presets.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

// Plain English -> patch.
//
// Stage 1 scores every preset in the bank by tag overlap and blends the top two
// when they score similarly. Stage 2 applies modifier words on top: character
// words (dark, dub, pumping...) turn up the matching character knob; the rest
// nudge synth parameters directly. Deterministic,
// offline, and every value is clamped to the parameter spec.

namespace dustbox {

struct MatchResult {
    Patch patch;
    std::string name;
    std::vector<std::string> sources;
    std::vector<std::string> applied;
};

namespace detail {

inline const char* const kStop[] = {"a","an","the","and","or","with","of","for","to","that","is",
    "it","i","want","need","sound","sounds","like","some","really","very","kinda","sort",
    "bit","more","make","me","my","in","on","style","type","vibe", nullptr};

// Scene words map onto the tag used by the research-informed presets, so
// "something like omar+ or max dean" finds that corner of the bank.
inline const char* const kSceneWords[] = {"omar","omarplus","max","dean","luke","locky",
    "london","uk", nullptr};
inline const char* const kSceneTag = "ukgroove";

struct FMod { int param; char op; float amount; };        // op: '*' multiply, '+' add, '=' set
struct CMod { int param; int value; };
struct ModWord { const char* word; FMod f[4]; CMod c[2]; };

#define F_(p,op,a) { p, op, a }
#define NF { -1, 0, 0.f }
#define C_(p,v) { p, v }
#define NC { -1, 0 }

inline const ModWord kModWords[] = {
  // timbre
  {"open",      { F_(pCutoff,'*',1.7f), NF, NF, NF }, { NC, NC }},
  {"crisp",     { F_(pCutoff,'*',1.6f), F_(pAmpD,'*',.8f), NF, NF }, { NC, NC }},
  {"dusty",     { F_(pDust,'+',.30f), F_(pDrive,'+',.10f), F_(pCutoff,'*',.8f), NF }, { NC, NC }},
  {"raw",       { F_(pDrive,'+',.20f), F_(pReverb,'+',-.08f), NF, NF }, { NC, NC }},
  {"warm",      { F_(pCutoff,'*',.8f), F_(pDrive,'+',.10f), F_(pResonance,'*',.8f), NF }, { NC, NC }},
  {"cold",      { F_(pCutoff,'*',1.3f), F_(pDrive,'+',-.10f), F_(pReverb,'+',.15f), NF }, { NC, NC }},
  {"narrow",    { F_(pSpread,'*',.4f), F_(pDetune,'*',.4f), NF, NF }, { C_(cChorusOn,0), NC }},
  {"mono",      { F_(pSpread,'*',.1f), F_(pDetune,'*',.2f), NF, NF }, { C_(cUnison,0), C_(cChorusOn,0) }},
  {"detuned",   { F_(pDetune,'*',1.8f), F_(pSpread,'*',1.4f), NF, NF }, { NC, NC }},
  {"thick",     { F_(pDetune,'*',1.4f), F_(pSubLevel,'+',.15f), NF, NF }, { C_(cUnison,3), NC }},
  {"thin",      { F_(pSubLevel,'+',-.20f), F_(pOscBLevel,'+',-.20f), F_(pCutoff,'*',1.3f), NF }, { NC, NC }},
  {"plucky",    { F_(pAmpD,'*',.40f), F_(pAmpS,'+',-.50f), F_(pFD,'*',.45f), F_(pAmpA,'*',.3f) }, { NC, NC }},
  {"snappy",    { F_(pAmpA,'*',.2f), F_(pAmpD,'*',.5f), F_(pFD,'*',.5f), F_(pAmpS,'+',-.4f) }, { NC, NC }},
  {"sustained", { F_(pAmpS,'+',.45f), F_(pAmpR,'*',1.8f), NF, NF }, { NC, NC }},
  {"slow",      { F_(pAmpA,'*',6.f), F_(pFA,'*',5.f), F_(pAmpR,'*',2.f), NF }, { NC, NC }},
  {"fast",      { F_(pAmpA,'*',.2f), F_(pFA,'*',.2f), NF, NF }, { NC, NC }},
  {"soft",      { F_(pDrive,'+',-.15f), F_(pCutoff,'*',.7f), F_(pAmpA,'*',3.f), NF }, { NC, NC }},
  {"hard",      { F_(pDrive,'+',.25f), F_(pCutoff,'*',1.4f), F_(pAmpA,'*',.3f), NF }, { NC, NC }},
  {"aggressive",{ F_(pDrive,'+',.30f), F_(pResonance,'*',1.5f), F_(pCutoff,'*',1.4f), NF }, { NC, NC }},
  {"resonant",  { F_(pResonance,'*',2.2f), NF, NF, NF }, { NC, NC }},
  {"squelchy",  { F_(pResonance,'*',2.6f), F_(pFEnvAmt,'*',1.5f), NF, NF }, { NC, NC }},
  {"smooth",    { F_(pResonance,'*',.5f), F_(pDrive,'+',-.10f), NF, NF }, { NC, NC }},
  {"sub",       { F_(pSubLevel,'+',.35f), F_(pCutoff,'*',.5f), NF, NF }, { NC, NC }},
  {"wobbly",    { F_(pLfoDepth,'+',.30f), F_(pLfoRate,'*',.5f), F_(pDust,'+',.20f), NF }, { C_(cLfoDest,1), NC }},
  {"moving",    { F_(pLfoDepth,'+',.25f), NF, NF, NF }, { NC, NC }},
  {"static",    { F_(pLfoDepth,'+',-.40f), NF, NF, NF }, { NC, NC }},
  {"loud",      { F_(pVolume,'+',.15f), F_(pDrive,'+',.10f), NF, NF }, { NC, NC }},
  {"quiet",     { F_(pVolume,'+',-.15f), NF, NF, NF }, { NC, NC }},
  {"glidey",    { F_(pGlide,'+',.12f), NF, NF, NF }, { NC, NC }},
  {"lofi",      { F_(pDust,'+',.40f), F_(pCutoff,'*',.6f), F_(pCrush,'=',.35f), NF }, { C_(cCrushOn,1), NC }},
  {"vintage",   { F_(pDust,'+',.30f), F_(pCutoff,'*',.75f), NF, NF }, { C_(cChorusOn,1), NC }},
  {"modern",    { F_(pDust,'+',-.30f), F_(pCutoff,'*',1.3f), NF, NF }, { NC, NC }},
  // fx
  {"dry",       { F_(pReverb,'+',-.40f), F_(pDelayMix,'+',-.3f), NF, NF }, { C_(cReverbOn,0), C_(cDelayOn,0) }},
  {"tight",     { F_(pReverb,'+',-.25f), F_(pAmpR,'*',.5f), F_(pAmpD,'*',.6f), NF }, { C_(cDelayOn,0), NC }},
  {"punchy",    { F_(pFEnvAmt,'*',1.4f), F_(pAmpD,'*',.6f), F_(pCompAttack,'=',20.f), NF }, { C_(cCompOn,1), NC }},
};
#undef F_
#undef NF
#undef C_
#undef NC


// Words that turn up a character knob rather than editing the patch, so the
// result stays visible and adjustable on the character page.
struct MacroSet { int param; float amount; };
struct MacroWord { const char* word; MacroSet set[2]; };
#define NM { -1, 0.f }
inline const MacroWord kMacroWords[] = {
  {"dark", { { pMDark, 0.8f }, NM } },
  {"darker", { { pMDark, 1.0f }, NM } },
  {"muffled", { { pMDark, 1.0f }, NM } },
  {"bright", { { pMBright, 0.8f }, NM } },
  {"brighter", { { pMBright, 1.0f }, NM } },
  {"deep", { { pMDeep, 0.8f }, NM } },
  {"dirty", { { pMDirty, 0.8f }, NM } },
  {"gritty", { { pMDirty, 0.7f }, NM } },
  {"clean", { { pMClean, 0.8f }, NM } },
  {"wide", { { pMWide, 0.8f }, NM } },
  {"wider", { { pMWide, 1.0f }, NM } },
  {"short", { { pMShort, 0.8f }, NM } },
  {"long", { { pMLong, 0.8f }, NM } },
  {"pumping", { { pMPumped, 0.9f }, NM } },
  {"bouncy", { { pMPumped, 0.6f }, NM } },
  {"rolling", { { pMPumped, 0.7f }, NM } },
  {"dub", { { pMDub, 0.9f }, NM } },
  {"echo", { { pMDub, 0.6f }, NM } },
  {"spacious", { { pMSpace, 0.8f }, NM } },
  {"wet", { { pMSpace, 0.6f }, NM } },
  {"crushed", { { pMCrushed, 0.8f }, NM } },
  {"phased", { { pMPhased, 0.8f }, NM } },
  {"swirly", { { pMPhased, 1.0f }, NM } },
  {"compressed", { { pMGlued, 0.8f }, NM } },
  {"huge", { { pMSpace, 1.0f }, { pMWide, 0.6f } } },
};
#undef NM

inline const char* const kAliases[][2] = {
  {"sad","soft"},{"melancholy","soft"},{"moody","dark"},{"hazy","dusty"},{"crunchy","gritty"},
  {"fat","thick"},{"big","huge"},{"massive","huge"},{"tiny","thin"},{"subtle","quiet"},
  {"harsh","aggressive"},{"nasty","dirty"},{"rough","gritty"},{"warped","wobbly"},
  {"broken","lofi"},{"old","vintage"},{"roomy","wet"},{"reverb","wet"},{"airy","bright"},
  {"boomy","deep"},{"heavy","thick"},{"mellow","soft"},{"gentle","soft"},{"driven","dirty"},
  {"saturated","dirty"},{"distorted","dirty"},{"stabby","plucky"},{"percussive","plucky"},
  {"clipped","short"},{"floaty","long"},{"delay","echo"},{"echoey","echo"},{"sidechain","pumping"},
  {"sidechained","pumping"},{"pump","pumping"},{"bitcrushed","crushed"},{"bitcrush","crushed"},
  {"phaser","phased"},{"glued","compressed"},{"glue","compressed"},{"swing","bouncy"},
  {"swingy","bouncy"},{"groovy","bouncy"},{nullptr,nullptr}
};

inline std::vector<std::string> tokenize(const std::string& in, bool dropStop) {
    std::vector<std::string> out; std::string cur;
    auto flush = [&] {
        if (cur.empty()) return;
        if (dropStop) for (int i = 0; kStop[i]; ++i) if (cur == kStop[i]) { cur.clear(); return; }
        out.push_back(cur); cur.clear();
    };
    for (char raw : in) { const auto ch = static_cast<unsigned char>(raw); if (std::isalnum(ch)) cur += (char) std::tolower(ch); else flush(); }
    flush();
    return out;
}
inline std::string resolveAlias(const std::string& w) {
    for (int i = 0; kAliases[i][0]; ++i) if (w == kAliases[i][0]) return kAliases[i][1];
    return w;
}
inline bool isSceneWord(const std::string& w) {
    for (int i = 0; kSceneWords[i]; ++i) if (w == kSceneWords[i]) return true;
    return false;
}
inline const ModWord* findMod(const std::string& w) {
    for (const auto& m : kModWords) if (w == m.word) return &m;
    return nullptr;
}
inline const MacroWord* findMacro(const std::string& w) {
    for (const auto& m : kMacroWords) if (w == m.word) return &m;
    return nullptr;
}
inline bool isModifier(const std::string& w) { return findMod(w) || findMacro(w); }

} // namespace detail

inline MatchResult matchVibe(const std::string& text) {
    using namespace detail;
    MatchResult r;
    auto toks = tokenize(text, true);
    for (auto& t : toks) if (isSceneWord(t)) t = kSceneTag;

    // ---- stage 1: score the bank ------------------------------------------
    int bestIdx = 0, bestScore = -1, secondIdx = 0, secondScore = -1;
    for (int i = 0; i < kNumPresets; ++i) {
        auto bag  = tokenize(std::string(kPresets[i].tags) + " " + kPresets[i].name, false);
        auto name = tokenize(kPresets[i].name, false);
        int score = 0;
        for (const auto& t : toks) {
            bool exact = false, partial = false;
            for (const auto& b : bag) {
                if (b == t) { exact = true; break; }
                if (b.size() > 3 && t.size() > 3 && (b.rfind(t, 0) == 0 || t.rfind(b, 0) == 0)) partial = true;
            }
            // Modifier words are applied in stage 2 anyway, so they count half
            // here; that lets instrument nouns (string, stab, bass) decide.
            const int weight = isModifier(resolveAlias(t)) ? 1 : 2;
            score += exact ? weight : (partial ? 1 : 0);
            // a word in the preset's own name is the most specific signal
            if (std::find(name.begin(), name.end(), t) != name.end()) score += 1;
        }
        score *= 2;                                     // leave room for the tie-break
        if (! kPresets[i].variant) score += 1;          // base sounds beat their variants on ties
        if (score > bestScore)        { secondScore = bestScore; secondIdx = bestIdx; bestScore = score; bestIdx = i; }
        else if (score > secondScore) { secondScore = score; secondIdx = i; }
    }

    if (bestScore <= 1) {
        r.patch = kPresets[0].patch;
        r.sources.push_back(std::string(kPresets[0].name) + " (fallback)");
    } else if (secondScore > 1 && (float) secondScore >= (float) bestScore * 0.6f
               && kPresets[secondIdx].category == kPresets[bestIdx].category) {
        // only blend within a category: half a bass and half a pad is neither
        const auto& A = kPresets[bestIdx].patch;
        const auto& B = kPresets[secondIdx].patch;
        const float w = (float) bestScore / (float) (bestScore + secondScore);
        for (int i = 0; i < kNumFloatParams; ++i) r.patch.v[i] = A.v[i] * w + B.v[i] * (1.f - w);
        for (int i = 0; i < kNumChoiceParams; ++i) r.patch.c[i] = (w >= 0.5f ? A : B).c[i];
        r.sources.push_back(kPresets[bestIdx].name);
        r.sources.push_back(kPresets[secondIdx].name);
    } else {
        r.patch = kPresets[bestIdx].patch;
        r.sources.push_back(kPresets[bestIdx].name);
    }

    // ---- stage 2: modifiers ----------------------------------------------
    for (const auto& raw : toks) {
        const std::string w = resolveAlias(raw);
        if (const auto* mw = findMacro(w)) {
            r.applied.push_back(mw->word);
            for (const auto& s : mw->set)
                if (s.param >= 0) r.patch.v[s.param] = std::max(r.patch.v[s.param], s.amount);
            continue;
        }
        const auto* m = findMod(w);
        if (! m) continue;
        r.applied.push_back(m->word);
        for (const auto& fm : m->f) {
            if (fm.param < 0) continue;
            float& x = r.patch.v[fm.param];
            if (fm.op == '*') x *= fm.amount; else if (fm.op == '+') x += fm.amount; else x = fm.amount;
        }
        for (const auto& cm : m->c) if (cm.param >= 0) r.patch.c[cm.param] = cm.value;
    }
    r.patch.clampAll();

    // ---- name ----------------------------------------------------------------
    static const char* const nouns[] = {"Room","Line","Hour","Street","Tape","Corner",
                                        "Window","Motor","Field","Signal","Lobby","Terrace"};
    std::string adj = toks.empty() ? "New" : toks.front();
    for (const auto& t : toks) if (isModifier(resolveAlias(t))) { adj = t; break; }
    if (adj == kSceneTag) adj = "Groove";
    if (! adj.empty()) adj[0] = (char) std::toupper((unsigned char) adj[0]);
    std::size_t h = 0; for (char ch : text) h = h * 31u + (unsigned char) ch;
    r.name = adj + " " + nouns[h % 12];
    return r;
}

} // namespace dustbox
