#include <cstdio>

#include "shell.hh"
//Start
int yyparse(void);
void yyrestart(FILE * file);
bool source_flag;
int last_pid;
char real_path[1024];
bool onError = false;
extern int return_code;
extern "C" void disp(int sig) {
	printf("ctrl-c called\n");
	Shell::prompt();
	 fflush(stdout);
}

extern "C" void zombie(int sig) {
	int pid = 1;
	while (pid > 0) {
		pid = waitpid(-1, NULL, WNOHANG);
		if (pid > 0 && isatty(0)) {
			fprintf(stderr, "\n[%d] exited.\n", pid);
			Shell::prompt();
		}	
	}
}
void Shell::prompt() {
	//
	char * prompt = getenv("PROMPT");
	//char * error = getenv("ON_ERROR");
	/*
	if (error && return_code != 0) {
                onError = true;
	}
	if (isatty(0) && onError == true) {
                printf("%s\n", error);
        }*/
	if (isatty(0) && !prompt) {
		if (source_flag == true) return;
		printf(" > ");
		fflush(stdout);
	}
	if (isatty(0) && prompt) {
		printf(" %s ", prompt);
	}
	fflush(stdout);
	onError = false;
	/*	
	if (isatty(0) && ) {
		if (source_flag == true) return;
		printf(" > ");
		fflush(stdout);
	}*/
  //printf("\n");
  //fflush(stdout);
}

int main(int argc, char * argv[]) {
	if (argv[0]) {
		char * relative = argv[0];
		realpath(relative, real_path);
	}
	source_flag = false;	
	struct sigaction sa;
        sa.sa_handler = disp;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if(sigaction(SIGINT, &sa, NULL)){
        	perror("sigaction");
        	exit(-1);
        }
	struct sigaction sa2;
        sa2.sa_handler = zombie;
        sigemptyset(&sa2.sa_mask);
        sa2.sa_flags = SA_RESTART;
        if(sigaction(SIGCHLD, &sa2, NULL)){
                perror("sigaction");
                exit(-1);
        }
	//printf("argv is %s\n", argv[0]);	
	last_pid = getpid();
	FILE * fd = fopen(".shellrc", "rw");
	if (fd && isatty(0)){
		yyrestart(fd);
		yyparse();
		yyrestart(stdin);
		fclose(fd);
	}
	else {
		Shell::prompt();
	}
  //Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
