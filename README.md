# 高性能异步日志库
> C++17 高性能异步日志库，模仿 Muduo 异步日志核心思想，双缓冲+多缓冲队列后台落盘，用户仅需一行宏即可输出日志，无需手动创建/管理日志实例。

## ✨ 特性
1. **极简使用**：只需要 `LOG_INFO << "hello world";`，无需手动构造 AsyncLogging 对象，内部使用 Magic Static 懒初始化，进程全局单例。
2. **异步写盘**：业务线程只把日志写入内存缓冲区，不阻塞磁盘IO；单独后台线程负责批量落盘，极大降低业务线程耗时。
3. **多缓冲池设计**：`BufferVector` 缓冲池，大缓冲区复用，减少频繁 `new/delete`，降低内存分配开销。
4. **日志保护**：日志洪峰过载时自动丢弃多余buffer并打印丢日志告警，防止内存暴涨OOM。
5. **可配置**：支持设置日志文件名、日志滚动大小、刷新间隔。
6. **基础日志级别**：`TRACE / DEBUG / INFO / WARN / ERROR / FATAL`。
7. **日志元信息**：自动附带日志级别、源码文件名、行号、函数名、时间戳。
8. **底层可选**：支持原生 `fd` 系统调用写文件（也可切换 FILE*），无C标准库缓冲封装，更贴近内核IO。

> 设计思想：生产者（业务线程）填充buffer；消费者（后台日志线程）批量把buffer写入磁盘，使用条件变量做超时唤醒。

## 📦 目录结构
```
me/
├── base/
│   ├── LogStream.h        // 流式缓冲区，重载operator<<，把数字/字符串序列化到buffer
│   ├── AsyncLogging.h     // 异步日志核心类，后台线程、缓冲队列
│   ├── AsyncLogging.cpp
│   ├── Logger.h           // Logger临时对象 + LOG_XXX宏定义
│   ├── Timestamp.h        // 时间戳格式化
│   ├── noncopyable.h      // 禁止拷贝基类
│   └── detail/
│       └── FormatBuffer.h // 固定大小内存缓冲区实现
└── base/test/
    └── testLog.cpp        // 单元测试demo
```

## 🚀 快速上手
### 1. 引入头文件
```cpp
#include "me/base/Logger.h"
int main()
{
    LOG_INFO << "hello async log";
    LOG_WARN << "warning test, value=" << 12345;
    LOG_ERROR << "error message";
    return 0;
}
```

### 2. cmake编译运行
```bash
mkdir build && cd build
cmake ..
make
./testLog
```

## 🧠 核心原理
### 1. 日志流转链路
`LOG_INFO宏` → 创建临时`LogStream`对象 → 把数据写入内存buffer → **LogStream析构**时，将buffer提交给全局`AsyncLogging`的缓冲队列。
- 业务线程：只做内存拷贝，**不等待磁盘写**
- 后台线程：等待buffer或者超时，一次性批量写盘

### 2. 缓冲模型（Muduo风格多缓冲）
- `currentBuffer_`：业务线程正在写的主buffer
- `buffers_`：已经写满、待交给后台线程的buffer列表
- `buffersToWrite`：后台线程拿到的buffer列表，循环批量写入文件
- 写完后保留空闲buffer复用，减少内存分配开销；过载时自动丢弃多余日志，保护进程。

### 3. 懒初始化（Magic Static C++17）
```cpp
AsyncLogging* getGlobalAsyncLogger()
{
    static AsyncLogging g_asyncLogging("app.log", 100*1024*1024, 3);
    return &g_asyncLogging;
}
```
- 第一次调用才构造实例，自动启动后台线程
- C++17保证静态局部变量初始化线程安全
- 程序退出时自动析构，后台线程安全退出，刷完剩余日志

## ⚙️ 配置参数说明
```cpp
AsyncLogging(const std::string& basename, off_t rollSize, int flushInterval = 3);
```
- `basename`：日志文件名前缀
- `rollSize`：文件滚动阈值，超过该大小生成新日志文件
- `flushInterval`：超时刷新时间（单位秒），即使没写满buffer，到时间也要落盘

## ⚠️ 注意事项 & 坑点
1. **宏悬垂else问题**：原版muduo LOG宏存在if+else二义性，本项目宏已规避。
2. **线程安全**：日志库内部自带锁，**多业务线程直接调用LOG_INFO无需额外加锁**。
3. **洪峰保护**：当待写buffer数量超过阈值，会丢弃多余日志并输出stderr告警，防止内存持续上涨OOM。
4. **fd版本特性**：使用原生`write(fd)`系统调用写盘，没有stdio的用户态buffer；可自行增加`fsync`强制刷到物理磁盘（注意fsync性能开销很大）。
5. **程序退出**：析构时会停止后台线程，把剩余buffer全部写入磁盘，避免丢日志。

## 📌 待实现功能（可选扩展）
- [x] 日志写满自动滚动
- [ ] 日志路径自定义
- [ ] 日志文件名自定义
- [ ] 日志文件滚动时间自定义
- [ ] 日志时间戳精度自定义
- [ ] 日志文件自动清理旧日志
- [ ] `fsync` 可选强制落盘开关
- [ ] 日志异步落地到网络（udp/tcp远程日志）
- [ ] 日志内容压缩
- [ ] 更多类型重载：自定义对象序列化

## 📝 License
BSD-style License（同muduo）

---
