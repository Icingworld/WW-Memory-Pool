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

Span::page_id Span::id() const noexcept
{
    return _Page_id;
}

void Span::setId(page_id page_id) noexcept
{
    _Page_id = page_id;
}

Span::page_count Span::count() const noexcept
{
    return _Page_count;
}

void Span::setCount(page_count count) noexcept
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

Span::block_count Span::used() const noexcept
{
    return _Used;
}

void Span::setUsed(block_count used) noexcept
{
    _Used = used;
}

Span::page_id Span::ptrToId(void * ptr) noexcept
{
    return reinterpret_cast<std::uintptr_t>(ptr) >> PAGE_SHIFT;
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
    : _Head(new Span())
    , _Mutex()
{
    _Head->setNext(_Head);
    _Head->setPrev(_Head);
}

SpanList::~SpanList()
{
    delete _Head;
}

Span & SpanList::front() noexcept
{
    return *_Head->next();
}

Span & SpanList::back() noexcept
{
    return *_Head->prev();
}

SpanList::iterator SpanList::begin() noexcept
{
    return iterator(_Head->next());
}

SpanList::iterator SpanList::end() noexcept
{
    return iterator(_Head);
}

void SpanList::push_front(Span * span)
{
    span->setNext(_Head->next());
    span->setPrev(_Head);
    _Head->next()->setPrev(span);
    _Head->setNext(span);
}

void SpanList::push_back(Span * span)
{
    span->setNext(_Head);
    span->setPrev(_Head->prev());
    _Head->prev()->setNext(span);
    _Head->setPrev(span);
}

void SpanList::pop_front()
{
    _Head->setNext(_Head->next()->next());
    _Head->next()->setPrev(_Head);
}

void SpanList::pop_back()
{
    _Head->setPrev(_Head->prev()->prev());
    _Head->prev()->setNext(_Head);
}

void SpanList::erase(Span * span)
{
    span->prev()->setNext(span->next());
    span->next()->setPrev(span->prev());
}

bool SpanList::empty() const noexcept
{
    return (_Head->next() == _Head);
}

std::recursive_mutex & SpanList::getMutex()
{
    return _Mutex;
}

} // namespace WW
