@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo   JavaScript语法解析器 - 测试套件
echo ========================================
echo.

REM 检查解析器是否存在
if not exist "js_parser.exe" (
    echo [91m错误: 找不到 js_parser.exe[0m
    echo 请先编译项目
    echo.
    exit /b 1
)

REM 检查tests目录
if not exist "tests" (
    echo [91m错误: 找不到 tests 目录[0m
    exit /b 1
)

set total=0
set passed=0
set failed=0

REM 测试合法脚本
echo [92m测试合法脚本 (tests/valid/)[0m
echo ----------------------------------------

if exist "tests\valid\*.js" (
    for %%f in (tests\valid\*.js) do (
        set /a total+=1
        echo   测试: %%~nxf
        
        js_parser.exe "%%f" >nul 2>&1
        if !errorlevel! equ 0 (
            echo     [92m✓ 通过[0m
            set /a passed+=1
        ) else (
            echo     [91m✗ 失败[0m
            set /a failed+=1
        )
    )
) else (
    echo   [93m未找到测试文件[0m
)

echo.

REM 测试错误脚本
echo [95m测试错误脚本 (tests/invalid/)[0m
echo ----------------------------------------

if exist "tests\invalid\*.js" (
    for %%f in (tests\invalid\*.js) do (
        set /a total+=1
        echo   测试: %%~nxf
        
        js_parser.exe "%%f" >nul 2>&1
        if !errorlevel! neq 0 (
            echo     [92m✓ 正确检测到错误[0m
            set /a passed+=1
        ) else (
            echo     [91m✗ 应该报错但未报错[0m
            set /a failed+=1
        )
    )
) else (
    echo   [93m未找到测试文件[0m
)

echo.
echo ========================================
echo   测试总结
echo ========================================
echo.
echo 总测试数: !total!
echo 通过: !passed!
echo 失败: !failed!
echo.

if !failed! equ 0 (
    echo [92m🎉 所有测试通过！[0m
    exit /b 0
) else (
    echo [93m⚠️  有 !failed! 个测试失败[0m
    exit /b 1
)
