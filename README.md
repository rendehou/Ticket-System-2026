

## 仓库结构

```
├── CMakeLists.txt
├── .gitignore
├── include
│   ├── ticket.hpp
│   ├── train.hpp
│   ├── user.hpp
│   ├── storage
│   │   └── bpt.hpp
│   └── utils
│       ├── chinese_string.hpp
│       ├── fixed_string.hpp
│       ├── parser.hpp
│       ├── string_check.hpp
│       ├── time.hpp
│       ├── vector
│       │   ├── vector.hpp
│       │   ├── exceptions.hpp
│       │   └── utility.hpp
│       └── map
│           └── src
│               ├── map.hpp
│               ├── exceptions.hpp
│               └── utility.hpp
└── src
    └── main.cpp
```

## 文件概述

`storage/bpt.hpp` 实现了一个外存中的 B+ 树模板类 `bpt<Key, Value, N>`，基于 `MemoryRiver` 以二进制形式在磁盘上存取任意类型记录，将 Key 映射到文件中的记录偏移，支持 `insert`、`remove`、`find_all`、`find_value`、`find_values_batch` 等操作，内部实现了节点分裂、合并及兄弟间借数据。

`utils/vector/vector.hpp`、`utils/map/src/map.hpp` 实现了与 STL 接近的 `vector`、`map` 的功能。

`utils/fixed_string.hpp` 实现了定长字符串 `FixedString<N>`，在栈上存储避免堆分配，派生 `UsernameStr`、`PasswordStr`、`MailStr`、`TrainIDStr`。`utils/chinese_string.hpp` 实现了 UTF-8 中文字符串 `ChineseString<MaxChars>`，派生 `NameStr`（≤5 汉字）、`StationStr`（≤10 汉字）。

`utils/time.hpp` 实现了日期（2026 年 6~8 月）与时刻的转换函数，如 `date_to_day`、`day_to_date`、`time_to_min`、`min_to_str`、`abs_to_str` 等。

`utils/parser.hpp` 实现了一个命令解析器，将输入命令按 `-<key> <argument>` 格式拆分并存入 `result` 结构体，便于指令与各模块的交互。`utils/string_check.hpp` 实现了各字段的合法性校验（username、password、name、mail、trainID、station 等）。

`user.hpp` 实现了 `User` 类与 `users` 用户管理类，支持添加用户、登录、登出、查询用户信息、修改用户信息等操作。

`train.hpp` 实现了 `Train` 类（内部维护到达/出发时间与累计票价的前缀和）与 `Trains` 火车票管理类，支持添加火车、删除火车、发布火车、查询车次、查询车票、查询换乘等功能。

`ticket.hpp` 实现了 `Order` 订单类（状态：成功/候补/已退）、`PendingEntry` 候补订单类与 `TicketSystem` 购票类，支持购票（含候补）、查询订单、退票等功能，退票后自动触发 `process_pending` 按序兑现候补。

## 数据库设计

##### 核心思想：

用 `MemoryRiver` 储存信息，将信息在文件中的索引与某个 key 绑定存入 B+ 树。查询时通过 B+ 树先查到索引，再从文件索引位置读出信息。

##### users (userpool):

`//key为username，value为索引的B+树`
`bpt<UsernameStr, User> userpool{"userpool"}; `

##### Trains (trainpool + ticket_pool + station_idx):

`//key为trainID，value为索引的B+树`
`bpt<TrainIDStr, Train, 40> trainpool{"trainpool"}; `

`//key为station，value为列车索引的B+树`
`bpt<StationStr, StationEntry, 100> stationIdx{"station_idx"}; //记录经过station的车的索引`

`//key为(trainID, day)，value为当日各区段余票`
`bpt<TicketKey, TicketDay, 150> ticketPool{"ticket_pool"}; //存放余票信息`

##### TicketSystem (order_pool + pending_pool):

`//key为username，value为订单索引的B+树`
`bpt<UsernameStr, Order, 100> orderPool{"order_pool"};`

`//存放订单信息，每个Order在文件中的位置与其username作为键值对插入到BPT中`

`//key为trainID+发售天数，value为候补订单索引的B+树`
`bpt<PendingKey, PendingEntry> pendingPool{"pending_pool"};`

`//存放候补订单信息，每个PendingEntry在文件中的位置与其购买的车次+天数作为键值对插入到BPT中`


  
