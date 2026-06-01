#include <iostream>
#include <string>
#include "../include/user.hpp"
#include "../include/train.hpp"

int main(){
    sjtu::users us;
    sjtu::Trains ts;

    while (true) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) continue;

        sjtu::result r = sjtu::parser(line);
        if (r.keyword == sjtu::keywords::UnKnown) continue;

        switch (r.keyword) {
        case sjtu::keywords::AddUser:
            if (us.insert(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            }
            break;

        case sjtu::keywords::Login:
            if (us.login(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            }
            break;

        case sjtu::keywords::Logout:
            if (us.logout(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            }
            break;

        case sjtu::keywords::QueryProfile: {
            std::string info = us.query_profile(r);
            if (info == "-1") {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] " << info << std::endl;
            }
            break;
        }

        case sjtu::keywords::ModifyProfile: {
            std::string info = us.modify_profile(r);
            if (info == "-1") {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] " << info << std::endl;
            }
            break;
        }


        case sjtu::keywords::AddTrain:
            if (ts.add_train(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            }
            break;

        case sjtu::keywords::DeleteTrain:
            if (ts.delete_train(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            }
            break;

        case sjtu::keywords::ReleaseTrain:
            if (ts.release_train(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << std::endl;
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << std::endl;
            }
            break;

        case sjtu::keywords::QueryTrain:
            std::cout << "[" << r.timestamp_ << "] ";
            ts.query_train(r);
            break;

        case sjtu::keywords::Clean:
            std::cout << "[" << r.timestamp_ << "] 0" << std::endl;
            break;

        case sjtu::keywords::Exit:
            std::cout << "[" << r.timestamp_ << "] bye" << std::endl;
            return 0;

        default:
            break;
        }
    }
    return 0;
}