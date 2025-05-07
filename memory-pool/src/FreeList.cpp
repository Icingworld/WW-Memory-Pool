#include "FreeList.h"

namespace WW
{

FreeObject::FreeObject()
    : _Next(nullptr)
{
}

FreeObject::FreeObject(FreeObject * next)
    : _Next(next)
{
}

FreeObject * FreeObject::next() const noexcept
{
    return _Next;
}

void FreeObject::setNext(FreeObject * next) noexcept
{
    _Next = next;
}

FreeListIterator::FreeListIterator(FreeObject * free_object) noexcept
    : _Free_object(free_object)
{
}

bool FreeListIterator::operator==(const FreeListIterator & other) const noexcept
{
    return _Free_object == other._Free_object;
}

bool FreeListIterator::operator!=(const FreeListIterator & other) const noexcept
{
    return _Free_object != other._Free_object;
}

FreeObject * FreeListIterator::operator*() noexcept
{
    return _Free_object;
}

FreeObject * FreeListIterator::operator->() noexcept
{
    return _Free_object;
}

FreeListIterator & FreeListIterator::operator++() noexcept
{
    _Free_object = _Free_object->next();
    return *this;
}

FreeListIterator FreeListIterator::operator++(int) noexcept
{
    FreeListIterator _Tmp = *this;
    ++*this;
    return _Tmp;
}

FreeList::FreeList()
    : _Head()
    , _Size(0)
    , _Max_size(1)
{
}

FreeObject * FreeList::front() noexcept
{
    return _Head.next();
}

void FreeList::push_front(FreeObject * free_object)
{
    free_object->setNext(_Head.next());
    _Head.setNext(free_object);
    ++_Size;
}

void FreeList::pop_front()
{
    _Head.setNext(_Head.next()->next());
    --_Size;
}

FreeList::iterator FreeList::begin() noexcept
{
    return iterator(_Head.next());
}

FreeList::iterator FreeList::end() noexcept
{
    return iterator(nullptr);
}

bool FreeList::empty() const noexcept
{
    return (_Head.next() == nullptr);
}

size_type FreeList::size() const noexcept
{
    return _Size;
}

size_type FreeList::max_size() const noexcept
{
    return _Max_size;
}

void FreeList::setMax(size_type size) noexcept
{
    _Max_size = size;
}

} // namespace WW
