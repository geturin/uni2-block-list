#pragma once
#include <cstdint>
#include <string>

enum class UbLanguage { Chinese, English };

UbLanguage UbDefaultLanguageFor(uint16_t language_id);
UbLanguage UbDefaultLanguage();
UbLanguage UbLoadLanguage();
void UbSaveLanguage(UbLanguage language);
// Display translation only. Keep the original exception text in diagnostics.
std::wstring UbTranslateError(const std::wstring& message, UbLanguage language);
