#include<iostream>
#include<string>
#include "./vector/vector.hpp"
#include "string_check.hpp"

namespace sjtu{
    enum class keywords{
        UnKnown = 0,
        AddUser,
        Login,
        Logout,
        QueryProfile,
        ModifyProfile,
        AddTrain,
        DeleteTrain,
        ReleaseTrain,
        QueryTrain,
        QueryTicket,
        QueryTransfer,
        BuyTicket,
        QueryOrder,
        Refund,
        Clean,
        Exit
    };
    struct result{
        keywords keyword;
        std::string data[25];
        int timestamp_;
        result() = default;
        //一共有16个参数，从1到16，分别是：
        /*
        1:当前用户 c
        2：用户名 u
        3：密码 p
        4：真名 n
        5：mail m
        6：优先级 g
        7：火车编号
        8：车站数量
        9：座位数量
        10：车站名字
        11：价格
        12: 出发时间
        13：路程时间，每两站之间的路程
        14：停站时间
        15：能买票的时间
        16：票类型
        17: 购票数量
        18：出发站
        19：到达站
        20：后补
        21：换乘的排序关键字
        22：退票排序
        */
    };
    /*
    sjtu::vector<size_t> piecefy(std::string s){//从大段字符中找到所有空格
        sjtu::vector<size_t> v;

        for(int i = 0;i <= s.length();i++){
            if(s[i] == ' '){

            }
        }
        return v;
    }*/
    std::string get_str(int i,std::string s){//从某一位置截取字符串直到遇到空格
        std::string str;
        for(int j = i;j < s.length();j++){
            if(s[j] == ' ') return str;
            str += s[j];
        }
        return str;
    }

    //按 | 拆分字符串，检查每段非空
    inline bool piecefy(const std::string& val) {
        std::string cur;
        for (size_t pos = 0; pos <= val.length(); ++pos) {
            if (pos == val.length() || val[pos] == '|') {
                if (cur.empty()) return false;
                cur.clear();
            } else {
                cur += val[pos];
            }
        }
        return true;
    }

    inline bool valid_station_num(const std::string& s) {
        return valid_positive(s, 100) && std::stoi(s) >= 2; 
    }
    inline bool valid_seat_num(const std::string& s){
        return valid_positive(s, 100000) && std::stoi(s) > 0; 
    }
    inline bool valid_price_item(const std::string& s) {
        return valid_positive(s, 100000); 
    }
    inline bool valid_travel_item(const std::string& s) { 
        return valid_positive(s, 10000); 
    }
    inline bool valid_stopover_item(const std::string& s) { 
        return (s == "_") || valid_positive(s, 10000); 
    }

    inline bool validator(keywords cmd, char key, const std::string& val) {//检查是否符合要求的表
        switch (cmd) {
        case keywords::AddUser:
        case keywords::ModifyProfile:
            if (key == 'c' || key == 'u') return valid_username(val);
            if (key == 'p') return valid_password(val);
            if (key == 'n') return valid_name(val);
            if (key == 'm') return valid_mail(val);
            if (key == 'g') return valid_privilege(val);
            break;
        case keywords::Login:
            if (key == 'u') return valid_username(val);
            if (key == 'p') return valid_password(val);
            break;
        case keywords::Logout:
        case keywords::QueryOrder:
            if (key == 'u') return valid_username(val);
            break;
        case keywords::QueryProfile:
            if (key == 'c' || key == 'u') return valid_username(val);
            break;
        case keywords::AddTrain:
            if (key == 'i') return valid_trainID(val);
            if (key == 'n') return valid_station_num(val);
            if (key == 'm') return valid_seat_num(val);
            if (key == 's') return piecefy(val);
            if (key == 'p') return piecefy(val);
            if (key == 'x') return valid_time(val);
            if (key == 't') return piecefy(val);
            if (key == 'o') return (val == "_") || piecefy(val);
            if (key == 'd') return valid_sale_date(val);
            if (key == 'y') return valid_type(val);
            break;
        case keywords::DeleteTrain:
        case keywords::ReleaseTrain:
            if (key == 'i') return valid_trainID(val);
            break;
        case keywords::QueryTrain:
            if (key == 'i') return valid_trainID(val);
            if (key == 'd') return valid_date(val);
            break;
        case keywords::QueryTicket:
        case keywords::QueryTransfer:
            if (key == 's') return valid_station(val);
            if (key == 't') return valid_station(val);
            if (key == 'd') return valid_date(val);
            if (key == 'p') return valid_sort(val);
            break;
        case keywords::BuyTicket:
            if (key == 'u') return valid_username(val);
            if (key == 'i') return valid_trainID(val);
            if (key == 'd') return valid_date(val);
            if (key == 'n') return valid_integer(val);
            if (key == 'f') return valid_station(val);
            if (key == 't') return valid_station(val);
            if (key == 'q') return valid_queue_flag(val);
            break;
        case keywords::Refund:
            if (key == 'u') return valid_username(val);
            if (key == 'n') return valid_integer(val);
            break;
        default:
            break;
        }
        return true;
    }
    inline int key_to_index(keywords cmd, char key) {
        switch (key) {
        case 'c': return 1;
        case 'u': return 2;
        case 'p':
            if (cmd == keywords::QueryTicket || cmd == keywords::QueryTransfer)
                return 21;
            return (cmd == keywords::AddTrain) ? 11 : 3;
        case 'n':
            if (cmd == keywords::BuyTicket) return 17;
            if (cmd == keywords::Refund) return 22;
            return (cmd == keywords::AddTrain) ? 8 : 4;
        case 'm':
            return (cmd == keywords::AddTrain) ? 9 : 5;
        case 'g': return 6;
        case 'i': return 7;
        case 's': return 10;
        case 'x': return 12;
        case 't':
            if (cmd == keywords::AddTrain) return 13;
            return 19;
        case 'o': return 14;
        case 'd': return 15;
        case 'y': return 16;
        case 'f': return 18;
        case 'q': return 20;
        default:  return 0;
        }   
    }
    result parser(std::string input) {//命令字符串由三部分组成，分别是时间戳，指令名称，具体数据
        result result_;
        std::string command,timestamp,arguments;

        //检查时间戳
        int i = 0;
        while(i < input.length() && input[i] != ' '){
            //if(input[i] == ' ') break;
            timestamp += input[i];
            i++;
        }
        if(timestamp[0] != '[' || timestamp.back() != ']') return result();
        try{
            std::string str = timestamp.substr(1,i-1);
            int time = std::stoi(str);
            result_.timestamp_ = time;
        }catch(...){
            return result();
        }

        //拆分命令块
        i++;
        while(i < input.length() && input[i] != ' '){
            command += input[i];
            i++;
        }
    
        if(command == "add_user") result_.keyword = keywords::AddUser;
        else if(command == "login") result_.keyword = keywords::Login;
        else if(command == "logout") result_.keyword = keywords::Logout;
        else if(command == "query_profile") result_.keyword = keywords::QueryProfile;
        else if(command == "modify_profile") result_.keyword = keywords::ModifyProfile;
        else if(command == "add_train") result_.keyword = keywords::AddTrain;
        else if(command == "delete_train") result_.keyword = keywords::DeleteTrain;
        else if(command == "release_train") result_.keyword = keywords::ReleaseTrain;
        else if(command == "query_train") result_.keyword = keywords::QueryTrain;
        else if(command == "query_ticket") result_.keyword = keywords::QueryTicket;
        else if(command == "query_transfer") result_.keyword = keywords::QueryTransfer;
        else if(command == "buy_ticket") result_.keyword = keywords::BuyTicket;
        else if(command == "query_order") result_.keyword = keywords::QueryOrder;
        else if(command == "refund_ticket") result_.keyword = keywords::Refund;
        else if(command == "clean") result_.keyword = keywords::Clean;
        else if(command == "exit") result_.keyword = keywords::Exit;
        else return result();

        //检验并存储
        while(i < input.length()){
            i++;
            if(input[i] == '-'){
                i++;
                char k = input[i];
                std::string ss = get_str(i+2,input);
                if(validator(result_.keyword,k,ss)) {
                    int index = key_to_index(result_.keyword,k);
                    result_.data[index] = ss;
                    i = i + 1 + ss.length();
                }
                else{
                    return result();
                }
            }
        }
        //检查每一个结果对应的data是否都存在
        if(result_.keyword == keywords::AddUser){
            if(result_.data[2].empty() || result_.data[3].empty() ||
               result_.data[4].empty() || result_.data[5].empty()) return result();
        }
        else if(result_.keyword == keywords::Login){
            if(result_.data[2].empty() || result_.data[3].empty()) return result();
        }
        else if(result_.keyword == keywords::Logout || result_.keyword == keywords::QueryOrder){
            if(result_.data[2].empty()) return result();
        }
        else if(result_.keyword == keywords::QueryProfile){
            if(result_.data[1].empty() || result_.data[2].empty()) return result();
        }
        else if(result_.keyword == keywords::ModifyProfile){
            if(result_.data[1].empty() || result_.data[2].empty()) return result();
        }
        else if(result_.keyword == keywords::AddTrain){
            //检查全部必选字段非空
            if(result_.data[7].empty() || result_.data[8].empty() || result_.data[9].empty() ||
               result_.data[10].empty() || result_.data[11].empty() || result_.data[12].empty() ||
               result_.data[13].empty() || result_.data[14].empty() || result_.data[15].empty() ||
               result_.data[16].empty())
                return result();

            int stNum = std::stoi(result_.data[8]);

            //检查 -s 站名数量
            int s_cnt = 0;
            for (char c : result_.data[10]) { 
                if (c == '|') ++s_cnt; 
            }
            if (s_cnt + 1 != stNum) return result();
            //校验 -s 每段是否为合法站名
            if (!piecefy(result_.data[10])) return result();
            {
                std::string cur;
                for (size_t p = 0; p <= result_.data[10].length(); ++p) {
                    if (p == result_.data[10].length() || result_.data[10][p] == '|') {
                        if (!valid_station(cur)) return result();
                        cur.clear();
                    } 
                    else { 
                        cur += result_.data[10][p]; 
                    }
                }
            }
            //检查 -p 价格数量
            int p_cnt = 0;
            for (char c : result_.data[11]) { 
                if (c == '|') ++p_cnt; 
            }
            if (p_cnt + 1 != stNum - 1) return result();
            //校验 -p 每段是否小于100000
            {
                std::string cur;
                for (size_t p = 0; p <= result_.data[11].length(); ++p) {
                    if (p == result_.data[11].length() || result_.data[11][p] == '|') {
                        if (!valid_price_item(cur)) return result();
                        cur.clear();
                    } 
                    else { 
                        cur += result_.data[11][p]; 
                    }
                }
            }
            //检查 -t 路程数量
            int t_cnt = 0;
            for (char c : result_.data[13]) { 
                if (c == '|') ++t_cnt; 
            }
            if (t_cnt + 1 != stNum - 1) return result();
            //校验 -t 每段是否正确
            {
                std::string cur;
                for (size_t p = 0; p <= result_.data[13].length(); ++p) {
                    if (p == result_.data[13].length() || result_.data[13][p] == '|') {
                        if (!valid_travel_item(cur)) return result();
                        cur.clear();
                    } else { 
                        cur += result_.data[13][p]; 
                    }
                }
            }
            //检查 -o 停留数量
            if (stNum == 2) {
                if (result_.data[14] != "_") return result();
            }
            else {
                int o_cnt = 0;
                for (char c : result_.data[14]) { 
                    if (c == '|') ++o_cnt; 
                }
                if (o_cnt + 1 != stNum - 2) return result();
            }
            //校验 -o 每段是否为_或者小于一万
            {
                std::string cur;
                for (size_t p = 0; p <= result_.data[14].length(); ++p) {
                    if (p == result_.data[14].length() || result_.data[14][p] == '|') {
                        if (!valid_stopover_item(cur)) return result();
                        cur.clear();
                    } 
                    else { 
                        cur += result_.data[14][p]; 
                    }
                }
            }
        }
        else if(result_.keyword == keywords::DeleteTrain || result_.keyword == keywords::ReleaseTrain){
            if(result_.data[7].empty()) return result();
        }
        else if(result_.keyword == keywords::QueryTrain){
            if(result_.data[7].empty() || result_.data[15].empty()) return result();
        }
        else if(result_.keyword == keywords::QueryTicket || result_.keyword == keywords::QueryTransfer){
            if(result_.data[10].empty() || result_.data[19].empty() || result_.data[15].empty()) return result();
            if(result_.data[21].empty()) result_.data[21] = "time";
        }
        else if(result_.keyword == keywords::BuyTicket){
            if(result_.data[2].empty() || result_.data[7].empty() || result_.data[15].empty() ||
               result_.data[17].empty() || result_.data[18].empty() || result_.data[19].empty()) return result();
            if(result_.data[20].empty()) result_.data[20] = "false";
        }
        else if(result_.keyword == keywords::Refund){
            if(result_.data[2].empty()) return result();
            if(result_.data[22].empty()) result_.data[22] = "1";
        }
        return result_;
    }
    
};