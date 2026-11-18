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
    explicit Stack(int maxCapacity = 100)
        : capacity(maxCapacity), count(0), topIndex(-1), bottomIndex(0)
    {
        data = new T[capacity];
    }

    ~Stack()
    {
        delete[] data;
    }

    Stack(const Stack& other)
        : capacity(other.capacity), count(other.count),
          topIndex(other.topIndex), bottomIndex(other.bottomIndex)
    {
        data = new T[capacity];
        for (int i = 0; i < capacity; i++)
            data[i] = other.data[i];
    }

    Stack& operator=(const Stack& other)
    {
        if (this != &other)
        {
            delete[] data;
            capacity    = other.capacity;
            count       = other.count;
            topIndex    = other.topIndex;
            bottomIndex = other.bottomIndex;

            data = new T[capacity];
            for (int i = 0; i < capacity; i++)
                data[i] = other.data[i];
        }
        return *this;
    }

    void push(const T& item)
    {
        if (count == capacity)
        {
            bottomIndex = (bottomIndex + 1) % capacity;
            topIndex    = (topIndex + 1) % capacity;
            data[topIndex] = item;
        }
        else
        {
            topIndex = (topIndex + 1) % capacity;
            data[topIndex] = item;
            count++;
        }
    }

    void push(T&& item)
    {
        if (count == capacity)
        {
            bottomIndex = (bottomIndex + 1) % capacity;
            topIndex    = (topIndex + 1) % capacity;
            data[topIndex] = std::move(item);
        }
        else
        {
            topIndex = (topIndex + 1) % capacity;
            data[topIndex] = std::move(item);
            count++;
        }
    }

    void pop()
    {
        if (count == 0) return;
        topIndex = (topIndex - 1 + capacity) % capacity;
        count--;
    }

    T& top()
    {
        if (count == 0)
            throw std::runtime_error("Stack is empty");
        return data[topIndex];
    }

    const T& top() const
    {
        if (count == 0)
            throw std::runtime_error("Stack is empty");
        return data[topIndex];
    }

    bool empty() const { return count == 0; }
    int size() const   { return count; }

    void clear()
    {
        count       = 0;
        topIndex    = -1;
        bottomIndex = 0;
    }
};
