@echo off
cls

cl /W3 /Ox test_suite.cpp convert/*.cpp security/*.cpp sync/*.cpp templates/*.cpp utf8/*.cpp /link /FILEALIGN:512 /OPT:REF /OPT:ICF /INCREMENTAL:NO advapi32.lib /out:test_suite.exe
