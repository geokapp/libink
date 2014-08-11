// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * libInk - A simple and thread-safe logger.
 *
 * Copyright (C) 2014 Giorgos Kappes <geokapp@gmail.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation. See file LICENSE.
 *
 */
#ifndef LIBINK_H
#define LIBINK_H

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <iostream>
#include <fstream>

namespace ink {

// Definitions for common log levels.
#define LEVEL_ERROR   0
#define LEVEL_WARNING 1
#define LEVEL_INFO    2
#define LEVEL_DEBUG   3
#define LEVEL_TRACE   4

// Macros for different log levels.
#define ERROR(m) log(0,m)
#define WARN(m)  log(1,m)
#define INFO(m)  log(2,m)
#define DEBUG(m) log(3,m)
#define TRACE(m) log(4,m)

/**
 * @name Logger - The logger object.
 *
 * Use this class to create a logger object and log important information
 * about your application.
 * First call the class constructor and set the maximum logging level,
 * the filename of the output file where the log information will be written,
 * and a header message. Then call the start method to initialize the logger
 * and start its worker thread.
 * Each time that you want to log something call the log method by passing
 * an integer that represents the level and the message. You can also use
 * one og the predefined macros ERROR(m), WARN(m), INFO(m), DEBUG(m), and
 * TRACE(m). However, feel free to use your own levels.
 * Finally, use the stop method to stop the logger.
 */
class Logger {
 private:
  /**
   * @name Queue - A lock-free queue
   *
   * This is a simple implementation of lock-free queue found in
   * the following link:
   * http://www.soa-world.de/echelon/2011/02/a-simple-lock-free-queue-in-c.html
   */
  class Queue {
   private:
    struct Node {
      Node(std::string val) : value(val), next(NULL) {}
      std::string value;
      Node* next;
    };
  
    Node* first;
    Node* divider;
  Node* last;
    
   public:
    Queue();
    ~Queue();
    
    void enqueue(const std::string &t);
    bool dequeue(std::string &result);
    bool contains_one();
    
  };
  
 private:
  std::string m_log_filename;
  std::ofstream m_log_filestream;
  std::string m_header;
  int32_t m_level;
  pthread_t m_logger_id;
  bool m_logger_started;
  bool m_logger_died;
  bool m_terminate;
  pthread_cond_t m_cond;
  pthread_mutex_t m_mut;
  Queue *m_queue;
  
 public:
  Logger();
  Logger(const int32_t level, const std::string log_file,
	 const std::string header);  
  ~Logger();
  void set_log_file(const std::string filename);
  std::string log_file();
  void set_level(const int32_t level);
  int32_t level();
  void set_header(const std::string header);
  std::string header();
  int32_t start();
  void stop();
  void log(const int32_t level, const std::string message);

 private:
  void logger_impl();
  static void *logger(void *ptr);
  
};

} // End of ink namespace.


#endif
