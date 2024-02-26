#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <cv.h>

void handler(int s)
{
    write(1, "is ", 3);
}

int main()
{
    int ch1, ch2;
    ch1 = fork();
    
    if (ch1 == 0) {
        struct sigaction act;
        act.sa_handler = handler;
        act.sa_flags = 0;
        sigemptyset(&act.sa_mask);
        sigaction(SIGUSR1, &act, NULL);
        sleep(2);
        cs_lock(1929);
        write(1, "writer\n", 7);
        exit(0);
    }
    else {
        ch2 = fork(); 
        if (ch2 == 0) {
            sleep(3);
            cs_lock(1929);
            write(1, "the best ", 9);
            exit(0);
        }
        else {
            cs_lock(1929);
            write(1, "Thomas Mann ", 12);
            sleep(4);
            kill(ch1, SIGUSR1);
            sleep(2);
            cs_unlock(1929);
            for (int i = 0; i < 2; ++i) {
                wait(NULL);
            }
        }
    }
}

