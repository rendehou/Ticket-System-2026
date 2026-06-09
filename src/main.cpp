#include <iostream>
#include <string>
#include "../include/user.hpp"
#include "../include/train.hpp"
#include "../include/ticket.hpp"

int main(){
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    sjtu::users us;
    sjtu::Trains ts;
    sjtu::TicketSystem tks;

    while (true) {
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) continue;

        sjtu::result r = sjtu::parser(line);
        if (r.keyword == sjtu::keywords::UnKnown) continue;

        switch (r.keyword) {
        case sjtu::keywords::AddUser:
            if (us.insert(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            }
            break;

        case sjtu::keywords::Login:
            if (us.login(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            }
            break;

        case sjtu::keywords::Logout:
            if (us.logout(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            }
            break;

        case sjtu::keywords::QueryProfile: {
            std::string info = us.query_profile(r);
            if (info == "-1") {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] " << info << "\n";
            }
            break;
        }

        case sjtu::keywords::ModifyProfile: {
            std::string info = us.modify_profile(r);
            if (info == "-1") {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] " << info << "\n";
            }
            break;
        }


        case sjtu::keywords::AddTrain:
            if (ts.add_train(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            }
            break;

        case sjtu::keywords::DeleteTrain:
            if (ts.delete_train(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            }
            break;

        case sjtu::keywords::ReleaseTrain:
            if (ts.release_train(r)) {
                std::cout << "[" << r.timestamp_ << "] 0" << "\n";
            } 
            else {
                std::cout << "[" << r.timestamp_ << "] -1" << "\n";
            }
            break;

        case sjtu::keywords::QueryTrain:
            std::cout << "[" << r.timestamp_ << "] ";
            ts.query_train(r);
            break;

        case sjtu::keywords::QueryTicket:
            std::cout << "[" << r.timestamp_ << "] ";
            ts.query_ticket(r);
            break;

        case sjtu::keywords::QueryTransfer:
            std::cout << "[" << r.timestamp_ << "] ";
            ts.query_transfer(r);
            break;

        case sjtu::keywords::BuyTicket: {
            std::string ret = tks.buy_ticket(r, ts, us);
            std::cout << "[" << r.timestamp_ << "] " << ret << "\n";
            break;
        }

        case sjtu::keywords::QueryOrder:
            std::cout << "[" << r.timestamp_ << "] ";
            tks.query_order(r, us, ts);
            break;

        case sjtu::keywords::Refund:
            std::cout << "[" << r.timestamp_ << "] ";
            tks.refund_ticket(r, ts, us);
            break;

        case sjtu::keywords::Clean:
            std::cout << "[" << r.timestamp_ << "] 0" << "\n";
            break;

        case sjtu::keywords::Exit:
            std::cout << "[" << r.timestamp_ << "] bye" << "\n";
            return 0;

        default:
            break;
        }
    }
    return 0;
}