@echo off
rem ===========================================================================
rem  Starts the server: loginserver, world, then the zone launcher.
rem  Sets up Perl for the server only, so quest plugins that use DBI/JSON
rem  (e.g. plugins/MySQL.pl -> plugin::LoadMysql) work. The server is built
rem  against Strawberry Perl 5.26 in C:\Straw526.
rem ===========================================================================

cd /d "%~dp0"

rem Perl 5.26 module folders (the perl526.dll copy here can't find them on its own)
set "PERL5LIB=C:\Straw526\perl\site\lib;C:\Straw526\perl\vendor\lib;C:\Straw526\perl\lib"
rem libmysql__.dll for DBD::mysql
set "PATH=C:\Straw526\c\bin;C:\Straw526\perl\bin;%PATH%"

start "Login Server" loginserver.exe
timeout /t 3 /nobreak >nul

start "World" world.exe
rem Give world time to finish booting (and run any database updates) before zones connect
timeout /t 10 /nobreak >nul

rem Optional, if you run them:
rem start "UCS" ucs.exe
rem start "Query Server" queryserv.exe

start "Zone Launcher" eqlaunch.exe zone
