#include <thread>
#include <string>
#include <chrono>

#include <gtest/gtest.h>
#include <allocator.h>

/**
 * @brief 测试用例
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

class MemoryTest : public testing::Test
{
public:
    allocator<TestCase> alloc;
};

TEST_F(MemoryTest, SingleThreadMemoryTest)
{
    TestCase * p = alloc.allocate(1);
    EXPECT_NE(p, nullptr);

    alloc.construct(p);
    EXPECT_EQ(p->name, "test");
    EXPECT_EQ(p->id, std::this_thread::get_id());

    alloc.destroy(p);
    
    alloc.deallocate(p, 1);
}