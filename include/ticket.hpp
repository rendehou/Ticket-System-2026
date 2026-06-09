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
    //候补队列二元组，火车名和日期
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
        bool operator==(const Order& o) const {
            return timestamp == o.timestamp && status == o.status;
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
        bool operator==(const PendingEntry& o) const {
            return timestamp == o.timestamp;
        }
    };
    class TicketSystem {
    private:
        
        bpt<UsernameStr, Order, 100> orderPool{"order_pool"};

        bpt<PendingKey, PendingEntry> pendingPool{"pending_pool"};

    public: 

        std::string buy_ticket(const result& r, Trains& ts, users& us) {

            UsernameStr username(r.data[2].c_str());
            TrainIDStr trainID(r.data[7].c_str());
            StationStr fromSta(r.data[18].c_str());
            StationStr toSta(r.data[19].c_str());
            int num = std::stoi(r.data[17]);
            int m = std::stoi(r.data[15].substr(0, 2));
            int d = std::stoi(r.data[15].substr(3, 2));
            int date = date_to_day(m, d);
            bool flag = (r.data[20] == "true");

            //检查购票者和车次是否存在
            if (!us.is_online(username)) return "-1";
            Train t;
            if (!ts.trainpool.find_value(trainID, t)) return "-1";
            if (!t.released) return "-1";

            //检查车次信息
            int fromIdx = -1, toIdx = -1;
            for (int i = 0; i < t.stationNum; i++) {
                if (t.stations[i] == fromSta) fromIdx = i;
                if (t.stations[i] == toSta)   toIdx   = i;
            }
            if (fromIdx == -1 || toIdx == -1 || fromIdx >= toIdx) return "-1";

            //计算时间和票价
            int startMin = time_to_min(t.startHour, t.startMin);
            int Origin = date - (startMin + t.depart[fromIdx]) / 1440;
            if (Origin < t.saleBeginIdx || Origin > t.saleEndIdx) return "-1";
            // 购票数不能超过总座位数
            if (num > t.seatNum) return "-1";
            int unitPrice = t.cum_price[toIdx] - t.cum_price[fromIdx];
            int totalPrice = unitPrice * num;

            // 一次查询当天所有段
            TicketKey tk; tk.trainID = trainID; tk.day = Origin;
            TicketDay td;
            bool enough = ts.ticketPool.find_value(tk, td);
            if (enough) {
                for (int i = fromIdx; i < toIdx; i++) {
                    if (td.seats[i] < num) { enough = false; break; }
                }
            }

            if (enough) {
                TicketDay old_td = td;
                for (int i = fromIdx; i < toIdx; i++) td.seats[i] -= num;
                ts.ticketPool.remove(tk, old_td);
                ts.ticketPool.insert(tk, td);

                Order o;
                o.status = 0;
                o.trainID  = trainID;
                o.fromStation = fromSta;
                o.toStation = toSta;
                o.fromIdx = fromIdx;
                o.toIdx = toIdx;
                o.dateDay = Origin;
                o.num = num;
                o.price = totalPrice;
                o.timestamp = r.timestamp_;

                orderPool.insert(username, o);//插入购票记录
                return std::to_string(totalPrice);
            }

            if (flag) {//可以进入队列后补
                Order o;
                o.status = 1;
                o.trainID = trainID;
                o.fromStation = fromSta;
                o.toStation = toSta;
                o.fromIdx = fromIdx;
                o.toIdx = toIdx;
                o.dateDay = Origin;
                o.num = num;
                o.price = totalPrice;
                o.timestamp = r.timestamp_;

                orderPool.insert(username, o);

                PendingEntry pe;
                pe.timestamp = r.timestamp_;
                pe.username = username;
                pe.fromIdx = fromIdx;
                pe.toIdx = toIdx;
                pe.num = num;
                PendingKey key;
                key.trainID = trainID;
                key.dateDay = Origin;

                pendingPool.insert(key, pe);
                return "queue";
            }

            return "-1";
        }

        /*
        返回值: 第一行订单数量, 后续每行 [STATUS] trainID FROM LEAVING -> TO ARRIVING PRICE NUM
        失败: -1
        */
        void query_order(const result& r, users& us, Trains& ts) {
            UsernameStr username(r.data[2].c_str());

            //检查登录
            if (!us.is_online(username)) {
                std::cout << "-1" << "\n";
                return;
            }

            auto orders = orderPool.find_all(username);

            //输出
            std::cout << orders.size() << "\n";
            for (int i = orders.size() - 1; i >= 0; i--) {
                Order& o = orders[i];


                const char* statusStr;
                if (o.status == 0) statusStr = "success";
                else if (o.status == 1) statusStr = "pending";
                else statusStr = "refunded";

                auto tv = ts.trainpool.find_all(o.trainID);
                if (tv.empty()) continue;
                Train& t = tv[0];
                int startMin = time_to_min(t.startHour, t.startMin);

                int departAbs = o.dateDay * 1440 + startMin + t.depart[o.fromIdx];
                int arriveAbs = o.dateDay * 1440 + startMin + t.arrive[o.toIdx];
                std::cout << "[" << statusStr << "] "
                          << o.trainID << " "
                          << o.fromStation << " "
                          << abs_to_str(departAbs) << " -> "
                          << o.toStation << " "
                          << abs_to_str(arriveAbs) << " "
                          << o.price / o.num << " "
                          << o.num << "\n";
            }
        }

        /*
        返回值: 退票成功 0, 失败 -1
        */
        void refund_ticket(const result& r, Trains& ts, users& us) {
            UsernameStr username(r.data[2].c_str());

            //检查登录
            if (!us.is_online(username)) {
                std::cout << "-1" << "\n";
                return;
            }

            int n = 1;
            if (!r.data[22].empty()) n = std::stoi(r.data[22]);
            auto orders = orderPool.find_all(username);

            //倒序找第n个订单
            int targetIdx = -1;
            for (int i = orders.size() - 1, cnt = 1; i >= 0; i--, cnt++) {
                if (cnt == n) {
                    targetIdx = i;
                    break;
                }
            }

            if (targetIdx == -1 || orders[targetIdx].status == 2) {
                std::cout << "-1" << "\n";
                return;
            }

            Order target = orders[targetIdx];

            //对火车释放座位
            auto tv = ts.trainpool.find_all(target.trainID);
            if (tv.empty()) {
                std::cout << "-1" << "\n";
                return;
            }
            //success订单才释放座位
            if (target.status == 0) {
                TicketKey tk; tk.trainID = target.trainID; tk.day = target.dateDay;
                auto tdVec = ts.ticketPool.find_all(tk);
                if (!tdVec.empty()) {
                    TicketDay td = tdVec[0];
                    for (int i = target.fromIdx; i < target.toIdx; i++) td.seats[i] += target.num;
                    ts.ticketPool.remove(tk, tdVec[0]);
                    ts.ticketPool.insert(tk, td);
                }
            }
            Order refunded = target;
            refunded.status = 2;
            orderPool.remove(username, target);
            orderPool.insert(username, refunded);

            //如果是pending订单，从候补队列中删除
            if (target.status == 1) {
                PendingKey pk;
                pk.trainID = target.trainID;
                pk.dateDay = target.dateDay;
                PendingEntry pe;
                pe.timestamp = target.timestamp;
                pe.username = username;
                pe.fromIdx = target.fromIdx;
                pe.toIdx = target.toIdx;
                pe.num = target.num;
                pendingPool.remove(pk, pe);
            } else {
                //只有release了座位才处理候补队列
                process_pending(ts, target.trainID, target.dateDay);
            }

            std::cout << "0" << "\n";
        }

    private:
        /*
        退票后处理候补队列:
        遍历 (trainID, dateDay) 的候补列表, 按下单时间顺序尝试满足
        能完全满足的就兑现 (改订单状态 + 扣票 + 删除候补)
        不能满足的跳过, 继续下一个
        */
        void process_pending(Trains& ts, const TrainIDStr& tid, int dateDay) {
            PendingKey key;
            key.trainID = tid;
            key.dateDay = dateDay;

            auto pendingList = pendingPool.find_all(key);
            if (pendingList.empty()) return;

            //按时间戳插入排序
            for (int a = 1; a < pendingList.size(); ++a) {
                PendingEntry key = pendingList[a];
                int b = a - 1;
                while (b >= 0 && pendingList[b].timestamp > key.timestamp) {
                    pendingList[b + 1] = pendingList[b];
                    b--;
                }
                pendingList[b + 1] = key;
            }

            //一次查询当天所有段
            TicketKey pkey; pkey.trainID = tid; pkey.day = dateDay;
            auto tdVec = ts.ticketPool.find_all(pkey);
            if (tdVec.empty()) return;
            TicketDay td = tdVec[0];
            bool modified = false;

            for (int idx = 0; idx < pendingList.size(); idx++) {
                PendingEntry& pe = pendingList[idx];

                //检查余票是否足够
                bool enough = true;
                for (int i = pe.fromIdx; i < pe.toIdx; i++) {
                    if (td.seats[i] < pe.num) { enough = false; break; }
                }
                if (!enough) continue;
                for (int i = pe.fromIdx; i < pe.toIdx; i++) td.seats[i] -= pe.num;
                modified = true;

                // 从候补队列删除
                pendingPool.remove(key, pe);

                //更新订单状态
                auto userOrders = orderPool.find_all(pe.username);
                for (int j = 0; j < userOrders.size(); j++) {
                    if (userOrders[j].timestamp == pe.timestamp
                        && userOrders[j].status == 1) {
                        Order full = userOrders[j];
                        full.status = 0;
                        orderPool.remove(pe.username, userOrders[j]);
                        orderPool.insert(pe.username, full);
                        break;
                    }
                }
            }
            // 写回修改后的 TicketDay
            if (modified) {
                ts.ticketPool.remove(pkey, tdVec[0]);
                ts.ticketPool.insert(pkey, td);
            }
        }
    };
};

