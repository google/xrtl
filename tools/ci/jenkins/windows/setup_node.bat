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

REM TODO: automate this

REM Setup for new Windows Jenkins nodes.
REM This requires an instance with:
REM   - Windows Server 2016 w/ desktop experience
REM   - Boot disk as SSD with >= 100GB
REM   - K80 GPU (optional)
REM
REM Once the instance has been setup you can RDP in and run this script.

REM Run these from an admin CMD:

REM Install chocolatey
@powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"

REM Install deps
choco install -y git python2 bazel wget
c:\Python27\Scripts\easy_install.exe protobuf

REM GPU drivers
REM http://www.nvidia.com/download/driverResults.aspx/115146/en-us
wget -O c:\tools\nvidia-driver.exe http://us.download.nvidia.com/Windows/Quadro_Certified/376.84/376.84-tesla-desktop-winserver2016-international-whql.exe

REM LLVM
wget -O c:\tools\llvm.exe http://releases.llvm.org/4.0.0/LLVM-4.0.0-win32.exe

REM VC++ 2015 build tools
REM WARNING: this should work, but doesn't
REM wget -O c:\tools\vcpp2015.iso http://go.microsoft.com/fwlink/?LinkId=691126&__hstc=268264337.b22b55fa90b145766c033516223589cb.1491591396310.1491591396310.1491591396310.1&__hssc=268264337.1.1491591396310&__hsfp=1270219171&fixForIE=.exe

REM VS 2015 community
REM wget -O c:\tools\vs2015.exe https://go.microsoft.com/fwlink/?LinkId=615448&clcid=0x409
REM VS 2017
REM https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017-rc

REM Setup Jenkins
wget http://35.185.235.106:8080/jnlpJars/slave.jar
