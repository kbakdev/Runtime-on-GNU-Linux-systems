#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

int main() {
    DIR *dp;
    struct dirent *ep;

    if((dp = opendir("."))) {
        while(NULL != (ep = readdir(dp))) {
            printf("%s\n", ep->d_name);
        }

        closedir(dp);
    }

    return 0;
}