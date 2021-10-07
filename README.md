# Waterpipe
A C++ event driven programming framework.

C++17 compiler is required.
The main framework is in Waterpipe::Base class, about 200 lines of C++ codes.
There are rising/falling edge detector, timer and counter classes included.

Other files make up a test console game using Waterpipe.
This game is tested on macOS and Ubuntu Linux, but it will not run on Windows due to terminal compatibility problems.

10/07/2021 Added:
  EdgeDetector.hpp    Rising/Falling edge detector.
  Timer.hpp           Timer.
  Counter.hpp         Counter.
