#ifndef simplcommand_hh
#define simplecommand_hh

#include <string>
#include <vector>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;
  //char ** _args;
  SimpleCommand();
  ~SimpleCommand();
  std::string * envExpansion(std::string * argument);
  void insertArgument( std::string * argument );
  void print();
};

#endif
