#include <string>
#include "utils/time.hpp"
#include "utils/fixed_string.hpp"
#include "utils/chinese_string.hpp"
#include "utils/parser.hpp"
#include "storage/bpt.hpp"
#include "utils/map/src/map.hpp"
#include "train.hpp"
#include "user.hpp"

namespace sjtu{
    // 候补队列 Key: (车次, 始发日期)
    struct PendingKey {
        TrainIDStr trainID;
        int dateDay;
        bool operator<(const PendingKey& o) const {
            if (trainID < o.trainID) return true;
            if (o.trainID < trainID) return false;
            return dateDay < o.dateDay;
        }
        bool operator==(const PendingKey& o) const {
            return trainID == o.trainID && dateDay == o.dateDay;
        }
    };

    struct Order {//订单类
        int status; //0success,1pending,2refunded
        TrainIDStr trainID;   
        StationStr fromStation; 
        StationStr toStation;  
        int fromIdx;//出发编号
        int toIdx;//到达编号
        int dateDay;  
        int num;  
        int price; 
        int timestamp;
        Order() : status(0), fromIdx(0), toIdx(0), dateDay(0), num(0), price(0), timestamp(0) {}
        bool operator<(const Order& o) const {
            return timestamp < o.timestamp;
        }
    };
    struct PendingEntry {//后补类
        int timestamp; 
        UsernameStr username;
        int fromIdx, toIdx;
        int num;

        bool operator<(const PendingEntry& o) const {
            return timestamp < o.timestamp;
        }
    };
    class TicketSystem {
    private:
        
        bpt<UsernameStr, Order> orderPool{"order_pool"};

        bpt<PendingKey, PendingEntry> pendingPool{"pending_pool"};

    public: 
        std::string buy_ticket(const result& r, Trains& ts, users& us);
        void query_order(const result& r, users& us, Trains& ts);
        void refund_ticket(const result& r, Trains& ts, users& us);

    private:
        void process_pending(Trains& ts, const TrainIDStr& tid, int dateDay, Train& t);
    };
};

