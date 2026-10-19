#pragma once
#include "Timestamp.h"
#include <string>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
namespace me {
namespace ProcessInfo {
    pid_t pid();
    string pidString();
    uid_t uid();
    string username();
    Timestamp startTime();
    string hostname();
}
}