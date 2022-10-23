mkdir output
cd output
cmake .. ^
	-DICU_DIR="D:\lib\cpp\icu\icu4c-71.1\icu4c-71_1-Win64-MSVC2019" ^
	-DBoost_INCLUDE_DIR="D:\lib\cpp\boost_1_80_0"
cd ..
pause