#pragma once
#include <string>
#include "utils/time.hpp"
#include "utils/fixed_string.hpp"
#include "utils/chinese_string.hpp"
#include "utils/parser.hpp"
#include "storage/bpt.hpp"
#include "utils/map/src/map.hpp"

namespace sjtu{
    //按天查车次的售票情况
    struct TicketKey {
        TrainIDStr trainID;
        int day;
        bool operator<(const TicketKey& o) const {
            if (trainID != o.trainID) return trainID < o.trainID;
            return day < o.day;
        }
        bool operator==(const TicketKey& o) const {
            return trainID == o.trainID && day == o.day;
        }
    };
    struct TicketDay {
        int seats[40];
        TicketDay() { 
            for(int i=0;i<40;i++) seats[i]=0; 
        }
        bool operator<(const TicketDay& o) const { return false; }
        bool operator==(const TicketDay& o) const { return true; }
    };
    //将一整句的车站读开
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
        StationStr stations[40];
        int seatNum;   
        int prices[40];                   
        int startHour, startMin;           
        int travelTimes[40];              
        int stopoverTimes[40];            
        int saleBeginMonth, saleBeginDay;  
        int saleEndMonth, saleEndDay;
        char type;                        
        bool released;
        //前缀和数组
        int arrive[41];
        int depart[41];
        int cum_price[41];
        int saleBeginIdx, saleEndIdx;
        
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
        bpt<StationStr, StationEntry, 100> stationIdx{"station_idx"};
    public:
        bpt<TrainIDStr, Train, 40> trainpool{"trainpool"};
        bpt<TicketKey, TicketDay, 150> ticketPool{"ticket_pool"};

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
            split_stations(r.data[10], t.stations, 40); 
            split_ints(r.data[11], t.prices, 40);   

            //出发时间
            size_t colon = r.data[12].find(':');
            t.startHour = std::stoi(r.data[12].substr(0, colon));
            t.startMin  = std::stoi(r.data[12].substr(colon + 1));
            split_ints(r.data[13], t.travelTimes, 40); 

            //停站时间
            if (r.data[14] == "_") {
                t.stopoverTimes[0] = 0;
            } 
            else {
                split_ints(r.data[14], t.stopoverTimes, 40);
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
            Train _tmp;
            if (trainpool.find_value(t.TrainID, _tmp)) return false;

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
            Train t;
            if (!trainpool.find_value(id, t)) return false;  
            if (t.released) return false; 

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

            Train t;
            if (!trainpool.find_value(id, t)) return false;    
            if (t.released) return false;  

            Train old_t = t; // 保存原值用于 remove
            t.released = true;

            // 发布时初始化ticket每天一个 TicketDay类票
            for (int day = t.saleBeginIdx; day <= t.saleEndIdx; ++day) {
                TicketKey tk;
                tk.trainID = id;
                tk.day = day;
                TicketDay td;
                for (int seg = 0; seg < t.stationNum - 1; ++seg) {
                    td.seats[seg] = t.seatNum;
                }
                ticketPool.insert(tk, td);
            }

            //更新车次数据
            trainpool.remove(id, old_t);
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

            //日期不在运营范围内
            if (m < 6 || m > 8) {
                std::cout << "-1" << "\n";
                return;
            }

            Train t;
            if (!trainpool.find_value(id, t)) {
                std::cout << "-1" << "\n";
                return;
            }
            int base_day = date_to_day(m, d);

            // 日期不在该车次销售区间内
            if (base_day < t.saleBeginIdx || base_day > t.saleEndIdx) {
                std::cout << "-1" << "\n";
                return;
            }

            int start_min = time_to_min(t.startHour, t.startMin);

            // 一次查询拿当天所有段票数
            TicketKey tkey; tkey.trainID = id; tkey.day = base_day;
            TicketDay td;
            bool has_td = ticketPool.find_value(tkey, td);

            std::cout << t.TrainID << " " << t.type << "\n";

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
                else if (!t.released) {
                    seat_str = std::to_string(t.seatNum);
                }
                else {
                    seat_str = has_td ? std::to_string(td.seats[i]) : std::to_string(t.seatNum);
                }
                std::cout << t.stations[i] << " " << arr_str << " -> " << dep_str << " " << t.cum_price[i] << " " << seat_str << "\n";
            }
        }

        struct TicketCandidate {//买票的所有成员
            TrainIDStr trainID;
            int fromIdx, toIdx;//在火车车站中的编号
            int price;  
            int time;
            int seat;
            StationStr fromStation, toStation;
            int depart_time, arrive_time;
            bool operator<(const TicketCandidate& o) const {
                if (time != o.time) return time < o.time;
                return trainID < o.trainID;
            }
        };
        int get_ticket(const TrainIDStr& trainID, int date, int dep_station, int des_station) {
            TicketKey tk; tk.trainID = trainID; tk.day = date;
            TicketDay td;
            if (!ticketPool.find_value(tk, td)) return 0;
            int min = 1e9;
            for (int i = dep_station; i < des_station; i++) {
                if (td.seats[i] < min) min = td.seats[i];
            }
            return min;
        }
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

            // 日期不在运营范围内 (6-8月)
            if (m < 6 || m > 8) {
                std::cout << "0" << "\n";
                return;
            }

            int date = date_to_day(m,d);
            StationStr departure,destination;
            departure = StationStr(r.data[10]);
            destination = StationStr(r.data[19]);
            std::string keyword = r.data[21];
            bool sortByTime = (keyword.empty() || keyword == "time");
            
            sjtu::vector<StationEntry> v_departure; v_departure = stationIdx.find_all(departure);
            sjtu::vector<StationEntry> v_destination; v_destination = stationIdx.find_all(destination);
            
            sjtu::vector<TicketCandidate> candidates;//所有的待选车

            //先对头尾车站来查找火车，各自返回一个vector，他们按照火车名字有序，用双指针查找重合即可
            int i = 0, j = 0;
            while(i < v_departure.size() && j < v_destination.size()) {
                if(v_departure[i].trainID < v_destination[j].trainID) i++;
                else if(v_destination[j].trainID < v_departure[i].trainID) j++;
                else {
                    int dep_id = v_departure[i].stationIndex;
                    int dest_id = v_destination[j].stationIndex;
                    if(dep_id < dest_id){
                        Train t;
                        if (!trainpool.find_value(v_departure[i].trainID, t)) continue;

                        if(t.released) {
                            int start_min = time_to_min(t.startHour,t.startMin);
                            int Origin = date - (start_min + t.depart[dep_id]) / 1440; 
                            if (Origin >= t.saleBeginIdx && Origin <= t.saleEndIdx) {
                                int remain = get_ticket(t.TrainID,Origin,dep_id,dest_id);
                                {
                                    //存入与买票相关的类里
                                    TicketCandidate c;
                                    c.trainID = t.TrainID;
                                    c.fromIdx = dep_id;
                                    c.toIdx = dest_id;
                                    c.price = t.cum_price[dest_id] - t.cum_price[dep_id];
                                    c.time = t.arrive[dest_id] - t.depart[dep_id];
                                    c.seat = remain;
                                    c.fromStation = t.stations[dep_id];
                                    c.toStation = t.stations[dest_id];
                                    c.depart_time = Origin * 1440 + start_min + t.depart[dep_id];
                                    c.arrive_time = Origin * 1440 + start_min + t.arrive[dest_id];
                                    candidates.push_back(c);
                                }
                            }
                        }
                    }
                    i++;
                    j++;
                }
            }
            //插入排序
            if(sortByTime){
                for (int a = 1; a < candidates.size(); ++a) {
                    TicketCandidate key = candidates[a];
                    int b = a - 1;
                    while (b >= 0 && (candidates[b].time > key.time ||
                           (candidates[b].time == key.time && candidates[b].trainID > key.trainID))) {
                        candidates[b + 1] = candidates[b];
                        b--;
                    }
                    candidates[b + 1] = key;
                }
            }else {
                for (int a = 1; a < candidates.size(); ++a) {
                    TicketCandidate key = candidates[a];
                    int b = a - 1;
                    while (b >= 0 && (candidates[b].price > key.price ||
                           (candidates[b].price == key.price && candidates[b].trainID > key.trainID))) {
                        candidates[b + 1] = candidates[b];
                        b--;
                    }
                    candidates[b + 1] = key;
                }
            } 
            std::cout << candidates.size() << "\n";

            for (int a = 0; a < candidates.size(); ++a) {
                TicketCandidate& c = candidates[a];
                //输出
                std::cout
                    << c.trainID << " "
                    << c.fromStation << " "
                    << abs_to_str(c.depart_time) << " -> "
                    << c.toStation << " "
                    << abs_to_str(c.arrive_time) << " "
                    << c.price << " "
                    << c.seat << "\n";
            }
        }

        struct transfer_result{
            TrainIDStr train1,train2;
            int fromID1,midID1,midID2,toID2;//站编号
            int start_time1,start_time2;
            int time,cost;
        };

        void query_transfer(const result& r) {
            /*
            在恰好换乘一次（换乘同一辆车不算恰好换乘一次）的情况下查询符合条件的车次，仅输出最优解。
            最优解的定义如下:
            * 若`(-p time)` 则总时间作为第一关键字，总价格作为第二关键字，第一辆车的 `Train ID` 作为第三关键字，第二辆车 `Train ID` 作为第四关键字。
            * 若`(-p cost)` 则总价格作为第一关键字，总时间作为第二关键字，第一辆车的 `Train ID` 作为第三关键字，第二辆车 `Train ID` 作为第四关键字。
            保证任意两种方案关键字均不同。
            请注意：这里的日期是列车从 `-s` 出发的日期，不是从列车始发站出发的日期。
        - 返回值
            查询失败（没有符合要求的车次）：`0`

            查询成功：输出2行，换乘中搭乘的两个车次，格式同 `query_ticket`。
            */
            //读入并切分输入信息
            int m = std::stoi(r.data[15].substr(0,2));
            int d = std::stoi(r.data[15].substr(3,2));

            //日期不在运营范围内
            if (m < 6 || m > 8) {
                std::cout << "0" << "\n";
                return;
            }

            int date = date_to_day(m,d);
            StationStr from = StationStr(r.data[10]);
            StationStr to = StationStr(r.data[19]);   
            bool flag = (r.data[21].empty() || r.data[21] == "time");

            transfer_result best;
            bool valid = 0;

            sjtu::vector<StationEntry> v_departure; v_departure = stationIdx.find_all(from);
            sjtu::vector<StationEntry> v_destination; v_destination = stationIdx.find_all(to);

            for(int i = 0;i < v_departure.size();i++){
                //读取基本信息
                TrainIDStr tid1 = v_departure[i].trainID;
                int start_station = v_departure[i].stationIndex;
                auto tv = trainpool.find_all(tid1);
                if(!tv[0].released) continue;
                
                //判定时间
                Train& t1 = tv[0];
                int start_min1 = time_to_min(t1.startHour,t1.startMin);
                int Origin1 = date - (start_min1 + t1.depart[start_station]) / 1440;
                if (Origin1 < t1.saleBeginIdx || Origin1 > t1.saleEndIdx) continue;

                //转换时间
                for(int j = start_station + 1;j < tv[0].stationNum;j++){
                    int arrive_time = Origin1 * 1440 + start_min1 + t1.arrive[j];
                    StationStr transfer = tv[0].stations[j];
                    auto mid_list = stationIdx.find_all(transfer);
                    
                    //思路类似ticket，用双指针遍历
                    for(int p = 0; p < mid_list.size(); p++) {
                        for(int q = 0; q < v_destination.size(); q++) {
                            if(!(mid_list[p].trainID == v_destination[q].trainID)) continue;
                            TrainIDStr tid2 = v_destination[q].trainID;
                            int midID2 = mid_list[p].stationIndex;
                            int destID2 = v_destination[q].stationIndex;

                            if(tid1 != tid2 && midID2 < destID2){
                                auto tv2 = trainpool.find_all(tid2);
                                Train& t2 = tv2[0];

                                if(t2 .released){
                                    //计算时间
                                    int start_min2 = time_to_min(t2.startHour, t2.startMin);
                                    int need = arrive_time - start_min2 - t2.depart[midID2];
                                    int Origin2 = (need + 1439) / 1440;
                                    if (Origin2 < 0) Origin2 = 0;

                                    //若Origin2早于销售期，从销售首日开始
                                    if (Origin2 < t2.saleBeginIdx) Origin2 = t2.saleBeginIdx;

                                    if (Origin2 >= t2.saleBeginIdx && Origin2 <= t2.saleEndIdx) {
                                        int departM_T2 = Origin2 * 1440 + start_min2 + t2.depart[midID2];
                                        if (departM_T2 < arrive_time) {
                                            Origin2++;
                                            if (Origin2 > t2.saleEndIdx) { 
                                                continue; 
                                            }
                                            departM_T2 = Origin2 * 1440 + start_min2 + t2.depart[midID2];
                                            if (departM_T2 < arrive_time) { 
                                                continue; 
                                            }
                                        }
                                        //计算价格
                                        int remain1,remain2;
                                        remain1 = get_ticket(t1.TrainID,Origin1,start_station,j);
                                        remain2 = get_ticket(t2.TrainID,Origin2,midID2,destID2);

                                        if(remain1 != 0 && remain2 != 0){
                                            int departFrom = Origin1 * 1440 + start_min1 + t1.depart[start_station];
                                            int arriveTo = Origin2 * 1440 + start_min2 + t2.arrive[destID2];
                                            int totalTime = arriveTo - departFrom;
                                            int totalPrice = (t1.cum_price[j] - t1.cum_price[start_station]) + (t2.cum_price[destID2] - t2.cum_price[midID2]);

                                            //结果类
                                            transfer_result cur;
                                            cur.train1 = tid1, cur.train2 = tid2;
                                            cur.fromID1 = start_station, cur.midID1 = j;
                                            cur.midID2 = midID2, cur.toID2 = destID2;
                                            cur.start_time1 = Origin1, cur.start_time2 = Origin2;
                                            cur.time  = totalTime;
                                            cur.cost  = totalPrice;

                                            bool better = 0;
                                            if (!valid) {
                                                better = true;
                                            } 
                                            else if (flag) {//以time划分
                                                if (cur.time != best.time) better = (cur.time < best.time);
                                                else if (cur.cost != best.cost) better = (cur.cost < best.cost);
                                                else if (cur.train1 != best.train1) better = (cur.train1 < best.train1);
                                                else better = (cur.train2 < best.train2);
                                            } 
                                            else { //以cost划分
                                                if (cur.cost != best.cost) better = (cur.cost < best.cost);
                                                else if (cur.time != best.time) better = (cur.time < best.time);
                                                else if (cur.train1 != best.train1) better = (cur.train1 < best.train1);
                                                else better = (cur.train2 < best.train2);
                                            }
                                            if(better){
                                                best = cur;
                                                valid = 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } 
            if (!valid) {
                std::cout << "0" << "\n";
                return;
            }


            //输出第一辆车
            auto tv1 = trainpool.find_all(best.train1);
            Train& t1 = tv1[0];
            int sm1 = time_to_min(t1.startHour, t1.startMin);
            int dep1 = best.start_time1 * 1440 + sm1 + t1.depart[best.fromID1];
            int arr1 = best.start_time1 * 1440 + sm1 + t1.arrive[best.midID1];
            int price1 = t1.cum_price[best.midID1] - t1.cum_price[best.fromID1];
            int seat1 = get_ticket(t1.TrainID, best.start_time1, best.fromID1, best.midID1);

            std::cout << t1.TrainID << " " << t1.stations[best.fromID1] << " "
                    << abs_to_str(dep1) << " -> " << t1.stations[best.midID1] << " "
                    << abs_to_str(arr1) << " " << price1 << " " << seat1 << "\n";

            //输出第二辆车
            auto tv2 = trainpool.find_all(best.train2);
            Train& t2 = tv2[0];
            int sm2 = time_to_min(t2.startHour, t2.startMin);
            int dep2 = best.start_time2 * 1440 + sm2 + t2.depart[best.midID2];
            int arr2 = best.start_time2 * 1440 + sm2 + t2.arrive[best.toID2];
            int price2 = t2.cum_price[best.toID2] - t2.cum_price[best.midID2];
            int seat2 = get_ticket(t2.TrainID, best.start_time2, best.midID2, best.toID2);

            std::cout << t2.TrainID << " " << t2.stations[best.midID2] << " "
                    << abs_to_str(dep2) << " -> " << t2.stations[best.toID2] << " "
                    << abs_to_str(arr2) << " " << price2 << " " << seat2 << "\n";
        }
    };
};
