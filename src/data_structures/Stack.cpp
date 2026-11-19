#include "../../include/data_structures/Stack.h"
#include "../../include/undo/UndoManager.h"

#include <stdexcept>
#include <utility>

template <typename T>
Stack<T>::Stack(int maxCapacity)
    : capacity(maxCapacity), count(0), topIndex(-1), bottomIndex(0)
{
    data = new T[capacity];
}

template <typename T>
Stack<T>::~Stack()
{
    delete[] data;
}

template <typename T>
Stack<T>::Stack(const Stack& other)
    : capacity(other.capacity), count(other.count),
      topIndex(other.topIndex), bottomIndex(other.bottomIndex)
{
    data = new T[capacity];
    for (int i = 0; i < capacity; ++i)
        data[i] = other.data[i];
}

template <typename T>
Stack<T>& Stack<T>::operator=(const Stack& other)
{
    if (this != &other)
    {
        delete[] data;
        capacity = other.capacity;
        count = other.count;
        topIndex = other.topIndex;
        bottomIndex = other.bottomIndex;
        data = new T[capacity];
        for (int i = 0; i < capacity; ++i)
            data[i] = other.data[i];
    }
    return *this;
}

template <typename T>
void Stack<T>::push(const T& item)
{
    if (count == capacity)
    {
        bottomIndex = (bottomIndex + 1) % capacity;
        topIndex = (topIndex + 1) % capacity;
        data[topIndex] = item;
    }
    else
    {
        topIndex = (topIndex + 1) % capacity;
        data[topIndex] = item;
        ++count;
    }
}

template <typename T>
void Stack<T>::push(T&& item)
{
    if (count == capacity)
    {
        bottomIndex = (bottomIndex + 1) % capacity;
        topIndex = (topIndex + 1) % capacity;
        data[topIndex] = std::move(item);
    }
    else
    {
        topIndex = (topIndex + 1) % capacity;
        data[topIndex] = std::move(item);
        ++count;
    }
}

template <typename T>
void Stack<T>::pop()
{
    if (count == 0) return;
    topIndex = (topIndex - 1 + capacity) % capacity;
    --count;
}

template <typename T>
T& Stack<T>::top()
{
    if (count == 0)
        throw std::runtime_error("Stack is empty");
    return data[topIndex];
}

template <typename T>
const T& Stack<T>::top() const
{
    if (count == 0)
        throw std::runtime_error("Stack is empty");
    return data[topIndex];
}

template <typename T>
bool Stack<T>::empty() const
{
    return count == 0;
}

template <typename T>
int Stack<T>::size() const
{
    return count;
}

template <typename T>
void Stack<T>::clear()
{
    count = 0;
    topIndex = -1;
    bottomIndex = 0;
}

template class Stack<Action>;
