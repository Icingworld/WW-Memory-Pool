#include <chrono>
#include <vector>
#include <thread>
#include <iostream>

#include <allocator.h>

using namespace WW;

/**
 * @brief 测试结构
 */
class TestCase
{
public:
    std::string name;
    std::thread::id id;
    std::chrono::time_point<std::chrono::system_clock> time;

public:
    TestCase()
        : TestCase("test")
    {
    }

    explicit TestCase(const std::string & name)
        : name(name)
        , id(std::this_thread::get_id())
        , time(std::chrono::system_clock::now())
    {
    }
};

constexpr size_type THREAD = 4;                 // 线程数
constexpr size_type ROUND = 100;                // 轮数
constexpr size_type SIZE = sizeof(TestCase);    // 单次操作内存大小
constexpr size_type TIMES = 1000;               // 单次测试操作次数

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;
using duration = std::chrono::duration<double, std::milli>;

int main()
{
    printf("================================== MEMORY BENCHMARK =======================================\n");
    printf("=== MALLOC TEST ===========================================================================\n");

    // malloc测试
    std::vector<std::thread> malloc_threads;
    malloc_threads.reserve(THREAD);

    time_point malloc_start = high_resolution_clock::now();

    for (size_type i = 0; i < THREAD; i++) {
        malloc_threads.emplace_back([]() {
            for (size_type j = 0; j < ROUND; j++) {
                std::vector<void *> ptrs;
                ptrs.reserve(TIMES);

                for (size_type k = 0; k < TIMES; k++) {
                    void * ptr = malloc(SIZE);
                    ptrs.emplace_back(ptr);
                }

                for (void * ptr : ptrs) {
                    free(ptr);
                }
            }
        });
    }

    for (auto & thread : malloc_threads) {
        thread.join();
    }

    time_point malloc_end = high_resolution_clock::now();
    duration malloc_duration = malloc_end - malloc_start;

    printf("%zu threads operated %zu rounds with %zu times each malloc and free, cost %.2f ms\n", THREAD, ROUND, TIMES, malloc_duration.count());
    printf("=== MEMORY POOL TEST =====================================================================\n");

    // memory-pool测试
    std::vector<std::thread> pool_threads;
    pool_threads.reserve(THREAD);

    time_point pool_start = high_resolution_clock::now();

    for (size_type i = 0; i < THREAD; ++i) {
        pool_threads.emplace_back([]() {
            allocator<TestCase> alloc;
            
            for (size_type j = 0; j < ROUND; ++j) {
                std::vector<TestCase *> ptrs;
                ptrs.reserve(TIMES);

                for (size_type k = 0; k < TIMES; ++k) {
                    TestCase * ptr = alloc.allocate(1);
                    ptrs.emplace_back(ptr);
                }

                for (size_type k = 0; k < TIMES; ++k) {
                    alloc.deallocate(ptrs[k], 1);
                }
            }
        });
    }

    for (auto & thread : pool_threads) {
        thread.join();
    }

    time_point pool_end = high_resolution_clock::now();
    duration pool_duration = pool_end - pool_start;

    printf("%zu threads operated %zu rounds with %zu times each allocate and deallocate, cost %.2f ms\n", THREAD, ROUND, TIMES, pool_duration.count());
    printf("==========================================================================================\n");
}