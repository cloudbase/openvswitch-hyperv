#include <stdio.h>

FILE *output_fp;
char* mode = "a";
char* output_fn = "temp.out";

inline void openlog(char* program_name_copy, int md, int type)
{
    char* mode = "r";
    if (program_name_copy != NULL)
        {
        fopen_s(&output_fp, output_fn, mode);
        }
}

inline void syslog(int syslog_level, char* type, char* line)
    {
    char* mode = "a";

    fopen_s(&output_fp, output_fn, mode);

    if (output_fp != NULL) 
        {
        fprintf(output_fp, "%type", line);
        }
    }

inline void closelog(void)
    {
    if ( output_fp != NULL )
        fclose(output_fp);
    }