#pragma once
#include <string>
#include "utils/time.hpp"
#include "utils/fixed_string.hpp"
#include "utils/chinese_string.hpp"
#include "utils/parser.hpp"
#include "storage/bpt.hpp"
#include "utils/map/src/map.hpp"

namespace sjtu{
    inline int split_stations(const std::string& src, StationStr* out, int maxN) {
        int cnt = 0; 
        std::string cur;
        for (size_t i = 0; i <= src.length(); ++i) {
            if (i == src.length() || src[i] == '|') {
                if (cnt < maxN) out[cnt++] = StationStr(cur.c_str());
                cur.clear();
            } 
            else {
                cur += src[i];
            }
        }
        return cnt;
    }
    inline int split_ints(const std::string& src, int* out, int maxN) {
        int cnt = 0; 
        std::string cur;
        for (size_t i = 0; i <= src.length(); ++i) {
            if (i == src.length() || src[i] == '|') {
                if (cnt < maxN) out[cnt++] = std::stoi(cur);
                cur.clear();
            } 
            else { 
                cur += src[i]; 
            }
        }
        return cnt;
    }
    
    class Train{
    public:
        TrainIDStr TrainID;
        int stationNum;      
        StationStr stations[100];
        int seatNum;   
        int prices[100];                   
        int startHour, startMin;           
        int travelTimes[100];              
        int stopoverTimes[100];            
        int saleBeginMonth, saleBeginDay;  
        int saleEndMonth, saleEndDay;
        char type;                        
        bool released;
        //前缀和数组
        int arrive[101];
        int depart[101];
        int cum_price[101];
        int saleBeginIdx, saleEndIdx;

        int ticket[93][101];//第一个分量代表第几天，第二个分量代表第几站的票
        
        bool operator<(const Train& o) const { return TrainID < o.TrainID; }
        bool operator==(const Train& o) const { return TrainID == o.TrainID; }
    };
    struct StationEntry {//包装车站顺序，记录车和站的二元组
        TrainIDStr trainID;
        int stationIndex;//该站在此车次中是第几站，0站是首发站
        bool operator<(const StationEntry& o) const {
            if (trainID < o.trainID) return true;
            if (o.trainID < trainID) return false;
            return stationIndex < o.stationIndex;
        }
        bool operator==(const StationEntry& o) const {
            return trainID == o.trainID && stationIndex == o.stationIndex;
        }
    };

    class Trains{
    private:
        bpt<StationStr, StationEntry> stationIdx{"station_idx"};
    public:
        bpt<TrainIDStr, Train> trainpool{"trainpool"};  // TicketSystem 需要读车次

        // 预计算前缀和
        void pre(Train& t) {
            t.cum_price[0] = 0;
            t.depart[0] = 0;
            for (int i = 1; i < t.stationNum; ++i) {
                t.arrive[i] = t.depart[i-1] + t.travelTimes[i-1];
                t.cum_price[i] = t.cum_price[i-1] + t.prices[i-1];
                if (i < t.stationNum - 1) t.depart[i] = t.arrive[i] + t.stopoverTimes[i-1];
            }
            t.saleBeginIdx = date_to_day(t.saleBeginMonth, t.saleBeginDay);
            t.saleEndIdx = date_to_day(t.saleEndMonth, t.saleEndDay);
        }

    public:
        /*
        返回值 添加成功：0 添加失败：-1

        添加 <trainID> 为 -i，<stationNum> 为 -n，<seatNum> 为 -m，
        <stations> 为 -s，<prices> 为 -p，<startTime> 为 -x，
        <travelTimes> 为 -t，<stopoverTimes> 为 -o，<saleDate> 为 -d，<type> 为 -y 的车次。
        由于 -s、-p、-t、-o 和 -d 由多个值组成，输入时两个值之间以 | 隔开。
        输入保证火车的座位数大于 0，站的数量不少于 2 不多于 100，
        且如果火车只有两站 -o 后的参数用下划线代替，且火车不会经过同一个站两次。
        如果 <trainID> 已经存在则添加失败。
        */
        bool add_train(const result& r) {
            Train t;
            t.TrainID = TrainIDStr(r.data[7].c_str()); 
            t.stationNum = std::stoi(r.data[8]); 
            t.seatNum = std::stoi(r.data[9]); 
            split_stations(r.data[10], t.stations, 100); 
            split_ints(r.data[11], t.prices, 100);   

            //出发时间
            size_t colon = r.data[12].find(':');
            t.startHour = std::stoi(r.data[12].substr(0, colon));
            t.startMin  = std::stoi(r.data[12].substr(colon + 1));
            split_ints(r.data[13], t.travelTimes, 100); 

            //停站时间
            if (r.data[14] == "_") {
                t.stopoverTimes[0] = 0;
            } 
            else {
                split_ints(r.data[14], t.stopoverTimes, 100);
            }

            //可以买票的日期区间
            size_t pipe = r.data[15].find('|');
            std::string d1 = r.data[15].substr(0, pipe);
            std::string d2 = r.data[15].substr(pipe + 1);

            size_t dash1 = d1.find('-');
            t.saleBeginMonth = std::stoi(d1.substr(0, dash1));
            t.saleBeginDay = std::stoi(d1.substr(dash1 + 1));

            size_t dash2 = d2.find('-');
            t.saleEndMonth = std::stoi(d2.substr(0, dash2));
            t.saleEndDay = std::stoi(d2.substr(dash2 + 1));

            t.type = r.data[16][0];
            t.released = false;
            pre(t);

            // 车次已存在则失败
            auto v = trainpool.find_all(t.TrainID);
            if (!v.empty()) return false;

            //写入车次数据与站名索引
            trainpool.insert(t.TrainID, t);
            for (int i = 0; i < t.stationNum; ++i) {
                stationIdx.insert(t.stations[i], {t.TrainID, i});
            }
            return true;
        }

        /*
        返回值 删除成功：0 删除失败：-1

        删除 <trainID> 为 -i 的车次，删除车次必须保证未发布。
        */
        bool delete_train(const result& r) {
            TrainIDStr id(r.data[7].c_str());  
            auto v = trainpool.find_all(id);
            if (v.empty()) return false;  
            if (v[0].released) return false; 
            Train& t = v[0];

            //从站名索引和车次池中移除该车次所有站名
            for (int i = 0; i < t.stationNum; ++i) {
                stationIdx.remove(t.stations[i], {id, i});
            }
            trainpool.remove(id, t);
            return true;
        }

        /*
        返回值 发布成功：0 发布失败：-1

        将车次 -i (<trainID>) 发布。
        发布前的车次，不可发售车票，无法被 query_ticket 和 query_transfer 操作所查询到；
        发布后的车次不可被删除。
        */
        bool release_train(const result& r) {
            TrainIDStr id(r.data[7].c_str());

            auto v = trainpool.find_all(id);
            if (v.empty()) return false;    
            if (v[0].released) return false;  

            Train t = v[0];
            t.released = true;

            // 发布时初始化 ticket 数组 (全部座位可用)
            for (int day = t.saleBeginIdx; day <= t.saleEndIdx; ++day) {
                for (int seg = 0; seg < t.stationNum - 1; ++seg) {
                    t.ticket[day][seg] = t.seatNum;
                }
            }

            //更新车次数据
            trainpool.remove(id, v[0]);
            trainpool.insert(id, t);
            return true;
        }

        /*
        返回值 查询成功：共 stationNum+1 行  查询失败：-1

        查询在日期 -d 发车的，车次 -i (<trainID>) 的情况，-d 的格式为 mm-dd。
        第一行为 <trainID> <type>。
        接下来 stationNum 行，第 i 行为
            <stations[i]> <ARRIVING_TIME> -> <LEAVING_TIME> <PRICE> <SEAT>
        其中 ARRIVING_TIME 和 LEAVING_TIME 格式为 mm-dd hr:mi，
        PRICE 为从始发站乘坐至该站的累计票价，
        SEAT 为从该站到下一站的剩余票数。
        始发站的到达时间和终点站的出发时间用 x 代替，终点站的剩余票数用 x 代替。
        */
        void query_train(const result& r) {
            TrainIDStr id(r.data[7].c_str());   
            int m = std::stoi(r.data[15].substr(0, 2));
            int d = std::stoi(r.data[15].substr(3, 2));

            auto v = trainpool.find_all(id);
            if (v.empty()) {
                std::cout << "-1" << std::endl;
                return;
            }

            Train& t = v[0];
            int base_day = date_to_day(m, d);
            int start_min = time_to_min(t.startHour, t.startMin);
            std::cout << t.TrainID << " " << t.type << std::endl;

            for (int i = 0; i < t.stationNum; ++i) {
                std::string arr_str;
                if (i == 0) {
                    arr_str = START_TIME;
                } 
                else {
                    arr_str = abs_to_str(base_day * 1440 + start_min + t.arrive[i]);
                }
                std::string dep_str;
                if (i == t.stationNum - 1) {
                    dep_str = START_TIME;
                } 
                else {
                    dep_str = abs_to_str(base_day * 1440 + start_min + t.depart[i]);
                }
                std::string seat_str;
                if (i == t.stationNum - 1) {
                    seat_str = "x";
                } 
                else {
                    seat_str = std::to_string(t.ticket[base_day][i]);
                }
                std::cout << t.stations[i] << " " << arr_str << " -> " << dep_str << " " << t.cum_price[i] << " " << seat_str << std::endl;
            }
        }

        struct TicketCandidate {
            TrainIDStr trainID;
            int fromIdx, toIdx;//在火车车站中的编号
            int price;  
            int time;
            int seat;
            bool operator<(const TicketCandidate& o) const {
                if (time != o.time) return time < o.time;
                return trainID < o.trainID;
            }
        };

        void query_ticket(const result& r) {
            /*
            参数列表 -s -t -d (-p time)
            查询日期为 -d 时从 -s 出发，并到达 -t 的车票。请注意：这里的日期是列车从 -s 出发的日期，不是从列车始发站出发的日期。
            -p的值为 time 和 cost 中的一个，若为 time 表示输出按照该车次所需时间从小到大排序，
            否则按照票价从低到高排序。如果按照时间排序车次所需时间相同，则把 <trainID> 作为第二关键字进行排序，按照票价排序；同理若出现车次票价相同，则同样把 <trainID> 作为第二关键字进行排序。

            第一行输出一个整数，表示符合要求的车次数量。
            接下来每一行输出一个符合要求的车次，按要求排序。格式为 <trainID> <FROM> <LEAVING_TIME> -> 
            <TO> <ARRIVING_TIME> <PRICE> <SEAT>，其中出发时间、到达时间格式同 query_train，<FROM> 和 <TO> 为出发站和到达站，<PRICE> 为累计价格，<SEAT> 为最多能购买的票数。

            */
            int m = std::stoi(r.data[15].substr(0,2));
            int d = std::stoi(r.data[15].substr(3,2));
            int date = date_to_day(m,d);
            StationStr departure,destination;
            departure = StationStr(r.data[18]);
            destination = StationStr(r.data[19]);
            std::string keyword = r.data[21];
            
            sjtu::vector<StationEntry> v_departure,v_destination;
            v_departure = stationIdx.find_all(departure);
            v_destination = stationIdx.find_all(destination);
            
            sjtu::vector<TicketCandidate> candidates;
            int i = 0, j = 0;
            while(i < v_departure.size() && j < v_destination.size()) {
                if(v_departure[i].trainID < v_destination[j].trainID) i++;
                else if(v_destination[j].trainID < v_departure[i].trainID) j++;
                else {//代表有重合
                    if(v_departure[i].stationIndex < v_destination[j].stationIndex){
                        auto tv = trainpool.find_all(v_departure[i].trainID);
                        Train t = tv[0];
                        if(t.released) {
                            int start_min = time_to_min(t.startHour,t.startMin);
                            int Origin = date - (start_min + t.depart[v_departure[i].stationIndex]) / 1440;
                            if (Origin >= t.saleBeginIdx && Origin <= t.saleEndIdx) {

                            }
                        }
                    }

                }
            } 
        }
        void query_transfer(const result& r) {
            // TODO
        }
    };
};
