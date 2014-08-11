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
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>
#include "libink.h"

using namespace ink;

/**
 * @name get_date_time - Get date and time.
 *
 * It prints the current date and time into a nicely formatted string.
 *
 * @return String that contains the date and time.
 */
const std::string get_date_time() {
  time_t     now = time(0);
  struct tm  tstruct;
  char       buf[80];
  memset(buf, 0, 80);
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
  return buf;
}

/**
 * @name Queue - Constructor.
 *
 * The constructor of the queue object.
 */
Logger::Queue::Queue() {
  first = new Node(std::string());
  divider = first;
  last = first;
}

/**
 * @name ~Queue - Destructor.
 *
 * The destructor of the queue object.
 */
Logger::Queue::~Queue() {
  while (first != NULL) {
    Node* tmp = first;
    first = tmp->next;
    delete tmp;
  }  
}

/**
 * @name enqueue - Enqueue.
 * @param t: A string that contain the message.
 *
 * Adds a new node to the end of the queue.
 *
 * @return Void.
 */
void Logger::Queue::enqueue(const std::string &t) {
  last->next = new Node(t);
  // add the new item.
  asm volatile("" ::: "memory");
  // prevend compiler reordering.
  // gcc atomic, cast to void to prevent "value computed is not used".
  (void)__sync_lock_test_and_set(&last, last->next);
  
  while(first != divider) {
    Node* tmp = first;
    first = first->next;
    delete tmp;
  }
}

/**
 * @name dequeue - Dequeue.
 * @param result: A string where the result message is stored.
 *
 * Dequeues a node from the begin of the queue.
 *
 * @return true: if a node was dequeued, false: if the queue was empty.
 */
bool Logger::Queue::dequeue(std::string &result) {
  if(divider != last) {
    result = divider->next->value;
    // C: copy it back
    asm volatile("" ::: "memory");
    // prevend compiler reordering
    // gcc atomic, cast to void to prevent "value computed is not used"
    (void)__sync_lock_test_and_set(&divider, divider->next);
    
    return true;
  }
  return false;
}


bool Logger::Queue::contains_one() {
  if (first->next == last)
    return true;
  else
    return false;
}


/**
 * @name Logger - Constructor.
 *
 * The default constructor of the queue object.
 */
Logger::Logger() {
  pthread_cond_init(&m_cond, NULL);
  pthread_mutex_init(&m_mut, NULL);
  m_queue = new Queue();
  m_terminate = false;
  m_logger_started = false;
  m_logger_died = false;
}

/**
 * @name Logger - Constructor.
 * @param level: The maximum logging level.
 * @param log_file: The name of the output log file.
 * @param header: A header message.
 *
 * Constructor of the queue object.
 */
Logger::Logger(const int32_t level, const std::string log_file,
	       const std::string header) {
  m_level = level;
  m_log_filename = log_file;
  m_header = header;
  pthread_cond_init(&m_cond, NULL);
  pthread_mutex_init(&m_mut, NULL);
  m_queue = new Queue();
  m_terminate = false;
  m_logger_started = false;
  m_logger_died = false;
}

/**
 * @name ~Logger - Destructor.
 *
 * The desstructor of the Logger object.
 */
Logger::~Logger() {
  if (!m_terminate && m_logger_started) {
    // Stop method has not been called. We have to stop the logging thread here.
    this->stop();
  }
  pthread_cond_destroy(&m_cond);
  pthread_mutex_destroy(&m_mut);
  
  if (m_queue)
    delete m_queue;
}

/**
 * @name set_log_file - Set the output file.
 * @param filename: The filename.
 *
 * Sets the output log file.
 *
 * @return Void.
 */
void Logger::set_log_file(const std::string filename) {
  m_log_filename = filename;
}

/**
 * @name log_file - Get the output filename.
 *
 * Returns the output log file.
 *
 * @return String that cotnains the filename of the log file.
 */
std::string Logger::log_file() {
  return m_log_filename;
}

/**
 * @name set_level - Set the maximum log level
 * @param level: The maximum log level.
 *
 * Sets the maximum log level.
 *
 * @return Void.
 */
void Logger::set_level(const int32_t level) {
  m_level = level;
}

/**
 * @name level - Get the maximum level.
 *
 * Returns the maximum level.
 *
 * @return The maximum level.
 */
int32_t Logger::level() {
  return m_level;
}

/**
 * @name set_header - Set the header.
 * @param header: The header message.
 *
 * Sets the header message.
 *
 * @return Void.
 */
void Logger::set_header(const std::string header) {
  m_header = header;
}

/**
 * @name header - Get the header.
 *
 * Get the header message.
 *
 * @return The header.
 */
std::string Logger::header() {
  return m_header;
}

/**
 * @name start - Start logging.
 *
 * This method starts the logging process by creating a new logger thread.
 *
 * @return 0 for success, 1 for error.
 */
int32_t Logger::start() {
  // First try to open the file for reading.
  m_log_filestream.open(m_log_filename.c_str(), std::ofstream::out | std::ofstream::app);
  if (m_log_filestream.is_open()) {
    // Now create the logger thread.
    int32_t status = pthread_create(&m_logger_id, NULL, logger, this);
    m_logger_started = true;
    return 0;
  } else {
    return 1;
  }
}

/**
 * @name stop - Stop the logging process.
 *
 * This method stops the logging process. It signals the logging thread and
 * waits for it to terminate.
 *
 * @return Void.
 */
void Logger::stop() {
  if (m_logger_started) {
    m_terminate = true;
    while (!m_logger_died) {
      pthread_mutex_lock(&m_mut);
      pthread_cond_signal(&m_cond);
      pthread_mutex_unlock(&m_mut);
    }
    
    //Wait for the logger thread to terminate.
    int32_t status = pthread_join(m_logger_id, NULL);
    m_log_filestream.close();
  }  
}

/**
 * @name log - Put a new message into the log.
 * @param level: The critical level of the message to log.
 * @param message: The message to log.
 *
 * Puts a new message into the log. The message is put into a lock-free
 * queue and is later dequeued by the logger thread wnd written to the log
 * file. 
 *
 * @return Void.
 */
void Logger::log(const int32_t level, const std::string message) {
  if (level <= m_level && !m_terminate) {
    m_queue->enqueue(message);
    // If this is the first node inform the logger thread.
    // There is no need to hold mutexes here.
    if (m_queue->contains_one())
      pthread_cond_signal(&m_cond);
  }
}

/**
 * @name logger - The logger thread's main method.
 * @param ptr: A pointer to an instance of the logger object.
 *
 * This method actually does nothing else by calling the logger_impl
 * method.
 *
 * @return Void.
 */
void *Logger::logger(void *ptr) {
  Logger *logger = static_cast<Logger *>(ptr);
  logger->logger_impl();
}

/**
 * @name logger_impl - The logger method.
 *
 * While the logger is active the logger thread tries to dequeue the next
 * message from the log queue and appends it to the log file. When the queue
 * becomes empty the thread sleeps by calling the pthread_cond_wait method.
 *
 * @return Void.
 */
void Logger::logger_impl() {
  bool status;
  std::string str;
  
  do {
    pthread_mutex_lock(&m_mut);
    pthread_cond_wait(&m_cond, &m_mut);
    pthread_mutex_unlock(&m_mut);
    
    status = m_queue->dequeue(str);
    while (status) {
      // Log the dequeued message.
      if (m_log_filestream.is_open()) {
	m_log_filestream << get_date_time() << " " << m_header << ": " << str << "\n";
	m_log_filestream.flush();
      } else {
	std::cerr << "ink::Logger::logger_impl: Output stream is not open.\n";
      }
      
      // Try to dequeue the next element.
      status = m_queue->dequeue(str);
    }
    // The queue is currently empty. Block until a signal reaches.
  } while (!m_terminate);
  m_logger_died = true;
}



    
    
  
