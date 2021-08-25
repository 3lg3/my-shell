/*//
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "command.hh"
#include "shell.hh"

extern char **environ;
char * last_arg;
int background_pid;
int return_code;
//using namespace std;
//extern std::vector<SimpleCommand *> _simpleCommands;

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = false;
    _outCount = 0;
    _inCount = 0;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
    last_arg = strdup(simpleCommand->_arguments[simpleCommand->_arguments.size() -1]->c_str());


}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
    }

    if ( _inFile ) {
        delete _inFile;
    }

    if ( _errFile ) {
	if (_errFile != _outFile) {
        	delete _errFile;
	}
    }
    _errFile = NULL;
    _outFile = NULL;
    _inFile = NULL;
    _background = false;
    _outFile = NULL;
    _inFile = NULL;
    _append = false;
    _inCount = 0;
    _outCount = 0;

}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }
    	//exit when call 'exit'
	if (strcmp((_simpleCommands[0]->_arguments[0])->c_str(), "exit") == 0) {
		std::cout << "Good bye!!" << std::endl;
		exit(1);
	}

    	// Print error message when there is an ambiguous outpur
	if (_outCount > 1 || _inCount > 1) {
		std::cout << "Ambiguous output redirect." << std::endl;
		clear();
		Shell::prompt();
		return;
	}
	

    // Print contents of Command data structure
    //	 print();
    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
    	//save in/out/error 
	
	int tmpin = dup(0);
	int tmpout = dup(1);
	int tmperr = dup(2);
	//set initial input/output/error
	int fdin;
	int fdout;
	int fderr;
	int ret;
	if (_inFile) {
		fdin = open(_inFile->c_str(),O_RDONLY);
	}
	else {
		//use default input
		fdin = dup(tmpin);
		close(fdin);
	}
	if (_errFile) {
		if (_append == true) {
			fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0600);
		}
		else {
			fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
		}
	}
	else {
		//use default error
		fderr = dup(tmperr);
	}
	dup2(fderr, 2);
	close(fderr);
	for (unsigned i = 0; i < _simpleCommands.size(); i++) {
		//for built-in command
		const char * arg = (_simpleCommands[i]->_arguments[0])->c_str();
        	if (strcmp(arg, "setenv") == 0) {
                	if (setenv((_simpleCommands[i]->_arguments[1])->c_str(), (_simpleCommands[i]->_arguments[2])->c_str(), 1)) {
                        	perror("setenv");
                	}
                	clear();
                	Shell::prompt();
			close(tmpin);
        		close(tmpout);
        		close(tmperr);
                	return;
        	}

        	if (strcmp(arg, "unsetenv") == 0) {
                	if (unsetenv((_simpleCommands[i]->_arguments[1])->c_str())) {
                //      	perror("unsetenv");
                	}
                	clear();
                	Shell::prompt();
			close(tmpin);
                        close(tmpout);
                        close(tmperr);
                	return;
        	}
        	if (strcmp(arg, "cd") == 0) {
                	int flag;
                	if (_simpleCommands[i]->_arguments.size() == 1) {
                        	flag = chdir(getenv("HOME"));
                	}
                	else {
                        	flag = chdir(_simpleCommands[i]->_arguments[1]->c_str());
                	}	
                	if (flag < 0) {
                        	fprintf(stderr, "cd: can't cd to %s", _simpleCommands[i]->_arguments[1]->c_str());
                	}	
                	clear();
                	Shell::prompt();
			close(tmpin);
                        close(tmpout);
                        close(tmperr);
                	return;
       	 	}

		//redirect input
		dup2(fdin, 0);
		close(fdin);
		//setup output
		if (i == _simpleCommands.size() - 1) {
			if (_outFile) {
				if (_append) {
					fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
				}
				else {
					fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
				}
			}	
			else {
				//use default output
				fdout = dup(tmpout);
			}
		}
		
		else {
			// Not last command then create pipe
			int fdpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];
		}
		//redirect the output
		dup2(fdout, 1);
		close(fdout);
		//std::cout << i << std::endl;	
		//create child process
		ret = fork();
		if (ret < 0) {
                        perror("fork");
			exit(2);
                }

		//std::cout << "ret is " << ret << std::endl;
		if (ret == 0) {
			//printenv
			if (strcmp((_simpleCommands[i]->_arguments[0])->c_str(), "printenv") == 0) {
				char ** env = environ;
				while (*env) {
					std::cout << *env << std::endl;
					env++;
				}
				close(tmpin);
                        	close(tmpout);
                        	close(tmperr);
				return;
			}
			
			//source
			/*
			if (strcmp((_simpleCommands[i]->_arguments[0])->c_str(), "source") == 0) {
				std::cout << "asda s d" << std::endl;


			}*/	
			



			else {
				//std::cout << (_simpleCommands[i]->_arguments[0])->c_str() << std::endl;
				char ** _args = (char **) malloc ((_simpleCommands[i]->_arguments).size() * sizeof(const char * ));
			
				for (unsigned j = 0; j < (_simpleCommands[i]->_arguments).size(); j++) {
					_args[j] =strdup((_simpleCommands[i]->_arguments[j])->c_str());
				}
				_args[_simpleCommands[i]->_arguments.size()] = NULL;
				//execlp(grep, cat, argv[2], (char *) 0);
				//std::cout << "stage 2" << std::endl;
//				std::cout << *_simpleCommands[i]->_arguments[0] << std::endl;
				close(tmpin);
                                close(tmpout);
                                close(tmperr);
				execvp((_simpleCommands[i]->_arguments[0])->c_str(), _args);
		//		perror("execvp");
				exit(1);
			}
		}
	}
    // Clear to prepare for next command
    	dup2(tmpin, 0);
        dup2(tmpout, 1);
        dup2(tmperr, 2);
	close(fdin);
        close(fdout);
        close(fderr);
        close(tmpin);
        close(tmpout);
        close(tmperr);
        if (!_background) {
		int status;
		return_code = 0;
                waitpid(ret, &status, 0);
		if (WIFEXITED(status)) {
			return_code = WEXITSTATUS(status);
		}
		char * error = getenv("ON_ERROR");
                if (error && return_code != 0) {
                	printf("%s\n", error);
               	}
	}
	else {
		background_pid = ret;
	}
    clear();

    // Print new prompt
    Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
