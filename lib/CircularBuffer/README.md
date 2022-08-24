### &#x26A0; **IMPORTANT**
 
> Please, before submitting a support request read carefully this README and check if an answer already exists among [previously answered questions](https://github.com/rlogiacco/CircularBuffer/issues?q=label:question): do not abuse of the Github issue tracker.

<!-- omit in toc -->
CircularBuffer [![Build Status][travis-status]][travis]
=============
[travis]: https://travis-ci.org/rlogiacco/CircularBuffer
[travis-status]: https://travis-ci.org/rlogiacco/CircularBuffer.svg?branch=master

The library itself has an implicit memory consumption of about *0.5Kb*: 580 bytes (max) of code and 8 bytes of memory, to my calculations. That does not consider the space used to store the items themselves, obviously.

<!-- toc -->
- [Usage](#usage)
  - [Declare and initialize](#declare-and-initialize)
  - [Store data](#store-data)
  - [Retrieve data](#retrieve-data)
  - [Additional operations](#additional-operations)
- [Advanced Usage](#advanced-usage)
  - [Automatic optimization](#automatic-optimization)
  - [Legacy optimization](#legacy-optimization)
  - [Interrupts](#interrupts)
- [Examples](#examples)
- [Limitations](#limitations)
  - [Reclaim dynamic memory](#reclaim-dynamic-memory)
- [CHANGE LOG](#change-log)
  - [1.3.3](#133)
  - [1.3.2](#132)
  - [1.3.1](#131)
  - [1.3.0](#130)
  - [1.2.0](#120)
  - [1.1.1](#111)
  - [1.1.0](#110)
  - [1.0.0](#100)
<!-- tocstop -->

## Usage

### Declare and initialize

When declaring your buffer you should specify the data type it must handle and the buffer capacity: those two parameters will influence the memory consumed by the buffer.

``` cpp
#include <CircularBuffer.h>

CircularBuffer<byte,100> bytes;     // uses 538 bytes
CircularBuffer<int,100> ints;       // uses 638 bytes
CircularBuffer<long,100> longs;     // uses 838 bytes
CircularBuffer<float,100> floats;   // uses 988 bytes
CircularBuffer<double,100> doubles; // uses 988 bytes
CircularBuffer<char,100> chars;     // uses 538 bytes 
CircularBuffer<void*,100> pointers; // uses 638 bytes
```

**Please note**: the memory usage reported above includes the program memory used by the library code, the heap memory is much less and is comparable to an array of the same size and type of the buffer.

### Store data

Let's start making things clear: the library doesn't support inserting data in the middle of the buffer.
You can add data to the buffer either before the first element via an `unshift()` operation or after the last element via a `push()` operation.
You can keep adding data beyond the buffer maximum capacity, but you'll lose the least significant information:

* since `unshift()` adds to the _head_, adding beyond capacity causes the element at _tail_ to be overwritten and lost
* since `push()` adds to the _tail_, adding beyond capacity causes the element at _head_ to be overwritten and lost

Both `unshift()` and `push()` return `true` if the addition didn't cause any information loss, `false` if an overwrite occurred:

``` cpp
CircularBuffer<int,5> buffer; // buffer capacity is 5

// all of the following return true
buffer.unshift(1); // [1] 
buffer.unshift(2); // [2,1]
buffer.unshift(3); // [3,2,1]
buffer.push(0);  // [3,2,1,0]
buffer.push(5);  // [3,2,1,0,5]

// buffer is now at full capacity, from now on any addition returns false
buffer.unshift(2);  // [2,3,2,1,0] returns false
buffer.unshift(10); // [10,2,3,2,1] returns false
buffer.push(-5);  // [2,3,2,1,-5] returns false
```

### Retrieve data

Similarly to data addition, data retrieval can be performed at _tail_ via a `pop()` operation or from _head_ via an `shift()` operation: both cause the element being read to be removed from the buffer.
Reading from an empty buffer is forbidden (the library will generate a segfault, which most probably will crash the program): see the _additional operations_ listed in the next section to verify the status of the buffer.

Non-destructive read operations are also available:

* `first()` returns the element at _head_
* `last()` returns the element at _tail_
* an array-like indexed read operation is also available so you can read any element in the buffer using the `[]` operator

Reading data beyond the actual buffer size has an undefined behaviour and is user's responsibility to prevent such boundary violations using the [_additional operations_](#additional-operations) listed in the next section.

``` cpp
CircularBuffer<char, 50> buffer; // ['a','b','c','d','e','f','g']

buffer.first(); // ['a','b','c','d','e','f','g'] returns 'a'
buffer.last(); // ['a','b','c','d','e','f','g'] returns 'g'
buffer.pop(); // ['a','b','c','d','e','f'] returns 'g'
buffer.pop(); // ['a','b','c','d','e'] returns 'f'
buffer.shift(); // ['b','c','d','e'] returns 'a'
buffer.shift(); // ['c','d','e'] returns 'b'
buffer[0]; // ['c','d','e'] returns 'c'
buffer[1]; // ['c','d','e'] returns 'd'
buffer[2]; // ['c','d','e'] returns 'e'

buffer[10]; // ['c','d','e'] returned value is unpredictable
buffer[15]; // ['c','d','e'] returned value is unpredictable
```

### Additional operations

* `isEmpty()` returns `true` only if no data is stored in the buffer
* `isFull()` returns `true` if no data can be further added to the buffer without causing overwrites/data loss
* `size()` returns the number of elements currently stored in the buffer; it should be used in conjunction with the `[]` operator to avoid boundary violations: the first element index is always `0` (if buffer is not empty), the last element index is always `size() - 1`
* `available()` returns the number of elements that can be added before saturating the buffer
* `capacity()` returns the number of elements the buffer can store, for completeness only as it's user-defined and never changes **REMOVED** from `1.3.0` replaced by the read-only member variable `capacity`
* `clear()` resets the whole buffer to its initial state

## Advanced Usage

### Automatic optimization

Starting from version `1.3.0` the library is capable to automatically detect which data type should be used for the index based on the buffer capacity: 
* if you declare a buffer with a capacity greater than `65535` then your index is going to be an `unsigned long`
* `unsigned int` for buffers with a declared capacity greater than `255`
* otherwise a `byte` is going to suffice

In addition, you can mix in the same code buffers with small index and buffers with normal index: previously this was not possible.

``` cpp
CircularBuffer<char,100> optimizedBuffer; // reduced memory footprint, index type is uint8_t (a.k.a. byte)
CircularBuffer<long,500> normalBuffer;    // standard memory footprint, index type is unit16_t (a.k.a. unsigned int)
CircularBuffer<int,66000> hugeBuffer;     // extended memory footprint, index type is unit32_t (a.k.a. unsigned long)
```

To obtain the maximum advantage of the optimization above, anytime you need to refer to the buffer index you should use the most appropriate type: this can be easily achieved using the `decltype` specifier, like in the following example:

```cpp
// the iterator variable i is of the correct type, even if  
// we don't know what's the buffer declared capacity
for (decltype(buffer)::index_t i = 0; i < buffer.size(); i++) {
    avg += buffer[i] / buffer.size();
}
```

If you prefer, you can alias the index type and refer to such alias:

```cpp
using index_t = decltype(buffer)::index_t;
for (index_t i = 0; i < buffer.size(); i++) {
    avg += buffer[i] / buffer.size();
}
```

### Legacy optimization

_The following applies to versions prior to `1.3.0` only._

By default the library uses `unsigned int` indexes, allowing for a maximum of `65535` items, but you'll rarely need such a huge store.

You can switch the library indexes to `byte` type defining the `CIRCULAR_BUFFER_XS` macro **BEFORE** the `#include` directive: this reduces the memory used by the library itself by only `36` bytes, but allows you to potentially squeeze out much more whenever you perform an indexed access, if you do any, by using the smaller data type.

``` cpp
#define CIRCULAR_BUFFER_XS
#include <CircularBuffer.h>

CircularBuffer<short,100> buffer;

void setup() { }

void loop() {
	// here i should be declared of type byte rather than unsigned int
    // in order to maximize the effects of the optimization
    for (byte i = 0; i < buffer.size() - 1; i++) {
        Serial.print(buffer[i]);
    }
}
```

**Please note**: this _macro switch_ forces the buffer to use an 8 bits data type as internal index, as such **all** your buffers will be limited to a maximum capacity of `255`.

### Interrupts

The library does help working with interrupts defining the `CIRCULAR_BUFFER_INT_SAFE` macro switch, which introduces the `volatile` modifier to the `count` variable, making the whole library more interrupt friendly at the price of disabling some compiler optimizations. The `#define` statement needs to be put somewhere before the `#include` statement:

```cpp
#define CIRCULAR_BUFFER_INT_SAFE
#include <CircularBuffer.h>
CircularBuffer<unsigned long, 10> timings;

void count() {
  timings.push(millis());
}

void setup() {
    attachInterrupt(digitalPinToInterrupt(2), count, RISING);
}

void loop() {
    Serial.print("buffer size is "); Serial.println(timings.size());
    delay(250);
}

```

> Please note this does **NOT** make the library _interrupt safe_, but it does help its usage in interrupt driven firmwares.

## Examples

Multiple examples are available in the `examples` folder of the library:

 * [CircularBuffer.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/CircularBuffer/CircularBuffer.ino) shows how you can use the library to create a continous averaging of the most recent readings
 * [EventLogging.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/EventLogging/EventLogging.ino) focuses on dumping the buffer when it becomes full and printing the buffer contents periodically at the same time
 * [Object.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/Object/Object.ino) is meant to demonstrate how to use the buffer to store dynamic structures
 * [Queue.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/Queue/Queue.ino) is a classical example of a queue, or a FIFO data structure
 * [Stack.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/Stack/Stack.ino) on the other end shows how to use the library to represent a LIFO data structure
 * [Struct.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/Struct/Struct.ino) answers to the question _can this library store structured data?_
 * [Interrupts.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/Interrupts/Interrupts.ino) demonstrates the use of the library in interrupt driven code

## Limitations

### Reclaim dynamic memory

If you use this library to store dynamically allocated objects, refrain from using the `clear()` method as that will **not** perform memory deallocation: you need to iterate over your buffer content and release memory accordingly to the allocation method used, either via `delete` (if you had used `new`) or `free` (in case of `malloc`):

```cpp
while (!buffer.isEmpty()) {
    // pick the correct one
    delete buffer.pop();
    free(buffer.pop());
}
```

The very same applies for the `pop()` and `shift()` operations as any dynamically allocated object is only _detached_ from the buffer, but the memory it uses is **not** automagically released (see the [Object.ino](https://github.com/rlogiacco/CircularBuffer/blob/master/examples/Object/Object.ino) example)

```cpp
Record* record = new Record(millis(), sample);  // a dynamically allocated object
buffer.push(record);

// somewhere else
if (!buffer.isEmpty()) {
    Record* current = buffer.pop();
    Serial.println(current.value());
    delete current; // not doing this will leaves the object in memory!!!
}
```

------------------------
## CHANGE LOG

### 1.3.3
* Fixes #27 compilation error

### 1.3.2
* Fixes #2 preventing `shift()` and `pop()` operations misuse to mess up the buffer
* Fixes #2 preventing _out of boundary_ access using the `[]` operator

### 1.3.1
* Fixes #21 _call to `abort()` is AVR-specific

### 1.3.0

Most of the major improvements below have been contributed by [Erlkoenig90](https://github.com/Erlkoenig90): thank you Niklas!

* Slightly reduced both flash and heap footprint
* Introduced _instance based_ control over index data type
* Replaced method `capacity()` in favour of the constant instance attribute `capacity`
* Added the `EventLogging` and `Interrupts` examples
* Dropped the `CIRCULAT_BUFFER_XS` _macro switch_ in favor of automatic index type identification
* Added support for very large buffers (capacity can go up to `UINT32_MAX`)

### 1.2.0
* Added interrupt related macro switch `CIRCULAR_BUFFER_INT_SAFE`
* Dropped unecessary call to `memset` when clearing

### 1.1.1
* Added tests
* Fixed `clear()` function
* Fixed `pop()` function

### 1.1.0
* Improved robustness against access outside the buffer boundaries
* Fixed `pop()` and `shift()` implementations
* Added test sketch
* Added `capacity()` function
* Added `debug()` function, disabled by pre processor by default

### 1.0.0
* Initial implementation
