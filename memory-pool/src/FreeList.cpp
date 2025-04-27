#include "FreeList.h"

namespace WW
{

FreeObject::FreeObject()
    : next(nullptr)
{
}

FreeObject::FreeObject(FreeObject * next)
    : next(next)
{
}

FreeObject::~FreeObject()
{
}

} // namespace WW
