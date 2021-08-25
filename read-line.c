/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_LINE 2048

extern void tty_raw_mode(void);
extern void tty_reset(void);
// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];
// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [1024];
int history_length = 0;
// asdas
void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  int cursor = 0;

	
  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32 && ch != 127) {
      // It is a printable character. 

      if (line_length == cursor) {	
      // Do echo
      	write(1,&ch,1);

      // If max number of character reached return.
      	if (line_length==MAX_BUFFER_LINE-2) break; 

      // add char to buffer.
      	line_buffer[line_length]=ch;
      	line_length++;
      	cursor++;
      }
      else {
	      int i;
	      for (i = line_length; i > cursor; i--) {
			line_buffer[i] = line_buffer[i-1];
	      }
	      line_buffer[cursor] = ch;
	      line_buffer[++line_length] = 0;
	      write(1, &ch, 1);
	      for (i = cursor+1; i < line_length; i++) {
			write(1, &line_buffer[i], 1); 

	      }
	      for (i = 0; i < line_length - cursor - 1; i++) {
                        ch = 8;
                        write(1,&ch,1);
              }
	      cursor++;
		
      }

    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      
      // Print newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    }
    else if ((ch == 8 || ch == 127) && line_length > 0 ) {
      // <backspace> was typed. Remove previous character read.
      // Go back one character
      if (cursor == line_length) {
      ch = 8;
      write(1,&ch,1);

      // Write a space to erase the last character read
      ch = ' ';
      write(1,&ch,1);

      // Go back one character
      ch = 8;
      write(1,&ch,1);

      // Remove one character from buffer
      line_length--;
      cursor--;
      }
      else if (line_length > 0 && cursor > 0){
	      ch = 8;
	      write(1, &ch, 1);
	      int i;
	      for (i = 0; i < line_length - cursor; i++) {
		      write(1, &line_buffer[i + cursor], 1);
		      line_buffer[i + cursor - 1] = line_buffer[i + cursor];
	      }
	      line_buffer[--line_length] = '\0';
	      ch = ' ';
	      write(1, &ch, 1);
	      ch = 8;
	      write(1,&ch,1);
	      cursor--;
	      for (i = 0; i < line_length - cursor; i++) {
			ch = 8;
              		write(1,&ch,1);
	      }
      }
    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      ///
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if (ch1==91 && ch2==65) {
	// Up arrow. Print next line in history.
	// Erase old line
	// Print backspaces
	int i = 0;
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}

	// Print spaces on top
	for (i =0; i < line_length; i++) {
	  ch = ' ';
	  write(1,&ch,1);
	}

	// Print backspaces
	for (i =0; i < line_length; i++) {
	  ch = 8;
	  write(1,&ch,1);
	}	
	
	// Copy line from history
	if (history_index >= 0 && history_length >=1) {
		strcpy(line_buffer, history[history_index]);
		line_length = strlen(line_buffer);
		history_index--;
		//history_index=(history_index)%history_length;
		if (history_index < 0) {
			history_index = history_length - 1;
		}
		cursor = line_length;
	}
	// echo line
	write(1, line_buffer, line_length);
      }
      else if (ch1 == 91 && ch2 == 66) {
		//Down arrow
	int i = 0;
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }

        // Print spaces on top
        for (i =0; i < line_length; i++) {
          ch = ' ';
          write(1,&ch,1);
        }

        // Print backspaces
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }

        // Copy line from history
	char temp[1024];
        strcpy(temp, line_buffer);
        if (history_index + 2< history_length && history_length > 0) {
                strcpy(line_buffer, history[history_index+2]);
		            history_index++;
        } 
	else if (history_index == history_length - 1) {
		strcpy(line_buffer, history[1]);
		history_index = 0;
	}
	else if (history_index + 2 == history_length) {
			strcpy(line_buffer, temp);
	}
		
	line_length = strlen(line_buffer);
	 cursor = line_length;
        // echo line
        write(1, line_buffer, line_length);
      }	
      else if (ch1 == 91 && ch2 == 67) {
		//Right arrow
		if (cursor < line_length) {
			ch = 27;
			write(1, &ch, 1);
			ch = 91;
			write(1, &ch, 1);
			ch = 67;
			write(1, &ch, 1);
			cursor++;
		}

      }
      else if (ch1 == 91 && ch2 == 68) {
		//Left arrow
                if (cursor > 0) { 
                        ch = 27;
                        write(1, &ch, 1);
                        ch = 91;
                        write(1, &ch, 1);
                        ch = 68;
                        write(1, &ch, 1);
                        cursor--;
                }
      }

      
    }
    else if (ch == 1) {
	    //ctrl-A
		while (cursor > 0) {
			ch = 27;
                        write(1, &ch, 1);
                        ch = 91;
                        write(1, &ch, 1);
                        ch = 68;
                        write(1, &ch, 1);
                        cursor--;	
		}	
    }
    else if (ch == 5) {
	//ctrl-E
		while(cursor != line_length) {
			ch = 27;
                        write(1, &ch, 1);
                        ch = 91;
                        write(1, &ch, 1);
                        ch = 67;
                        write(1, &ch, 1);
                        cursor++;
		}

    }    
    else if (ch == 4) {
	    int i;
	    for (i = 1; i < line_length - cursor + 1; i++) { 
		      write(1, &line_buffer[i+cursor], 1);
                      line_buffer[i + cursor - 1] = line_buffer[i + cursor];
            }
	    line_buffer[--line_length] = '\0';
	    //printf("cursor is %d\n", cursor);
	    //printf("length is %d\n", line_length);
	    //printf("buffer is %s\n", line_buffer);
	    ch = ' ';
            write(1, &ch, 1);
            ch = 8;
            write(1,&ch,1);
	    for (i = 0; i < line_length - cursor; i++) {
                        ch = 8;
                        write(1,&ch,1);
            }

    }
  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;
  // history implentaiton
  history[history_length] = (char *)malloc(strlen(line_buffer) * sizeof(char) + 1);
  strcpy(history[history_length], line_buffer);
  history[history_length][strlen(line_buffer)-1] = '\0';
  history_index = history_length;
  history_length++;
  tty_reset();
  return line_buffer;
}
