#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

// 根据操作系统引入相应的头文件
#ifdef _WIN32
#include <windows.h>
// Windows 平台，通过 strrchr 找到最后一个 '\' 来获取文件基本名
#define file_base_name(file) (strrchr(file,'\\') ? strrchr(file,'\\')+1 : file)
#else
// 非 Windows 平台（如 Linux/Unix），引入 pthread.h 用于线程安全
#include <pthread.h>
// 非 Windows 平台，通过 strrchr 找到最后一个 '/' 来获取文件基本名
#define file_base_name(file) (strrchr(file,'/') ? strrchr(file,'/')+1 : file)
#endif
#define _str(x) #x

// =========================================================
// 日志等级定义
// =========================================================
#define DBG_LEVEL_NONE    0   // 无日志输出
#define DBG_LEVEL_ERROR   1   // 错误（最紧急）
#define DBG_LEVEL_WARN    2   // 警告
#define DBG_LEVEL_INFO    3   // 信息
#define DBG_LEVEL_DEBUG   4   // 调试（最不紧急）

// =========================================================
// 全局配置变量
// =========================================================


// 每个文件可定义自己的局部日志等级
// 优先级高于全局等级，但不能超过全局等级
#ifndef DBG_MODULE_LEVEL
#define DBG_MODULE_LEVEL DBG_GLOBAL_LEVEL
#endif

// =========================================================
// 彩色定义
// =========================================================
#define CLR_RED     "\033[31m"   // 红色
#define CLR_YELLOW  "\033[33m"  // 黄色
#define CLR_GREEN   "\033[32m"   // 绿色
#define CLR_BLUE    "\033[34m"   // 蓝色
#define CLR_RESET   "\033[0m"   // 重置颜色



// 获取当前时间戳，格式化到 buf
static inline void dbg_timestamp(char *buf, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info); // 格式: 年-月-日 时:分:秒
}

// 将日志等级数字转换为对应的字符串）
static inline const char *dbg_level_str(int lvl) {
    switch (lvl) {
        case DBG_LEVEL_ERROR: return "ERROR";
        case DBG_LEVEL_WARN:  return "WARN";
        case DBG_LEVEL_INFO:  return "INFO";
        case DBG_LEVEL_DEBUG: return "DEBUG";
        default: return "LOG";
    }
}

// =========================================================
// 可选线程安全（通过 DBG_THREAD_SAFE 控制）
// =========================================================
#ifdef _WIN32
    // Windows 平台暂未实现线程锁
    #define DBG_LOCK()
    #define DBG_UNLOCK()
#else
    // 非 Windows 平台使用 pthread 互斥锁
    static pthread_mutex_t DBG_MUTEX = PTHREAD_MUTEX_INITIALIZER;
    #define DBG_LOCK()   do { if (DBG_THREAD_SAFE) pthread_mutex_lock(&DBG_MUTEX); } while(0)
    #define DBG_UNLOCK() do { if (DBG_THREAD_SAFE) pthread_mutex_unlock(&DBG_MUTEX); } while(0)
#endif

// =========================================================
// 核心日志打印函数（可变参数宏）
// =========================================================
/*
 *  实际执行日志格式化和输出的核心宏
 * 使用方法: 供 log_err, log_warn 等用户宏调用
 * 参数:
 *   level: 日志等级
 *   color: 日志等级对应的颜色宏
 *   M: 格式字符串（包含可变参数的日志信息）
 *   ...: 可变参数列表
 * 输出格式占位符说明 (由 dbg_log_format 控制):
 *   %T: 时间戳 (e.g., "2025-10-28 23:00:00")
 *   %L: 日志等级字符串 (e.g., "[ERROR]", "[INFO]")，可带颜色
 *   %F: 源文件基本名 (e.g., "main.c")
 *   %l: 行号 (e.g., "100")
 *   %f: 函数名 (e.g., "my_function()")
 *   %M: 用户传入的日志信息 (M, ##__VA_ARGS__)
 * 默认格式: "%T [%L] %F:%l %f() %M\n"
 */
#define DBG_LOG(level, color, M, ...) \
    do { \
        if ((level) <= DBG_MODULE_LEVEL && (level) <= DBG_GLOBAL_LEVEL) { \
            DBG_LOCK(); \
            char tbuf[32]; dbg_timestamp(tbuf, sizeof(tbuf));  \
            FILE *out = DBG_OUTPUT ? DBG_OUTPUT : stderr; \
            const char *lvl_str = dbg_level_str(level); \
            const char *clr_on = (DBG_ENABLE_COLOR ? color : "");\
            const char *clr_off = (DBG_ENABLE_COLOR ? CLR_RESET : "");  \
            \
             \
            const char *fmt = dbg_log_format ? dbg_log_format : "%T [%L] %F:%l %f() %M\n"; /* 获取格式串 */ \
            const char *p = fmt; \
            while (*p) { \
                if (*p == '%' && *(p+1)) { \
                    p++; \
                    switch (*p) { \
                        case 'T': fprintf(out, "%s", tbuf); break;  \
                        case 'L': fprintf(out, "%s%s%s", clr_on, lvl_str, clr_off); break; \
                        case 'F': fprintf(out, "%s", file_base_name(__FILE__)); break; \
                        case 'l': fprintf(out, "%d", __LINE__); break;\
                        case 'f': fprintf(out, "%s", __func__); break;  \
                        case 'M': fprintf(out, M, ##__VA_ARGS__); break; \
                        default: fputc(*p, out); break;  \
                    } \
                } else fputc(*p, out);  \
                p++; \
            } \
            fflush(out);\
            DBG_UNLOCK(); \
        } \
    } while(0)

// =========================================================
// 用户接口宏
// =========================================================

// 功能: 打印 ERROR 等级日志 (红色),执行goto error
#define log_err_goto(M, ...)  do{DBG_LOG(DBG_LEVEL_ERROR, CLR_RED, M, ##__VA_ARGS__);goto error;}while(0)

// 功能: 打印 ERROR 等级日志 (红色)
#define log_err(M, ...)  DBG_LOG(DBG_LEVEL_ERROR, CLR_RED, M, ##__VA_ARGS__)

// 功能: 打印 WARN 等级日志 (黄色)
#define log_warn(M, ...) DBG_LOG(DBG_LEVEL_WARN,  CLR_YELLOW, M, ##__VA_ARGS__)

// 功能: 打印 INFO 等级日志 (绿色)
#define log_info(M, ...) DBG_LOG(DBG_LEVEL_INFO,  CLR_GREEN, M, ##__VA_ARGS__)

//  打印 DEBUG 等级日志 (蓝色)
#define debug(M, ...)    DBG_LOG(DBG_LEVEL_DEBUG, CLR_BLUE,  M, ##__VA_ARGS__)

//  清除当前 errno 并返回其描述字符串，如果 errno 为 0 则返回 "None"
#define clean_errno() (errno == 0 ? "None" : strerror(errno))

//  检查条件 A 是否成立，若不成立，则打印 ERROR 日志并跳转到 `error:` 标签
#define check(A, M, ...)     do { if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; } } while(0)

//  无条件打印 ERROR 日志并跳转到 `error:` 标签 (用于默认分支或不可达代码)
#define sentinel(M, ...)     do { log_err(M, ##__VA_ARGS__); errno=0; goto error; } while(0)

//  检查内存分配是否成功，若失败则打印 ERROR 日志并跳转到 `error:` 标签
#define check_mem(A)         check((A), "Memory alloc failed: %s", _str(A))

//  检查条件 A 是否成立，若不成立，则打印 DEBUG 日志并跳转到 `error:` 标签
#define check_debug(A, M, ...) do { if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; } } while(0)



// =========================================================
// 全局变量默认值
// =========================================================
#ifndef DBG_DEFINE_GLOBALS
int DBG_GLOBAL_LEVEL = DBG_LEVEL_DEBUG;          // 默认日志等级为 DEBUG
int DBG_ENABLE_COLOR = 1;                        // 默认启用颜色
int DBG_THREAD_SAFE  = 0;                        // 默认禁用线程安全
FILE *DBG_OUTPUT     = NULL;                     // 默认输出到 stderr
const char *dbg_log_format = "%T [%L] %F:%l %f() %M\n"; // 默认日志格式: 时间 [等级] 文件名:行号 函数名() 用户信息\n
#endif // !dbg_log_format




#endif // __dbg_h__