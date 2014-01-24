struct tm_struct
{
    int tm_sec
    int tm_min
    int tm_hour
    int tm_mday
    int tm_mon
    int tm_year
    int tm_wday
    int tm_yday
    int tm_isdst
}

long time(long^ timer);
long mktime(tm_struct^ tmptr);
double difftime(long end, long beg);
char^ ctime(long^ timer);

long clock();

