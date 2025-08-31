#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define DELIMITER " "
#define FIELD_COUNT 6
#define BUF_LEN 1024
#define NAME_ABBREV_LEN 3

// Special characters
#define ASTERISK "*"
#define COMMA ","
#define HYPHEN "-"

// Valid values
#define MINUTE_RANGE_MIN 0
#define MINUTE_RANGE_MAX 59
#define HOUR_RANGE_MIN 0
#define HOUR_RANGE_MAX 23
#define DAY_OF_MONTH_RANGE_MIN 1
#define DAY_OF_MONTH_RANGE_MAX 31
#define MONTH_RANGE_MIN 1
#define MONTH_RANGE_MAX 12
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
#define MONTHS_COUNT 12
#define DAY_OF_WEEK_RANGE_MIN 0
#define DAY_OF_WEEK_RANGE_MAX 6
const char* DAYS_OF_WEEK[] = {"Sunday",
                              "Monday",
                              "Tuesday",
                              "Wednesday",
                              "Thursday",
                              "Friday",
                              "Saturday"};
#define DAYS_COUNT 7

// Field types
bool parse_numeric_list(char* field, char** buf, int min, int max);
bool parse_named_field_list(char* field,
                            char** buf,
                            int min,
                            int max,
                            const char** names,
                            unsigned int names_len,
                            int offset);
bool parse_numeric_range(char* field, char** buf, int min, int max);
bool parse_named_field_range(char* field,
                             char** buf,
                             int min,
                             int max,
                             const char** names,
                             unsigned int names_len,
                             int offset);
bool parse_number(char* field, int* value, int min, int max);
bool parse_named_field(char* field,
                       char** buf,
                       int min,
                       int max,
                       const char** names,
                       unsigned int names_len,
                       int offset);

// Fields
char* parse_minute_field(char* field);
char* parse_hour_field(char* field);
char* parse_day_of_month_field(char* field);
char* parse_month_field(char* field);
char* parse_day_of_week_field(char* field);

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

    // Day of month field
    field = fields[2];
    char* day_of_month_field = parse_day_of_month_field(field);
    if (!day_of_month_field) return 1;

    // Month field
    field = fields[3];
    char* month_field = parse_month_field(field);
    if (!month_field) return 1;

    // Day of week field
    field = fields[4];
    char* day_of_week_field = parse_day_of_week_field(field);
    if (!day_of_week_field) return 1;

    // Command
    char* command = fields[5];

    // Pretty print the cron expression
    printf("Run '%s' %s %s %s %s %s\n",
           command,
           minute_field,
           hour_field,
           day_of_month_field,
           month_field,
           day_of_week_field);

    // Free dynamically allocated memory
    for (unsigned int i = 0; i < FIELD_COUNT; i++) {
        free(fields[i]);
    }
    free(minute_field);
    free(hour_field);
    free(day_of_month_field);
    free(month_field);
    free(day_of_week_field);

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
        int value;
        if (!parse_number(token, &value, min, max)) {
            return false;
        }
        char value_fmt[3];
        sprintf(value_fmt, "%d", value);
        strcat(*buf, value_fmt);
        token = strtok(NULL, COMMA);
        if (token) {
            strcat(*buf, ", ");
        }
    }
    return true;
}

bool parse_named_field_list(char* field,
                            char** buf,
                            int min,
                            int max,
                            const char** names,
                            unsigned int names_len,
                            int offset) {
    *buf = malloc(BUF_LEN);
    if (!*buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    strcpy(*buf, "");
    char* token = strtok(field, COMMA);
    while (token) {
        char* value = NULL;
        if (!parse_named_field(
                token, &value, min, max, names, names_len, offset)) {
            return false;
        }
        strcat(*buf, value);
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
    int value_from;
    if (!parse_number(token, &value_from, min, max)) {
        return false;
    }
    token = strtok(NULL, HYPHEN);
    int value_to;
    if (!parse_number(token, &value_to, min, max)) {
        return false;
    }
    if (value_from >= value_to) {
        fprintf(
            stderr, "Error: Invalid range: %d to %d\n", value_from, value_to);
        return false;
    }
    char value_fmt[9];
    sprintf(value_fmt, "%d to %d", value_from, value_to);
    strcat(*buf, value_fmt);
    return true;
}

bool parse_named_field_range(char* field,
                             char** buf,
                             int min,
                             int max,
                             const char** names,
                             unsigned int names_len,
                             int offset) {
    *buf = malloc(BUF_LEN);
    if (!*buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    strcpy(*buf, "");
    char* token = strtok(field, HYPHEN);
    char* value_from = NULL;
    if (!parse_named_field(
            token, &value_from, min, max, names, names_len, offset)) {
        return false;
    }
    token = strtok(NULL, HYPHEN);
    char* value_to = NULL;
    if (!parse_named_field(
            token, &value_to, min, max, names, names_len, offset)) {
        return false;
    }
    int idx_from = -1;
    int idx_to = -1;
    for (unsigned int i = 0; i < names_len; i++) {
        if (!strcmp(value_from, names[i])) {
            idx_from = i;
        }
        if (!strcmp(value_to, names[i])) {
            idx_to = i;
        }
    }
    if (idx_from >= idx_to) {
        fprintf(
            stderr, "Error: Invalid range: %s to %s\n", value_from, value_to);
        return false;
    }
    char value_fmt[BUF_LEN];
    sprintf(value_fmt, "%s to %s", value_from, value_to);
    strcat(*buf, value_fmt);
    return true;
}

bool parse_number(char* field, int* value, int min, int max) {
    unsigned long len = strlen(field);
    for (unsigned int i = 0; i < len; i++) {
        if (!isdigit(field[i])) {
            fprintf(stderr, "Error: Invalid number provided: %s\n", field);
            return false;
        }
    }
    *value = atoi(field);
    if (*value < min || *value > max) {
        fprintf(stderr,
                "Error: Number out of valid range [%d, %d]: %d\n",
                min,
                max,
                *value);
        return false;
    }
    return true;
}

bool parse_named_field(char* field,
                       char** buf,
                       int min,
                       int max,
                       const char** names,
                       unsigned int names_len,
                       int offset) {
    // Find the name at the specified index
    bool is_number = true;
    unsigned long len = strlen(field);
    for (unsigned int i = 0; i < len; i++) {
        if (!isdigit(field[i])) {
            is_number = false;
        }
    }
    if (is_number) {
        int value = atoi(field);
        if (value < min || value > max) {
            fprintf(stderr,
                    "Error: Number out of valid range [%d, %d]: %d\n",
                    min,
                    max,
                    value);
            return false;
        }
        *buf = malloc(BUF_LEN);
        if (!*buf) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            return NULL;
        }
        const char* str = names[value + offset];
        strcpy(*buf, str);
        return true;
    }
    // Check if the field is a name
    if (len != NAME_ABBREV_LEN) {
        fprintf(stderr, "Error: Invalid value: %s\n", field);
        return false;
    }
    for (unsigned int i = 0; i < names_len; i++) {
        if (!strncasecmp(field, names[i], NAME_ABBREV_LEN)) {
            *buf = malloc(BUF_LEN);
            if (!*buf) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                return NULL;
            }
            const char* str = names[i];
            strcpy(*buf, str);
            return true;
        }
    }
    return false;
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
        char* str = "every minute";
        strcpy(buf, str);
        return buf;
    };
    // N,N,N,...
    if (strstr(field, COMMA)) {
        char* str = "at minutes ";
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
        char* str = "from minutes ";
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
    char* str = "at minute ";
    strcpy(buf, str);
    int value;
    if (!parse_number(field, &value, MINUTE_RANGE_MIN, MINUTE_RANGE_MAX)) {
        if (buf) free(buf);
        return NULL;
    }
    char value_fmt[3];
    sprintf(value_fmt, "%d", value);
    strcat(buf, value_fmt);
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
    int value;
    if (!parse_number(field, &value, HOUR_RANGE_MIN, HOUR_RANGE_MAX)) {
        if (buf) free(buf);
        return NULL;
    }
    char value_fmt[3];
    sprintf(value_fmt, "%d", value);
    strcat(buf, value_fmt);
    return buf;
}

char* parse_day_of_month_field(char* field) {
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
        if (!parse_numeric_list(
                field, &list, DAY_OF_MONTH_RANGE_MIN, DAY_OF_MONTH_RANGE_MAX)) {
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
        if (!parse_numeric_range(field,
                                 &range,
                                 DAY_OF_MONTH_RANGE_MIN,
                                 DAY_OF_MONTH_RANGE_MAX)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, range);
        return buf;
    }
    // N
    char* str = "on day ";
    strcpy(buf, str);
    int value;
    if (!parse_number(
            field, &value, DAY_OF_MONTH_RANGE_MIN, DAY_OF_MONTH_RANGE_MAX)) {
        if (buf) free(buf);
        return NULL;
    }
    char value_fmt[3];
    sprintf(value_fmt, "%d", value);
    strcat(buf, value_fmt);
    return buf;
}

char* parse_month_field(char* field) {
    if (!field) return NULL;
    char* buf = malloc(BUF_LEN);
    if (!buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    // *
    if (!strcmp(field, ASTERISK)) {
        char* str = "of every month";
        strcpy(buf, str);
        return buf;
    };
    // N,N,N,...
    if (strstr(field, COMMA)) {
        char* str = "of months ";
        strcpy(buf, str);
        char* list = NULL;
        if (!parse_named_field_list(field,
                                    &list,
                                    DAY_OF_MONTH_RANGE_MIN,
                                    DAY_OF_MONTH_RANGE_MAX,
                                    MONTHS_OF_YEAR,
                                    MONTHS_COUNT,
                                    -1)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, list);
        return buf;
    }
    // N-N
    if (strstr(field, HYPHEN)) {
        char* str = "of months ";
        strcpy(buf, str);
        char* range = NULL;
        if (!parse_named_field_range(field,
                                     &range,
                                     DAY_OF_MONTH_RANGE_MIN,
                                     DAY_OF_MONTH_RANGE_MAX,
                                     MONTHS_OF_YEAR,
                                     MONTHS_COUNT,
                                     -1)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, range);
        return buf;
    }
    // N
    char* str = "in ";
    strcpy(buf, str);
    char* value = NULL;
    if (!parse_named_field(field,
                           &value,
                           MONTH_RANGE_MIN,
                           MONTH_RANGE_MAX,
                           MONTHS_OF_YEAR,
                           MONTHS_COUNT,
                           -1)) {
        if (buf) free(buf);
        return NULL;
    }
    strcat(buf, value);
    return buf;
}

char* parse_day_of_week_field(char* field) {
    if (!field) return NULL;
    char* buf = malloc(BUF_LEN);
    if (!buf) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return NULL;
    }
    // *
    if (!strcmp(field, ASTERISK)) {
        char* str = "on every day";
        strcpy(buf, str);
        return buf;
    };
    // N,N,N,...
    if (strstr(field, COMMA)) {
        char* str = "on days ";
        strcpy(buf, str);
        char* list = NULL;
        if (!parse_named_field_list(field,
                                    &list,
                                    DAY_OF_WEEK_RANGE_MIN,
                                    DAY_OF_WEEK_RANGE_MAX,
                                    DAYS_OF_WEEK,
                                    DAYS_COUNT,
                                    0)) {
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
        if (!parse_named_field_range(field,
                                     &range,
                                     DAY_OF_WEEK_RANGE_MIN,
                                     DAY_OF_WEEK_RANGE_MAX,
                                     DAYS_OF_WEEK,
                                     DAYS_COUNT,
                                     0)) {
            if (buf) free(buf);
            return NULL;
        }
        strcat(buf, range);
        return buf;
    }
    // N
    char* str = "on ";
    strcpy(buf, str);
    char* value = NULL;
    if (!parse_named_field(field,
                           &value,
                           DAY_OF_WEEK_RANGE_MIN,
                           DAY_OF_WEEK_RANGE_MAX,
                           DAYS_OF_WEEK,
                           DAYS_COUNT,
                           0)) {
        if (buf) free(buf);
        return NULL;
    }
    strcat(buf, value);
    return buf;
}
