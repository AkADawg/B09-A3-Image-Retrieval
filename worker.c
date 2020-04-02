#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include "worker.h"

#define LINE_LENGTH 70


/*
* Read an image from a file and create a corresponding
* image struct
*/

Image* read_image(char *filename)
{
	FILE *file;
	int error;
	char* end;
	char line[LINE_LENGTH + 1];
	int word;

	Image *img = (Image*)malloc(sizeof(Image));

	/**Error checking for successful memory allocation*/
	if (!img) {
		fprintf(stderr, "Unable to allocate memory\n");
		return NULL;
	}


	/*Open File and check for errors*/
	file = fopen(filename, "r");

	if (file == NULL) {
		fprintf(stderr, "Error opening file\n");
		return NULL;
	}

	/*Scan through file to get required information and store in struct*/
	fscanf(file, "%s", line);
	if (strcmp(line, "P3") != 0) {
		return NULL;
	}

	// Read and store width
	fscanf(file, "%s", line);
	int width = strtol(line, &end, 10);
	img->width = width;
	//printf("width: %d\n", width);

	// Read and store height
	fscanf(file, "%s", line);
	int height = strtol(line, &end, 10);
	img->height = height;
	//	printf("height: %d\n", height);

		// Read and store max value
	fscanf(file, "%s", line);
	int maxValue = strtol(line, &end, 10);
	img->max_value = maxValue;
	//	printf("max value: %d\n", maxValue);

		/*Allocate memory for pixels and check for errors*/
	img->p = (Pixel*)malloc(width * height * sizeof(Pixel));

	if (!img->p) {
		fprintf(stderr, "Unable to allocate memory\n");
	}

	/*Scan through file and store pixel information*/
	for (int i = 0; i < width *height; i++) {

		//Read and store red pixel
		fscanf(file, "%d", &word);
		img->p[i].red = word;
		//	printf("red: %d\n", word);

			//Read and store green pixel
		fscanf(file, "%d", &word);
		img->p[i].green = word;
		//	printf("green: %d\n", word);


			//Read and store blue pixel
		fscanf(file, "%d", &word);
		img->p[i].blue = word;
		//	printf("blue: %d\n", word);

	}

	/*Close file and check for errors*/
	error = fclose(file);

	if (error != 0) {
		fprintf(stderr, "Error closing file\n");
		return NULL;
	}

	//free(img->p);
	//free(img);
	return img;
}

/*
* Print an image based on the provided Image struct
*/

void print_image(Image *img) {
	printf("P3\n");
	printf("%d %d\n", img->width, img->height);
	printf("%d\n", img->max_value);

	for (int i = 0; i < img->width*img->height; i++)
		printf("%d %d %d  ", img->p[i].red, img->p[i].green, img->p[i].blue);
	printf("\n");
}

/*
* Compute the Euclidian distance between two pixels
*/
float eucl_distance(Pixel p1, Pixel p2) {
	return sqrt(pow(p1.red - p2.red, 2) + pow(p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));
}

/*
* Compute the average Euclidian distance between the pixels
* in the image provided by img1 and the image contained in
* the file filename
*/

float compare_images(Image *img1, char *filename)
{
	FILE* file;
	char* end;
	char line[LINE_LENGTH + 1];
	int word;
	int euclD = 0;
	int count = 0;


	/*Open File and check for errors*/
	file = fopen(filename, "r");

	if (file == NULL) {
		fprintf(stderr, "Error oepning file\n");
		exit(1);									//ALERT DO WE EXIT INSTEAD
	}

	fscanf(file, "%s", line);

	// Read and store width
	fscanf(file, "%s", line);
	int width = strtol(line, &end, 10);

	// Read and store height
	fscanf(file, "%s", line);
	int height = strtol(line, &end, 10);
	fscanf(file, "%s", line);


	//Check if dimensions are the same
	if (img1->height != height || img1->width != width) {
		return FLT_MAX;
	}

	/*Allocate memory for pixels*/
	Pixel *p = (Pixel*)malloc(width * height * sizeof(Pixel));

	/*Scan and store pixel information*/
	for (int w = 1; w <= 4; w++) {
		for (int h = 1; h <= 4; h++) {

			//Read and store red pixel
			fscanf(file, "%d", &word);
			p[count].red = word;

			//Read and store green pixel
			fscanf(file, "%d", &word);
			p[count].green = word;

			//Read and store blue pixel
			fscanf(file, "%d", &word);
			p[count].blue = word;

			count++;

		}

	}

	/*Calculate euclidean distance for both images*/
	for (int i = 0; i < width *height; i++) {
		euclD += eucl_distance(img1->p[i], p[i]);
	}

	printf("%d", euclD);
	free(p);
	return euclD;
}

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it
* - for each file read the image in it
* - compare the image read to the image passed as parameter
* - keep track of the image that is most similar
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
CompRecord process_dir(char *dirname, Image *img, int out_fd) {

	CompRecord CRec;
	struct dirent *dp;
	char path[PATHLENGTH];
	struct stat sbuf;
	DIR *dirp;
	float mostSimilar = FLT_MAX;
	float result;
	char* filename;

	/*Open directory and check for error*/
	dirp = opendir(dirname);
	if ((dirp == NULL)) {
		fprintf(stderr, "Unable to open directory");
		exit(1);
	}


	/*Find all the files in the directory*/
	while (dp = readdir(dirp)) {

		if (strcmp(dp->d_name, ".") == 0 ||
			strcmp(dp->d_name, "..") == 0 ||
			strcmp(dp->d_name, ".svn") == 0) {
			continue;
		}
		strncpy(path, dirname, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);


		if (stat(path, &sbuf) == -1) {
			fprintf(stderr, "stat erorr");
			exit(1);
		}

		if (S_ISREG(sbuf.st_mode)) {
			printf("%s\n", dp->d_name);
			printf("the path is: %s and the directory name is: %s\n", path, dirname); //use path as it contains file name
			result = compare_images(img, path);
			if (result < mostSimilar) {
				mostSimilar = result;

				filename = path;
			}
		}

	}

	strcpy(CRec.filename, filename);
	CRec.distance = mostSimilar;

	return CRec;
}


int main() {
	process_dir("C:/Users/Amarp/OneDrive/Documents/Fax/Inbox", read_image("sample.ppm"), STDOUT_FILENO);
	//compare_images(read_image("sample.ppm"), "top10.txt");
	//print_image(read_image("sample.ppm"));
	//read_image("hello.txt");
	return 0;
}

