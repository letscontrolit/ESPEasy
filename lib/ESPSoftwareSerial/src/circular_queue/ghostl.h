/*
ghostl.h - Implementation of a bare-bones, mostly no-op, C++ STL shell
           that allows building some Arduino ESP8266/ESP32
           libraries on Aruduino AVR.
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

#ifndef __ghostl_h
#define __ghostl_h

#if defined(ARDUINO_ARCH_SAMD)
#include <atomic>
#endif

using size_t = decltype(sizeof(char));

namespace std
{
#if !defined(ARDUINO_ARCH_SAMD)
    typedef enum memory_order {
        memory_order_relaxed,
        memory_order_acquire,
        memory_order_release,
        memory_order_seq_cst
    } memory_order;
    template< typename T > class atomic {
    private:
        T value;
    public:
        atomic() {}
        atomic(T desired) { value = desired; }
        void store(T desired, std::memory_order = std::memory_order_seq_cst) volatile noexcept { value = desired; }
        T load(std::memory_order = std::memory_order_seq_cst) const volatile noexcept { return value; }
    };
    inline void atomic_thread_fence(std::memory_order order) noexcept {}
    template< typename T > T&& move(T& t) noexcept { return static_cast<T&&>(t); }
#endif

    template< typename T, size_t long N > struct array
    {
        T _M_elems[N];
        decltype(sizeof(0)) size() const { return N; }
        T& operator[](decltype(sizeof(0)) i) { return _M_elems[i]; }
        const T& operator[](decltype(sizeof(0)) i) const { return _M_elems[i]; }
    };

    template< typename T > class unique_ptr
    {
    public:
        using pointer = T*;
        unique_ptr() noexcept : ptr(nullptr) {}
        unique_ptr(pointer p) : ptr(p) {}
        pointer operator->() const noexcept { return ptr; }
        T& operator[](decltype(sizeof(0)) i) const { return ptr[i]; }
        void reset(pointer p = pointer()) noexcept
        {
            delete ptr;
            ptr = p;
        }
        T& operator*() const { return *ptr; }
    private:
        pointer ptr;
    };

    template< typename T > using function = T*;
    using nullptr_t = decltype(nullptr);

    template<typename T>
    struct identity {
        typedef T type;
    };

    template <typename T>
    inline T&& forward(typename identity<T>::type& t) noexcept
    {
        return static_cast<typename identity<T>::type&&>(t);
    }
}

#endif // __ghostl_h
