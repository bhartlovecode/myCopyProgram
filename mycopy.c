/*
 * Author: Hartlove, Bradley
 * Version: 1.0, July 3, 2021
 * Description: This program operates from the terminal, allowing the user to input 
 * as follows: ./mycopy <sourcefile> <destinationfile>. The bytes from said source
 * file will be read and written into the destination file as so. There are error 
 * checks for non-matching file extensions, improper permissions, and all read,
 * write, and opens within the program. All of this is done using system calls
 * instead of functions such as fgets() for file manipulation.
*/

//header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv) {
    //open the source file using open() or exit with an error message
    
    //Case: Incorrect number of arguments (format: mycopy <fileToCopy> <destinationFile>)
    if(argc != 3){
        printf("\nmycopy: invalid number of arguments");
        printf("\nusage: mycopy <sourcefile> <destinationfile>\n");
        exit(1);
    }

    //Case: Valid number of arguments
    else{
        char * fileName = argv[1]; //holds source file

        //file opened for read only
        int file = open(fileName, O_RDONLY, 0); 

        //error code
        if(file < 0) {
            fprintf(stderr, "Unable to open %s:  %s\n", fileName, strerror(errno));
            exit(1);
        }

        //open file was successful
        else{
            printf("\nOpening file successful...\n");

            //create the destination file using creat() of exit with an error message
            char * destFile = argv[2];

            //error creating file
            if(creat(destFile, O_CREAT| S_IRWXU | S_IRWXO) < 0){
                fprintf(stderr, "Unable to create file %s: %s\n", destFile, strerror(errno));
                exit(1);
            }

            //file was created/accessed successfully
            else{
                
                //determine if file extensions are equal, exit with code (1) if not
                //src ext
                int srcLastDot = 0; //holds position of last period
                int srcExtLength = 0; //holds length of extension
                for(int i = 0; fileName[i] != '\0'; i++) {
                    if(fileName[i] == '.'){
                        srcLastDot = i;
                        srcExtLength = 0;
                    }

                    else{
                        srcExtLength++;
                    }
                }

                char srcExt[srcExtLength + 1]; //holds extension "string"
                srcExt[srcExtLength] = '\0'; //sets escape character at the end 
                int srcExtPos = 0; //tracks the position within our "string"
                for(int x = srcLastDot + 1; fileName[x] != '\0'; x++){
                    srcExt[srcExtPos] = fileName[x];
                    srcExtPos++;
                }

                printf("\nSrc Ext: %s", srcExt);

                //dest ext
                int destLastDot = 0; //holds position of last period
                int destExtLength = 0; //holds length of extension
                for(int i = 0; destFile[i] != '\0'; i++) {
                    if(destFile[i] == '.'){
                        destLastDot = i;
                        destExtLength = 0;
                    }

                    else{
                        destExtLength++;
                    }
                }

                char destExt[destExtLength + 1]; //"string" for extension
                destExt[destExtLength] = '\0'; //escape character at the end
                int destExtPos = 0; //position in "string"
                for(int x = destLastDot + 1; destFile[x] != '\0'; x++){
                    destExt[destExtPos] = destFile[x];
                    destExtPos++;
                }
                
                printf("\nDest Ext: %s", destExt);

                //compare the two extensions
                if(strcmp(srcExt, destExt) != 0){
                    printf("\nmycopy error: Extensions not equal\n");
                    exit(1);
                }

                //opens dest file for writing
                int writeFile = open(destFile, O_RDWR | O_APPEND);

                char buff[2097152]; //temporary buffer storage (~ 2MB to ensure copy speeds for large files are moderate)
                int count = 2097152; //count for blocks of data
                int readReturnVal = 0; //keeps track of read return value
                int totalBytes = 0; //keeps track of total bytes read
                int eof = 0; //keeps track of whether file end was reached
                int error = 0; //keeps track of negative return value as bool

                //main loop for reading data
                 /* loop {
                *      read() a chunk of data from the source file
                *          if there was an error reading data, exit with error message
                *          if there are is no more data left to read, exit loop but continue
                *      write() that same chunk of data to the destination file
                *          if not all data was written, exit the program with an error
                *          message
                *      }
                */
               printf("\nReading file...\n");

               //while not at the end, and no error is encountered
                while(eof == 0 && error == 0){
                    readReturnVal = read(file, buff, count);

                    //error reading the input file
                    if(readReturnVal == -1){
                        fprintf(stderr, "Unable to read file %s: %s\n", fileName, strerror(errno));
                        error = 1;
                        exit(1);
                    }

                    else{
                        //read and write permissions, appends to end of file
                        writeFile = open(destFile, O_RDWR | O_APPEND);

                        //error writing to destination file
                        if(writeFile == -1){
                            fprintf(stderr, "Unable to write to file %s: %s\n", destFile, strerror(errno));
                        }

                        else{
                            if(readReturnVal == 0){
                                write(writeFile, buff, readReturnVal);
                                totalBytes += readReturnVal;
                                printf("\rWriting %d bytes...", totalBytes);
                                fflush(stdout);
                                eof = 1;
                            }

                            else{
                                write(writeFile, buff, readReturnVal);
                                totalBytes += readReturnVal;
                                printf("\rWriting %d bytes...", totalBytes);
                                fflush(stdout);
                            }

                        }
                        
                        close(writeFile);
                    }
                }

                printf("\nSuccessfully wrote %d bytes from %s to %s\n", totalBytes, fileName, destFile);
                
                printf("\nReading destination file...");
                writeFile = open(destFile, O_RDONLY);
                //checking that all data was copied correctly
                int writtenBytes = 0; //holds all written bits
                int reachedEndOfFile = 0; //reached of file or not
                int errorReading = 0; //error reading destination file
                int checkReturnVal = 0; //holds read return value
                char checkBuff[2097152]; //temp memory for chunks of data
                while(reachedEndOfFile == 0 && errorReading == 0){
                    checkReturnVal = read(writeFile, checkBuff, count);

                    //error reading destination file
                    if(checkReturnVal == -1){
                        fprintf(stderr, "Unable to read destination file %s: %s\n", destFile, strerror(errno));
                        errorReading = 1;
                        exit(1);
                    }

                    else{
                        if(checkReturnVal == 0){
                            writtenBytes += checkReturnVal;
                            reachedEndOfFile = 1;
                        }

                        else{
                            writtenBytes += checkReturnVal;
                        }
                    }

                    
                }

                //if number of bytes written to file is correct
                if(writtenBytes == totalBytes){
                    printf("\nByte count verified...\n");
                }

                //incorrect number of byts written
                else{
                    printf("\nByte counts do not match: %d %d...", writtenBytes, totalBytes);
                    exit(1);
                }

                
                //close() both the source and destination files
                close(file);
                close(writeFile);


            }

            //exit
            return 0;
        }
   
    }

 

}
