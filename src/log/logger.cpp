#include "logger.h"
using namespace tubekit::log;

const char *logger::s_flag[FLAG_COUNT] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"};

logger *logger::m_instance = nullptr;

logger::logger() : m_fp(nullptr)
{
}

logger::~logger()
{
    close();
}

logger *logger::instance()
{
    if (m_instance == nullptr)
    {
        m_instance = new logger();
    }
    return m_instance;
}

void logger::open(const string &log_file_path)
{
    close();
    m_fp = fopen(log_file_path.c_str(), "a+");
    if (m_fp == nullptr)
    {
        printf("open log file failed: %s\n", log_file_path.c_str());
        exit(1);
    }
}

void logger::close()
{
    if (m_fp != nullptr)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

void logger::debug(const char *file, int line, const char *format, ...)
{
    //定义一个具有va_list型的变量，这个变量是指向参数的指针
    va_list arg_ptr;
    //第一个参数指向可变列表的地址，地址自动增加
    va_start(arg_ptr, format);
    log(DEBUG, file, line, format, arg_ptr);
    va_end(arg_ptr); //结束可变参数列表
}

void logger::info(const char *file, int line, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log(INFO, file, line, format, arg_ptr);
    va_end(arg_ptr);
}

void logger::warn(const char *file, int line, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log(WARN, file, line, format, arg_ptr);
    va_end(arg_ptr);
}

void logger::error(const char *file, int line, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log(ERROR, file, line, format, arg_ptr);
    va_end(arg_ptr);
}

void logger::fatal(const char *file, int line, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log(FATAL, file, line, format, arg_ptr);
    va_end(arg_ptr);
}

void logger::log(flag f, const char *file, int line, const char *format, va_list arg_ptr)
{
    if (m_fp == nullptr)
    {
        printf("open log file failed: m_fp==nullptr\n");
        exit(1);
    }
    //获取时间信息
    time_t ticks = time(nullptr);
    struct tm *ptm = localtime(&ticks);
    char buf[32];
    memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S  ", ptm);
    //锁定日志文件保证不同线程不会同时使用
    flockfile(m_fp); //输出时间
    //使用日志文件
    fprintf(m_fp, buf);
    fprintf(m_fp, "%s  ", s_flag[f]); //输出flag
    fprintf(m_fp, "%s:%d  ", file, line);
    vfprintf(m_fp, format, arg_ptr); //格式化打印
    fprintf(m_fp, "\r\n");
    //刷新文件并释放锁
    fflush(m_fp);
    funlockfile(m_fp);
}