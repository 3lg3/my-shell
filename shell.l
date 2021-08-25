
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <unistd.h>
#include "y.tab.hh"
#include "command.hh"
extern bool source_flag;
static  void yyunput (int c,char *buf_ptr  );
extern "C" char * read_line();

int mygetc(FILE * f) {
 static char *p;
 char ch;

 if (!isatty(0)) {
 // stdin is not a tty. Call real getc
   return getc(f);
 }

 // stdin is a tty. Call our read_line.
 if (p==NULL || *p == 0) {
   char * s = read_line();
   p = s;
 }

 ch = *p;
 p++;

 return ch;
}

#undef getc
#define getc(f) mygetc(f)

void myunputc(int c) {
  	unput(c);
}

%}


%%

\n {
  	return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  	return GREAT;
}

"&" {
	return AND;
}

"|" {
	return PIPE;
}

"<" {
	return LESS;
}

">>" {
	return GREATGREAT;
}	

">&" {
	return GREATAND;
}

">>&" {
	return GREATGREATAND;
}

"2>" {
	return TWOGREAT;
}

$\([^\n\)]*\) {
	char * cmd = strdup(yytext + 2);
        if (cmd[yyleng - 3] == ')') {
                cmd[yyleng - 3] = '\0';
        }
        int tmpin = dup(0);
        int tmpout = dup(1);
        int cmdpipe[2];
        int fdpipe[2];
        pipe(cmdpipe);
        pipe(fdpipe);
        write(cmdpipe[1], cmd, strlen(cmd));
        write(cmdpipe[1], "\n", 1);
        close(cmdpipe[1]);
        dup2(cmdpipe[0], 0);
        close(cmdpipe[0]);
        dup2(fdpipe[1], 1);
        close(fdpipe[1]);
        int ret;
        ret = fork();
        if (ret < 0) {
              	perror("fork");
                _exit(1);
        }
        if (ret == 0) {
                execvp("/proc/self/exe", NULL);

                _exit(1);
        }
        dup2(tmpin, 0);
        dup2(tmpout, 1);
        close(tmpin);
        close(tmpout);
        int i;
        char c;
        char * str = new char[5000];
        for (i = 0; read(fdpipe[0], &c, 1) != 0; i++) {
                if (c == '\n') {
                        str[i] = ' ';
                }
                else {
                        str[i] = c;
                }
        }
	/*
	if (str[i] == '\0') {
		printf("last c is %c\n", str[i]);
	}
	printf("i is %d\n", i);*/
        i = strlen(str)-1;
        while (i >= 0) {
                unput(str[i]);
                i--;
        }
}

`[^\n`]*` {
	char * cmd = strdup(yytext + 1);
	if (cmd[yyleng - 2] == '`') {
		cmd[yyleng - 2] = '\0';
	}
	int tmpin = dup(0);
	int tmpout = dup(1);
	int cmdpipe[2];
	int fdpipe[2];
	pipe(cmdpipe);
	pipe(fdpipe);
	write(cmdpipe[1], cmd, strlen(cmd));
	write(cmdpipe[1], "\n", 1);
	close(cmdpipe[1]);
	dup2(cmdpipe[0], 0);
	close(cmdpipe[0]);
	dup2(fdpipe[1], 1);
	close(fdpipe[1]);
	int ret;
	ret = fork();
	if (ret < 0) {
		perror("fork");
		_exit(1);
	}
	if (ret == 0) {
		execvp("/proc/self/exe", NULL);

		_exit(1);
	}	
	dup2(tmpin, 0);
	dup2(tmpout, 1);
	close(tmpin);
	close(tmpout);
	int i;
	char c;
	char * str = new char[2000];
	for (i = 0; read(fdpipe[0], &c, 1) != 0; i++) {
		if (c == '\n') {
			str[i] = ' ';
		}
		else {
			str[i] = c;
		}
			}
	i = strlen(str)-1;	
	while (i >= 0) {
		unput(str[i]);
		i--;
	}

}

^source[ ][^ \t\n|><][^ \t\n|><]* {
	source_flag = true;
	char * str = strdup(yytext + 7);
	FILE * fd = fopen(str, "r");	
	
	//printf("buffer is %s\n", yyin);
	char cmdline[1024];
	yypush_buffer_state(yy_create_buffer(fd, YY_BUF_SIZE));
	yyparse();
	//while (fgets(cmdline, 1023, yyin)) {
	//	yyparse();
	//}
	yyrestart(fd);
	yypop_buffer_state();
	source_flag = false;
	if (isatty(0)) {
		printf(" > ");
		fflush(stdout);
	}
}

["][^\n\"]*["] {
	char * str = strdup(yytext+1);
	if (str[yyleng-2] == '"') {
		str[yyleng-2] = 0;
		yylval.cpp_string = new std::string(str);
		return WORD;
	}	

}

[^ \t\n|><&]*\\[^ \t\n]* {
	char * s = strdup(yytext);
	char * ret = new char[500];
	int i;
	for (i = 0; *s != 0; i++) {
		if (*s == '\\') {
			if (*(s+1) == '\\') {
				ret[i] = '\\';
				s = s + 2;
			}
			else {
				s++;
				ret[i] = *s;
				
			}
		}
		else {
			ret[i] = *s;
		}	
		s++;
	} 
	ret[i+1] = '\0';
	yylval.cpp_string = new std::string(ret);
	return WORD;

}


[^ \t\n|><][^ \t\n|><]*  {
	//printf("in last");
  /* Assume that file names have only alpha chars */
  	yylval.cpp_string = new std::string(yytext);
  	return WORD;
}
