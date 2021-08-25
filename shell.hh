#ifndef shell_hh
#define shell_hh

#include "command.hh"
#include "signal.h"
#include <sys/types.h>
#include <regex.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "y.tab.hh"
#include <sys/wait.h>

struct Shell {

  static void prompt();

  static Command _currentCommand;
};

#endif
