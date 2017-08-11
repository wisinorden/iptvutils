if not defined DevEnvDir (
  CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64"
)
@ECHO ON

mkdir build-win64
mkdir artefacts-win64
cd build-win64
qmake ../../
nmake /f Makefile.release
cd ..
windeployqt --dir artefacts-win64 build-win64\release\IPTVUtils.exe
copy build-win64\release\IPTVUtils.exe artefacts-win64\IPTVUtils.exe
copy dependencies64\* artefacts-win64
