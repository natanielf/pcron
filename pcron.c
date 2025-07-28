#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIMITER " "
#define FIELD_COUNT 6
#define BUF_LEN 1024

// Special characters
#define ASTERISK "*"
#define COMMA ","
#define HYPHEN "-"

// Valid values
#define MINUTE_RANGE_MIN 0
#define MINUTE_RANGE_MAX 59
#define HOUR_RANGE_MIN 0
#define HOUR_RANGE_MAX 23
#define DAY_RANGE_MIN 1
#define DAY_RANGE_MAX 31
#define DAYS_OF_WEEK                               \
    {"Sunday",   "Monday", "Tuesday", "Wednesday", \
     "Thursday", "Friday", "Saturday"}
#define MONTHS_OF_YEAR                                                  \
    {"January", "February", "March",     "April",   "May",      "June", \
     "July",    "August",   "September", "October", "Nobember", "December"}

char* parse_minute_field(char* field);

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"* * * * * [COMMAND]\" \n", argv[0]);
        return 1;
    }

    char* cron_expr = argv[1];

    char* token = NULL;
    token = strtok(cron_expr, DELIMITER);
    unsigned int field_count = 0;
    while (token) {
        token = strtok(NULL, DELIMITER);
        field_count++;
    }

    if (field_count != FIELD_COUNT) {
        fprintf(stderr, "Error: Invalid field count: %d (expected %d)\n",
                field_count, FIELD_COUNT);
        return 1;
    }

    token = strtok(cron_expr, DELIMITER);
    char* minute_field = parse_minute_field(token);
    if (!minute_field) {
        fprintf(stderr, "Error parsing minute field\n");
        return 1;
    }

    printf("%s\n", minute_field);
    free(minute_field);

    return 0;
}

char* parse_minute_field(char* field) {
    if (!field) return NULL;
    char* buf = malloc(BUF_LEN);
    if (!buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    // *
    if (!strcmp(field, ASTERISK)) {
        char* str = "Every minute";
        strcpy(buf, str);
        return buf;
    };
    // N,N,N,...
    if (strstr(field, COMMA)) {
        char* str = "At minutes ";
        strcpy(buf, str);
        char* token = NULL;
        token = strtok(field, COMMA);
        while (token) {
            unsigned long len = strlen(token);
            for (unsigned int i = 0; i < len; i++) {
                if (!isdigit(token[i])) {
                    free(buf);
                    return NULL;
                }
                int minute = atoi(token);
                if (minute < MINUTE_RANGE_MIN || minute > MINUTE_RANGE_MAX) {
                    free(buf);
                    return NULL;
                }
                char minute_fmt[3];
                sprintf(minute_fmt, "%d", minute);
                strcat(buf, minute_fmt);
            }
            token = strtok(NULL, COMMA);
            if (token) {
                strcat(buf, ", ");
            }
        }
        return buf;
    }
    // N-N
    if (strstr(field, HYPHEN)) {
        char* str = "From minutes ";
        strcpy(buf, str);
        char* token = NULL;
        token = strtok(field, HYPHEN);
        if (!token) {
            free(buf);
            return NULL;
        }
        unsigned long len = 0;
        len = strlen(token);
        for (unsigned int i = 0; i < len; i++) {
            if (!isdigit(token[i])) {
                free(buf);
                return NULL;
            }
        }
        int minute_from = atoi(token);
        if (minute_from < MINUTE_RANGE_MIN || minute_from > MINUTE_RANGE_MAX) {
            free(buf);
            return NULL;
        }
        token = strtok(NULL, HYPHEN);
        len = strlen(token);
        for (unsigned int i = 0; i < len; i++) {
            if (!isdigit(token[i])) {
                free(buf);
                return NULL;
            }
        }
        int minute_to = atoi(token);
        if (minute_to < MINUTE_RANGE_MIN || minute_to > MINUTE_RANGE_MAX) {
            free(buf);
            return NULL;
        }
        char minute_fmt[9];
        sprintf(minute_fmt, "%d to %d", minute_from, minute_to);
        strcat(buf, minute_fmt);
        return buf;
    }
    // N
    unsigned long len = strlen(field);
    for (unsigned int i = 0; i < len; i++) {
        if (!isdigit(field[i])) {
            free(buf);
            return NULL;
        }
    }
    int minute = atoi(field);
    if (minute < MINUTE_RANGE_MIN || minute > MINUTE_RANGE_MAX) {
        free(buf);
        return NULL;
    }
    snprintf(buf, BUF_LEN, "At minute %d", minute);
    return buf;
}
