/*********************************************************
 * my-grep.c 
 * 
 * Your Name: Jonah Tuckman 
 *********************************************************/


// Man Grep -> The grep utility searches any given input files, selecting lines that match one or more patterns.  By default,
//  a pattern matches an input line if the regular expression (RE) in the pattern matches the input line without
//  its trailing newline
//
//  1. Need to open a file and read it, similar to cat
// 	Man fopen -> fopen(const char * restrict path, const char * restrict mode);
//	"r" -> open for reading
// 2. Need to get individual lines from this file
//  	Man getline ->  The getdelim() function reads a line from stream
// 		example getline -> getline(&line, &length, file)
// 		&line is size_t length in documentation
// 3. Check first argument [argv[1], the word)  within each line 
// Documentation: Man strstr strstr(const char *haystack, const char *needle);
// 	example: if(strstr(sentence,word) != NULL)

// If passed no matching argument will return no lines as nothing is matched
//

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	size_t length = 0;
        char *str = NULL; // to be string 
	char *input = argv[1]; // first argument is the word 

	if (argc == 1) { //  if there are no arguments given other than the program name
        printf("my-grep: searchterm [file]\n");
        return 1; // Print statement and return 1 
	}

     //looping through multiple files
     for(int i = 2; i < argc; i++) { // Starting at 2 because there is the program name, the word, and then file 
	     FILE *file = fopen(argv[i], "r"); // second argument onward will be file, open this file and read
	     if (file == NULL) { //if file cannot be read then fopen is null
                printf("my-grep: cannot open file \n");
                return 1; // exit loop with return value of 1 
	     }
	     else {
	while(getline(&str, &length, file) > 0) { // if there is no line, getline returns 0
		if(strstr(str,input) != NULL) { // searches line to see if there are matches
		printf("%s", str);
		}
	
	}
	     
	fclose(file);	// close the file
     }
	}
return 0;

}
