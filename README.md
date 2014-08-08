libInk - A simple and thread-safe logger
===============================================================

libInk is a simple and thread-safe logger for C++ applications. It uses a lock-free
queue structure to manage the logs. Multiple producer threads can add log messages to
this queue by calling the log method, while a single consumer thread dequeues
these logs and writes them to the log file.


Building and installing libInk
------------------------------

To build libInk just type:
   `make`

To build a development version of libInk with support for debugging
symbols type:
   `make dev`

To build only the tests type:
   `make tests`

To install the libInk library, run the following as root:
   `make install`


Removing libInk
---------------

To remove the libInk library type as root:
   `make remove`


Testing libInk
--------------

We have included several tests which check whether the libInk library
works correctly. Check the `tests` folder for details.


Using libInk
-------------

To use the libInk library you need to include `#include <libink/libink.h` into your C++
source file. Then, you can build your application as follows:

   `$(CC) -o application_binary application.cc -link`

Use the Logger class to create a logger object and log important information about your
application. First call the class constructor and set the maximum logging level,
the filename of the output file where the log information will be written,
and a header message:

  ```C
  ink::Logger logger;
  logger.set_level(WARNING);
  logger.set_header("App name");
  logger.set_log_file("/var/log/app/app.log");

  ```

Then call the start method to initialize the logger and start its worker thread:

  ```C
  logger.start();
  ```

Each time that you want to log something call the log method by passing an integer that represents
the level and the message. You can also use one og the predefined macros ERROR(m), WARN(m), INFO(m),
DEBUG(m), and TRACE(m). However, feel free to use your own levels:

  ```C
  logger.log(0, "message1");
  ...
  logger.ERROR("message2");

Finally, call the stop method to stop the logger:

  ```C
  logger.stop();
  ```

Development and Contributing
----------------------------

The initial developer of libInk is [Giorgos Kappes](http://cs.uoi.gr/~gkappes). Feel free to
contribute to the development of libInk by cloning the repository:
`git clone https://github.com/geokapp/libInk`.
You can also send feedback, new feature requests, and bug reports to
<geokapp@gmail.com>.