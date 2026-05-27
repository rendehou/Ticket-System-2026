#ifndef CHINESE_STRING_HPP
#define CHINESE_STRING_HPP

#include <cstring>
#include <iostream>
#include <string>

template <size_t MaxChars>
class ChineseString {
public:
    static constexpr size_t MaxBytes = MaxChars * 3;

private:
    char data_[MaxBytes + 1];

    static int utf8_char_len(unsigned char c) {
        if ((c & 0x80) == 0x00) return 1;
        if ((c & 0xE0) == 0xC0) return 2;
        if ((c & 0xF0) == 0xE0) return 3;
        if ((c & 0xF8) == 0xF0) return 4;
        return -1;
    }

public:
    ChineseString() { std::memset(data_, 0, MaxBytes + 1); }

    ChineseString(const char* s) {
        std::memset(data_, 0, MaxBytes + 1);
        if (s) {
            size_t len = std::strlen(s);
            if (len > MaxBytes) len = MaxBytes;
            std::memcpy(data_, s, len);
        }
    }

    ChineseString(const std::string& s) {
        std::memset(data_, 0, MaxBytes + 1);
        size_t len = s.size();
        if (len > MaxBytes) len = MaxBytes;
        std::memcpy(data_, s.data(), len);
    }

    ChineseString(const ChineseString& o) {
        std::memcpy(data_, o.data_, MaxBytes + 1);
    }

    ChineseString& operator=(const ChineseString& o) {
        if (this != &o) std::memcpy(data_, o.data_, MaxBytes + 1);
        return *this;
    }
    ChineseString& operator=(const char* s) {
        std::memset(data_, 0, MaxBytes + 1);
        if (s) {
            size_t len = std::strlen(s);
            if (len > MaxBytes) len = MaxBytes;
            std::memcpy(data_, s, len);
        }
        return *this;
    }
    ChineseString& operator=(const std::string& s) {
        std::memset(data_, 0, MaxBytes + 1);
        size_t len = s.size();
        if (len > MaxBytes) len = MaxBytes;
        std::memcpy(data_, s.data(), len);
        return *this;
    }

    bool operator==(const ChineseString& o) const { return std::strncmp(data_, o.data_, MaxBytes) == 0; }
    bool operator!=(const ChineseString& o) const { return !(*this == o); }
    bool operator< (const ChineseString& o) const { return std::strncmp(data_, o.data_, MaxBytes) <  0; }
    bool operator> (const ChineseString& o) const { return std::strncmp(data_, o.data_, MaxBytes) >  0; }
    bool operator<=(const ChineseString& o) const { return std::strncmp(data_, o.data_, MaxBytes) <= 0; }
    bool operator>=(const ChineseString& o) const { return std::strncmp(data_, o.data_, MaxBytes) >= 0; }

    char& operator[](size_t i)             { return data_[i]; }
    const char& operator[](size_t i) const { return data_[i]; }

    friend std::istream& operator>>(std::istream& is, ChineseString& s) {
        std::string tmp;
        is >> tmp;
        s = tmp;
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const ChineseString& s) {
        os << s.data_;
        return os;
    }

    const char* c_str() const { return data_; }
    std::string to_string() const { return std::string(data_); }
    size_t byte_length() const { return std::strlen(data_); }
    bool empty() const { return data_[0] == '\0'; }
    void clear() { std::memset(data_, 0, MaxBytes + 1); }

    int char_size() const {
        int cnt = 0;
        size_t i = 0, len = std::strlen(data_);
        while (i < len) {
            int sz = utf8_char_len((unsigned char)data_[i]);
            if (sz <= 0 || i + sz > len) return -1;
            ++cnt;
            i += sz;
        }
        return cnt;
    }

    bool valid_chinese(size_t minChars, size_t maxChars) const {
        size_t i = 0, len = std::strlen(data_);
        size_t cnt = 0;
        while (i < len) {
            if (i + 3 > len) return false;
            unsigned char c0 = (unsigned char)data_[i];
            unsigned char c1 = (unsigned char)data_[i + 1];
            unsigned char c2 = (unsigned char)data_[i + 2];
            if ((c0 & 0xF0) != 0xE0) return false;
            if ((c1 & 0xC0) != 0x80) return false;
            if ((c2 & 0xC0) != 0x80) return false;
            int cp = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
            if (cp < 0x4E00 || cp > 0x9FFF) return false;
            ++cnt;
            i += 3;
        }
        return cnt >= minChars && cnt <= maxChars;
    }

    static constexpr size_t max_chars()  { return MaxChars; }
    static constexpr size_t max_bytes()  { return MaxBytes; }
};

using NameStr    = ChineseString<5>;
using StationStr = ChineseString<10>;

#endif
