#ifndef SJTU_TIME_HPP
#define SJTU_TIME_HPP

#include <string>

namespace sjtu {

//转换成天
inline int date_to_day(int month, int day) {
    if (month == 6) return day - 1;
    if (month == 7) return 30 + day - 1;
    return 61 + day - 1;
}

//天转换成日期
inline void day_to_date(int day_idx, int& month, int& day) {
    if (day_idx < 30) {
        month = 6;
        day = day_idx + 1;
    }
    else if (day_idx < 61) {
        month = 7;
        day = day_idx - 30 + 1;
    }
    else {
        month = 8;
        day = day_idx - 61 + 1;
    }
}

//时刻变为绝对分钟
inline int time_to_min(int hour, int minute) {
    return hour * 60 + minute;
}

inline void min_to_time(int minutes, int& hour, int& minute) {
    hour = minutes / 60;
    minute = minutes % 60;
}

//分钟转为时刻字符串
inline std::string min_to_str(int minutes) {
    int h = minutes / 60;
    int m = minutes % 60;
    std::string s;
    if (h < 10) s += "0";
    s += std::to_string(h);
    s += ":";
    if (m < 10) s += "0";
    s += std::to_string(m);
    return s;
}

//绝对分钟转为日期和时间
inline std::string abs_to_str(int abs_min) {
    int month, day;
    day_to_date(abs_min / 1440, month, day);
    std::string s;
    if (month < 10) s += "0";
    s += std::to_string(month);
    s += "-";
    if (day < 10) s += "0";
    s += std::to_string(day);
    s += " ";
    s += min_to_str(abs_min % 1440);
    return s;
}

const char START_TIME[] = "xx-xx xx:xx";

} // namespace sjtu

#endif
