#pragma once
#include "types.h"

namespace stringutils {

std::u16string utf8ToUtf16(const std::string& utf8String);

std::string utf16toUtf8(const std::u16string& utf16String);

std::string dotToSlashNotatation(const std::string& name);

std::string slashToDotNotation (const std::string& name);

}