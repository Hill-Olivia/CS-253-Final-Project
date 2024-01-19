/*
 * File: ProcEntry.c
 * Author: Olivia Hill
 * Description: This file contains functions used in myps.c that handle
 *          ProcEntry objects
 * Date: Tue Aug  8 16:17:41 MDT 2023
 * Module 6, Final Project
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "ProcEntry.h"

#define MAX_STR_SIZE 100

ProcEntry * CreateProcEntry(void) {
    // Allocate memory and check for error
    ProcEntry * newProcEntry = (ProcEntry *)malloc(sizeof(ProcEntry));
    if (newProcEntry == NULL) {
        fprintf(stderr, "Error: Not enough memory to be allocated\n");
        free(newProcEntry);
        return NULL;
    }

    // Initialize struct info
    newProcEntry->pid = 0;
    newProcEntry->ppid = 0;
    newProcEntry->comm = NULL;
    newProcEntry->state = 0;
    newProcEntry->utime = 0;
    newProcEntry->stime = 0;
    newProcEntry->num_threads = 0.0;
    newProcEntry->path = NULL;

    return newProcEntry;
}

ProcEntry * CreateProcEntryFromFile(const char statFile[]) {
    // File verification
    if (statFile == NULL) {
        fprintf(stderr, "Error: File not set\n");
        return NULL;
    }
    FILE * procStats = fopen(statFile, "r");
    if (procStats == NULL) {
        fprintf(stderr, "Error: Unable to open stat file\n");
        return NULL;
    }

    // Create ProcEntry
    ProcEntry *newProc = CreateProcEntry();
    if (newProc == NULL) {
        return NULL;
    }
    // Variables
    int valuesEntered; 
    char comm[MAX_STR_SIZE];
    int commLastIndex;
    int pid;
    char state;
    int ppid;
    int emptyScan;
    unsigned long int utime;
    unsigned long int stime;
    signed long int numThreads;

    // Scan through file for Proc data 
    valuesEntered = fscanf(procStats, "%d %s",
        &pid,
        comm);
    if (valuesEntered < 2) {
        fprintf(stderr, "Error: Values unsuccessfully read from %s\n", statFile);
        return NULL;
    }

    // Test for space in str variable
    commLastIndex = 0;
    for (int i = 0; comm[i] != '\0'; i++) {
        commLastIndex++;
    }
    commLastIndex--;
    while (comm[commLastIndex] != 41) { // If last char in str != ')'
        char tempStr[20];
        valuesEntered = fscanf(procStats, "%s", tempStr);
        if (valuesEntered < 1) {
            fprintf(stderr, "Error: Values unsuccessfully read from %s\n", statFile);
            return NULL;
        }
        strcat(comm, " ");
        strcat(comm, tempStr);    
        commLastIndex = 0;
        // Recalulate commLastIndex
        for (int i = 0; comm[i] != '\0' ; i++) {
            commLastIndex++;
        }
        commLastIndex--;
    }

    // Continues to scan file for Proc data
    valuesEntered = fscanf(procStats, " %c %d", &state, &ppid);
    if (valuesEntered < 2) {
        fprintf(stderr, "Error: Values unsuccessfully read from %s\n", statFile);
        return NULL;
    }

    valuesEntered = fscanf(procStats, "%d %d %d %d %d %d %d %d %d",
        &emptyScan,
        &emptyScan,
        &emptyScan,
        &emptyScan,
        &emptyScan,
        &emptyScan,
        &emptyScan,
        &emptyScan,
        &emptyScan);
    if (valuesEntered < 9) {
        fprintf(stderr, "Error: Unable to parse file values\n");
        return NULL;
    }

    valuesEntered = fscanf(procStats, "%lu %lu",
        &utime,
        &stime);
    if (valuesEntered < 2) {
        fprintf(stderr, "Error: Values unsuccessfully read from %s\n", statFile);
        return NULL;
    }

    valuesEntered = fscanf(procStats, "%d %d %d %d",
        &emptyScan,
        &emptyScan,
        &emptyScan,
        &emptyScan);
    if (valuesEntered < 4) {
        fprintf(stderr, "Error: Unable to parse file values\n");
        return NULL;
    }

    valuesEntered = fscanf(procStats, "%ld", &numThreads);
    if (valuesEntered < 1) {
        fprintf(stderr, "Error: Values unsuccessfully read from %s\n", statFile);
        return NULL;
    }

    // Allocate memory for comm and path members
    newProc->comm = (char *)malloc(sizeof(char) * MAX_STR_SIZE);
    if (newProc->comm == NULL) {
        fprintf(stderr, "Error: Not enough memory to be allocated\n");
        free(newProc->comm);
        free(newProc);
        return NULL;
    }
    newProc->path = (char *)malloc(sizeof(char) * MAX_STR_SIZE);
    if (newProc->path == NULL) {
        fprintf(stderr, "Error: Not enough memory to be allocated\n");
        free(newProc->comm);
        free(newProc->path);
        free(newProc);
        return NULL;
    }

    // Store values in ProcEntry object
    newProc->pid = pid;
    strcpy(newProc->comm, comm);
    newProc->state = state;
    newProc->ppid = ppid;
    newProc->utime = utime;
    newProc->stime = stime;
    newProc->num_threads = numThreads;
    strcpy(newProc->path, statFile);

    fclose(procStats);

    return newProc;
}

void DestroyProcEntry(ProcEntry * entry) {
    if(entry == NULL) {
        return;
    }

    free(entry->comm);
    free(entry->path);
    free(entry);
}

void PrintProcEntry(ProcEntry * entry) {
    unsigned long int utime = entry->utime / sysconf(_SC_CLK_TCK);
    unsigned long int stime = entry->stime / sysconf(_SC_CLK_TCK);
    fprintf(stdout, "%7d %7d %5c %5lu %5lu %7ld %-25s %-20s\n",
        entry->pid,
        entry->ppid,
        entry->state,
        utime,
        stime,
        entry->num_threads,
        entry->comm,
        entry->path);
}