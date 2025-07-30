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
const char* DAYS_OF_WEEK[] = {"Sunday",
                              "Monday",
                              "Tuesday",
                              "Wednesday",
                              "Thursday",
                              "Friday",
                              "Saturday"};
const char* MONTHS_OF_YEAR[] = {"January",
                                "February",
                                "March",
                                "April",
                                "May",
                                "June",
                                "July",
                                "August",
                                "September",
                                "October",
                                "Nobember",
                                "December"};

// Field types
bool parse_numeric_list(char* field, char** buf, int min, int max);
bool parse_numeric_range(char* field, char** buf, int min, int max);
bool parse_number(char* field, char** buf, int min, int max);

// Fields
char* parse_minute_field(char* field);
char* parse_hour_field(char* field);
char* parse_day_field(char* field);

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"* * * * * [COMMAND]\" \n", argv[0]);
        return 1;
    }

    char* cron_expr = strdup(argv[1]);
    if (!cron_expr) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return 1;
    }

    char* fields[FIELD_COUNT] = {0};

    char* token = strtok(cron_expr, DELIMITER);
    unsigned int field_no = 0;
    while (token) {
        if (field_no < FIELD_COUNT) {
            fields[field_no] = strdup(token);
            if (!fields[field_no]) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                return 1;
            }
        }
        token = strtok(NULL, DELIMITER);
        field_no++;
    }

    free(cron_expr);

    if (field_no != FIELD_COUNT) {
        fprintf(stderr,
                "Error: Invalid field count: %d (expected %d)\n",
                field_no,
                FIELD_COUNT);
        return 1;
    }

    char* field = NULL;

    // Minute field
    field = fields[0];
    char* minute_field = parse_minute_field(field);
    if (!minute_field) return 1;

    // Hour field
    field = fields[1];
    char* hour_field = parse_hour_field(field);
    if (!hour_field) return 1;

    // Day field
    field = fields[2];
    char* day_field = parse_day_field(field);
    if (!day_field) return 1;

    printf("%s %s %s\n", minute_field, hour_field, day_field);

    // Free dynamically allocated memory
    for (unsigned int i = 0; i < FIELD_COUNT; i++) {
        free(fields[i]);
    }
    free(minute_field);
    free(hour_field);
    free(day_field);

    return 0;
}

bool parse_numeric_list(char* field, char** buf, int min, int max) {
    *buf = malloc(BUF_LEN);
    if (!*buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    strcpy(*buf, "");
    char* token = strtok(field, COMMA);
    while (token) {
        unsigned long len = strlen(token);
        for (unsigned int i = 0; i < len; i++) {
            if (!isdigit(token[i])) {
                fprintf(stderr, "Error: Invalid number provided: %s\n", token);
                return false;
            }
            int value = atoi(token);
            if (value < min || value > max) {
                fprintf(stderr,
                        "Error: Number out of valid range [%d, %d]: %d\n",
                        min,
                        max,
                        value);
                return false;
            }
            char value_fmt[3];
            sprintf(value_fmt, "%d", value);
            strcat(*buf, value_fmt);
        }
        token = strtok(NULL, COMMA);
        if (token) {
            strcat(*buf, ", ");
        }
    }
    return true;
}

bool parse_numeric_range(char* field, char** buf, int min, int max) {
    *buf = malloc(BUF_LEN);
    if (!*buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    strcpy(*buf, "");
    char* token = strtok(field, HYPHEN);
    unsigned long len = 0;
    len = strlen(token);
    for (unsigned int i = 0; i < len; i++) {
        if (!isdigit(token[i])) {
            fprintf(stderr, "Error: Invalid number provided: %s\n", token);
            return false;
        }
    }
    int value_from = atoi(token);
    if (value_from < min || value_from > max) {
        fprintf(stderr,
                "Error: Number out of valid range [%d, %d]: %d\n",
                min,
                max,
                value_from);
        return false;
    }
    token = strtok(NULL, HYPHEN);
    len = strlen(token);
    for (unsigned int i = 0; i < len; i++) {
        if (!isdigit(token[i])) {
            fprintf(stderr, "Error: Invalid number provided: %s\n", token);
            return false;
        }
    }
    int value_to = atoi(token);
    if (value_to < min || value_to > max) {
        fprintf(stderr,
                "Error: Number out of valid range [%d, %d]: %d\n",
                min,
                max,
                value_to);
        return false;
    }
    char value_fmt[9];
    sprintf(value_fmt, "%d to %d", value_from, value_to);
    strcat(*buf, value_fmt);
    return true;
}

bool parse_number(char* field, char** buf, int min, int max) {
    *buf = malloc(BUF_LEN);
    if (!*buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    strcpy(*buf, "");
    unsigned long len = strlen(field);
    for (unsigned int i = 0; i < len; i++) {
        if (!isdigit(field[i])) {
            fprintf(stderr, "Error: Invalid number provided: %s\n", field);
            return false;
        }
    }
    int value = atoi(field);
    if (value < min || value > max) {
        fprintf(stderr,
                "Error: Number out of valid range [%d, %d]: %d\n",
                min,
                max,
                value);
        return false;
    }
    sprintf(*buf, "%d", value);
    return true;
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
        char* list = NULL;
        if (!parse_numeric_list(
                field, &list, MINUTE_RANGE_MIN, MINUTE_RANGE_MAX)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, list);
        return buf;
    }
    // N-N
    if (strstr(field, HYPHEN)) {
        char* str = "From minutes ";
        strcpy(buf, str);
        char* range = NULL;
        if (!parse_numeric_range(
                field, &range, MINUTE_RANGE_MIN, MINUTE_RANGE_MAX)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, range);
        return buf;
    }
    // N
    char* str = "At minute ";
    strcpy(buf, str);
    char* value = NULL;
    if (!parse_number(field, &value, MINUTE_RANGE_MIN, MINUTE_RANGE_MAX)) {
        if (buf) free(buf);
        return NULL;
    }
    strcat(buf, value);
    return buf;
}

char* parse_hour_field(char* field) {
    if (!field) return NULL;
    char* buf = malloc(BUF_LEN);
    if (!buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    // *
    if (!strcmp(field, ASTERISK)) {
        char* str = "of every hour";
        strcpy(buf, str);
        return buf;
    };
    // N,N,N,...
    if (strstr(field, COMMA)) {
        char* str = "of hours ";
        strcpy(buf, str);
        char* list = NULL;
        if (!parse_numeric_list(field, &list, HOUR_RANGE_MIN, HOUR_RANGE_MAX)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, list);
        return buf;
    }
    // N-N
    if (strstr(field, HYPHEN)) {
        char* str = "during hours ";
        strcpy(buf, str);
        char* range = NULL;
        if (!parse_numeric_range(
                field, &range, HOUR_RANGE_MIN, HOUR_RANGE_MAX)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, range);
        return buf;
    }
    // N
    char* str = "of hour ";
    strcpy(buf, str);
    char* value = NULL;
    if (!parse_number(field, &value, HOUR_RANGE_MIN, HOUR_RANGE_MAX)) {
        if (buf) free(buf);
        return NULL;
    }
    strcat(buf, value);
    return buf;
}

char* parse_day_field(char* field) {
    if (!field) return NULL;
    char* buf = malloc(BUF_LEN);
    if (!buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    // *
    if (!strcmp(field, ASTERISK)) {
        char* str = "of every day";
        strcpy(buf, str);
        return buf;
    };
    // N,N,N,...
    if (strstr(field, COMMA)) {
        char* str = "on days ";
        strcpy(buf, str);
        char* list = NULL;
        if (!parse_numeric_list(field, &list, DAY_RANGE_MIN, DAY_RANGE_MAX)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, list);
        return buf;
    }
    // N-N
    if (strstr(field, HYPHEN)) {
        char* str = "on days ";
        strcpy(buf, str);
        char* range = NULL;
        if (!parse_numeric_range(field, &range, DAY_RANGE_MIN, DAY_RANGE_MAX)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, range);
        return buf;
    }
    // N
    char* str = "on day ";
    strcpy(buf, str);
    char* value = NULL;
    if (!parse_number(field, &value, DAY_RANGE_MIN, DAY_RANGE_MAX)) {
        if (buf) free(buf);
        return NULL;
    }
    strcat(buf, value);
    return buf;
}
