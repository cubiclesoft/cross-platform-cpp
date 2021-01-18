Cross-platform C++ Snippet Library
==================================

This is a cross-platform C++ snippet library.  Most C++ libraries (wxWidgets, Qt, Poco, etc.) attempt to be the "be all, end all" solution for the project you are building.  Those libraries are great for large projects but have the unfortunate tendency to introduce heavy dependencies for little projects, especially when writing third-party libraries for the general user.

[![Donate](https://cubiclesoft.com/res/donate-shield.png)](https://cubiclesoft.com/donate/) [![Discord](https://img.shields.io/discord/777282089980526602?label=chat&logo=discord)](https://cubiclesoft.com/product-support/github/)

This Library is Different
-------------------------

Yeah, yeah, everyone says that, but it really is!  Instead of dropping a whole library into your project, just find and grab the files you need.  For example, if you need a cross-platform, cross-process, named mutex, the only files needed are:

* sync/sync_mutex.h
* sync/sync_mutex.cpp
* sync/sync_util.h
* sync/sync_util.cpp

Which only add a few kilobytes to the project.  Each category (e.g. 'sync') may have its own 'util' files that include common functionality for that category.  If you need to know how to use something, crack open the 'test_suite.cpp' file.  It contains a bunch of examples.  However, the various classes were designed to be intuitive and most IDEs will pick up the comments scattered around the .h files.

Features
--------

* Small object files.  Just a few KB each for the most part.
* Very few interdependencies.
* Cross-platform, cross-process, named:  Mutex, semaphore, event, and reader-writer objects.
* Cross-platform, thread local temporary memory management via Sync::TLS.  Sync::TLS outperforms system malloc()/free()!  (See Notes)
* Cross-platform CSPRNG.
* Detachable node queue, linked list, and ordered hash(!) implementations.  (See Notes)
* Cache support.  A C++ template that implements a partial hash.
* Static vector implementation.
* Integer to string conversion.  With file size options as well (i.e. MB, GB, etc).
* Packed ordered hash.  Up to three times faster than the detachable node OrderedHash.
* Minimalist Unicode conversion support.  Just enough useful logic without having to drag in a multi-MB Unicode support library.
* Cross-platform, UTF-8 file and directory manipulation classes.
* Cross-platform, UTF-8 storage location functions (e.g. a user's home folder).
* Cross-platform, static buffer JSON serializer class.
* FastFind and FastReplace templates.  Works on any binary data.  FastFind probably outperforms std::search (See Notes).  FastReplace supports alternate allocators (e.g. Sync::TLS) and comparison functions (e.g. case-insensitive comparison).
* Variable data storage via StaticMixedVar, UTF8::UTF8MixedVar, and Sync::TLS::MixedVar.  For when you want lightweight dynamic typing with basic string support or just want to avoid std::string.
* Has a liberal open source license.  MIT or LGPL, your choice.
* Designed for relatively painless integration into your project.
* Sits on GitHub for all of that pull request and issue tracker goodness to easily submit changes and ideas respectively.

Test Suite
----------

The test suite that comes with the library is able to be built using 'build.bat' on Windows and 'build.sh' on other OSes.  Running the test suite (e.g. test_suite.exe) without any options will run the basic startup tests and verify that everything is working properly on the target platform.

The following commands will run various performance benchmarks:

* test_suite synctls
* test_suite hashkey
* test_suite list
* test_suite hash
* test_suite loop  (Helps identify bad benchmarks)

Output looks like:

```
# test_suite list
List performance benchmark
--------------------------
Running List speed tests...
        Insertion - 23,179,293 nodes added/sec
        Detach/attach performance - 64,911,010 nodes/sec
        Find performance (1 million nodes) - 17,245 nodes/sec


# test_suite hash
Hash performance benchmark
--------------------------
Running OrderedHash speed tests...
        Integer keys, djb2 hash keys - 5,905,388 nodes added/sec
        String keys, djb2 hash keys - 3,500,444 nodes added/sec
        Integer keys, SipHash hash keys - 2,578,463 nodes added/sec
        String keys, SipHash hash keys - 2,180,187 nodes added/sec
        Integer keys, detach/attach performance - 19,256,672 nodes/sec
        String keys, detach/attach performance - 11,858,135 nodes/sec
        Integer keys, find performance (1 million nodes) - 8,277,900 nodes/sec
        String keys, find performance (1 million nodes) - 6,764,330 nodes/sec

Running PackedOrderedHash speed tests...
        Integer keys, djb2 hash keys - 17,638,442 nodes added/sec
        String keys, djb2 hash keys - 5,592,405 nodes added/sec
        Integer keys, SipHash hash keys - 3,154,760 nodes added/sec
        String keys, SipHash hash keys - 3,713,171 nodes added/sec
        Integer keys, find performance (1 million nodes) - 22,020,249 nodes/sec
        String keys, find performance (1 million nodes) - 13,426,850 nodes/sec
```

System used for example output above:  Intel i7-6700K @ 4GHz, 32GB RAM, Windows 10 Pro

Notes
-----

Some classes require files in the 'templates' subdirectory.  However, again, the number of dependencies is kept to the bare minimum for proper functionality.  These templates were written primarily to reduce the overall size of object files, but a few of them introduce several features that are missing in the Standard library.  Plus the Standard library templates tend to be rather heavy.

In testing, Sync::TLS outperformed system malloc()/free() by a factor of 1.8 to 19.0 times on a single thread.  Performance varied greatly depending on hardware, OS, and compiler settings.  The approach I used appears to be similar to TCMalloc (both utilize Thread Local Storage in a similar manner), but Sync::TLS has a much simpler implementation and is intended for short-lived data that would normally be placed in a fixed-size stack.  Multithreading was not tested but there are probably significant additional performance improvements over system malloc()/free() due to the utilization of Thread Local Storage.

There are three very slow operations in all programs:  External data access (e.g. hard drive, network), memory allocations, and system calls - in that order.  Detachable nodes in data structures help mitigate the second problem.

The detachable node ordered hash is similar to PHP 5 arrays.  It accepts both integer and string keys in the same hash, has almost constant time insert, lookup, delete, and iteration operations, and, most importantly, maintains the desired order of elements.  This is almost the last std::map-like C++ data structure you will ever need.

The packed ordered hash is similar to PHP 7 arrays.  The PackedOrderedHash template implements a hybrid array + hash and accepts both integer and string keys in the same hash but has better performance metrics for the specific but common scenario of inserting new nodes only at the end, frequent key- and index-based lookups, some iteration, and few deletions.  Each node only has 24 bytes of overhead instead of the 56 bytes of overhead for OrderedHashNode on 64-bit OSes.  Nodes are inline and therefore can't be detached, but they can be overwritten and unset.  The tradeoff for inline nodes is reduced memory overhead, generally fewer allocations, and increased performance by leveraging CPU cache lines.  The test suite benchmarks show up to a 3x improvement in performance over OrderedHash for the most common hashing use-cases.

Handling Unicode is HARD.  Once upon a time, many years ago, I started writing my own Unicode implementation but eventually gave up.  There are three main sections of code plus large lookup tables in a full-blown, up-to-date Unicode implementation:  Code point handling (easy-ish), Combining and Precomposed characters, Line Breaking, and Normalization (hard), and finally Case Folding (nearly impossible).  Code point handling is all this snippet library offers and so all Unicode strings that you handle should generally be treated as opaque data.  If you need something more refined than code points in C++, then there is only one legitimate option, which is the IBM ICU implementation of Unicode but will add ~25MB of dependencies to your project.  For some reason I can't find my original software, but I recall getting through the aforementioned Hard bits with around 65KB of lookup tables for common Normalization and the code even supported unlimited combining code points, which was very cool but extremely nerdy.  Regardless, 65KB of tables doesn't really work well for this project (i.e. it wouldn't really count as a "snippet").  Therefore, only code point handling makes any sense.  Note that applications on Windows that use the UTF-8 code snippets for directory and file management will run a bit slower than their *NIX counterparts due to translating between UTF-8 and UTF-16 with correct surrogate support for the latter, of course.

Only JSON serialization is supported in this library at this time.  There are quite a few JSON parser + serializer libraries for C++ that you should consider if you need a JSON _parser_ ([See these benchmarks](https://github.com/miloyip/nativejson-benchmark) to get started).  Most of the JSON libraries out there require a separate library compilation step and do their own memory management.  The serialization class here is different than most libraries since it relies on a static buffer that is set by the application via the SetBuffer() call.  As a result, only a small static vector depth stack (StaticVector) is allocated by the class and should therefore be very light on RAM and very fast even when generating extremely large multi-GB JSON blobs.  At only ~600 lines of code, the snippet here is roughly half the size of [PicoJSON](https://github.com/kazuho/picojson).  As far as parsers go, [RapidJSON](https://github.com/Tencent/rapidjson) is the fastest benchmarked library but requires a separate compilation step and is more awkward to use than [JSON for Modern C++](https://github.com/nlohmann/json), which is a single 900KB header file (25K lines of code).  JSON for Modern C++ is 25 times larger than PicoJSON but the parser is about 2 times faster.

FastFind has an average case (and probably worst case) of O(5n) or better, which is implicitly better than the naive std::search() O(m * n).  Up to five "points of interest" are located in the pattern - beginning, end, two minimal values, and one midpoint - to minimize the number of compares before performing the full comparison.  This approach mitigates security vulnerabilities in naive implementations while simultaneously outperforming most alternate algorithms including Knuth-Morris-Pratt and Boyer-Moore that construct a lookup table (i.e. allocate memory - a rather sluggish operation not needed for the average string search).
