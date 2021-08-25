#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstring>
#include <stdlib.h>
// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  int _outCount;
  int _inCount;
  bool _background;
  bool _append;
  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute();

  static SimpleCommand *_currentSimpleCommand;
};

#endif
