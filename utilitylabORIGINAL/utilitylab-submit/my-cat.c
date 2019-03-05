/*********************************************************
 * my-cat.c 
 * 
 * Your Name: Jonah Tuckman
 *********************************************************/

//  man page: The cat utility reads files sequentially, writing them to the standard output
// Need to first read the files, then write them out into standard output
//  1. Need to open file from input string -> fopen().
// man page Man fopen -> fopen(const char * restrict path, const char * restrict mode);
//      "r" -> open for reading
// 2.  Once file is open we need to read the file. Built in function to read a line of a file. -> readline 
// 3. Once file is read, we then need to write it to standard output.
// 	we can do this simply using printf
// 4. Once the whole file is read and written we need to close this file. fclose formerly fopened file 
//

#include <stdio.h> // for opening closing and reading file
//#include <string.h>


int main(int argc, char *argv[]) {
 	FILE *filereader; // file to be opened with argv arguments
	char *str;  // char arrays to be printed when reading lines
	size_t length = 0; // to be used in get line 

	if(argc == 1) {
		return 0; // if the only argument is the program name then exit and return 1
	}
	for (int i = 1; i < argc; i++){ // loop through the input string  starting at 1 which will be the file name

		filereader = fopen(argv[i], "r"); // read the first file given as input (can be multiple) with create flag.

	if (filereader == NULL) { //if file cannot be read then fopen is null 
		printf("my-cat: cannot open file \n");
		return 1; // exit loop with return value of 1
	}
	while (getline(&str, &length,  filereader) > 0) { // We are indicating to read one byte at a time into readbyte
		//read returns number of bytes read. 
		// This while loop says while we are still reading bytes, thus there is still info to be read
		// printf("%d", readbyte); // prints bytes not characters, not what we want
		//write(fileno(stdout), &readbyte, 1 ); // write the written byte into standard output
		printf("%s",str);
	} 
	fclose(filereader); // while loop has terminated
	     // do not want to return here because need to continue loop for more arguments 
	}
	return 0; 
}
