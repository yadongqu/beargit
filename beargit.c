#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);
  
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");

  return 0;
}


/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}


/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");
  char line[FILENAME_SIZE];
  int old_lines = 0;
  int new_lines = 0;

  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    old_lines += 1;
    if (strcmp(line, filename) != 0) {
      fprintf(fnewindex, "%s\n", line);
      new_lines += 1;
    }
  }
  fclose(findex);
  fclose(fnewindex);
  if(old_lines == new_lines){
    fs_rm(".beargit/.newindex");
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    return 1;
  } else {
    fs_mv(".beargit/.newindex", ".beargit/.index");
    return 0;
  }
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";

int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */
  if(strstr(msg, go_bears) != NULL){
      return 0;
  }
  else {
      return 1;
  }
  

}

// helper function to get the right commit_id format
int power(int base, unsigned int exp) {
    int i, result = 1;
    for (i = 0; i < exp; i++)
        result *= base;
    return result;
 }

char * toId(int num){
    char* result = malloc(41);
    int i = 39;
    for(; i >=0; i--,num/=3){
       result[i] = "61c"[num % 3];
    }
    result[40] = '\0';
    return (char *)result;
}

int toNum(char* id){
    int result = 0;
    for(int i = 0; i < 40; i++){
        if(id[i] == 'c'){
            result += 2 * (power(3, 39-i));
        } else if(id[i] == '1'){
            result += power(3, 39-i);
        }
    }
    return result;
}

//helper function to cancat string
char* concat(const char * s1, const char *s2)
{
   char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void next_commit_id(char* commit_id) {
  /* COMPLETE THE REST */
  char *new_id;
  if(commit_id[0] == '0'){
    new_id = toId(0);
  } else {
    int num = toNum(commit_id);
    new_id = toId(num+1);
  }
  for(int i = 0; i < COMMIT_ID_SIZE; i++){
    *commit_id++ = *new_id++;
  }
}


int beargit_commit(const char* msg) {
  if (is_commit_msg_ok(msg) != 0) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  /* COMPLETE THE REST */
  char* dir_name = concat(".beargit/", commit_id);
  fs_mkdir(dir_name);
  fs_cp(".beargit/.index", concat(dir_name, "/.index"));
  fs_cp(".beargit/.prev", concat(dir_name, "/.prev"));
  
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fs_cp(line, concat(dir_name, concat("/", line)));
  }
  fclose(findex);
  FILE* msgFile = fopen(concat(dir_name, "/.msg"), "w");
  fprintf(msgFile, "%s\n", msg);
  fclose(msgFile);

  write_string_to_file(".beargit/.prev", commit_id);

  return 0;

}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  char file_name[FILENAME_SIZE];
  int lines = 0;
  printf("Tracked files:\n\n");
  while(fgets(file_name, sizeof(file_name), findex)){ 
    printf("  %s", file_name);
    lines += 1;
  }
  printf("\n%d files total\n", lines);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 *
 */

int beargit_log() {
  /* COMPLETE THE REST */
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  if (commit_id[0] == '0'){
    fprintf(stderr, "ERROR: There are no commits!\n");
    return 1;
  } else {
    int num = toNum(commit_id);
    char* id = commit_id;
  while(num >= 0){
    printf("\ncommit %s\n", id);
    char* dir_name = concat(".beargit/", id);
    char* file_name = concat(dir_name, "/.msg");
    char msg[MSG_SIZE] = {0};
    read_string_from_file(file_name, msg, MSG_SIZE);
    printf("    %s", msg);
    num -= 1;
    id = toId(num);
  }
  printf("\n");
  return 0;
  }
  
}
