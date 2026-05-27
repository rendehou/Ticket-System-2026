#include "include/utils/chinese_string.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

static int passed = 0, failed = 0;

#define TEST(name) do { std::cout << "  TEST: " << (name) << " ... "; } while(0)
#define PASS() do { std::cout << "PASSED" << std::endl; ++passed; } while(0)
#define FAIL(msg) do { std::cout << "FAILED - " << (msg) << std::endl; ++failed; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); } else { PASS(); } } while(0)

// ========== 构造函数测试 ==========
void test_default_constructor() {
    TEST("默认构造");
    ChineseString<5> s;
    CHECK(s.empty(), "默认应为空");
}

void test_cstr_constructor_common() {
    TEST("const char* 构造 (常见字)");
    ChineseString<5> s("北京西");
    CHECK(!s.empty(), "不应为空");
    CHECK(s.byte_length() == 9, "北京西 = 9 字节");
    CHECK(s.char_size() == 3, "北京西 = 3 个字");
}

void test_cstr_constructor_rare() {
    TEST("const char* 构造 (生僻字: 龘靐齉)");
    ChineseString<5> s("龘靐齉");  // 每个都是 3 字节 UTF-8
    CHECK(!s.empty(), "不应为空");
    CHECK(s.byte_length() == 9, "龘靐齉 = 9 字节");
    CHECK(s.char_size() == 3, "龘靐齉 = 3 个字");
}

void test_cstr_constructor_mixed() {
    TEST("const char* 构造 (常见+生僻混排: 北京龘)");
    ChineseString<5> s("北京龘");
    CHECK(s.byte_length() == 9, "北京龘 = 9 字节");
    CHECK(s.char_size() == 3, "北京龘 = 3 个字");
}

void test_string_constructor() {
    TEST("std::string 构造");
    std::string stds = "爨癵";  // 两个生僻字
    ChineseString<5> s(stds);
    CHECK(s.byte_length() == 6, "爨癵 = 6 字节");
    CHECK(s.char_size() == 2, "爨癵 = 2 个字");
}

void test_copy_constructor() {
    TEST("拷贝构造 (生僻字)");
    ChineseString<5> a("龘靐");
    ChineseString<5> b(a);
    CHECK(a == b, "拷贝应相等");
    CHECK(b.char_size() == 2, "拷贝后 char_size 应为 2");
}

void test_truncate_overflow() {
    TEST("截断: 超过 MaxChars 的输入");
    ChineseString<3> s("一二三四五六");
    // MaxBytes = 3*3 = 9, 一二三四五六 = 18 字节, 应截断为 9 字节 = "一二三"
    CHECK(s.byte_length() == 9, "应截断为 9 字节");
    CHECK(s.char_size() == 3, "应截断为 3 个字");
}

// ========== 赋值运算符测试 ==========
void test_copy_assignment() {
    TEST("拷贝赋值");
    ChineseString<5> a("龘龘");
    ChineseString<5> b;
    b = a;
    CHECK(a == b, "赋值后应相等");
}

void test_cstr_assignment() {
    TEST("const char* 赋值 (生僻字)");
    ChineseString<5> s;
    s = "靐";
    CHECK(s == ChineseString<5>("靐"), "应等于'靐'");
    CHECK(s.char_size() == 1, "应为 1 个字");
}

void test_self_assignment() {
    TEST("自赋值");
    ChineseString<5> s("爨");
    s = s;
    CHECK(s == ChineseString<5>("爨"), "自赋值应安全");
}

// ========== 比较运算符测试 ==========
void test_equality_rare() {
    TEST("operator== (生僻字)");
    ChineseString<5> a("龘"), b("龘"), c("靐");
    CHECK(a == b, "相同生僻字应相等");
    CHECK(!(a == c), "不同生僻字应不等");
}

void test_less_than_rare() {
    TEST("operator< (生僻字 Unicode 序)");
    // 龘 U+9F98, 齉 U+9F49 → 齉 < 龘
    ChineseString<5> a("龘"), b("齉");
    CHECK(b < a, "齉(U+9F49) < 龘(U+9F98)");
}

void test_compare_mixed() {
    TEST("比较 (常见 vs 生僻)");
    // 一 U+4E00, 龘 U+9F98 → 一 < 龘
    ChineseString<5> a("一"), b("龘");
    CHECK(a < b, "一(U+4E00) < 龘(U+9F98)");
}

// ========== c_str / to_string / byte_length / empty / clear ==========
void test_c_str() {
    TEST("c_str()");
    ChineseString<5> s("龘");
    CHECK(std::strcmp(s.c_str(), "龘") == 0, "c_str 应返回 '龘'");
}

void test_to_string() {
    TEST("to_string()");
    ChineseString<5> s("靐爨");
    CHECK(s.to_string() == "靐爨", "to_string 应返回 '靐爨'");
}

void test_byte_length() {
    TEST("byte_length()");
    ChineseString<10> s("一二三");
    CHECK(s.byte_length() == 9, "一二三 = 9 字节");
    ChineseString<10> s2("a");
    CHECK(s2.byte_length() == 1, "a = 1 字节");
}

void test_empty_and_clear() {
    TEST("empty() & clear()");
    ChineseString<5> s("齉");
    CHECK(!s.empty(), "非空");
    s.clear();
    CHECK(s.empty(), "clear 后为空");
    CHECK(s.byte_length() == 0, "clear 后 byte_length = 0");
}

// ========== char_size 测试 ==========
void test_char_size_chinese() {
    TEST("char_size (纯中文)");
    ChineseString<10> s("北京西站");
    CHECK(s.char_size() == 4, "北京西站 = 4 个字");
}

void test_char_size_rare() {
    TEST("char_size (生僻字)");
    ChineseString<10> s("龘靐齉爨癵");
    CHECK(s.char_size() == 5, "5 个生僻字");
}

void test_char_size_ascii() {
    TEST("char_size (ASCII)");
    ChineseString<10> s("ABC");
    CHECK(s.char_size() == 3, "ABC = 3 个字符");
}

void test_char_size_mixed_ascii_chinese() {
    TEST("char_size (ASCII+中文混排)");
    ChineseString<10> s("A龘B");
    CHECK(s.char_size() == 3, "A龘B = 3 个字符");
}

void test_char_size_empty() {
    TEST("char_size (空串)");
    ChineseString<10> s;
    CHECK(s.char_size() == 0, "空串 = 0");
}

// ========== valid_chinese 测试 ==========
void test_valid_chinese_normal() {
    TEST("valid_chinese (常见字: 北京西)");
    ChineseString<10> s("北京西");
    CHECK(s.valid_chinese(1, 10), "北京西 应在 1~10 字范围");
}

void test_valid_chinese_rare() {
    TEST("valid_chinese (生僻字在基本CJK: 龘靐齉)");
    // 龘 U+9F98, 靐 U+9750, 齉 U+9F49 都在 U+4E00..U+9FFF 范围内
    ChineseString<10> s("龘靐齉");
    CHECK(s.valid_chinese(1, 10), "龘靐齉 都是基本 CJK 汉字");
}

void test_valid_chinese_out_of_range_count() {
    TEST("valid_chinese (字数超出范围)");
    ChineseString<5> s("一二三四五");
    CHECK(!s.valid_chinese(1, 3), "5 个字不在 1~3 范围");
}

void test_valid_chinese_ascii_reject() {
    TEST("valid_chinese (含 ASCII 应被拒)");
    ChineseString<10> s("北京A");
    CHECK(!s.valid_chinese(1, 10), "含 ASCII 不应通过");
}

void test_valid_chinese_empty() {
    TEST("valid_chinese (空串)");
    ChineseString<10> s;
    CHECK(!s.valid_chinese(1, 10), "空串不合 1~10 范围");
    CHECK(s.valid_chinese(0, 10), "空串符合 0~10 范围");
}

// ========== 流运算符测试 ==========
void test_stream_output() {
    TEST("operator<< (生僻字输出)");
    ChineseString<5> s("龘靐");
    std::ostringstream oss;
    oss << s;
    CHECK(oss.str() == "龘靐", "输出应为'龘靐'");
}

void test_stream_input() {
    TEST("operator>> (生僻字输入)");
    ChineseString<10> s;
    std::istringstream iss("爨癵");
    iss >> s;
    CHECK(s == ChineseString<10>("爨癵"), "输入应为'爨癵'");
}

// ========== 边界测试 ==========
void test_boundary_exact_maxchars() {
    TEST("边界: 恰好 MaxChars 个生僻字");
    ChineseString<3> s("龘靐齉");  // 恰好 3 个汉字, 9 字节
    CHECK(s.byte_length() == 9, "应为 9 字节");
    CHECK(s.char_size() == 3, "应为 3 个字");
    CHECK(s.valid_chinese(1, 3), "应为合法中文");
}

void test_boundary_maxbytes_overflow() {
    TEST("边界: 字节级别溢出截断");
    ChineseString<2> s("一二三");  // MaxBytes=6, 一二三=9字节 → 截断为一二
    CHECK(s.byte_length() == 6, "应截断为 6 字节");
    CHECK(s.char_size() == 2, "应截断为 2 个字");
}

void test_empty_cstr() {
    TEST("边界: 空字符串构造");
    ChineseString<5> s("");
    CHECK(s.empty(), "空串应为空");
    CHECK(s.byte_length() == 0, "空串 byte_length = 0");
}

// ========== 类型别名测试 ==========
void test_name_str() {
    TEST("NameStr (5个汉字)");
    NameStr n("龘靐齉爨癵");  // 5 个生僻字, 恰好 15 字节
    CHECK(n.char_size() == 5, "NameStr 应存 5 个字");
    CHECK(n.valid_chinese(1, 5), "应为合法中文名");
}

void test_station_str() {
    TEST("StationStr (10个字)");
    StationStr s("乌鲁木齐南站龘靐");  // 8 个字
    CHECK(s.char_size() == 8, "StationStr 应存 8 个字");
}

// ========== 常量表达式测试 ==========
void test_constexpr() {
    TEST("max_chars() / max_bytes()");
    CHECK(ChineseString<5>::max_chars() == 5, "max_chars = 5");
    CHECK(ChineseString<5>::max_bytes() == 15, "max_bytes = 15");
    CHECK(NameStr::max_chars() == 5, "NameStr max_chars = 5");
}

// ========== operator[] 测试 ==========
void test_index_read() {
    TEST("operator[] 字节级读取");
    ChineseString<5> s("龘");  // UTF-8: E9 BE 98 (3 bytes)
    unsigned char b0 = (unsigned char)s[0];
    unsigned char b1 = (unsigned char)s[1];
    unsigned char b2 = (unsigned char)s[2];
    CHECK(b0 == 0xE9 && b1 == 0xBE && b2 == 0x98, "龘 UTF-8 = E9 BE 98");
}

void test_index_write() {
    TEST("operator[] 字节级修改");
    ChineseString<5> s("A");  // ASCII 'A' = 0x41
    s[0] = 'B';
    CHECK(s[0] == 'B', "修改后为 B");
    CHECK(s == ChineseString<5>("B"), "应等于 'B'");
}

int main() {
    std::cout << "=== ChineseString 单元测试（含生僻字） ===" << std::endl << std::endl;

    std::cout << "[构造]" << std::endl;
    test_default_constructor();
    test_cstr_constructor_common();
    test_cstr_constructor_rare();
    test_cstr_constructor_mixed();
    test_string_constructor();
    test_copy_constructor();
    test_truncate_overflow();

    std::cout << std::endl << "[赋值]" << std::endl;
    test_copy_assignment();
    test_cstr_assignment();
    test_self_assignment();

    std::cout << std::endl << "[比较]" << std::endl;
    test_equality_rare();
    test_less_than_rare();
    test_compare_mixed();

    std::cout << std::endl << "[c_str / to_string / byte_length / empty / clear]" << std::endl;
    test_c_str();
    test_to_string();
    test_byte_length();
    test_empty_and_clear();

    std::cout << std::endl << "[char_size]" << std::endl;
    test_char_size_chinese();
    test_char_size_rare();
    test_char_size_ascii();
    test_char_size_mixed_ascii_chinese();
    test_char_size_empty();

    std::cout << std::endl << "[valid_chinese]" << std::endl;
    test_valid_chinese_normal();
    test_valid_chinese_rare();
    test_valid_chinese_out_of_range_count();
    test_valid_chinese_ascii_reject();
    test_valid_chinese_empty();

    std::cout << std::endl << "[流运算符]" << std::endl;
    test_stream_output();
    test_stream_input();

    std::cout << std::endl << "[边界]" << std::endl;
    test_boundary_exact_maxchars();
    test_boundary_maxbytes_overflow();
    test_empty_cstr();

    std::cout << std::endl << "[operator[]]" << std::endl;
    test_index_read();
    test_index_write();

    std::cout << std::endl << "[类型别名 / constexpr]" << std::endl;
    test_name_str();
    test_station_str();
    test_constexpr();

    std::cout << std::endl << "=============================================" << std::endl;
    std::cout << "通过: " << passed << ", 失败: " << failed << std::endl;

    return failed > 0 ? 1 : 0;
}
