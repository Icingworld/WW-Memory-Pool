#include <vector>
#include <thread>

#include <allocator.h>

using namespace WW;

constexpr size_type THREAD = 2;                 // 线程数
constexpr size_type ROUND = 1000;               // 轮数
constexpr size_type TIMES = 1000;               // 单次测试操作次数

/**
 * @brief 测试结构
 */
template <size_type Size>
class TestCase
{
public:
    char padding[Size];
};

int main()
{
    std::vector<std::thread> pool_threads;
    pool_threads.reserve(THREAD);

    // 512bytes
    pool_threads.emplace_back([]() {
        allocator<TestCase<512>> alloc;

        for (size_type j = 0; j < ROUND; ++j) {
            std::vector<TestCase<512> *> ptrs;
            ptrs.reserve(TIMES);

            for (size_type k = 0; k < TIMES; ++k) {
                TestCase<512> * ptr = alloc.allocate(1);
                ptrs.emplace_back(ptr);
            }

            for (size_type k = 0; k < TIMES; ++k) {
                alloc.deallocate(ptrs[k], 1);
            }
        }
    });

    // 1024bytes
    pool_threads.emplace_back([]() {
        allocator<TestCase<1024>> alloc;        

        for (size_type j = 0; j < ROUND; ++j) {
            std::vector<TestCase<1024> *> ptrs;
            ptrs.reserve(TIMES);

            for (size_type k = 0; k < TIMES; ++k) {
                TestCase<1024> * ptr = alloc.allocate(1);
                ptrs.emplace_back(ptr);
            }

            for (size_type k = 0; k < TIMES; ++k) {
                alloc.deallocate(ptrs[k], 1);
            }
        }
    });
    
    for (std::thread & thread : pool_threads) {
        thread.join();
    }
}
