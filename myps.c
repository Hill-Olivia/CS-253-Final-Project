/* 
 * Files: [myps.c], ProcEntry.h, ProcEntry.c
 * Author: Olivia Hill
 * Description: This program finds and displays statistics found in proc files. 
 *     This is accomplished by accessing '/proc' or a directory given by the user
 *     with the -d flag. Other flags include -p to sort by PID, -c to sort by
 *     Comm, and -z to display only those in the zombie state.
 * Date: Tue Aug  8 16:17:41 MDT 2023
 * Module 6 Final Project
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include "ProcEntry.h"

#define MAX_DIR_LEN 4096
#define MAX_STR_SIZE 100
#define UNUSED(x) (void)x
typedef struct dirent Dirent;

// Functions
/*
 * The DirFilter functions goes through all files of the directory
 *     given to it and returns 1, in order to give the directory 
 *     to the list of directories that are directories and their
 *     names are numbers.
 * Parameter: A pointer to the current directory object used by scandir
 * Returns: 1 for correct directory, and 0 for incorrect
 */
int DirFilter(const Dirent *current);
/*
 * The NoSort function is passed to the scandir function
 *     in order to satisfy the requirement for the function
 *     without actually sorting anything.
 * Parameters: Two Dirent pointer entries
 * Returns: 0
 */
int NoSort(const Dirent **entryA, const Dirent **entryB);
/*
 * The PidSort functions sorts the ProcEntries list by the
 *     PID member variable.
 * Parameters: Two ProcEntry pointers
 * Returns: Positive integer if a is bigger than b, 0 if they
 *     are equal, and a negative integer if a is less than b.
 */
static int PidSort(const void *a, const void *b);
/*
 * The CommSort function sorts the ProcEntry objects alphabetically 
 *     by their comm member variable.
 * Parameters: Two ProcEntry pointers
 * Returns: A positive integer if a is before b, zero if they are
 *     equal, and a negative integer if a is after b
 */
static int CommSort(const void *a, const void *b);
/*
 * The PrintProcs function prints the header information and 
 *     calls PrintProcEntry for each object, which outputs 
 *     all of the statistics
 * Parameters: A pointer to the list of ProcEntry pointers,
 *     the number of objects in the list, the zombie flag
 *     which indicates whether or not to print only those
 *     in the zombie state.
 * Returns: void
 */
void PrintProcs(ProcEntry **procs, int count, int zFlag);

int main (int argc, char * argv[]) {
    // Initializes variables
    char dirPath[MAX_DIR_LEN];
    strcpy(dirPath, "/proc");
    int (*sortFunctionPtr) (const void *a, const void *b) = &PidSort;
    int zombieFlag = false;
    int opt;
    Dirent **dirObjects;
    int num;
    ProcEntry **myProcs;
    ProcEntry *tempProcPtr;

    // Parse user input
    while( (opt = getopt(argc, argv, "d:pczh")) != -1) {
        switch (opt) {
            case 'd':
                strncpy(dirPath, optarg, MAX_DIR_LEN);
                break;
            case 'p':
                // Pid sort is default, no action needed
                break;
            case 'c':
                sortFunctionPtr = &CommSort;
                break;
            case 'z':
                zombieFlag = true;
                break;
            case 'h':
                fprintf(stderr, "Usage: ./myps [-d <path>] [-p] [-c] [-z] [-h]\n");
                fprintf(stderr, "\t-d <path> Directory containing proc entries (default: /proc)\n");
                fprintf(stderr, "\t-p        Display proc entries sorted by pid (default)\n");
                fprintf(stderr, "\t-c        Display proc entries sorted by command lexicographically\n");
                fprintf(stderr, "\t-z        Display ONLY proc entries in the zombie state\n");
                fprintf(stderr, "\t-h        Display this help message\n");
                return 0;
                break;
            default:
                fprintf(stderr, "Usage: ./myps [-d <path>] [-p] [-c] [-z] [-h]\n");
                break;
        }
    }

    // Fill array with PID directories
    num = 0;
    num = scandir(dirPath, &dirObjects, &DirFilter, &NoSort);
    if (num < 0) {
        fprintf(stderr, "Error: Unable to scan directory\n");
        exit(1);
    }

    // Create array of proc objects to hold stats
    myProcs = (ProcEntry **)malloc(sizeof(ProcEntry *) * num);
    if (myProcs == NULL) {
        fprintf(stderr, "Error: Not enough memory to create Proc Stats\n");
        exit(1);
    }

    // For each dirent object, create corresponding proc object
    for (int i = 0; i < num; i++) { 
        char statFile[MAX_DIR_LEN];
        strcpy(statFile, dirPath);
        strcat(statFile, "/");
        strcat(statFile, dirObjects[i]->d_name);
        strcat(statFile, "/stat");
        tempProcPtr = CreateProcEntryFromFile(statFile);
        if (tempProcPtr == NULL) {
            fprintf(stderr, "Error: Could not gather data from all the files\n");
            DestroyProcEntry(tempProcPtr);
            exit(1);
        }
        myProcs[i] = tempProcPtr;
    }

    // Sort Procs with specified sort function
    qsort(myProcs, num, sizeof(ProcEntry *), sortFunctionPtr);
    
    // Print stats to stdout
    PrintProcs(myProcs, num, zombieFlag);

    // Free allocated memory
    for (int i = 0; i < num; i++) {
        free(dirObjects[i]);
    }
    free(dirObjects);
    for(int i = 0; i < num; i++) {
        DestroyProcEntry(myProcs[i]);
    }
    free(myProcs);

    return 0;
}

int DirFilter(const Dirent *current) {
    // Make sure directory is a proc directory
    if (current->d_type == DT_DIR && isdigit(current->d_name[0])) {
        return 1;
    }

    return 0;
}

int NoSort(const Dirent **entryA, const Dirent **entryB) {
    UNUSED(entryA);
    UNUSED(entryB);

    return 0;
}

static int PidSort(const void *a, const void *b) {
    ProcEntry *f = *(ProcEntry **)a;
    ProcEntry *s = *(ProcEntry **)b;
    int rval = f->pid - s->pid;

    return rval;
}

static int CommSort(const void *a, const void *b) {
    ProcEntry *f = *(ProcEntry **)a;
    ProcEntry *s = *(ProcEntry **)b;

    // Creates temporary strings
    char str1[MAX_STR_SIZE];
    strcpy(str1, f->comm);
    char str2[MAX_STR_SIZE];
    strcpy(str2, s->comm);

    // Sets up string 1 to be compared
    int strSize = (sizeof(str1) / sizeof(char));
    // This would make sure capitalization wouldn't affect sorting
        // for (int i = 0; i < strSize - 1; i++) {
        //     str1[i] = tolower(str1[i]);
        // }
    // This makes sure that if there are double () that the second isn't compared to a letter
    if(str1[1] == '(') {
        for (int i = 0; i < strSize - 1; i++) {
            str1[i] = str1[i + 1];
        }
    }

    // Sets up string 2 to be compared
    strSize = (sizeof(str2) / sizeof(char));
    // This would make sure capitalization wouldn't affect sorting
        // for (int i = 0; i < strSize - 1; i++) {
        //     str2[i] = tolower(str2[i]);
        // }
    // This makes sure that if there are double () that the second isn't compared to a letter
    if(str2[1] == '(') {
        for (int i = 0; i < strSize - 1; i++) {
            str2[i] = str2[i + 1];
        }
    }

    // Compares the strings
    int val = strcmp(str1, str2);

    return val;
}

void PrintProcs(ProcEntry **procs, int count, int zFlag) {
    // Tests for errors
    if (procs == NULL || count <= 0) {
        fprintf(stderr, "Error: Attempted to print nothing.\n");
        return;
    }

    // Outputs the statistics
    fprintf(stdout,"%7s %7s %5s %5s %5s %7s %-25s %-20s\n",
        "PID","PPID","STATE","UTIME","STIME","THREADS","CMD","STAT_FILE");
    for (int i = 0; i < count; i++) {    
        if(zFlag && procs[i]->state != 'Z') {
            continue;
        }
        PrintProcEntry(procs[i]);
    }
}