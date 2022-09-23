/*
    Author: Anthony LoRe Starleaf
    Last modified: September 19, 2022
    Email: aglorestarleaf@gmail.com

    Description:
    This program reads a .DAT file for an airfoil, then graphs it
    in the command line.

    Does not accept negative coordinates.
*/

// Includes
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

// Function Prototypes
int is_file (char* input_file);
int sanitize_file(char* f_in_name, char* output_path);
int points_array(char* input_file_path, int** x_coordinates_out, int** y_coordinates_out, int* length, int max_width);

int main (int argc, char **argv) {
    /*
        TODO:
        Add max_width as a command line argument.
    */

    if (argc < 2) {
        puts("Error. Too few arguments.");
        return 1;
    } else if (argc > 2) {
        puts("Error. Too many arguments.");
        return 1;
    }

    if (is_file(argv[1]) == 0) {
        puts("File does not exist.");
        return 1;
    }

    char* input_file = argv[1];
    char output_path[100];

    if (sanitize_file(input_file, output_path) == 1) {
        puts("There was an error processing the file.");
        return 1;
    }

    // get coordinates of points
    int * x_coords;
    int * y_coords;
    int length;
    int max_width = 200;
    if (points_array(output_path, &x_coords, &y_coords, &length, max_width) == 1) {
        printf("Error graphing airfoil.");
        return 1;
    };

    // allocate space for coordinate array
    char graph[max_width][max_width];
    // fill with spaces
    for (int i = 0; i < max_width; i++) {
        for (int j = 0; j < max_width; j++) {
            graph[i][j] = ' ';
        }
    }

    // add (*)'s for each point
    int greatest_y = 0;
    for (int i = 0; i < length; i++) {
        graph[x_coords[i]][y_coords[i]] = '*';
        if (y_coords[i] > greatest_y) greatest_y = y_coords[i];
    }
    free((void*)x_coords);
    free((void*)y_coords);

    // print out airfoil graph
    for (int i = greatest_y + 2; i >= 0; i--) {
        for (int j = 0; j < max_width; j++) {
            printf("%c", graph[j][i]);
        }
        printf("\n");
    }
    printf("\n\n");
}

int is_file (char* input_file) {
    /*
        :input: (char*) path to input file
        :return: (int) 0 if file is found, 1 if not

        Description:
        Determines if input_file exists.
    */
    
    struct stat st;
    if (stat(input_file, &st) == -1) return 0;
    return 1;
}

int sanitize_file(char* f_in_name, char* output_path) {
    /*
        :input: (char*) path to input file, (char*) variable to store output path
        :return: (int) 0 for success, 1 for failure

        Description:
        Removes letters and extra spaces from .DAT files. Modifies output_path
        to be the path to the sanitized file

        TODO:
        Convert coordinates to positive numbers.
    */

    // get the name of the input file (without path)
    int len = 0;
    int pos = 0;
    for (int i = 0; f_in_name[i] != '\0'; i++) {
        len++;
        pos = f_in_name[i] == '/' ? i : pos;
    }
    char f_out_name[len];
    int j = 0;
    for (int i = pos + 1; i < len; i++) {
        f_out_name[j] = f_in_name[i];
        j++;
    }

    // construct file path for output file
    char * f_out_folder = "DAT_files_sanitized";
    int len_f_out_path = sizeof(*f_out_folder) / sizeof(char) + len;
    char f_out_path[ len_f_out_path ];
    sprintf(f_out_path, "%s/%s", f_out_folder, f_out_name);
    strcpy(output_path, f_out_path);

    // open input and output files
    FILE * f_in = fopen(f_in_name, "r");
    FILE * f_out = fopen(f_out_path, "w");

    // throw error if either file could not be opened/created
    if (f_in == NULL) return 1;
    if (f_out == NULL) return 1;

    struct stat st;
    stat(f_in_name, &st);
    size_t f_in_size = st.st_size;
    
    // read file to buffer
    char * buffer = (char*)malloc((f_in_size + 1) * sizeof(char));
    char line[f_in_size + 1];
    while (fgets(line, f_in_size + 1, f_in) != NULL) strcat(buffer, line);
    fclose(f_in);

    char * word;
    char * ptr = buffer;
    int num_words = 0;
    while ((word = strtok(ptr, " \t\n"))) {
        if (isdigit(word[0])) {
            fprintf(f_out, num_words % 2 == 0 ? "%s " : "%s\n", word);
            num_words++;
        }
        ptr = NULL;
    }

    free(buffer);
    fclose(f_out);

    return 0;
}

int points_array(char* input_file_path, int** x_coordinates_out, int** y_coordinates_out, int* length, int max_width) {
    /*
        :input: TODO
        :return: 1 on error, 0 on completion
    */
    FILE * f = fopen(input_file_path, "r");
    if (f == NULL) return 1;

    // get number of lines in file to allocate space for coordinate arrays
    int lines_in_file = 0;
    char c;
    for (c = getc(f); c != EOF; c = getc(f)) {
        if (c == '\n') lines_in_file++;
    }
    fclose(f);

    double x_coords[lines_in_file];
    double y_coords[lines_in_file];

    double x_coord, y_coord;
    double greatest_x = 0;
    int line = 0;
    FILE * fp = fopen(input_file_path, "r");
    if (fp == NULL) return 1;
    // construct arrays
    while (fscanf(f, "%lf %lf", &x_coord, &y_coord) != EOF) {
        if (x_coord > greatest_x) greatest_x = x_coord;
        x_coords[line] = x_coord;
        y_coords[line] = y_coord;
        line++;
    };
    fclose(fp);

    // scale coordinates to fit max_width
    double scale_factor = max_width / greatest_x;

    int * out_x = (int*)malloc(lines_in_file * sizeof(int));
    int * out_y = (int*)malloc(lines_in_file * sizeof(int));
    for (int i = 0; i < sizeof(x_coords)/sizeof(double); i++) {
        x_coords[i] *= scale_factor;
        y_coords[i] *= scale_factor;

        out_x[i] = (int)round(x_coords[i]);
        out_y[i] = (int)round(y_coords[i]);
    }

    *x_coordinates_out = out_x;
    *y_coordinates_out = out_y;
    *length = lines_in_file;

    return 0;
}
