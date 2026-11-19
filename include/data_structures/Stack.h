#pragma once

#include <stdexcept>
#include <utility>

// ============================================================================
// Class: Stack<T>
// A circular ring buffer stack implemented using a dynamic array.
// Fixed capacity bounds memory consumption and prevents heap fragmentation.
// ============================================================================
template <typename T>
class Stack
{
private:
    T* data;
    int capacity;
    int count;
    int topIndex;
    int bottomIndex;

public:
    explicit Stack(int maxCapacity = 100);
    ~Stack();
    Stack(const Stack& other);
    Stack& operator=(const Stack& other);

    void push(const T& item);
    void push(T&& item);
    void pop();
    T& top();
    const T& top() const;
    bool empty() const;
    int size() const;
    void clear();
};
