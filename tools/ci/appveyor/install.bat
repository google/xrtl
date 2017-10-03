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
REM Fetch and Install Dependencies
REM ============================================================================

IF NOT EXIST %INSTALL_CACHE% (
  MKDIR %INSTALL_CACHE%
)

REM Install protobuf dep.
c:\Python27\Scripts\easy_install.exe protobuf

REM Download bazel into install cache, which is on the path.
IF NOT EXIST %INSTALL_CACHE%\bazel.exe (
  appveyor DownloadFile https://github.com/bazelbuild/bazel/releases/download/0.6.0/bazel-msvc-0.6.0-without-jdk-windows-msvc-x86_64.exe -FileName %INSTALL_CACHE%\bazel.exe
)

REM Download and install LLVM.
IF NOT EXIST %INSTALL_CACHE%\llvm-installer.exe (
  appveyor DownloadFile http://releases.llvm.org/5.0.0/LLVM-5.0.0-win64.exe -FileName %INSTALL_CACHE%\llvm-installer.exe
)
START /WAIT %INSTALL_CACHE%\llvm-installer.exe /S /D="C:\Program Files\LLVM"

CD %WORKSPACE%

REM ============================================================================
REM Prepare Build Environment
REM ============================================================================

COPY tools\ci\appveyor\.bazelrc .

REM Fix permissions on bazel folders.
CACLS c:\bazel_root\ /t /e /g Everyone:f

CMD /C xtool.bat setup
IF %ERRORLEVEL% NEQ 0 EXIT /b %ERRORLEVEL%

EXIT /b 0
