# Simple game test script
$process = Start-Process -FilePath ".\Game.exe" -NoNewWindow -PassThru -RedirectStandardOutput "output.txt" -RedirectStandardError "error.txt"

# Send input after short delay
Start-Sleep -Milliseconds 500
[System.Windows.Forms.SendKeys]::SendWait("demo")
[System.Windows.Forms.SendKeys]::SendWait("{ENTER}")

# Wait for game to process
Start-Sleep -Milliseconds 1000

# Exit game
[System.Windows.Forms.SendKeys]::SendWait("q")
[System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
[System.Windows.Forms.SendKeys]::SendWait("y")
[System.Windows.Forms.SendKeys]::SendWait("{ENTER}")

# Wait for process to exit
$process.WaitForExit()

# Show captured output
Write-Host "=== STANDARD OUTPUT ==="
Get-Content output.txt | Select-Object -First 100
Write-Host "`n=== STANDARD ERROR ==="
Get-Content error.txt
