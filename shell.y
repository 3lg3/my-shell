
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <string.h>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE AND PIPE LESS GREATGREAT GREATAND GREATGREATAND TWOGREAT SOURCE

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"

void yyerror(const char * s);
int yylex();


//wildcards function
void expandWildcardsIfNecessary(std::string  * argument);
void expandWildcards(char * prefix, char * suffix);
int myCompare(const void * a, const void *b);
void sort(char ** arr, int n);



%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
       ;

simple_command:	
  pipe_list iomodifier_list background_opt NEWLINE {
   // printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
	expandWildcardsIfNecessary( $1 );
  }
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

pipe_list:
         pipe_list PIPE command_and_args
         | command_and_args
         ;

iomodifier_opt:
  	GREAT WORD {
    		//printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    		Shell::_currentCommand._outFile = $2;
		Shell::_currentCommand._outCount++;
	} 
	| GREATGREAT WORD {
		//printf("   Yacc: insert output \"%s\"\n", $2->c_str());
		Shell::_currentCommand._outFile = $2;
		Shell::_currentCommand._append = true;
		Shell::_currentCommand._outCount++;	
	} 
	| LESS WORD {
		//printf("   Yacc: insert output \"%s\"\n", $2->c_str());
		Shell::_currentCommand._inFile = $2;
		Shell::_currentCommand._inCount++;
	} 
	| GREATAND WORD {
		//printf("   Yacc: insert output \"%s\"\n", $2->c_str());
		Shell::_currentCommand._outFile = $2;
		Shell::_currentCommand._errFile = $2;
		Shell::_currentCommand._outCount++;
	} 
	| GREATGREATAND WORD {
	 	//printf("   Yacc: insert output \"%s\"\n", $2->c_str());
		Shell::_currentCommand._outFile = $2;
		Shell::_currentCommand._errFile = $2;
		Shell::_currentCommand._append = true;
		Shell::_currentCommand._outCount++;
	} 
	| TWOGREAT WORD {
		Shell::_currentCommand._errFile = $2;
		Shell::_currentCommand._outCount++;
	}
	| SOURCE {

	}
	;

iomodifier_list:
	iomodifier_list iomodifier_opt
	| iomodifier_opt
	|
	;

background_opt:
	AND {
		Shell::_currentCommand._background = true;	
	}
	|
	;
%%

int maxEntries = 20;
int nEntries = 0;
char ** array;
int match;

void expandWildcardsIfNecessary(std::string  * argument) {
	array = (char **) malloc (maxEntries * sizeof(char *));
	char * arg = strdup(argument->c_str());
	if (strchr(arg, '{') && strchr(arg, '}')) {
		Command::_currentSimpleCommand->insertArgument(argument);
                return;
	}
        if (!strchr(arg, '*') && !strchr(arg, '?')) {
                Command::_currentSimpleCommand->insertArgument(argument);
                return;
        }
	else {
		match = 0;
		expandWildcards(NULL, arg);
		int i;
		/*
		if (!match_flag) {
			Command::_currentSimpleCommand->insertArgument(argument);
                	return;
		}*/
		if (!match) {
			Command::_currentSimpleCommand->insertArgument(argument);
			return;
		}
		sort(array, nEntries);
		for (i = 0; i < nEntries; i++) {
			if (array[i]) {
				Command::_currentSimpleCommand->insertArgument(new std::string(array[i]));
				//printf("insertArgument %s\n", array[i]);
			}
		}
		free(array);
        	maxEntries = 20;
        	nEntries = 0;
        	//printf("Successfully end!!\n");	
		return;
	}


}


void expandWildcards(char * prefix, char * suffix) {
	char * mark = suffix;
	if (suffix[0] == 0) {
		match = 1;
		Command::_currentSimpleCommand->insertArgument(new std::string(prefix));
		return;
	}
	char component[1024];
	int flagx = 0;
	if (suffix[0] == '/') {
		//printf("suffix[0] is / \n");
		component[0] = '/';
		suffix++;
		component[1] = '\0';
		flagx = 1;
	}
	char * s = strchr(suffix, '/');
	if (s) {
		if (flagx) {
			strncpy(component+1, suffix, s-suffix);
		}
		else {
			strncpy(component, suffix, s-suffix);
		}
		suffix = s + 1;
	}
	else {
		strcpy(component, suffix);
		suffix = suffix + strlen(suffix);
	}
	//printf("component is %s\n", component);	
	char newPrefix[1024];
	if (!strchr(component, '*') && !strchr(component, '?')) {
		if (prefix) {
		sprintf(newPrefix, "%s/%s", prefix, component);
		}
		else {
			sprintf(newPrefix, "%s", component);
		}
		//printf("first newPrefix is %s\n", newPrefix);	
		//printf("first new suffix is %s\n", suffix);
		//if ( s != NULL) {
		expandWildcards(newPrefix, suffix);
		//}
	}
	else {
        	char * reg = (char *) malloc(2 * strlen(suffix) + 10);
        	char * a = component;
		if (!prefix && mark[0] == '/') {
                	prefix = strdup("/");
                      	a = a + 1;
		}
		char * b = component;
        	char * r = reg;
		char * normal = (char *) malloc(2 * strlen(suffix) + 10);
		char * n = normal;
        	*r = '^';
        	r++;
		*n = '^';
		n++;
		while (*b) {
			*n = *b;
			n++;
			b++;
		}
		*n = '$';
		n++;
		*n = '\0';
        	while (*a) {
            		if (*a == '*') {
                		*r = '.';
                		r++;
                		*r = '*';
                		r++;
            		}
            		else if (*a == '?') {
                		*r = '.';
                		r++;
            		}
            		else if (*a == '.') {
                		*r = '\\';
                		r++;
                		*r = '.';
                		r++;
            		}
            		else {
                		*r = *a;
                		r++;
            		}
            		a++;
        	}	
        	*r = '$';
        	r++;
        	*r = '\0';
		//printf("reg is %s\n ", reg);
		regex_t preg;
        	int expbuf = regcomp(&preg, reg, REG_EXTENDED|REG_NOSUB);
		if (expbuf != 0) {
			return;
		}
		if (strchr(mark, '?') && strchr(mark, '.') && strchr(mark, 'l')) {
			return;
		}
		DIR * dir;
		//printf("prefix is %s \n", prefix);
		if (!prefix) {
        		dir = opendir(".");
		}
		else {
			dir = opendir(prefix);
		}
		
        	if (dir == NULL) {
            	//	perror("opendir");
            		return;
        	}
		//printf("next\n");
        	struct dirent * ent;
        	regmatch_t pmatch;
        	while ( (ent = readdir(dir)) != NULL) {
            		if (!regexec(&preg, ent->d_name, 1, &pmatch, 0)) {
				//printf("match!!\n");
				/* subdirectory	 */
				if (s != NULL) {
					//printf("right in subdirectory");
					//printf("previous prefix is %s\n", prefix);
					if (!prefix)  {
						sprintf(newPrefix, "%s", ent->d_name);
					}
					else if (!strcmp(prefix,"/")) {
						sprintf(newPrefix, "/%s",ent->d_name);

					}
					else {
						sprintf(newPrefix, "%s/%s", prefix, ent->d_name);
					}
					//printf("new prefix is %s\n", newPrefix);
				
					expandWildcards(newPrefix, suffix);
					
				}
				/*  *s = 0 */
				else {
					match = 1;
					//printf("not include subdirectory!\n");	
					if (nEntries == maxEntries) {
						maxEntries = maxEntries * 2;
						array = (char **)realloc(array, maxEntries*sizeof(char*));
					}
					char _arg[1024];
					_arg[0] = '\0';
					int flag = 0;
					if (prefix) {
						sprintf(_arg, "%s/%s", prefix, ent->d_name);
						flag = 1;
					}
					//printf("ent->dname is %s\n", ent->d_name);
					if (ent->d_name[0] == '.') {
						if (component[0] == '.') {
							//printf("flag is %d\n", flag);
							if (flag == 1) {
								array[nEntries++] = strdup(_arg);
							}
							else {
							//	printf("entdnmae is storing\n");
							//	printf("array inserting %s\n", ent->d_name);
								array[nEntries++] = strdup(ent->d_name);
							}
						}
					}
					else {
						if (flag == 1) {
                                                	array[nEntries++] = strdup(_arg);
                                                }
						
                                              	else {
							//printf("array inserting!\n");
                                                	array[nEntries++] = strdup(ent->d_name);
                                                }

					}
	
					
					
				}



			}


		}
		closedir(dir);
	}
	


}

int myCompare(const void * a, const void *b) {
	return strcmp(*(const char **) a, *(const char **) b);

}
void sort(char ** arr, int n) {
	qsort(arr, n, sizeof(char *), myCompare);
}


void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
