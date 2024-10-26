#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

typedef struct HashNode {
    int count;
    char* word;
    struct HashNode* next;
} HashNode;

static HashNode** table = NULL;
static int tableFullness = 0, tableSize = 4;

int hash(char* word) {
    unsigned long long hash = 0;
	int len = (int) strlen(word);
    for (int i = 0; i < len; i++)
        hash += word[i] * pow(31, len - (i + 1));
    return hash % tableSize;
}


void resize() {
	tableSize *= 2;

	HashNode** oldTable = table;	
	table = (HashNode**)calloc(tableSize, sizeof(HashNode*));
	
	HashNode* ptrOld;
	HashNode* ptr;

	for(int i = 0; i < (tableSize / 2); i++)
	{
		ptrOld = oldTable[i];
		while(ptrOld)
		{
			ptr = ptrOld->next;
			int hashNum = hash(ptrOld->word);
			ptrOld->next = table[hashNum];
			table[hashNum] = ptrOld;
			ptrOld = ptr;
		} 
			
	}
	free(oldTable);
}

void update(char* word, int k) {

	if(!table)
	{
		table = (HashNode**)calloc(tableSize, sizeof(HashNode*)); 
	}

	int hashNum = hash(word);
	HashNode* ptr = table[hashNum];

	while(ptr && strcmp(ptr->word, word) != 0)
	{
		ptr = ptr->next;
	}
	if(!ptr)
	{
		HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
		newNode->word = strdup(word);
		newNode->count = 1;
		newNode->next = table[hashNum];

		table[hashNum] = newNode;
		tableFullness++;
	}
	else if(k == 1)
	{
		ptr->count = ptr->count + 1;
	}
	else if(k == -1)
	{
		ptr->count = -1;
	}
	if(tableFullness >= (tableSize / 2))
	{
		resize();
	}
}


void free_hashtable() {
	
	HashNode* ptrOld;
	HashNode* ptr;

	for(int i = 0; i < tableSize; i++)
	{
		ptrOld = table[i];
		
		while(ptrOld)
		{
			ptr = ptrOld->next;
			free(ptrOld->word);
			free(ptrOld);
			ptrOld = ptr;
		}
		
	}

	free(table);
}

void print_hashtable() {
	if(!table)
	{
		return;
	}
    for(int i = 0; i < tableFullness; i++) {
		int maxCount = 0;
		char* maxWord;
		for(int i = 0; i < tableSize; i++)
		{
			HashNode* ptr = table[i];
			while(ptr)
			{
				if((ptr->count > maxCount) || (ptr->count == maxCount && strcmp(maxWord, ptr->word) > 0)) {
					maxCount = ptr->count;
					maxWord = ptr->word;
					update(maxWord, -1);
				}
				ptr = ptr->next;
			}
		}
		if(maxCount > 0) {
			printf("%s %d\n", maxWord, maxCount);
		}
	}
	free_hashtable();
}


void getString(char* dest, char* src, int start, int end, int unfinished) {
    for (int i = 0; i < end - start; i++) {
        dest[unfinished + i] = src[start + i];
    }
    dest[end - start + unfinished] = '\0';
}

void wordCount(char* name) {
    //temp implementation
    //printf("%s\n", name);
    int fp = open(name, O_RDONLY);
    char buffer[200];
    int charsRead = -1;
    char word[200];//idk how big the word size limit is
    int unfinished = 0; //In case the word is not finished by the end of the buffer, we need to know how long our unfinished word is so that we can complete the word next buffer
    do {
        charsRead = read(fp, buffer, 200);
        if (charsRead == -1) {
            printf("something went wrong. \n");
            return;
        }
        int indexStart = 0; //start and end of word
        int indexEnd = 0;
        for (int i = 0; i < charsRead; i++) {
            if(buffer[i] == '.' || buffer[i] == ' ' || buffer[i] == '!' || buffer[i] == '?' || buffer[i] == ','|| buffer[i] == '\n') {
                indexEnd = i;
                if (i == 0 || buffer[i - 1] == '.' || buffer[i - 1] == ' ' || buffer[i - 1] == '!' || buffer[i - 1] == '?' || buffer[i - 1] == ','|| buffer[i] == '\n') {
                    indexStart = i + 1;
                    continue;
                }
                getString(word, buffer, indexStart, indexEnd, unfinished); //a function to get the word
                if (strlen(word) == 1 && word[0] == '\'') { //word can't be a random apostophe
                    word[0] = '\n';
                }
				update(word, 1);
                indexStart = i + 1;
                unfinished = 0; //finished the word
            }
        }
        if (indexStart == indexEnd) { //means we haven't found the end of the word yet, so we must continue searching for it next buffer
            unfinished = 200 - indexStart - 1;
        }
    }
    while (charsRead == 200);
}

void traverse(char* fName) {
    struct stat buf;
    if (stat(fName, &buf) != 0) {
        return;
    }
    bool isDir = S_ISDIR(buf.st_mode);
    
    if (isDir) {
        DIR* directory = opendir(fName);
        struct dirent* entry = NULL;
        while ((entry = readdir(directory)) != NULL) {
            char full_name[300];
            snprintf(full_name, 300, "%s/%s", fName, entry->d_name);

            if (entry->d_type == DT_DIR) { //if the thing is a directory
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    traverse(full_name);
                }
            } else {
                wordCount(full_name); //if it is not a directory
            }
        }

        closedir(directory);
    }
    else {
        wordCount(fName); //happens when the file given is a file
    }
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        traverse(argv[i]); //traverse over every name given
    }
	print_hashtable();
    return 0;
}

