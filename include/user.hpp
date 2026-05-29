#include <string>
#include "../utils/fixed_string.hpp"
#include "../utils/chinese_string.hpp"
class User{
    private:
        sjtu::UsernameStr user_name;
        sjtu::PasswordStr password;
        sjtu::MailStr mail;
        int privilege;
        sjtu::NameStr chinese_name;
    public:
        User() = default;
        User() 
};