@ECHO OFF
REM Copyright 2017 Google Inc.
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM      http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.

SET DIR=%~dp0

REM ============================================================================
REM Environment Validation
REM ============================================================================

CALL :check_python
IF %_RESULT% NEQ 0 (
  ECHO.
  ECHO Python 2.7 must be installed and on PATH:
  ECHO https://www.python.org/ftp/python/2.7.13/python-2.7.13.msi
  GOTO :exit_error
)


REM ============================================================================
REM Trampoline into xtool
REM ============================================================================

%PYTHON_EXE% %DIR%xtool %*
EXIT /b %ERRORLEVEL%


REM ============================================================================
REM Utilities
REM ============================================================================

:check_python
SETLOCAL
SET FOUND_PYTHON_EXE=""
1>NUL 2>NUL CMD /c where python2
IF NOT ERRORLEVEL 1 (
  ECHO FOUND PYTHON 2
  SET FOUND_PYTHON_EXE=python2
)
IF %FOUND_PYTHON_EXE% EQU "" (
  1>NUL 2>NUL CMD /c where python
  IF NOT ERRORLEVEL 1 (
    SET FOUND_PYTHON_EXE=python
  )
)
IF %FOUND_PYTHON_EXE% EQU "" (
  ECHO ERROR: no Python executable found on PATH.
  ECHO Make sure you can run 'python' or 'python2' in a Command Prompt.
  ENDLOCAL & SET _RESULT=1
  GOTO :eof
)
CMD /C %FOUND_PYTHON_EXE% -c "import sys; sys.exit(1 if not sys.version_info[:2] == (2, 7) else 0)"
IF %ERRORLEVEL% NEQ 0 (
  ECHO ERROR: Python version mismatch - not 2.7
  ENDLOCAL & SET _RESULT=1
  GOTO :eof
)
ENDLOCAL & (
  SET _RESULT=0
  SET PYTHON_EXE=%FOUND_PYTHON_EXE%
)
GOTO :eof
