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

FreeList::FreeList()
    : head(nullptr)
{
}

FreeObject * FreeList::front() const noexcept
{
    return head;
}

void FreeList::push_front(FreeObject * free_object)
{
    free_object->next = head;
    head = free_object;
}

void FreeList::pop_front()
{
    head = head->next;
}

bool FreeList::empty() const noexcept
{
    return head == nullptr;
}

void FreeList::clear()
{
    head = nullptr;
}

} // namespace WW
