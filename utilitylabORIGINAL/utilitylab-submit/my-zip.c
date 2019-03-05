/*********************************************************
 * my-zip.c 
 * 
 * Your Name: Jonah Tuckman
 *********************************************************/

// Zip 
// man zip ->  zip is a compression and file packaging utility for Unix,
// examples on man page -> zip -r foo.zip foo
// EXAMPLE:  aaaaaaaaaabbbb -> 10a4b 
//
// need fwrite(). man fwrite -> The function fwrite() writes nitems objects, each size bytes long, to the stream pointed to by stream, obtaining them from the location given by ptr.
//  fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
// 1. need to open a file and then read it
// 	man fgetc -> The fgetc() function obtains the next input character (if present) from the stream pointed at by stream, or 		the next character pushed back on the stream via ungetc(3).
// 	this is better than getline for this because it focuses on characters rather than full lines which would need to be broken up again
// 2. need to loop through characters output by fgetc and  count reoccuring characters
// 3. Need to print this count of reocurring characters alongside the character
//
// Functional when writing to a file using '>' notation


#include <stdio.h> // for fwrite(), fgetc, fopen, sizeof

int main(int argc, char *argv[]) {

	int count = 1; // will be used for counting number of characters in a row
	int chary; // hunsberger variable naming method (ie. listy), will hold characters in it as we loop through
	int temp; // temporary compared character
	int notfirstCycle = 0; // to be incrementd as cycles increase, use to store original temp value
 	
	if(argc < 2) { // there is no file attatched, only a program name
		printf("my-zip: file1 [file2 …]  \n ");
		return 1;
	}
		
	FILE *file = fopen(argv[1], "r");

	while((chary = fgetc(file)) !=  EOF) {
		if (!notfirstCycle)  {// this tests if a loop has been run, if first loop this is 0 thus !firstcycle is 1
			notfirstCycle++; // stating that first cycle has been run
			temp = chary; // saving the first character 
		} 
		else if (notfirstCycle)  {
		// not first cycle, check if temp from first cycle == new chary value
			if(temp == chary){
				count++; // counting consecutive repetitions of characters
			}

			else {
				//printf("%d",count); // print the count before the character
				// Above commented line prints out 10a4b for input file from website
				
				//want binary format of word so need fwrite
				// this fassion prints out ^@^@^@a^D^@^@^@b which can then be read by the unzip converter
				fwrite(&count, sizeof(int), 1, stdout);
				fwrite(&temp, sizeof(char), 1, stdout); // write the repreated char to standard out
				// must use fwrite rather than print because is currently a pointer to the char
				count = 1; // reset the counter for next char
				temp = chary; // new temp is the new char
			}
		}
	}
	printf("\n");
	fclose(file);

}
