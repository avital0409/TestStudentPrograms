#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define BYTES_TO_READ 1

void validate (int condition, char* errorMessage)
{
    if (!condition)
    {
        exit(0);
    }
}

int openFileAndValidate(char* file)
{
    int fd = open(file, O_RDONLY);
    validate(fd >= 0, "Couldn't open file");
    return fd;
}

int fileSizesEqual(int fd1, int fd2)
{
    long ls1 = lseek(fd1, 0, SEEK_END);
    long ls2 = lseek(fd2, 0, SEEK_END);
    return ls1 == ls2;
}

void resetOffsetsToBeginning(int fd1, int fd2)
{
    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);
}

void closeFds(int fd1, int fd2)
{
    close(fd1);
    close(fd2);
}

int main(int argc, char* argv[])
{
    validate(argc == 3, "The program compares between the content of two files, please send 2 args that represent the path of the files");
    
    char*  file1Path = argv[1];
    char*  file2Path = argv[2];
    
    int fd1 = openFileAndValidate(file1Path);
    int fd2 = openFileAndValidate(file2Path);
    
    if (!fileSizesEqual(fd1, fd2))
    {
        return 1;
    }
    else
    {
        resetOffsetsToBeginning(fd1, fd2);
        char *buf1[BYTES_TO_READ + 1];
        char *buf2[BYTES_TO_READ + 1];
        long bytesRead1 = read(fd1, buf1, BYTES_TO_READ);
        long bytesRead2 = read(fd2, buf2, BYTES_TO_READ);
        
        while (bytesRead1 > 0 && bytesRead2 > 0)
        {
            if (buf1[0] != buf2[0])
            {
                closeFds(fd1, fd2);
                return 1;
            }
            bytesRead1 = read(fd1, buf1, BYTES_TO_READ);
            bytesRead2 = read(fd2, buf2, BYTES_TO_READ);
        }
        
        validate(bytesRead1 == 0 && bytesRead2 == 0, "An unknown error has occured");
        closeFds(fd1, fd2);
        return 2;
    }
}
