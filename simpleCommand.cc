#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "simpleCommand.hh"

extern char * last_arg;
extern int last_pid;
extern int background_pid;
extern int return_code;
extern char real_path[];
SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}
//check for the expansion
std::string * SimpleCommand::envExpansion(std::string * argument) {
	char * arg = strdup(argument->c_str());
	char * dollar = strchr(arg, '$');
	char * brace = strchr(arg, '{');
	char * ret = new char[sizeof(arg) + 1000];
	if (dollar && brace) {
		int i;
		for (i = 0; *arg != '$'; i++) {
			ret[i] = *arg;
			arg++;
		}
		ret[i] = '\0';
		while (dollar) {
			if (*(dollar + 1) == '{' && *(dollar + 2) != '}') {
				char * temp = dollar + 2;
				char * env = new char[200];
				for (i = 0; *temp != '}'; i++) {
					env[i] = *temp;
					temp++;
				}
				env[i] = '\0';
				int flag = 0;
				if (strcmp(env, "_") == 0) {
					strcat(ret, last_arg);
					flag = 1;
				}
				if (strcmp(env, "$") == 0){
					char id[1024];
					sprintf(id, "%d", last_pid);
					strcat(ret, id);
					flag = 1;
				}
				if (strcmp(env, "!") == 0){
                                        char bg_id[1024];
                                        sprintf(bg_id, "%d", background_pid);
                                        strcat(ret, bg_id);
                                        flag = 1;
                                }
				if (strcmp(env, "?") == 0) {
					char code[1024];
					sprintf(code, "%d", return_code);
                                        strcat(ret, code);
                                        flag = 1;

				}
				if (strcmp(env, "SHELL") == 0) {
					strcat(ret, real_path);
					flag = 1;

				}
				if (!flag) {
				strcat(ret, getenv(env));
				}
				while (*arg != '}') {
					arg++;
				}
				arg++;
				char * buffer = new char[200];
				for (i = 0; *arg != '$' &&  *arg != 0; i++){
					buffer[i] = *arg;
					arg++;
				}
				buffer[i] = '\0';
				strcat(ret, buffer);
			}
			dollar++;
			dollar = strchr(dollar, '$');
		}
		argument = new std::string(ret);
		return argument;
		
	
	}

	return NULL;	

}


void SimpleCommand::insertArgument( std::string * argument ) {
	//check for expansion
	if (envExpansion(argument)) {
		argument = envExpansion(argument);

	}
	
	//tilde
	char * arg = strdup(argument->c_str());
	char * home = strdup(getenv("HOME"));
	if (arg[0] == '~' && arg[1] == '\0') {
		//list the home directory:
		//  ls ~
		arg = home;
	}
	else if (arg[0] == '~' && arg[1] != '\0') {
		if (arg[1] == '/') {
		//ls ~/...
			arg = arg + 1;
			arg = strcat(home, arg);
		}
		else {
			//ls ~user/...
			arg  = arg + 1;
			int i;
			char *user = new char[100];
			for (i = 0; *arg != '\0' && *arg != '/'; i++) {
				user[i] = *arg;
				arg++;
			}
			user[i] = '\0';
			if (*arg == '/') {
				struct passwd *pw = getpwnam(user);
				arg = strdup(strcat(pw->pw_dir, arg));
			}
			else {
				struct passwd *pw = getpwnam(user);
				arg = strdup(pw->pw_dir);


			}

		}

	}
	argument = new std::string(arg);
  	// simply add the argument to the vector
  	_arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}
