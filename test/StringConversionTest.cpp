#include <gtest/gtest.h>
#include <jx/StringUtils.h>

TEST(StringUtilsTest, toAndFromUtf16) {
    std::string utf8 = "Hello World äöü";
    std::u16string utf16 = stringutils::utf8ToUtf16(utf8);
    std::string back = stringutils::utf16toUtf8(utf16);
    ASSERT_EQ(utf8, back);
}

TEST(StringUtilsTest, notationChange){
    ASSERT_EQ("java/lang/Class", stringutils::dotToSlashNotatation("java.lang.Class"));
    ASSERT_EQ("java.lang.Class", stringutils::slashToDotNotation("java/lang/Class"));
}

