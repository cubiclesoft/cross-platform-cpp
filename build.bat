@echo off
cls

cl /W3 /Ox test_suite.cpp convert/*.cpp security/*.cpp sync/*.cpp templates/*.cpp environment/*.cpp utf8/*.cpp network/*.cpp /link /FILEALIGN:512 /OPT:REF /OPT:ICF /INCREMENTAL:NO advapi32.lib shell32.lib ws2_32.lib /out:test_suite.exe
