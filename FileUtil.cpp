#include "FileUtil.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace me {
namespace FileUtil {

    AppendFile::AppendFile(std::string filename)
        // O_WRONLY写追加 | O_CREAT不存在就创建 | O_APPEND原子追加 | O_CLOEXEC exec时自动关闭fd
        : fd_(::open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644))
        , writtenBytes_(0) {
        assert(fd_ >= 0);
        // 原生fd没有setbuffer！用户缓冲区逻辑我们在append里自己管理
    }

    AppendFile::~AppendFile() { ::close(fd_); }

    void AppendFile::append(const char* logline, size_t len) {
        size_t written = 0;
        while (written != len) {
            size_t remain = len - written;
            size_t n = write(logline + written, remain);
            if (n != remain) {
                int err = errno;
                if (err != 0) {
                    fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
                    break;
                }
            }
            written += n;
        }
        writtenBytes_ += written;
    }

    void AppendFile::flush() {
        // fd版本：fflush是FILE*接口，fd用fsync/fdatasync刷内核pagecache到磁盘
        // fdatasync只刷数据，不更新元数据，日志优先选这个，性能更好
        ::fdatasync(fd_);
    }

    size_t AppendFile::write(const char* logline, size_t len) {
        // ssize_t: 有符号，可以返回负数表示错误
        ssize_t ret = ::write(fd_, logline, len);
        if (ret < 0) {
            return 0;
        }
        return static_cast<size_t>(ret);
    }

} // namespace FileUtil
} // namespace me
