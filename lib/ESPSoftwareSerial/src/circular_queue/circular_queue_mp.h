/*
circular_queue_mp.h - Implementation of a lock-free circular queue for EspSoftwareSerial.
Copyright (c) 2019 Dirk O. Kaar. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __circular_queue_mp_h
#define __circular_queue_mp_h

#include "circular_queue.h"

#if defined(ESP8266)
#include <interrupts.h>
using esp8266::InterruptLock;
#endif

/*!
    @brief	Instance class for a multi-producer, single-consumer circular queue / ring buffer (FIFO).
            This implementation is lock-free between producers and consumer for the available(), peek(),
            pop(), and push() type functions.
*/
template< typename T, typename ForEachArg = void >
class circular_queue_mp : protected circular_queue<T, ForEachArg>
{
public:
    circular_queue_mp() : circular_queue<T, ForEachArg>()
    {
        m_inPos_mp.store(0);
        m_concurrent_mp.store(0);
    }
    circular_queue_mp(const size_t capacity) : circular_queue<T, ForEachArg>(capacity)
    {
        m_inPos_mp.store(0);
        m_concurrent_mp.store(0);
    }
    circular_queue_mp(circular_queue_mp<T, ForEachArg>&& cq) : circular_queue<T, ForEachArg>(std::move(cq))
    {
        m_inPos_mp.store(cq.m_inPos_mp.load());
        m_concurrent_mp.store(cq.m_concurrent_mp.load());
    }
    circular_queue_mp& operator=(circular_queue_mp&& cq)
    {
        circular_queue<T, ForEachArg>::operator=(std::move(cq));
        m_inPos_mp.store(cq.m_inPos_mp.load());
        m_concurrent_mp.store(cq.m_concurrent_mp.load());
    }
    circular_queue_mp& operator=(const circular_queue_mp&) = delete;

    using circular_queue<T, ForEachArg>::capacity;
    using circular_queue<T, ForEachArg>::flush;
    using circular_queue<T, ForEachArg>::peek;
    using circular_queue<T, ForEachArg>::pop;
    using circular_queue<T, ForEachArg>::pop_n;
    using circular_queue<T, ForEachArg>::for_each;
    using circular_queue<T, ForEachArg>::for_each_rev_requeue;

    T& pushpeek() = delete;
    bool push() = delete;

    inline size_t IRAM_ATTR available() const ALWAYS_INLINE_ATTR
    {
        return circular_queue<T, ForEachArg>::available();
    }
    inline size_t IRAM_ATTR available_for_push() const ALWAYS_INLINE_ATTR
    {
        return circular_queue<T, ForEachArg>::available_for_push();
    }

    /*!
        @brief	Resize the queue. The available elements in the queue are preserved.
                This is not lock-free and concurrent producer or consumer access
                will lead to corruption.
        @return True if the new capacity could accommodate the present elements in
                the queue, otherwise nothing is done and false is returned.
    */
    bool capacity(const size_t cap);

    /*!
        @brief	Move the rvalue parameter into the queue, guarded
                for multiple concurrent producers.
        @return true if the queue accepted the value, false if the queue
                was full.
    */
    bool push(T&& val);

    /*!
        @brief	Push a copy of the parameter into the queue, guarded
                for multiple concurrent producers.
        @return true if the queue accepted the value, false if the queue
                was full.
    */
    inline bool IRAM_ATTR push(const T& val) ALWAYS_INLINE_ATTR
    {
        T v(val);
        return push(std::move(v));
    }

    /*!
        @brief	Push copies of multiple elements from a buffer into the queue,
                in order, beginning at buffer's head. This is safe for
                multiple producers.
        @return The number of elements actually copied into the queue, counted
                from the buffer head.
    */
#if defined(ESP8266) || defined(ESP32) || !defined(ARDUINO)
    size_t push_n(const T* buffer, size_t size);
#endif

protected:
    std::atomic<size_t> m_inPos_mp;
    std::atomic<int> m_concurrent_mp;
};

template< typename T, typename ForEachArg >
bool circular_queue_mp<T, ForEachArg>::capacity(const size_t cap)
{
    if (cap + 1 == circular_queue<T, ForEachArg>::m_bufSize) return true;
    else if (!circular_queue<T, ForEachArg>::capacity(cap)) return false;
    m_inPos_mp.store(circular_queue<T, ForEachArg>::m_inPos.load(std::memory_order_relaxed),
        std::memory_order_relaxed);
    m_concurrent_mp.store(0, std::memory_order_relaxed);
    return true;
}

template< typename T, typename ForEachArg >
bool IRAM_ATTR circular_queue_mp<T, ForEachArg>::push(T&& val)
{
    size_t inPos_mp;
    size_t next;
#if !defined(ESP32) && defined(ARDUINO)
    class InterruptLock {
    public:
        InterruptLock() {
            noInterrupts();
        }
        ~InterruptLock() {
            interrupts();
        }
    };
    {
        InterruptLock lock;
#else
    ++m_concurrent_mp;
    do
    {
#endif
        inPos_mp = m_inPos_mp.load(std::memory_order_relaxed);
        next = (inPos_mp + 1) % circular_queue<T, ForEachArg>::m_bufSize;
        if (next == circular_queue<T, ForEachArg>::m_outPos.load(std::memory_order_relaxed)) {
#if !defined(ESP32) && defined(ARDUINO)
            return false;
        }
        m_inPos_mp.store(next, std::memory_order_relaxed);
        m_concurrent_mp.store(m_concurrent_mp.load(std::memory_order_relaxed) + 1,
            std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
    }
#else
            int concurrent_mp;
            do
            {
                inPos_mp = m_inPos_mp.load();
                concurrent_mp = m_concurrent_mp.load();
                if (1 == concurrent_mp)
                {
                    circular_queue<T, ForEachArg>::m_inPos.store(inPos_mp, std::memory_order_release);
                }
            }
            while (!m_concurrent_mp.compare_exchange_weak(concurrent_mp, concurrent_mp - 1));
            return false;
        }
    }
    while (!m_inPos_mp.compare_exchange_weak(inPos_mp, next));
#endif

    circular_queue<T, ForEachArg>::m_buffer[inPos_mp] = std::move(val);

    std::atomic_thread_fence(std::memory_order_release);

#if !defined(ESP32) && defined(ARDUINO)
    {
        InterruptLock lock;
        if (1 == m_concurrent_mp.load(std::memory_order_relaxed))
        {
            inPos_mp = m_inPos_mp.load(std::memory_order_relaxed);
            circular_queue<T, ForEachArg>::m_inPos.store(inPos_mp, std::memory_order_relaxed);
        }
        m_concurrent_mp.store(m_concurrent_mp.load(std::memory_order_relaxed) - 1,
            std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
    }
#else
    int concurrent_mp;
    do
    {
        inPos_mp = m_inPos_mp.load();
        concurrent_mp = m_concurrent_mp.load();
        if (1 == concurrent_mp)
        {
            circular_queue<T, ForEachArg>::m_inPos.store(inPos_mp, std::memory_order_release);
        }
    }
    while (!m_concurrent_mp.compare_exchange_weak(concurrent_mp, concurrent_mp - 1));
#endif

    return true;
}

#if defined(ESP8266) || defined(ESP32) || !defined(ARDUINO)
template< typename T, typename ForEachArg >
size_t circular_queue_mp<T, ForEachArg>::push_n(const T* buffer, size_t size)
{
    const auto outPos = circular_queue<T, ForEachArg>::m_outPos.load(std::memory_order_relaxed);
    size_t inPos_mp;
    size_t next;
    size_t blockSize;
#if !defined(ESP32) && defined(ARDUINO)
    {
        InterruptLock lock;
#else
    ++m_concurrent_mp;
    do
    {
#endif
        inPos_mp = m_inPos_mp.load(std::memory_order_relaxed);
        blockSize = (outPos > inPos_mp) ? outPos - 1 - inPos_mp : (outPos == 0) ? circular_queue<T, ForEachArg>::m_bufSize - 1 - inPos_mp : circular_queue<T, ForEachArg>::m_bufSize - inPos_mp;
        blockSize = min(size, blockSize);
        if (!blockSize)
        {
#if !defined(ESP32) && defined(ARDUINO)
            return 0;
        }
        next = (inPos_mp + blockSize) % circular_queue<T, ForEachArg>::m_bufSize;
        m_inPos_mp.store(next, std::memory_order_relaxed);
        m_concurrent_mp.store(m_concurrent_mp.load(std::memory_order_relaxed) + 1,
            std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
    }
#else
            int concurrent_mp = m_concurrent_mp.load();
            do
            {
                inPos_mp = m_inPos_mp.load();
                concurrent_mp = m_concurrent_mp.load();
                if (1 == concurrent_mp)
                {
                    circular_queue<T, ForEachArg>::m_inPos.store(inPos_mp, std::memory_order_release);
                }
            }
            while (!m_concurrent_mp.compare_exchange_weak(concurrent_mp, concurrent_mp - 1));
            return false;
        }
    }
    while (!m_inPos_mp.compare_exchange_weak(inPos_mp, next));
#endif

    auto dest = circular_queue<T, ForEachArg>::m_buffer.get() + inPos_mp;
    std::copy_n(std::make_move_iterator(buffer), blockSize, dest);
    size = min(size - blockSize, outPos > 1 ? static_cast<size_t>(outPos - next - 1) : 0);
    next += size;
    dest = circular_queue<T, ForEachArg>::m_buffer.get();
    std::copy_n(std::make_move_iterator(buffer + blockSize), size, dest);

    std::atomic_thread_fence(std::memory_order_release);

#if !defined(ESP32) && defined(ARDUINO)
    {
        InterruptLock lock;
        if (1 == m_concurrent_mp.load(std::memory_order_relaxed))
        {
            inPos_mp = m_inPos_mp.load(std::memory_order_relaxed);
            circular_queue<T, ForEachArg>::m_inPos.store(inPos_mp, std::memory_order_relaxed);
        }
        m_concurrent_mp.store(m_concurrent_mp.load(std::memory_order_relaxed) - 1,
            std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
    }
#else
    int concurrent_mp;
    do
    {
        inPos_mp = m_inPos_mp.load();
        concurrent_mp = m_concurrent_mp.load();
        if (1 == concurrent_mp)
        {
            circular_queue<T, ForEachArg>::m_inPos.store(inPos_mp, std::memory_order_release);
        }
    }
    while (!m_concurrent_mp.compare_exchange_weak(concurrent_mp, concurrent_mp - 1));
#endif

    return blockSize + size;
}

#endif

#endif // __circular_queue_mp_h
