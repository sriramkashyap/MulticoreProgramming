 # Building for android
   cmake -DCMAKE_TOOLCHAIN_FILE=/home/sriram/Android/Sdk/ndk/29.0.13846066/build/cmake/android.toolchain.cmake CMakeLists.txt
   cmake --build .
 
 # Building for any other platform
   cmake CMakeLists.txt
   cmake --build .

