Sources: 
- The main entry point is test.cc.
- nbody.cc / nbody.h contain single and multi-threaded implementations of the nbody simulation program.
- nbody.cu contains the CUDA (GPU) sources for nbody.cc
- image.cc / image.h contain basic image creation utilities, for saving visualizations.
- scheduler.cc / scheduler.h contain utilities to control CPU / thread scheduling.
- stats.cc / stats.h contains utilities to compute time taken and log execution statistics.
- thread_pool.h has a implementation of thread pool based on std::thread.
- util.cc / util.h has common utilities (right now just directory creation).

Building:
Use build.bat or build.sh to run cmake and build the 'test' program.

Dependencies:
- Perfetto: 
  - Download perfetto-cpp-sdk-src.zip from Assets in https://github.com/google/perfetto/releases.
  - Modify PERFETTO_PATH in CMakeLists.txt to reflect the path where you unzip the sdk.
- OpenCV:
  - On Linux, `sudo apt install libopencv-dev`
  - On Windows, download sources from https://opencv.org/releases/ and unzip into some installation path.
    Add `<Your installation path>\opencv\build\x64\vc16\bin` to the Windows Path variable.
    Modify 'build.bat' to set DCMAKE_PREFIX_PATH so it correctly points to your opencv library.
- CUDA
  - Install the CUDA SDK from https://developer.nvidia.com/cuda/toolkit
  
Executing:
- Windows: After executing build.bat, run `build\Release\test`
- Linux: After executing build.sh, run `./build/test`

Perfetto:
  - To actually collect traces, download the corresponding Assets for Windows or Linux
    from https://github.com/google/perfetto/releases.
  - Modify run_trace.sh or run_trace.bat to correctly point to tracebox.exe / tracebox.
  - Modify duration_ms inside scheduling.cfg to increase duration of trace collection,
    so it runs longer than your test binary.
  - Generate traces by using run_trace.sh / run_trace.bat to run your program.
  - Visualize traces by loading trace_file.perfetto-trace into https://ui.perfetto.dev

