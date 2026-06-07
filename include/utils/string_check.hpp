#ifndef SJTU_VALIDATOR_HPP
#define SJTU_VALIDATOR_HPP

#include <string>
#include <cstring>
#include "fixed_string.hpp"
#include "chinese_string.hpp"

namespace sjtu {


inline bool is_digit(char c) { return c >= '0' && c <= '9'; }
inline bool is_big(char c) { return c >= 'A' && c <= 'Z'; }
inline bool is_small(char c) {return c >= 'a' && c <= 'z'; }
inline bool is_alpha(char c) { return is_big(c) || is_small(c); }
inline bool is_alpha_or_num(char c) { return is_alpha(c) || is_digit(c); }
inline bool is_visible(char c) { return c >= 33 && c <= 126; }

// username 必须是字母开头
inline bool valid_username(const std::string& s) {
    size_t len = s.length();
    if (len == 0 || len > 20) return false;
    if (!is_alpha(s[0])) return false;
    for (size_t i = 1; i < len; ++i) {
        if (!is_alpha_or_num(s[i]) && s[i] != '_') return false;
    }
    return true;
}

// password 可见字符,长度不超过30
inline bool valid_password(const std::string& s) {
    size_t len = s.length();
    if (len == 0 || len > 30) return false;
    for (size_t i = 0; i < len; ++i) {
        if (!is_visible(s[i])) return false;
    }
    return true;
}

// name 2-5个汉字
inline bool valid_name(const std::string& s) {
    if (s.length() > ChineseString<5>::MaxBytes) return false;
    ChineseString<5> cs(s.c_str());
    return cs.valid_chinese(2, 5);
}

// mailAddr  字母数字@或. 长度不超过30
inline bool valid_mail(const std::string& s) {
    size_t len = s.length();
    if (len == 0 || len > 30) return false;
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (!is_alpha_or_num(c) && c != '@' && c != '.') return false;
    }
    return true;
}

// trainID  char范围字符
inline bool valid_trainID(const std::string& s) {
    size_t len = s.length();
    if (len == 0 || len > 20) return false;
    return true;
}

// station 汉字十个以内
inline bool valid_station(const std::string& s) {
    if (s.length() > ChineseString<10>::MaxBytes) return false;
    ChineseString<10> cs(s.c_str());
    return cs.valid_chinese(1, 10);
}

// 是数字
inline bool valid_integer(const std::string& s) {
    size_t len = s.length();
    if (len == 0) return false;
    for (size_t i = 0; i < len; ++i) {
        if (!is_digit(s[i])) return false;
    }
    return true;
}

// 特权等级 10以内数字
inline bool valid_privilege(const std::string& s) {
    if (!valid_integer(s)) return false;
    int v = std::stoi(s);
    return v >= 0 && v <= 10;
}

// 时间
inline bool valid_time(const std::string& s) {
    if (s.length() != 5) return false;
    if (s[2] != ':') return false;
    if (!is_digit(s[0]) || !is_digit(s[1])) return false;
    if (!is_digit(s[3]) || !is_digit(s[4])) return false;
    int hh = (s[0]-'0')*10 + (s[1]-'0');
    int mm = (s[3]-'0')*10 + (s[4]-'0');
    return hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59;
}

// 日期
inline bool valid_date(const std::string& s) {
    if (s.length() != 5) return false;
    if (s[2] != '-') return false;
    if (!is_digit(s[0]) || !is_digit(s[1])) return false;
    if (!is_digit(s[3]) || !is_digit(s[4])) return false;
    int mm = (s[0]-'0')*10 + (s[1]-'0');
    int dd = (s[3]-'0')*10 + (s[4]-'0');
    if (mm < 1 || mm > 12) return false;
    if (dd < 1 || dd > 31) return false;
    return true;
}

// saleDate:也是日期
inline bool valid_sale_date(const std::string& s) {
    size_t pos = s.find('|');
    if (pos == std::string::npos) return false;
    std::string d1 = s.substr(0, pos);
    std::string d2 = s.substr(pos + 1);
    return valid_date(d1) && valid_date(d2);
}

inline bool valid_type(const std::string& s) {
    return s.length() == 1 && is_big(s[0]);
}

// 候补 是否同意后补
inline bool valid_queue_flag(const std::string& s) {
    return s == "true" || s == "false";
}

// 排序方式 time或cost
inline bool valid_sort(const std::string& s) {
    return s == "time" || s == "cost";
}

// 验证小于limit的整数
inline bool valid_positive(const std::string& s, int limit) {
    if (!valid_integer(s)) return false;
    return std::stoi(s) <= limit;
}

} // namespace sjtu

#endif
