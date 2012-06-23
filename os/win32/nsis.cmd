@echo off
cd ../../out
for %%I in (*.exe *.dll) do SignTool.exe sign /f %SCHAT_SIGN_FILE% /p %SCHAT_SIGN_PASSWORD% /t http://timestamp.comodoca.com/authenticode %%~fI

cd plugins
for %%I in (*.dll) do SignTool.exe sign /f %SCHAT_SIGN_FILE% /p %SCHAT_SIGN_PASSWORD% /t http://timestamp.comodoca.com/authenticode %%~fI

cd ../..
nmake install
if ERRORLEVEL 1 exit 1

cd os/win32
makensis setup.nsi
if ERRORLEVEL 1 exit 1

makensis server.nsi
if ERRORLEVEL 1 exit 1

cd wix
candle -nologo -ext WixUIExtension Files.wxs Product.wxs Features.wxs Shortcuts.wxs
if ERRORLEVEL 1 exit 1

light -nologo -out ..\out\schat2-%SCHAT_VERSION%.msi -cultures:en-US -ext WixUIExtension Files.wixobj Product.wixobj Features.wixobj Shortcuts.wixobj
if ERRORLEVEL 1 exit 1

cd ..\out
for %%I in (*.exe) do SignTool.exe sign /f %SCHAT_SIGN_FILE% /p %SCHAT_SIGN_PASSWORD% /t http://timestamp.comodoca.com/authenticode %%~fI
for %%I in (*.msi) do SignTool.exe sign /f %SCHAT_SIGN_FILE% /p %SCHAT_SIGN_PASSWORD% /t http://timestamp.comodoca.com/authenticode %%~fI

cd ..
exit 0