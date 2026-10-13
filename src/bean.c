/* 
 * File:       bean.c
 * Date:   2026-04-21
 *
 * Program for counting characters, words, and lines from files or input.
 * Supplied with a directory name, it will scan all the text files contained
 * within it.
 *
 * Licensed under the MIT License. Copyright (c) 2026 Ryan A. Rashidian
 */

#include <ctype.h>
#include <getopt.h>
#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define FILE_START_CAP 32

typedef struct {
    size_t size;
    size_t cap;
    char **arr;
} FileArr;

FileArr filearr;
bool char_opt = false;
bool word_opt = false;
bool line_opt = false;
bool loc_opt  = false;
bool file_opt = false;
int char_count = 0;
int word_count = 0;
int line_count = 0;
int loc_count  = 0;
int file_count = 0;

void terminate(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void file_arr_init(void)
{
    filearr.arr = malloc(sizeof(char *) * FILE_START_CAP);
    if (filearr.arr == NULL) {
        terminate("file_arr_init: failed to allocate memory");
    }
    filearr.cap = FILE_START_CAP;
    filearr.size = 0;
}

void file_arr_check_cap(void)
{
    if (filearr.size >= filearr.cap - 1) {
        filearr.cap *= 2;
        char **temp = realloc(filearr.arr, filearr.cap * sizeof(char *));
        if (temp == NULL) {
            terminate("file_arr_check_cap: failed to allocate memory");
        }
        filearr.arr = temp;
    }
}

char *build_path(const char *root, const char *branch)
{
    char *path;
    size_t size = strlen(root) + strlen(branch) + 2;
    if ((path = malloc(size)) == NULL) {
        terminate("file_arr_add: failed to allocate memory");
    }
    strcpy(path, root);
    strcat(path, "/");
    strcat(path, branch);
    return path;
}

void scan_directory(const char *dir_name)
{
    DIR *dirp;
    struct dirent *entry;

    if ((dirp = opendir(dir_name)) == NULL) {
        fprintf(stderr, "Failed to open: %s\n", dir_name);
        terminate("Usage: bean <directory-name>");
    }
    while ((entry = readdir(dirp)) != NULL) {
        if (strcmp(entry->d_name, ".")  == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        switch (entry->d_type) {
            case DT_REG: { 
                file_arr_check_cap();
                char *file_path = build_path(dir_name, entry->d_name);
                filearr.arr[filearr.size++] = file_path;
            } break;
            case DT_DIR: {
                /* Depth-first recursive search */
                char *next_dir = build_path(dir_name, entry->d_name);
                scan_directory(next_dir);
                free(next_dir);
            } break;
            default: break;
        }
    }
    closedir(dirp);
}

void count_fp(FILE *fp)
{
    int ch;
    bool word_flag = false;
    bool comment_line = false;
    bool comment_block = false;
    bool blank_line = true;

    while ((ch = fgetc(fp)) != EOF) {
        char_count++;
        if (!isspace(ch)) {
            if (ch == '/' && blank_line) {
                if ((ch = fgetc(fp)) == EOF) break;
                char_count++;
                if (ch == '/') comment_line = true;
                if (ch == '*') comment_block = true;
            }
            if (ch == '*' && comment_block) {
                if ((ch = fgetc(fp)) == EOF) break;
                char_count++;
                if (ch == '/') {
                    comment_block = false;
                    comment_line = true;
                }
            }
            word_flag = true;
            blank_line = false;
        }
        if (ch == '\n') {
            if (!comment_line && !comment_block && !blank_line) {
                loc_count++;
            }
            line_count++;
            comment_line = false;
            blank_line = true;
        }
        if (isspace(ch) && word_flag) {
            word_count++;
            word_flag = false;
        }
    }
    file_count++;
}

void read_file(const char *file_name)
{
    FILE *fp;
    if ((fp = fopen(file_name, "rb")) == NULL) {
        fprintf(stderr, "Failed to read file: %s\n", file_name);
        return;
    }
    count_fp(fp);
    fclose(fp);
}

void read_all_files(void)
{
    for (int i = 0; i < (int)filearr.size; i++) {
        read_file(filearr.arr[i]);
    }
}

void print_counts(void)
{
    if (char_opt) { printf("Char count: %d\n", char_count); }
    if (word_opt) { printf("Word count: %d\n", word_count); }
    if (line_opt) { printf("Line count: %d\n", line_count); }
    if (loc_opt)  { printf("LOC count: %d\n", loc_count);   }
    if (file_opt) { printf("File count: %d\n", file_count); }
}

void cleanup(void)
{
    for (int i = 0; i < (int)filearr.size; i++) {
        free(filearr.arr[i]);
    }
    free(filearr.arr);
}

void directory_mode(const char *dir_name)
{
    atexit(cleanup);
    file_arr_init();
    scan_directory(dir_name);
    read_all_files();
    print_counts();
}

void file_mode(const char *file_name)
{
    read_file(file_name);
    print_counts();
}

void input_mode(void)
{
    count_fp(stdin);
    print_counts();
}

void detect_mode(const char *user_arg)
{
    struct stat st;
    if (stat(user_arg, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            directory_mode(user_arg);
        } else if (S_ISREG(st.st_mode)) {
            file_mode(user_arg);
        }
    }
}

int get_options(int argc, char *argv[])
{
    /* Get options and return remaining arguments */
    int opt;

    struct option long_opts[] = {
        {"chars", no_argument, 0, 'c'},
        {"words", no_argument, 0, 'w'},
        {"lines", no_argument, 0, 'l'},
        {"loc",   no_argument, 0, 'L'},
        {"files", no_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "cwlLf", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'c': { char_opt = true; } break;
            case 'w': { word_opt = true; } break;
            case 'l': { line_opt = true; } break;
            case 'L': { loc_opt  = true; } break;
            case 'f': { file_opt = true; } break;
            case '?': { terminate("Try options: -c -w -l -L -f"); }
            default: break;
        }
    }
    if (!char_opt && !word_opt && !line_opt && !file_opt && !loc_opt) {
        /* Default to show all options */
        char_opt = word_opt = line_opt = file_opt = true;
    }
    int remaining_args = argc - optind;
    return remaining_args;
}

int main(int argc, char *argv[])
{
    int remaining_args = get_options(argc, argv);
    if (remaining_args == 1) {
        detect_mode(argv[optind]);
    } else if (remaining_args == 0) {
        input_mode();
    } else {
        fprintf(stderr, "Usage: %s [option] [file-path]", argv[0]);
    }

    return 0;
}

