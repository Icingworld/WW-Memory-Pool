#pragma once

namespace WW
{

/**
 * @brief 空闲内存块
 */
class FreeObject
{
public:
    FreeObject * next;  // 下一个空闲内存块

public:
    FreeObject();

    explicit FreeObject(FreeObject * next);

    ~FreeObject();
};

} // namespace WW
