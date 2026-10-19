#pragma once
#include "noncopyable.h"
#include <string>
#include <sys/types.h>
namespace me {
namespace FileUtil {
    class AppendFile : noncopyable {
    public:
        explicit AppendFile(std::string filename);

        ~AppendFile();

        void append(const char* logline, size_t len);

        void flush();

        off_t writtenBytes() const { return writtenBytes_; }

    private:
        size_t write(const char* logline, size_t len);

        int fd_;
        off_t writtenBytes_;
    };
}
}