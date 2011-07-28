@echo off
rem
rem

pushd .

if "%1" == "" goto hhc_hhk

goto do_it

:hhc_hhk

cd html

for %%f in ( *.html ) do call %~fs0 "%%f"

copy /Y ..\doxygen.css .

goto ende


:do_it

copy /Y %1 ___tmp.htm 
rem hack it: add CSS styling to specific phrases
echo. >> ___tmp.htm

awk -f "%~f0.awk" ___tmp.htm > %1

del ___tmp.htm

goto ende



:ende
 
popd
