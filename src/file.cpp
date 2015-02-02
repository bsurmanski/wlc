
#include "file.hpp"

#include <string.h>
#include <sys/stat.h>

#if defined WIN32
const char *builtinInclude = "C:/Program Files/WLC/include/;C:/INSTALL/WLC/include/";
#else
const char *builtinInclude = "/usr/local/include/wl/;/usr/include/wl/";
#endif

/**
 * successively copies each environment variable into buf with each call.
 *
 * buf: a buffer to hold the environment variable
 * env: a pointer to a ';' seperated list of environment variables. will be modified
 * after each call to point to the start of the next environment variable.
 */
void envitern(char *buf, const char **env, size_t max) {
    if(!buf || !env || !*env || !max) return;

    const char *start = *env;

    // find occurance of ';' or NULL
    while(**env && **env != ';') {
        (*env)++;
    }

    int n = (*env - start);
    n = (n >= max ? max : n);
    if(**env == ';') (*env)++;

    memcpy(buf, start, n);
    buf[n] = '\0';
}

bool fileExists(std::string filenm) {
    struct stat st;
    return stat(filenm.c_str(), &st) == 0;
}

std::string findFile(std::string filenm) {
    struct stat st;
    if(stat(filenm.c_str(), &st) == 0) {
        return filenm;
    }

    char buffer[256] = {0};
    const char *incenv = getenv("WLINCLUDE");
    if(!incenv) incenv = builtinInclude;

    while(incenv && *incenv) {
        envitern(buffer, &incenv, 255);
        if(fileExists(buffer + filenm)) {
            return buffer + filenm;
        }
    }

    return filenm; //cannot find
}
