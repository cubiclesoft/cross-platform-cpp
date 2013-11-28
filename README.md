Cross-platform C++ Snippet Library
==================================

This is a cross-platform C++ snippet library.  Most C++ libraries (wxWidgets, Qt, Poco, etc.) attempt to be the "be all, end all" solution for the project you are building.  Those libraries are great for large projects but have the unfortunate tendency to introduce heavy dependencies for little projects, especially when writing third-party libraries for the general user.

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
* Integer to string conversion.  With file size options as well (MB, GB, etc).
* Cross-platform CSPRNG.
* Cross-platform, cross-process, named:  Mutex, semaphore, event, and reader-writer objects.
* Cross-platform, thread local temporary memory management via Sync::TLS.  Sync::TLS outperforms system malloc()/free()!*
* Cache support.  A C++ template that implements a partial hash.
* Detachable node queue and linked list implementations.
* Static vector implementation.
* Minimal Unicode conversion support.
* Cross-platform, UTF-8 file and directory manipulation classes.
* Has a liberal open source license.  MIT or LGPL, your choice.
* Designed for relatively painless integration into your project.
* Sits on GitHub for all of that pull request and issue tracker goodness to easily submit changes and ideas respectively.

Notes
-----

Some classes require files in the 'templates' subdirectory.  However, again, the number of dependencies is kept to the bare minimum for proper functionality.  These templates were written primarily to reduce the overall size of object files, but a few of them introduce several features that are missing in the Standard library.  Plus the Standard library templates tend to be rather heavy.

* In testing, Sync::TLS outperformed system malloc()/free() by a factor of 1.8 to 19.0 times on a single thread.  Performance varied depending on hardware, OS, and compiler settings.  The approach I used appears to be similar to TCMalloc (both utilize Thread Local Storage in a similar manner), but Sync::TLS has a much simpler implementation and is intended for short-lived data that would normally be placed in a fixed-size stack.  Multithreading was not tested but there are probably significant additional performance improvements over system malloc()/free() due to the utilization of Thread Local Storage.
