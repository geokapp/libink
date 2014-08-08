// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * 
 *
 * Copyright (C) 2014 Giorgos Kappes <geokapp@gmail.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file LICENSE.
 *
 */

#include <iostream>
#include <unistd.h>
#include "../src/libink.h"

using namespace ink;

int main() {

  Logger logger;
  logger.set_log_file("logger.log");
  logger.set_level(5);
  logger.set_header("LOGGER");
  int status = logger.start();
  if (status)
    std::cout << "Logger start failed\n";

  std::string str="hello";
  
  logger.log(0,"1.This should be logged");
  logger.log(1,"2.This should be logged");
  logger.log(5,"3.This should be logged");
  logger.log(7,"4.This should not be logged");
  logger.log(0,"8.This should be logged");
  logger.log(0,"8.This should be logged" + str);
  //sleep(5);  
  logger.stop();

  
  return 0;

}

