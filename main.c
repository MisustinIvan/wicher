#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_FILES 256
static char* filepaths[MAX_FILES];
int filepath_count = 0;
int filepath_index = 0;

void set_wallpaper(const char path[], int dpy) {
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "nitrogen --head=%d --set-zoom-fill \"%s\"", dpy, path);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execl("/bin/sh", "sh", "-c", buffer, (char*)NULL);
        perror("execl failed\n");
        exit(EXIT_FAILURE);
    } else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

bool setup(char directory[]) {
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        perror("Failed to open directory");
        return false;
    }

    int dir_strlen = strlen(directory);

    struct dirent* current_file;

    while ((current_file = readdir(dir)) != NULL) {
        if (strcmp(current_file->d_name, ".") == 0 || strcmp(current_file->d_name, "..") == 0) {
            continue;
        }

        if (!strstr(current_file->d_name, ".jpg") && !strstr(current_file->d_name, ".png")) {
            continue;
        }

        if (filepath_count >= MAX_FILES) {
            fprintf(stderr, "Reached maximum number of files\n");
            break;
        }

        int filename_len = strlen(current_file->d_name);

        filepaths[filepath_count] = malloc(dir_strlen + filename_len + 1);
        if (filepaths[filepath_count] == NULL) {
            perror("Failed to allocate memory for file path");
            closedir(dir);
            return false;
        }

        strcpy(filepaths[filepath_count], directory);
        strcpy(filepaths[filepath_count]+dir_strlen, current_file->d_name);
        filepath_count++;
    }

    closedir(dir);

    printf("Read %d image files\n", filepath_count);

    if(filepath_count == 0) {
        return false;
    }

    srand(time(NULL));
    filepath_index = rand() % filepath_count;

    return true;
}

void cleanup() {
    for (int i = 0; i<filepath_count; i++) {
        free(filepaths[i]);
    }
}

int schedule(int delay_seconds) {
    while (true) {
        static char buffer[128];
#define cmd "xrandr --listactivemonitors | wc -l"

        FILE* fp;
        int monitor_count;

        fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen failed\n");
            return EXIT_FAILURE;
        }

        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            monitor_count = atoi(buffer);
            monitor_count -= 1;
        }

        pclose(fp);

        for (int monitor=0; monitor < monitor_count; monitor++) {
            set_wallpaper(filepaths[filepath_index], monitor);
        }
        filepath_index = (filepath_index + 1) % filepath_count;
        sleep(delay_seconds);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s [directory] [delay] [mode:normal/daemon]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (!setup(argv[1])) {
        printf("Failed to initialize the program, exitting\n");
        return EXIT_FAILURE;
    }

    atexit(cleanup);

    if (strcmp(argv[3], "daemon") == 0) {
        if (daemon(0, 0) == -1) {
            perror("daemon failed\n");
            exit(EXIT_FAILURE);
        }
    }

    int delay;
    delay = atoi(argv[2]);

    schedule(delay);

    return EXIT_SUCCESS;
}
