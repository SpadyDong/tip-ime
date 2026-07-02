#pragma once

namespace tip {

enum class LanguageMode {
    Chinese,
    English,
};

enum class CharWidthMode {
    Half,
    Full,
};

enum class PunctuationMode {
    Chinese,
    English,
};

class InputState {
public:
    LanguageMode languageMode = LanguageMode::Chinese;
    CharWidthMode charWidthMode = CharWidthMode::Half;
    PunctuationMode punctuationMode = PunctuationMode::Chinese;

    void ToggleLanguage();
    void ToggleCharWidth();
    void TogglePunctuation();
};

} // namespace tip
