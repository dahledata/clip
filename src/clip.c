/*
  Clip version 0.2.1
  Copyright 2001, 2007 Jan Kroken & Ole Dahle
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>. 
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define FALSE 0
#define TRUE  1

void printShortHelp() {
    fprintf(stderr,"Usage: clip [-b <bitrate>] offset length <infile> <outfile>\n");
 }

void printLongHelp() {
    fprintf(stderr,"Usage: clip [-b <bitrate>] offset[K|M|s] length[K|M|s]|\"end\" <infile> <outfile>\n");
    fprintf(stderr,"Description:\nClip clips out a number of bytes from the middle of a file.\n");
    fprintf(stderr,"\nParameters:\n");
    fprintf(stderr,"-b --bitrate     Optional: Specify the bitrate (Kilobits per second) Clip should use to calculate offset and length if they are set in seconds or the HH:MM:ss format. If not set, birate is assumed to be 128 (Kbps).\n");
    fprintf(stderr,"-h --help        Print this description of Clip.\n");
    fprintf(stderr,"offset           Length into the file to skip before starting the clip.\n");
    fprintf(stderr,"lenght           Lenght of the clip.\n");
    fprintf(stderr,"infile           File to clip from.\n");  
    fprintf(stderr,"outfile          File to store the clip in.\n");
 
    fprintf(stderr,"\nOffset and length may be specified as bytes (default), kilobytes (K), megabytes (M), seconds (s) or hours, minutes and seconds constant-bitrate audio (\"HH::MM::ss\").\n");
    fprintf(stderr,"Length may also be set to 0 or \"end\", which will cause the clip to clip to stretch to the end of the input file..\n\n");
    fprintf(stderr,"Example: clip 0:30 end input.mp3 output.mp3\n");
    fprintf(stderr,"Example: clip -b 256 4:00 3M input.mp3 output.mp3\n");
}

long parseParameter(char* parameter, int bitrate) {
    int strLength = strlen(parameter);
 
    if(parameter[strLength-1]== 'K') {
        parameter[strLength-1] = 0;
        return strtol(parameter, NULL, 10) * 1024;
    }
    else if(parameter[strLength-1]== 's') {
        parameter[strLength-1] = 0;
        return strtol(parameter, NULL, 10) * bitrate;
    }
    else if(parameter[strLength-1]== 'M') {
        parameter[strLength-1] = 0;
        /* 1 MB = 1048576 bytes */
        return strtol(parameter, NULL, 10) * 1048576;
    }
    else {
        int num_colons = 0;
        int colon[2] = {-1,-1};
        int i;
        char *part;
        long result = 0;
        for(i = 0; i < strLength; i++) {
            if(parameter[i] == ':') {
                colon[num_colons] = i;
                num_colons++;
            }
        }
        switch (num_colons) {
        case 0:
          /* Just a number of bytes */
            return strtol(parameter, NULL, 10);  
            break;
        case 1:
            part = &parameter[colon[0] + 1];
            result += strtol(part, NULL, 10) * bitrate; //seconds
            parameter[colon[0]] = 0;
            part = parameter;
            result += strtol(part, NULL, 10) * bitrate * 60; //minutes
            return result;
            break;
        case 2:
            part = &parameter[colon[1] + 1];
            result = strtol(part, NULL, 10) * bitrate; //seconds
            parameter[colon[1]] = 0;
            part = &parameter[colon[0] + 1];
            result += strtol(part, NULL, 10) * bitrate * 60; //minutes
            parameter[colon[0]] = 0;
            part = parameter;
            result += strtol(part, NULL, 10) * bitrate * 3600; //hours
            return result;
            break;
        default:
             fprintf(stderr,"Clip: Malformed format string.\n");
             exit(EXIT_FAILURE);
             break;
        }
    }
}

int main(int argc, char *args[])
{
  long offset, length, bytesCopied, bitrate;
  int bufArray[1024];
  FILE *input, *output;
  void *buf = bufArray;
  int c, argIndex, endMode, eofFound, statusCount;
  statusCount = 0;
  endMode = eofFound = FALSE;
  bytesCopied = 0;
  argIndex = 1;
  bitrate = 16384; /* 128 Kbps */
  
  if(argc == 2) {
      /* Print help info */ 
      printLongHelp();
      exit(EXIT_SUCCESS); 
  }
 
  if(argc < 5 || argc > 7)
  {
      /* Wrong number of arguments */
      fprintf(stderr,"Clip: Wrong number of arguments.\n");
      printShortHelp();
      exit(EXIT_FAILURE);
  }

  /* Bitrate */
  if (strcmp(args[1],"-b") == 0 ||
      strcmp(args[1],"--bitrate") == 0) {   
      bitrate = strtol(args[2], NULL, 10) * 1024 / 8; //Kbps -> bytes
      argIndex = 3;
  }

  /* Offset */
  offset = parseParameter(args[argIndex], bitrate);
  argIndex++;


  /* Length */
  if(strcmp(args[argIndex],"end") == 0) {
      length = 0;
  }
  else {
      length = parseParameter(args[argIndex], bitrate);
  }
  argIndex++;

  if (length == 0) {
      endMode = TRUE;
  }
  
  if (offset < 0) {
    fprintf(stderr,"Clip: Negative offset (%d bytes), quitting.\n",
            offset);
    exit(EXIT_FAILURE);   
  }

 if (length < 0) {
    fprintf(stderr,"Clip: Negative length (%d bytes), quitting.\n",
            length); 
    exit(EXIT_FAILURE);   
  }

  if(argc != argIndex + 2)
  {
      /* Wrong number of arguments */
      fprintf(stderr,"Clip: Wrong number of arguments.\n");
      printShortHelp();
      exit(EXIT_FAILURE);
  }

  input  = fopen(args[argIndex], "r");
  argIndex++;
  output = fopen(args[argIndex], "w+");

  if(!input) {
      fprintf(stderr,"Unable to open input file %s\n", args[argIndex - 1]);
      exit(EXIT_FAILURE);
  }

  if(!output) {
      fprintf(stderr,"Unable to open output file %s\n", args[argIndex]);
      exit(EXIT_FAILURE);
  }
 
  /* Skip past the first part */
  printf("Skipping: ...\n");
  fflush(stdout);
  fseek(input, offset, SEEK_SET);

  printf("Copying: .");
  fflush(stdout);

  /* Copy a KB at a time, if we can */
  while(bytesCopied < length - 1024 || (endMode && !eofFound)) {
      size_t bytesRead = fread(buf, 1, 1024, input);
      statusCount++;
      /* Hearbeat */
      if (statusCount == 1000) {
          statusCount = 0;
          printf(".");
          fflush(stdout);
      }
           
      if (bytesRead < 1024) {
          if (feof(input)) {
              //fprintf(stderr,"reached EOF\n");
              eofFound = TRUE;
          }
          else if (ferror(input)) {
              fprintf(stderr,"Clip: Error no. %d during read, quitting.\n",
                      ferror(input));
              fclose(input);
              fclose(output);
              exit(EXIT_FAILURE);
          }
      }
      if (fwrite(buf, bytesRead, 1, output) < 1) {
          fprintf(stderr,"Clip: Error no. %d during write, quitting.\n",
                  ferror(output));
          fclose(input);
          fclose(output);
          exit(EXIT_FAILURE);  
      }
      //fprintf(stderr,"BytesCopied is %d.\n", bytesCopied);
      bytesCopied += bytesRead;
      //fprintf(stderr,"*");
      if (bytesRead < 1024) {
          //fprintf(stderr,"Got only %d bytes\n", bytesRead);
          break;
      }
  }
  //fprintf(stderr,"Only %d bytes left.\n", length - bytesCopied);
  //fprintf(stderr,"BytesCopied is %d.\n", bytesCopied);
  /* Copy the last part bytewise */
  while(bytesCopied < length) {
      c = fgetc(input);
      if (c == EOF)
      {
          if(ferror(input)) {
              fprintf(stderr,"Clip: Input too short, quitting.\n");
              fclose(input);
              fclose(output);
              exit(EXIT_FAILURE);
          }
          else {
              eofFound = TRUE;
          }
      }
      //fprintf(stderr,".");
      fputc(c, output);
      bytesCopied++;
      if (eofFound) {
          break;
      }
  }

  printf("\n");
  fclose(input);
  fclose(output);
  exit(EXIT_SUCCESS);
}
  
	  

  
