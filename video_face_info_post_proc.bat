rem echo off

xcopy "%~2\*.*" "%~1_face_info\" /Y
rd /s /q "%~2"
rem explorer "%~1_face_info"