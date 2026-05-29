#ifndef FIXED_STRING_HPP
#define FIXED_STRING_HPP

#include <string>
#include <cstring>
#include <iostream>

namespace sjtu{
        template<size_t N>
    class FixedString{
        private:
            char data_[N+1];
        public:
            FixedString(){
                std::memset(data_,0,N+1);
            }
            FixedString(const char* s){
                std::memset(data_, 0, N + 1);
                if (s) {
                    size_t len = std::strlen(s);
                    if (len > N) len = N;
                    std::memcpy(data_, s, len);
                }
            }
            FixedString(std::string &s){
                std::memset(data_, 0, N + 1);
                size_t len = s.size();
                if (len > N) len = N;
                std::memcpy(data_, s.data(), len);
            }
            FixedString(const FixedString& o) {
                std::memcpy(data_, o.data_, N + 1);
            }

            FixedString& operator=(const FixedString& o) {
                if (this != &o) std::memcpy(data_, o.data_, N + 1);
                return *this;
            }
            FixedString& operator=(const char* s) {
                std::memset(data_, 0, N + 1);
                if (s) {
                    size_t len = std::strlen(s);
                    if (len > N) len = N;
                    std::memcpy(data_, s, len);
                }
                return *this;
            }
            FixedString& operator=(const std::string& s) {
                std::memset(data_, 0, N + 1);
                size_t len = s.size();
                if (len > N) len = N;
                std::memcpy(data_, s.data(), len);
                return *this;
            }

            bool operator== (const FixedString& other) const {return std::strncmp(data_,other.data_,N) == 0;}
            bool operator!= (const FixedString& other) const {return std::strncmp(data_,other.data_,N) != 0;}
            bool operator< (const FixedString& other) const {return std::strncmp(data_, other.data_, N) < 0;}
            bool operator> (const FixedString& other) const {return std::strncmp(data_, other.data_, N) > 0;}
            bool operator<= (const FixedString& other) const {return std::strncmp(data_, other.data_, N) <= 0;}
            bool operator>= (const FixedString& other) const {return std::strncmp(data_, other.data_, N) >= 0;}

            char& operator[](size_t i){
                return data_[i];
            }
            const char& operator[](size_t i) const {
                return data_[i];
            }

            friend std::istream& operator>>(std::istream& is, FixedString& f){
                std::string s;
                is >> s;
                f = s;
                return is;
            }
            friend std::ostream& operator<<(std::ostream& os, const FixedString& f){
                os << f.data_;
                return os;
            }

            bool empty() const { return data_[0] == '\0';}
            void clear() { std::memset(data_, 0, N + 1);}
    };
    using UsernameStr = FixedString<20>;
    using PasswordStr = FixedString<30>;
    using MailStr = FixedString<30>;
    using TrainIDStr = FixedString<20>;

};

#endif