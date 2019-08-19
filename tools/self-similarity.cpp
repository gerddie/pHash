/*

    pHash, the open source perceptual hash library
    Copyright (C) 2019 Collabora Ltd,
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <algorithm>
#include "pHash.h"

using namespace std;

#define TRUE 1
#define FALSE 0

//data structure for a hash and id
struct ph_imagepoint{
    ulong64 hash;
    char *id;
};

//aux function to create imagepoint data struct
struct ph_imagepoint* ph_malloc_imagepoint(){

    return (struct ph_imagepoint*)malloc(sizeof(struct ph_imagepoint));

}

//auxiliary function for sorting list of hashes
bool cmp_lt_imp(struct ph_imagepoint dpa, struct ph_imagepoint dpb){
    int result = strcmp(dpa.id, dpb.id);
    if (result < 0)
	return TRUE;
    return FALSE;
}
/**
 *  The program reads all images from one directory and evaluate the hamming
 *  distance of the hashes of each image with the all the others.
**/

int main(int argc, char **argv){

    const char *msg = ph_about();
    printf(" %s\n", msg);

    if (argc < 3){
	printf("no input args\n");
	printf("expected: \"phash-self-similarity [dir name]  [output file]\n");
	exit(1);
    }
    const char *dir_name = argv[1];
    const char *out_file_name = argv[2];
    struct dirent *dir_entry;
    vector<ph_imagepoint> hashlist1; //for hashes in first directory

    ph_imagepoint *dp = NULL;

    FILE *out_file = fopen(out_file_name, "w");
    if (!out_file) {
       printf("unable to open file '%s' for writing\n", out_file_name);
       exit(1);
    }

    //first directory
    DIR *dir = opendir(dir_name);
    if (!dir){
	printf("unable to open directory\n");
        fclose(out_file);
	exit(1);
    }
    errno = 0;
    ulong64 tmphash;
    char path[100];
    path[0] = '\0';
    while ((dir_entry = readdir(dir)) != nullptr){
	if (dir_entry->d_type == DT_REG) {
	    strcat(path, dir_name);
	    strcat(path, "/");
	    strcat(path, dir_entry->d_name);
            fprintf(stderr, "\rhash %s ", dir_entry->d_name);

            if (ph_dct_imagehash(path, tmphash) < 0) { //calculate the hash
               fprintf(stderr, "ignored\n");
		continue;
            }
            dp = ph_malloc_imagepoint();              //store in structure with file name
	    dp->id = dir_entry->d_name;
	    dp->hash = tmphash;
	    hashlist1.push_back(*dp);
        }
	errno = 0;
        path[0]='\0';
    }
    fprintf(stderr, "\n");

    if (errno){
	printf("error reading directory '%s'\n", dir_name);
        fclose(out_file);
	exit(1);
    }

    sort(hashlist1.begin(),hashlist1.end(),cmp_lt_imp);

    auto nbfiles1 = hashlist1.size();

    errno = 0;
    for (unsigned i=0;i<nbfiles1;i++){
	for (unsigned int j=i+1;j<nbfiles1;j++){
           fprintf(out_file, "%s %s %d\n", hashlist1[i].id, hashlist1[j].id,
                   ph_hamming_distance(hashlist1[i].hash,hashlist1[j].hash));

	}
    }
    if (errno){
	printf("error writing to '%s'\n", out_file_name);
	exit(1);
    }
    fclose(out_file);

    return 0;
}
