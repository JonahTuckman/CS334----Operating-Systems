/*********************************************************
 * my-unzip.c
 * 
 * Your Name: Jonah Tuckman 
 *********************************************************/

// Unzip
// man unzip ->  unzip - list, test and extract compressed files in a ZIP archive
// Will use file zipped of 10a4b from my-zip
// 1. open and read file
// 2. create variables to hold character read and integer indicating number of times it was read
//	using fread(). man fread() ->  fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
//	man (cont) ->  The function fread() reads nitems objects, each size bytes long, from the stream pointed to by stream, storin them at the location given by ptr.
//3. Loop through this file and assign these variables every 4 bytes (one integer)
// 4. Loop count # of times and  Print out this character 

#include <stdio.h> // fopen, sizeof

int main(int argc, char *argv[]) {
  
	int count;
	char chary;
	int i;
	
	if(argc < 2) { // if the only argument is the program name (thus there is no file)
		printf("my-unzip: file1 [file2 …] \n");
		return 1;
	}


	FILE *file = fopen(argv[1], "r"); // open file in read only form
	
	if (file == NULL) { //if file cannot be read then fopen is null
                printf("my-unzip: cannot open file \n");
                return 1; // exit loop with return value of 1 
             }

	while(fread(&count, sizeof(int), 1, file)) { // count is the first four bytes (one integer number)
		fread(&chary, sizeof(char), 1, file); // chary is the next character (1 byte char if zipper file)
		for (i = 0; i < count; i++){ 
		printf("%c",chary); // print character count amount of times
		}
	}
	fclose(file);
	printf("\n"); // adding line break for command line readability

}

