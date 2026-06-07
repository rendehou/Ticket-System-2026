#pragma once
#include <string>
#include "utils/fixed_string.hpp"
#include "utils/chinese_string.hpp"
#include "utils/parser.hpp"
#include "storage/bpt.hpp"
#include "utils/map/src/map.hpp"
namespace sjtu{
    class User{
        private:
            sjtu::UsernameStr user_name;
            sjtu::PasswordStr pass_word;
            sjtu::MailStr mail;
            int privilege;
            sjtu::NameStr chinese_name;
        public:
            User() = default;
            User(sjtu::UsernameStr username_,sjtu::PasswordStr password_,sjtu::MailStr mail_,int privilege_,sjtu::NameStr name){
                user_name = username_;
                pass_word = password_;
                mail = mail_;
                privilege = privilege_;
                chinese_name = name;
            } 
            int pri(){
                return privilege;
            }
            UsernameStr username(){
                return user_name;
            }
            PasswordStr password(){
                return pass_word;
            }
            NameStr chinesename(){
                return chinese_name;
            }
            MailStr mail_(){
                return mail;
            }
            bool operator<(const User& o) const { return user_name < o.user_name; }
            bool operator==(const User& o) const { return user_name == o.user_name; }
    };
    class users{//user存储池子
        private:
        bpt<UsernameStr,User> userpool{"userpool"};
        sjtu::map<UsernameStr,int> online_user;//在线用户的列表
        public:
        users(){
            online_user.clear();
        }
        bool is_online(const UsernameStr& u) const {
            return online_user.find(u) != online_user.cend();
        }
        bool insert(const result& r){//插入一个用户,0失败1成功
            /*
            返回值 注册成功：0 注册失败：-1
            权限要求
            -c 已登录，且新用户的权限低于 -c 的权限
            当前用户 -c（<cur_username>），创建一个账户名为 -u（<username>），密码为 -p（<password>），
            用户真实姓名为 -n（<name>），用户邮箱为 -m（<mailAddr>），权限为 -g（<privilege>）的用户
            特别地，创建第一个用户时，新用户权限为 10，不受上述权限规则约束。
            具体来讲，当创建第一个用户时，忽略 -c 和 -g 参数，并认为新用户优先级为 10
            如果 <username> 已经存在则注册失败
            */
            UsernameStr current_username(r.data[1].c_str());
            UsernameStr username_(r.data[2].c_str());
            PasswordStr password_(r.data[3].c_str());
            NameStr name_(r.data[4].c_str());
            MailStr mail_(r.data[5].c_str());
            int privilege_ = r.data[6].empty() ? -1 : std::stoi(r.data[6]);
            User u(username_, password_, mail_, privilege_, name_);
            if(userpool.empty()) {
                User first_user(u.username(), u.password(), u.mail_(), 10, u.chinesename());
                userpool.insert(first_user.username(), first_user);
                return 1;
            }
            auto v = userpool.find_all(u.username());
            if(!v.empty() || online_user.find(current_username) == online_user.end()) return 0;
            if(online_user[current_username] < u.pri()) return 0;
            userpool.insert(u.username(), u);
            return 1;
        }
        bool login(const result& r){
            /*
            说明
            用户 -u (<username>)尝试用 -p (<password>) 登录。 若登录成功，当前用户列表增加此用户。
            第一次进入系统时，当前用户列表为空。 若用户不存在或密码错误或者用户已经登录，则登陆失败。
            返回值
            登录成功：0 登录失败：-1
            */
            UsernameStr username(r.data[2].c_str());
            PasswordStr password(r.data[3].c_str());
            auto v = userpool.find_all(username);
            if(v.empty() || v[0].password() != password) return 0;
            if(online_user.find(username) != online_user.end()) return 0;
            online_user.insert({username, v[0].pri()});
            return 1;
        }
        bool logout(const result& r){
            /*
            说明
            -u (<username>) 用户退出登录。若退出成功，将此用户移出当前用户列表。
            返回值
            登出成功：0 登出失败：-1
            */
           UsernameStr username(r.data[2].c_str());
           if(online_user.find(username) == online_user.end()) return 0;
           online_user.erase(online_user.find(username));
           return 1;
        }
        std::string query_profile(const result& r) {
            /*
            说明
            用户 -c（<cur_username>）查询-u (<username>) 的用户信息。
            权限要求：-c 已登录，且「-c 的权限大于 -u 的权限，或是 -c 和 -u 相同」。
            查询成功：一行字符串，
            依次为被查询用户的 <username>，<name>，<mailAddr> 和 <privilege>，用一个空格隔开。
            查询失败：-1
            */
            UsernameStr current_username(r.data[1].c_str());
            UsernameStr username(r.data[2].c_str());
            if(online_user.find(current_username) == online_user.end()) return "-1";
            int p = online_user[current_username];

            auto v = userpool.find_all(username);
            if(v.empty()) return "-1";
            if(p <= v[0].pri() && current_username != username) return "-1";

            //拼接输出
            std::string information;
            information += v[0].username().to_string() + " ";
            information += v[0].chinesename().to_string() + " ";
            information += v[0].mail_().to_string() + " "; 
            information += std::to_string(v[0].pri());
            return information;
        }
        std::string modify_profile(const result& r) {
            /*
            说明
            用户-c(<cur_username>) 修改 -u(<username>) 的用户信息。修改参数同注册参数，且均可以省略。
            权限要求：-c 已登录，且「-c 的权限大于 -u 的权限，或是 -c 和 -u 相同」，且 -g 需低于 -c 的权限。
            返回值
            修改成功：返回被修改用户的信息，格式同 query_profile。
            修改失败：-1
            */
            UsernameStr current_username(r.data[1].c_str());
            UsernameStr username(r.data[2].c_str());
            if(online_user.find(current_username) == online_user.end()) return "-1";
            int p = online_user[current_username];
            auto v = userpool.find_all(username);
            if(v.empty()) return "-1";
            if(p <= v[0].pri() && current_username != username) return "-1";

            //读出原用户
            PasswordStr new_pw  = r.data[3].empty() ? v[0].password() : PasswordStr(r.data[3].c_str());
            NameStr new_nm  = r.data[4].empty() ? v[0].chinesename() : NameStr(r.data[4].c_str());
            MailStr new_mail= r.data[5].empty() ? v[0].mail_() : MailStr(r.data[5].c_str());
            int new_g = r.data[6].empty() ? v[0].pri() : std::stoi(r.data[6]);

            if(p <= new_g) return "-1";
            User u(username,new_pw,new_mail,new_g,new_nm);
            //写入修改后的用户到数据库
            userpool.remove(v[0].username(), v[0]);
            userpool.insert(u.username(), u);
            //如果被修改用户在线，更新在线列表中的权限
            if(online_user.find(u.username()) != online_user.end()) {
                online_user[u.username()] = u.pri();
            }
            std::string information;
            information += u.username().to_string() + " ";
            information += u.chinesename().to_string() + " ";
            information += u.mail_().to_string() + " "; 
            information += std::to_string(u.pri());
            return information;
        }
    };
};
