#include "input_state.h"

namespace tip {

void InputState::ToggleLanguage() {
    languageMode = (languageMode == LanguageMode::Chinese)
        ? LanguageMode::English
        : LanguageMode::Chinese;
}

void InputState::ToggleCharWidth() {
    charWidthMode = (charWidthMode == CharWidthMode::Half)
        ? CharWidthMode::Full
        : CharWidthMode::Half;
}

void InputState::TogglePunctuation() {
    punctuationMode = (punctuationMode == PunctuationMode::Chinese)
        ? PunctuationMode::English
        : PunctuationMode::Chinese;
}

} // namespace tip
