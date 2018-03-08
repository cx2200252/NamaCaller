echo off

for /f "delims=" %%a in ('dir /b/a-d/oN v*.*.*.txt') do set VERSION=%%~na

set OUT_PATH=E:\fu_tools\NamaCaller_%VERSION%\
echo %OUT_PATH%
rem pause

rem copy bin
xcopy ..\x64\Release\platforms %OUT_PATH%bin\platforms\ /Y /S
copy ..\x64\Release\NamaCaller.exe %OUT_PATH%bin\ /Y

copy ..\x64\Release\nama.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\cnnmobile.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\dde_core.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\libsgemm.dll %OUT_PATH%bin\ /Y

copy ..\x64\Release\opencv_highgui310.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\opencv_videoio310.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\opencv_imgcodecs310.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\opencv_imgproc310.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\opencv_core310.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\Qt5Core.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\Qt5OpenGL.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\Qt5Test.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\Qt5Widgets.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\Qt5Gui.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\Qt5Concurrent.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\glew32.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\opencv_ffmpeg310_64.dll %OUT_PATH%bin\ /Y
copy ..\x64\Release\msvcp140.dll %OUT_PATH%bin\ /Y

rem copy resources
md %OUT_PATH%resources
md %OUT_PATH%resources\json

copy ..\resources\v3.bundle %OUT_PATH%resources\ /Y
copy ..\resources\ardata_ex.bundle %OUT_PATH%resources\ /Y
copy ..\resources\face_beautification.bundle %OUT_PATH%resources\ /Y
xcopy ..\resources\video_filters %OUT_PATH%resources\video_filters\ /Y /S
copy ..\resources\json\*.json %OUT_PATH%resources\json\ /Y
copy ..\resources\json\*.bat %OUT_PATH% /Y

copy %VERSION%.txt %OUT_PATH%version.txt
rem explorer %OUT_PATH%