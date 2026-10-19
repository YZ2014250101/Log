#include "LogFile.h"
#include "Timestamp.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
using namespace me;

LogFile::LogFile(const string& basename, off_t rollSize, bool threadSafe, int flushInterval, int checkEveryN)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , checkEveryN_(checkEveryN)
    , count_(0)
    , startOfPeriod_(0)
    , lastRoll_(0)
    , lastFlush_(0) {
    assert(basename.find('/') == string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        append_unlocked(logline, len);
    }
}

void LogFile::flush() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len) {
    file_->append(logline, len);

    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    } else {
        ++count_;
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_) {
                rollFile();
            } else if (now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new FileUtil::AppendFile(filename));
        return true;
    }
    return false;
}

void LogFile::setBasename(const string& basename) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        basename_ = basename;
    }
}
string LogFile::getLogFileName(const string& basename, time_t* now) {
    string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;
    Timestamp ts = Timestamp::now();
    if (now != nullptr) {
        *now = ts.secondsSinceEpoch();
    }
    filename += "_";
    filename += ts.toFormattedString();
    filename += "_";
    char buf[256];
    if (::gethostname(buf, sizeof buf) == 0) {
        buf[sizeof(buf) - 1] = '\0';
        filename += buf;
    } else {
        filename += "unknownHostname";
    }
    filename += "_";
    filename += std::to_string(::getpid());

    filename += ".log";

    return filename;
}
