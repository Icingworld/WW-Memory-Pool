#include "SpanList.h"

namespace WW
{

Span::Span()
    : _Free_list()
    , _Page_id(0)
    , _Prev(nullptr)
    , _Next(nullptr)
    , _Page_count(0)
    , _Used(0)
{
}

size_type Span::id() const noexcept
{
    return _Page_id;
}

void Span::setId(size_type page_id) noexcept
{
    _Page_id = page_id;
}

size_type Span::count() const noexcept
{
    return _Page_count;
}

void Span::setCount(size_type count) noexcept
{
    _Page_count = count;
}

Span * Span::prev() const noexcept
{
    return _Prev;
}

void Span::setPrev(Span * prev) noexcept
{
    _Prev = prev;
}

Span * Span::next() const noexcept
{
    return _Next;
}

void Span::setNext(Span * next) noexcept
{
    _Next = next;
}

size_type Span::used() const noexcept
{
    return _Used;
}

void Span::setUsed(size_type used) noexcept
{
    _Used = used;
}

FreeList * Span::getFreeList() noexcept
{
    return &_Free_list;
}

size_type Span::ptrToId(void * ptr) noexcept
{
    return reinterpret_cast<std::uintptr_t>(ptr) >> PAGE_SHIFT;
}

void * Span::idToPtr(size_type id) noexcept
{
    return reinterpret_cast<void *>(id << PAGE_SHIFT);
}

SpanListIterator::SpanListIterator(Span * span) noexcept
    : _Span(span)
{
}

bool SpanListIterator::operator==(const SpanListIterator & other) const noexcept
{
    return _Span == other._Span;
}

bool SpanListIterator::operator!=(const SpanListIterator & other) const noexcept
{
    return _Span != other._Span;
}

Span & SpanListIterator::operator*() noexcept
{
    return *_Span;
}

Span * SpanListIterator::operator->() noexcept
{
    return _Span;
}

SpanListIterator & SpanListIterator::operator++() noexcept
{
    _Span = _Span->next();
    return *this;
}

SpanListIterator SpanListIterator::operator++(int) noexcept
{
    SpanListIterator _Tmp = *this;
    ++*this;
    return _Tmp;
}

SpanListIterator & SpanListIterator::operator--() noexcept
{
    _Span = _Span->prev();
    return *this;
}

SpanListIterator SpanListIterator::operator--(int) noexcept
{
    SpanListIterator _Tmp = *this;
    --*this;
    return _Tmp;
}

SpanList::SpanList()
    : _Head()
    , _Mutex()
{
    _Head.setNext(&_Head);
    _Head.setPrev(&_Head);
}

Span & SpanList::front() noexcept
{
    return *_Head.next();
}

Span & SpanList::back() noexcept
{
    return *_Head.prev();
}

SpanList::iterator SpanList::begin() noexcept
{
    return iterator(_Head.next());
}

SpanList::iterator SpanList::end() noexcept
{
    return iterator(&_Head);
}

void SpanList::push_front(Span * span) noexcept
{
    span->setNext(_Head.next());
    span->setPrev(&_Head);
    _Head.next()->setPrev(span);
    _Head.setNext(span);
}

void SpanList::pop_front() noexcept
{
    Span * _Front = _Head.next();
    _Head.setNext(_Front->next());
    _Front->next()->setPrev(&_Head);
}

void SpanList::erase(Span * span) noexcept
{
    span->prev()->setNext(span->next());
    span->next()->setPrev(span->prev());
}

bool SpanList::empty() const noexcept
{
    return (_Head.next() == &_Head);
}

std::recursive_mutex & SpanList::getMutex()
{
    return _Mutex;
}

} // namespace WW
