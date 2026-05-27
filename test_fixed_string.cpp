#include "include/utils/fixed_string.hpp"
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
    TEST("default constructor");
    FixedString<10> s;
    CHECK(s.empty(), "default should be empty");
}

void test_cstr_constructor() {
    TEST("const char* constructor (normal)");
    FixedString<10> s("hello");
    CHECK(!s.empty(), "should not be empty");
    CHECK(s[0] == 'h', "s[0] should be h");
    CHECK(s[4] == 'o', "s[4] should be o");
}

void test_cstr_constructor_truncate() {
    TEST("const char* constructor (truncation)");
    FixedString<5> s("hello world");
    CHECK(s[0] == 'h' && s[4] == 'o', "should truncate to 5 chars");
    CHECK(s == FixedString<5>("hello"), "should equal 'hello'");
}

void test_cstr_constructor_null() {
    TEST("const char* constructor (nullptr)");
    FixedString<10> s(nullptr);
    CHECK(s.empty(), "nullptr should be empty");
}

void test_string_constructor() {
    TEST("std::string constructor");
    std::string stds = "world";
    FixedString<10> s(stds);
    CHECK(!s.empty(), "should not be empty");
    CHECK(s[0] == 'w', "s[0] should be w");
}

void test_copy_constructor() {
    TEST("copy constructor");
    FixedString<10> a("hello");
    FixedString<10> b(a);
    CHECK(a == b, "copy should equal original");
    CHECK(b[0] == 'h', "b[0] should be h");
}

// ========== 赋值运算符测试 ==========
void test_copy_assignment() {
    TEST("copy assignment");
    FixedString<10> a("hello");
    FixedString<10> b;
    b = a;
    CHECK(a == b, "assigned should equal original");
}

void test_cstr_assignment() {
    TEST("const char* assignment");
    FixedString<10> s;
    s = "test";
    CHECK(s == FixedString<10>("test"), "should equal 'test'");
}

void test_string_assignment() {
    TEST("std::string assignment");
    FixedString<10> s;
    std::string stds = "abc";
    s = stds;
    CHECK(s == FixedString<10>("abc"), "should equal 'abc'");
}

void test_self_assignment() {
    TEST("self assignment");
    FixedString<10> s("hello");
    s = s;
    CHECK(s == FixedString<10>("hello"), "self-assign should be safe");
}

// ========== 比较运算符测试 ==========
void test_equality() {
    TEST("operator==");
    FixedString<10> a("abc"), b("abc"), c("abd");
    CHECK(a == b, "same content should be equal");
    CHECK(!(a == c), "different content should not be equal");
}

void test_inequality() {
    TEST("operator!=");
    FixedString<10> a("abc"), b("abd");
    CHECK(a != b, "different content should be not equal");
}

void test_less_than() {
    TEST("operator<");
    FixedString<10> a("abc"), b("abd");
    CHECK(a < b, "abc < abd");
    CHECK(!(b < a), "abd not < abc");
}

void test_greater_than() {
    TEST("operator>");
    FixedString<10> a("abc"), b("abd");
    CHECK(b > a, "abd > abc");
}

void test_less_equal() {
    TEST("operator<=");
    FixedString<10> a("abc"), b("abc"), c("abd");
    CHECK(a <= b, "abc <= abc");
    CHECK(a <= c, "abc <= abd");
}

void test_greater_equal() {
    TEST("operator>=");
    FixedString<10> a("abc"), b("abc"), c("abd");
    CHECK(b >= a, "abc >= abc");
    CHECK(c >= a, "abd >= abc");
}

// ========== 索引运算符测试 ==========
void test_index_operator() {
    TEST("operator[] (read/write)");
    FixedString<10> s("hello");
    CHECK(s[0] == 'h', "read s[0]");
    s[0] = 'H';
    CHECK(s[0] == 'H', "write s[0]");
}

void test_const_index_operator() {
    TEST("const operator[]");
    const FixedString<10> s("hello");
    CHECK(s[0] == 'h', "const read s[0]");
}

// ========== empty / clear 测试 ==========
void test_empty_and_clear() {
    TEST("empty() and clear()");
    FixedString<10> s("hello");
    CHECK(!s.empty(), "non-empty string");
    s.clear();
    CHECK(s.empty(), "after clear should be empty");
    CHECK(s[0] == '\0', "first char should be null");
}

// ========== 流运算符测试 ==========
void test_stream_output() {
    TEST("operator<<");
    FixedString<10> s("hello");
    std::ostringstream oss;
    oss << s;
    CHECK(oss.str() == "hello", "output should be 'hello'");
}

void test_stream_input() {
    TEST("operator>>");
    FixedString<10> s;
    std::istringstream iss("world");
    iss >> s;
    CHECK(!s.empty(), "after input should not be empty");
}

// ========== 边界测试 ==========
void test_boundary_exact_fit() {
    TEST("boundary: exact fit");
    FixedString<5> s("12345");
    CHECK(s[4] == '5', "last char should be 5");
}

void test_boundary_empty_string() {
    TEST("boundary: empty string");
    FixedString<10> s("");
    CHECK(s.empty(), "empty cstr should be empty");
}

void test_large_string() {
    TEST("large FixedString");
    FixedString<100> s("A very long string that fits within 100 characters");
    CHECK(!s.empty(), "large string should work");
}

// ========== 类型别名测试 ==========
void test_type_aliases() {
    TEST("type aliases exist");
    UsernameStr u("user");
    PasswordStr p("pass123");
    CHECK(!u.empty() && !p.empty(), "aliases should work");
}

int main() {
    std::cout << "=== FixedString Unit Tests ===" << std::endl << std::endl;

    std::cout << "[Constructors]" << std::endl;
    test_default_constructor();
    test_cstr_constructor();
    test_cstr_constructor_truncate();
    test_cstr_constructor_null();
    test_string_constructor();
    test_copy_constructor();

    std::cout << std::endl << "[Assignment]" << std::endl;
    test_copy_assignment();
    test_cstr_assignment();
    test_string_assignment();
    test_self_assignment();

    std::cout << std::endl << "[Comparison]" << std::endl;
    test_equality();
    test_inequality();
    test_less_than();
    test_greater_than();
    test_less_equal();
    test_greater_equal();

    std::cout << std::endl << "[Index / empty / clear]" << std::endl;
    test_index_operator();
    test_const_index_operator();
    test_empty_and_clear();

    std::cout << std::endl << "[Stream]" << std::endl;
    test_stream_output();
    test_stream_input();

    std::cout << std::endl << "[Boundary]" << std::endl;
    test_boundary_exact_fit();
    test_boundary_empty_string();
    test_large_string();

    std::cout << std::endl << "[Type Aliases]" << std::endl;
    test_type_aliases();

    std::cout << std::endl << "==============================" << std::endl;
    std::cout << "Passed: " << passed << ", Failed: " << failed << std::endl;

    return failed > 0 ? 1 : 0;
}
