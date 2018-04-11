@SET HOST=127.0.0.1
@SET PORT=5557

@start cmd /C NumberClientDemo.exe -h %HOST% -p %PORT% ^< dat1.txt
@start cmd /C NumberClientDemo.exe -h %HOST% -p %PORT% ^< dat1.txt
@pause