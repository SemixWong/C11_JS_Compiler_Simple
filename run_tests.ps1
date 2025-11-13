# JavaScript Parser Test Runner
# Run all test cases in tests folder

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  JavaScript Parser - Test Suite" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if parser exists
if (-not (Test-Path ".\js_parser.exe")) {
    Write-Host "[ERROR] js_parser.exe not found" -ForegroundColor Red
    Write-Host "Please compile the project first" -ForegroundColor Yellow
    Write-Host ""
    exit 1
}

# Check if tests directory exists
if (-not (Test-Path ".\tests")) {
    Write-Host "[ERROR] tests directory not found" -ForegroundColor Red
    exit 1
}

# Initialize counters
$totalTests = 0
$passedTests = 0
$failedTests = 0
$validPassed = 0
$validFailed = 0
$invalidPassed = 0
$invalidFailed = 0

# Test valid JavaScript files (should pass)
Write-Host "[VALID] Testing valid scripts (tests/valid/)" -ForegroundColor Green
Write-Host "----------------------------------------" -ForegroundColor Gray

$validFiles = Get-ChildItem -Path ".\tests\valid\*.js" -ErrorAction SilentlyContinue

if ($validFiles) {
    foreach ($file in $validFiles) {
        $totalTests++
        Write-Host "  Test: $($file.Name)" -NoNewline
        
        $output = & .\js_parser.exe $file.FullName 2>&1
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -eq 0) {
            Write-Host " [PASS]" -ForegroundColor Green
            $passedTests++
            $validPassed++
        } else {
            Write-Host " [FAIL]" -ForegroundColor Red
            $failedTests++
            $validFailed++
            $output | ForEach-Object { Write-Host "      $_" -ForegroundColor DarkGray }
        }
    }
} else {
    Write-Host "  [WARNING] No test files found" -ForegroundColor Yellow
}

Write-Host ""

# Test invalid JavaScript files (should fail)
Write-Host "[INVALID] Testing invalid scripts (tests/invalid/)" -ForegroundColor Magenta
Write-Host "----------------------------------------" -ForegroundColor Gray

$invalidFiles = Get-ChildItem -Path ".\tests\invalid\*.js" -ErrorAction SilentlyContinue

if ($invalidFiles) {
    foreach ($file in $invalidFiles) {
        $totalTests++
        Write-Host "  Test: $($file.Name)" -NoNewline
        
        $output = & .\js_parser.exe $file.FullName 2>&1
        $exitCode = $LASTEXITCODE
        
        # For invalid tests, we expect failure (exitCode != 0)
        if ($exitCode -ne 0) {
            Write-Host " [PASS] Error detected" -ForegroundColor Green
            $passedTests++
            $invalidPassed++
        } else {
            Write-Host " [FAIL] Should report error" -ForegroundColor Red
            $failedTests++
            $invalidFailed++
        }
    }
} else {
    Write-Host "  [WARNING] No test files found" -ForegroundColor Yellow
}

Write-Host ""

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Test Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Total tests: $totalTests" -ForegroundColor White
Write-Host "Passed: $passedTests" -ForegroundColor Green
Write-Host "Failed: $failedTests" -ForegroundColor $(if ($failedTests -eq 0) { "Green" } else { "Red" })
Write-Host ""
Write-Host "Valid scripts: $validPassed/$($validFiles.Count) passed" -ForegroundColor $(if ($validFailed -eq 0) { "Green" } else { "Yellow" })
Write-Host "Invalid scripts: $invalidPassed/$($invalidFiles.Count) passed" -ForegroundColor $(if ($invalidFailed -eq 0) { "Green" } else { "Yellow" })
Write-Host ""

if ($failedTests -eq 0) {
    Write-Host "[SUCCESS] All tests passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "[WARNING] $failedTests test(s) failed" -ForegroundColor Yellow
    exit 1
}
