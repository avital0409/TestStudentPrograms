#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

#define EXECUTABLE_NAME "main.out"
#define EXE_OUTPUT "actualOutput.txt"
#define RESULT_FILE "results.csv"
#define MAX_LINE_LENGTH 50
#define SPRINTF_OUTPUT 260
#define CONFIG_LINES 3
#define CHILD_PROCESS 0
#define SUCCESSFUL_OPEN(fd) (fd >= 0)
#define SUCCESSFUL_OPEN_DIR(dirp) (dirp != NULL)
#define SUCCESSFUL_READ(bytes) (bytes > 0)

typedef enum { false, true } bool;
typedef struct Configs {
    char* rootFolder;
    char* input;
    char* expectedOutput;
} Configs;

void freePointers(int, ...);
void validate(int, const char*, bool);
int equals(const char*, const char*);
int isCfile(const char*);
char* getLine(int);
void* mallocWithValidation(int);
int forkAndExecvp(char**);
bool compileFile(const char*, const char*);
bool isSubdir(const char*);
char* buildPath(const char*, const char*);
int runExecutable(const char*, const char*, const char*);
int compareExpectedWithActual(const char*, const char*);
DIR* openDirAndValidate(const char*);
void runAndTestAllExecutables(Configs*, int);
void writeRowResult (char*, int, int, Configs*);
int createResultFile();
Configs* createConfigs(const char*);
void freeConfigs(Configs*);

int main(int argc, char* argv[]) {
	validate(argc == 2, "Error: missing configuration file path", true);

    Configs* configs = createConfigs(argv[1]);
    int resultsFd = createResultFile();

	runAndTestAllExecutables(configs, resultsFd);

    close(resultsFd);
    freeConfigs(configs);
    return 0;
}

void validate(int condition, const char* errorMessage, bool exitOnFailure) {
    if (!condition) {
        printf("Error: %s\n",errorMessage);
        if (exitOnFailure) {
            exit(EXIT_FAILURE);
        }
    }
}

void freePointers(int n, ...) {
    va_list args;
    va_start(args, n);

    for (int i = 0; i < n; i++) {
        free(va_arg(args, void*));
    }
    va_end(args);
}

char* getLine(int configFd) {
    char* line = (char*)malloc((MAX_LINE_LENGTH + 1)*sizeof(char));
    long bytesRead = 1;
    char lastCharRead = 0;
    int i;

    for (i = 0; i < MAX_LINE_LENGTH && bytesRead > 0 && lastCharRead != '\n'; i++)
    {
        bytesRead = read(configFd, line + i, 1);
        validate(SUCCESSFUL_READ(bytesRead), "Could not read from config file, please make sure your configuration file has everything required and try again", true);
        lastCharRead = line[i];
    }
    line[i - 1] = '\0';
    return line;
}

int equals(const char* str1, const char* str2)
{
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return 0;
        }
        str1++;
        str2++;
    }
    return (*str1 == '\0' && *str2 == '\0');
}

void* mallocWithValidation(int size) {
    void* alloc = malloc(size);
    validate(alloc != NULL, "Memory allocation failed", true);
    return alloc;
}

Configs* createConfigs(const char* configFilePath) {
    int fd = open(configFilePath, O_RDONLY);
    validate(SUCCESSFUL_OPEN(fd), "Failed to open config file", true);

    char* lines[CONFIG_LINES];
    for (int i = 0; i < CONFIG_LINES; i++)
    {
        lines[i] = (char*)mallocWithValidation(CONFIG_LINES * sizeof(char));
        lines[i] = getLine(fd);
    }
    close(fd);

    Configs* configs = (Configs*)mallocWithValidation(sizeof(Configs));

    configs->rootFolder = lines[0];
    configs->input = lines[1];
    configs->expectedOutput = lines[2];

    return configs;
}

void freeConfigs(Configs* configs) {
    free(configs->rootFolder);
    free(configs->input);
    free(configs->expectedOutput);
    free(configs);
}

int createResultFile() {
    int fd = open(RESULT_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    validate(SUCCESSFUL_OPEN(fd), "Failed to create results file\n", true);
    return fd;
}

void writeRowResult(char* name, int score, int resultsFd, Configs* configs) {
    int originalStdOut = dup(STDOUT_FILENO);
    dup2(resultsFd, STDOUT_FILENO);
    printf("%s,%d\n", name, score);
    fflush(stdout);
    fsync(resultsFd);
    dup2(originalStdOut, STDOUT_FILENO);
}

char* buildPath(const char* root, const char* subPath) {
    char* result = (char*)mallocWithValidation(sizeof(char) * SPRINTF_OUTPUT);
    sprintf(result, "%s/%s", root, subPath);
    return result;
}


void runAndTestAllExecutables(Configs* configs, int resultsFd) {
    DIR* rootDir = openDirAndValidate(configs->rootFolder);

    struct dirent* dir;
    while ((dir = readdir(rootDir)) != NULL)
    {
        if (!isSubdir(dir->d_name)) continue;

        char* userDirPath = buildPath(configs->rootFolder, dir->d_name);
        DIR* userDir = openDirAndValidate(userDirPath);

        if (dir->d_type == DT_DIR) {
            bool foundCFile = false;
            int score = 0;

            struct dirent* file;
            while ((file = readdir(userDir)) != NULL) {
                if (foundCFile || !isSubdir(file->d_name) || !isCfile(file->d_name)) continue;
                
                foundCFile = true;

                char* filePath = buildPath(userDirPath, file->d_name);
                char* executablePath = buildPath(userDirPath, EXECUTABLE_NAME);
                char* actualOutputPath = buildPath(userDirPath, EXE_OUTPUT);

                if (compileFile(filePath, executablePath)) {
                    runExecutable(executablePath, actualOutputPath, configs->input);
                    score = compareExpectedWithActual(configs->expectedOutput, actualOutputPath) == 2 ? 100 : 0;
                }
                writeRowResult(dir->d_name, score, resultsFd, configs);
                validate(unlink(actualOutputPath) == 0, "Failed to delete the output file", true);
                freePointers(2, filePath, executablePath, actualOutputPath);
            }
            if (!foundCFile) {
                writeRowResult(dir->d_name, score, resultsFd, configs);
            }
        }
        free(userDirPath);
        closedir(userDir);
    }
    closedir(rootDir);
}

int isCfile(const char* filename) {
    int i;
    for (i = 0; filename[i] != '\0'; i++);
    if (i >= 4 && filename[i - 2] == '.' && (filename[i - 1] == 'c' || filename[i - 1] == 'C'))
    {
        return 1;
    }
    return 0;
}

bool isSubdir(const char* name) {
    return !(equals(name, ".") || equals(name, ".."));
}

DIR* openDirAndValidate(const char* dirPath) {
    DIR* dir = opendir(dirPath);
    validate(SUCCESSFUL_OPEN_DIR(dir), "Could not open directory", true);
    return dir;
}

bool compileFile(const char* sourcePath, const char* executablePath) {
    char* args[] = { "gcc", (char*)sourcePath, "-o",(char*)executablePath, NULL };
    int compileStatus = forkAndExecvp(args);
    validate(compileStatus == 0, "Compilation failed", false);
    return compileStatus == 0;
}

int runExecutable(const char* exePath, const char* actualOutputPath, const char* inputPath) {
    int inputFd = open(inputPath, O_RDONLY);
    int outputFd = open(actualOutputPath, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    validate(SUCCESSFUL_OPEN(inputFd) && SUCCESSFUL_OPEN(outputFd), "Failed to redirect I/O", true);

    int originalStdin = dup(STDIN_FILENO);
    int originalStdout = dup(STDOUT_FILENO);

    dup2(inputFd, STDIN_FILENO);
    dup2(outputFd, STDOUT_FILENO);

    char* args[] = { (char*)exePath, NULL };
    int result = forkAndExecvp(args);

    dup2(originalStdin, STDIN_FILENO);
    dup2(originalStdout, STDOUT_FILENO);

    close(inputFd);
    close(outputFd);
    close(originalStdin);
    close(originalStdout);

    return result;
}

int compareExpectedWithActual(const char* expectedOutput, const char* userActualOutput)
{
    char* args[] = { "./comp.out", (char*)userActualOutput, (char*)expectedOutput, NULL };
    return forkAndExecvp(args);
}

int forkAndExecvp(char* args[]) {
    pid_t pid = fork();
    validate(pid >= 0, "fork error", true);

    if (pid == CHILD_PROCESS)
    {
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
