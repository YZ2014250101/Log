#include "AsyncLogging.h"
#include "LogStream.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
using namespace me;
// 总日志条数
constexpr int kTotalLogs = 3000000;
// 并发线程数
constexpr int kThreadNum = 8;

std::atomic<int> g_count { 0 };

void log_worker() {
    while (true) {
        int idx = ++g_count;
        if (idx > kTotalLogs)
            break;
        // 模拟业务日志，固定长度消息
        LOG_INFO << "perf test log message, idx=" << idx << ", hello async logging";
    }
}

int main() {
    getGlobalAsyncLogging()->setBasename("我的名字");
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> workers;
    for (int i = 0; i < kThreadNum; ++i) {
        workers.emplace_back(log_worker);
    }
    for (auto& t : workers) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    double cost_s = std::chrono::duration<double>(end - start).count();
    double qps = kTotalLogs / cost_s;

    std::cout << "Total logs: " << kTotalLogs << "\n";
    std::cout << "Time cost: " << cost_s << " s\n";
    std::cout << "QPS: " << qps << " logs/s\n";
    std::cout << "Avg per log cost: " << cost_s / kTotalLogs * 1e6 << " us\n";

    return 0;
}
