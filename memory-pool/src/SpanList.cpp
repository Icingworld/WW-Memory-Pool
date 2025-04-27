#include "SpanList.h"

namespace WW
{

Span::Span()
    : page_id(0)
    , page_count(0)
    , prev(nullptr)
    , next(nullptr)
    , freelist(nullptr)
    , size(0)
{
}

Span::~Span()
{
}

SpanList::SpanList()
    : head(new Span())
{
    head->prev = head;
    head->next = head;
}

SpanList::~SpanList()
{
    delete head;
}

Span * SpanList::front() const noexcept
{
    return head->next;
}

Span * SpanList::back() const noexcept
{
    return head->prev;
}

void SpanList::push_front(Span * span)
{
    span->next = head->next;
    span->prev = head;
    head->next->prev = span;
    head->next = span;
}

void SpanList::push_back(Span * span)
{
    span->next = head;
    span->prev = head->prev;
    head->prev->next = span;
    head->prev = span;
}

void SpanList::pop_front()
{
    head->next = head->next->next;
    head->next->prev = head;
}

void SpanList::pop_back()
{
    head->prev = head->prev->prev;
    head->prev->next = head;
}

void SpanList::erase(Span * span)
{
    span->prev->next = span->next;
    span->next->prev = span->prev;
}

bool SpanList::empty() const noexcept
{
    return head->next == head;
}

std::mutex & SpanList::get_mutex()
{
    return mutex;
}

} // namespace WW
