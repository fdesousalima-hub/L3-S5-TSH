#include "../libs/utils.h"

int pwd() {
    char *current_path = getenv("CURRENT_PATH"); // get the env value 

    if (current_path == NULL) { // check getenv return value
        perror("Getenv.\n");
        exit(errno);
    }
    display(1, 2, current_path, "\n"); // display it
    return 1;
}

int main(int ac, char **av) {
    if (ac != 1) { // if pwd is used with arguments
        display(1, 1, "pwd: usage: pwd\n"); // display error message
        return 1;
    }
    return pwd() == 1 ? 0 : 1;
}
