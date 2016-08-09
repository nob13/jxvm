#include "StringUtils.h"
#include <locale>



namespace stringutils {

// See http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-closed.html#721
template<class I, class E, class S>
struct codecvt : std::codecvt<I, E, S>
{
    ~codecvt()
    { }
};

std::wstring_convert<codecvt<char16_t,char,std::mbstate_t>,char16_t> convertUtf8ToUtf16;

std::u16string utf8ToUtf16(const std::string& utf8String) {
    return convertUtf8ToUtf16.from_bytes(utf8String);
}

std::string utf16toUtf8(const std::u16string& utf16String) {
    return convertUtf8ToUtf16.to_bytes(utf16String);
}

std::string dotToSlashNotatation(const std::string& name) {
    std::string result (name);
    for (char & c : result){
        if (c == '.'){
            c = '/';
        }
    }
    return result;
}

std::string slashToDotNotation (const std::string& name) {
    std::string result (name);
    for (char & c : result){
        if (c == '/'){
            c = '.';
        }
    }
    return result;
}


}