@echo off
:: Batch script to add, commit, and push changes to Git
:: Usage: git_commit_push.bat "Your commit message"

if "%~1"=="" (
    echo Error: Please provide a commit message.
    echo Usage: %~nx0 "Your commit message here"
    pause
    exit /b 1
)

:: Git commands
git add . || (
    echo Error: Failed to 'git add .'
    pause
    exit /b 1
)

git commit -m "%~1" || (
    echo Error: Failed to commit changes.
    pause
    exit /b 1
)

git push origin master || (
    echo Error: Failed to push to origin/master.
    pause
    exit /b 1
)

echo Successfully added, committed, and pushed changes!
pause