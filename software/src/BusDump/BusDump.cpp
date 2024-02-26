#include <thread>

#include <Dump.hpp>
#include <atomic>
#include <cstring>
#include <signal.h>
#include <unistd.h>

std::atomic<bool> quit = false;

void got_signal(int) {
    // Signal handler function.
    // Set the flag and return.
    // Never do real work inside this function.
    // See also: man 7 signal-safety
    quit.store(true);
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    CAN::Interfaces::Dump dumper;
    dumper.startReceiving();
    // while (true) { // Require force killing of program with ^C
    //     using namespace std::chrono_literals;
    //     std::this_thread::sleep_for(1s);
    //     if (quit.load()) {
    //         break;
    //     }
    // }
    return 0;
}