CD dist
XCOPY ..\out\build\Release\three_d_renderer.exe .\ /Y
git add .
git commit -m "updating dist with most recent Release"
CD ..
