#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>
#include "worker.h"
#define BUF_SIZE 4
#define MSG_SIZE 13

int main(int argc, char **argv) {

	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";
	char *image_file = NULL;
	char buf[BUF_SIZE];


	while ((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
		case 'd':
			startdir = optarg;
			break;
		default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
			exit(1);
		}
	}

	if (optind != argc - 1) {
		fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
	}
	else
		image_file = argv[optind];

	// Open the directory provided by the user (or current working directory)

	DIR *dirp;
	if ((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	}

	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the image files contained in the directory.
	*/

	struct dirent *dp;
	CompRecord CRec;

	while ((dp = readdir(dirp)) != NULL) {

		if (strcmp(dp->d_name, ".") == 0 ||
			strcmp(dp->d_name, "..") == 0 ||
			strcmp(dp->d_name, ".svn") == 0) {
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if (stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		}

		int fd[2], nbytes;
		pipe(fd);
		//Pipe error checking
		if (pipe(fd) == -1) {
			perror("pipe");
			exit(1);
		}


		// Only call process_dir if it is a directory
		// Otherwise ignore it.
		if (S_ISDIR(sbuf.st_mode)) {
			//printf("Processing all images in directory: %s \n", path);

			if (fork() == 0) {
				/*Child process closes up input side of pipe*/
				close(fd[0]);

				/*Send CompRecord through output side of pipe*/
				CompRecord CRec = process_dir(startdir, read_image(image_file), -1);
				write(fd[1], &CRec, sizeof(CompRecord));
			}
			else {
				close(fd[1]);

				//read(fd[0], )
			}
		}
	}

	printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);

	return 0;
}
