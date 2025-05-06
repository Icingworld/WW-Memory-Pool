#include <chrono>
#include <vector>
#include <thread>
#include <iostream>

#include <allocator.h>

using namespace WW;

struct TestCase
{
    char padding[48];
};

int main()
{
    constexpr size_type THREAD = 4;     // 线程数
    constexpr size_type SIZE = 48;     // 单次操作内存大小
    constexpr size_type TIMES = 1000;  // 单次测试操作次数

    std::vector<void *> memory_pool_pointers;
    
    memory_pool_pointers.reserve(TIMES);

    // 测试operator new
    std::vector<std::thread> threads1;
    threads1.reserve(THREAD);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < THREAD; ++i) {
        threads1.emplace_back([]() {
            std::vector<void *> operator_new_pointers;
            operator_new_pointers.reserve(TIMES);

            for (int j = 0; j < TIMES; ++j) {
                void * p = operator new(SIZE);
                operator_new_pointers.emplace_back(p);
            }

            for (int j = 0; j < TIMES; ++j) {
                operator delete(operator_new_pointers[j]);
            }
        });
    }

    for (int i = 0; i < THREAD; ++i) {
        threads1[i].join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "operator new: " << duration << "ms" << std::endl;

    // 测试allocate
    std::vector<std::thread> threads2;
    threads2.reserve(THREAD);

    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < THREAD; ++i) {
        threads2.emplace_back([]() {
            allocator<TestCase> alloc;
            std::vector<TestCase *> allocate_pointers;
            allocate_pointers.reserve(TIMES);

            for (int j = 0; j < TIMES; ++j) {
                TestCase * p = alloc.allocate(1);
                allocate_pointers.emplace_back(p);
            }

            for (int j = 0; j < TIMES; ++j) {
                alloc.deallocate(allocate_pointers[j], 1);
            }
        });
    }

    for (int i = 0; i < THREAD; ++i) {
        threads2[i].join();
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "memory pool: " << duration << "ms" << std::endl;
}