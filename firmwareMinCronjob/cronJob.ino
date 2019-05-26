#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#define CRON_MAX_SECONDS 60
#define CRON_MAX_MINUTES 60
#define CRON_MAX_HOURS 24
#define CRON_MAX_DAYS_OF_WEEK 8
#define CRON_MAX_DAYS_OF_MONTH 32
#define CRON_MAX_MONTHS 12
#define CRON_MAX_YEARS_DIFF 4

#define CRON_CF_SECOND 0
#define CRON_CF_MINUTE 1
#define CRON_CF_HOUR_OF_DAY 2
#define CRON_CF_DAY_OF_WEEK 3
#define CRON_CF_DAY_OF_MONTH 4
#define CRON_CF_MONTH 5
#define CRON_CF_YEAR 6

#define CRON_CF_ARR_LEN 7

#define CRON_INVALID_INSTANT ((time_t) -1)

const char* const DAYS_ARR[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
#define CRON_DAYS_ARR_LEN 7
const char* const MONTHS_ARR[] = { "FOO", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
#define CRON_MONTHS_ARR_LEN 13

#define CRON_MAX_STR_LEN_TO_SPLIT 256
#define CRON_MAX_NUM_TO_SRING 1000000000
/* computes number of digits in decimal number */
#define CRON_NUM_OF_DIGITS(num) (abs(num) < 10 ? 1 : \
                                 (abs(num) < 100 ? 2 : \
                                  (abs(num) < 1000 ? 3 : \
                                   (abs(num) < 10000 ? 4 : \
                                    (abs(num) < 100000 ? 5 : \
                                     (abs(num) < 1000000 ? 6 : \
                                      (abs(num) < 10000000 ? 7 : \
                                       (abs(num) < 100000000 ? 8 : \
                                        (abs(num) < 1000000000 ? 9 : 10)))))))))

const char PROGMEM E1[] = "Unsigned integer parse error 1";
const char PROGMEM E2[] = "Specified range requires two fields";
const char PROGMEM E3[] = "Unsigned integer parse error 2";
const char PROGMEM E4[] = "Unsigned integer parse error 3";
const char PROGMEM E5[] = "Specified range exceeds maximum";
const char PROGMEM E6[] = "Specified range is less than minimum";
const char PROGMEM E7[] = "Specified range start exceeds range end";
const char PROGMEM E8[] = "Comma split error";
const char PROGMEM E9[] = "Incrementer must have two fields";
const char PROGMEM E10[] = "Unsigned integer parse error 4";
const char PROGMEM E11[] = "Incrementer may not be zero";
const char PROGMEM E12[] = "Invalid NULL expression";
const char PROGMEM E13[] = "Invalid number of fields, expression must consist of 6 fields";

const char* const error_table[] PROGMEM = {E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13};
#define cron_malloc(x) malloc(x);
#define cron_free(x) free(x);

void getError(char *eror_code, char * err_show) {
  eror_code[0] = ' ';
  int index = atoi(eror_code) - 1;
  if (index < 0)index = 0;
  strcpy_P(err_show, (char*)pgm_read_word(&(error_table[index]))); // Necessary casts and dereferencing, just copy.
}


void cron_set_bit(uint8_t* rbyte, int idx) {
  uint8_t j = (uint8_t) (idx / 8);
  uint8_t k = (uint8_t) (idx % 8);

  rbyte[j] |= (1 << k);
}

void cron_del_bit(uint8_t* rbyte, int idx) {
  uint8_t j = (uint8_t) (idx / 8);
  uint8_t k = (uint8_t) (idx % 8);

  rbyte[j] &= ~(1 << k);
}
uint8_t cron_get_bit(uint8_t* rbyte, int idx) {
  uint8_t j = (uint8_t) (idx / 8);
  uint8_t k = (uint8_t) (idx % 8);

  if (rbyte[j] & (1 << k)) {
    return 1;
  } else {
    return 0;
  }
}

void free_splitted(char** splitted, size_t len) {
  size_t i;
  if (!splitted) return;
  for (i = 0; i < len; i++) {
    if (splitted[i]) {
      cron_free(splitted[i]);
    }
  }
  cron_free(splitted);
}

char* strdupl(const char* str, size_t len) {
  if (!str) return NULL;
  char* res = (char*) cron_malloc(len + 1);
  if (!res) return NULL;
  memset(res, 0, len + 1);
  memcpy(res, str, len);
  return res;
}
int to_upper(char* str) {
  if (!str) return 1;
  int i;
  for (i = 0; '\0' != str[i]; i++) {
    int c = (int)str[i];
    str[i] = (char) toupper(c);
  }
  return 0;
}

char* to_string(int num) {
  if (abs(num) >= CRON_MAX_NUM_TO_SRING) return NULL;
  char* str = (char*) cron_malloc(CRON_NUM_OF_DIGITS(num) + 1);
  if (!str) return NULL;
  int res = sprintf(str, "%d", num);
  if (res < 0) {
    cron_free(str);
    return NULL;
  }
  return str;
}

char* str_replace(char *orig, const char *rep, const char *with) {
  char *result; /* the return string */
  char *ins; /* the next insert point */
  char *tmp; /* varies */
  size_t len_rep; /* length of rep */
  size_t len_with; /* length of with */
  size_t len_front; /* distance between rep and end of last rep */
  int count; /* number of replacements */
  if (!orig) return NULL;
  if (!rep) rep = "";
  if (!with) with = "";
  len_rep = strlen(rep);
  len_with = strlen(with);

  ins = orig;
  for (count = 0; NULL != (tmp = strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  /* first time through the loop, all the variable are set correctly
    from here on,
    tmp points to the end of the result string
    ins points to the next occurrence of rep in orig
    orig points to the remainder of orig after "end of rep"
  */
  tmp = result = (char*) cron_malloc(strlen(orig) + (len_with - len_rep) * count + 1);
  if (!result) return NULL;

  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep; /* move to next "end of rep" */
  }
  strcpy(tmp, orig);
  return result;
}

unsigned int parse_uint(const char* str, int* errcode) {
  char* endptr;
  errno = 0;
  long int l = strtol(str, &endptr, 0);
  if (errno == ERANGE || *endptr != '\0' || l < 0 || l > INT_MAX) {
    *errcode = 1;
    return 0;
  } else {
    *errcode = 0;
    return (unsigned int) l;
  }
}

char** split_str(const char* str, char del, size_t* len_out) {
  size_t i;
  size_t stlen = 0;
  size_t len = 0;
  int accum = 0;
  char* buf = NULL;
  char** res = NULL;
  size_t bi = 0;
  size_t ri = 0;
  char* tmp;

  if (!str) goto return_error;
  for (i = 0; '\0' != str[i]; i++) {
    stlen += 1;
    if (stlen >= CRON_MAX_STR_LEN_TO_SPLIT) goto return_error;
  }

  for (i = 0; i < stlen; i++) {
    int c = str[i];
    if (del == str[i]) {
      if (accum > 0) {
        len += 1;
        accum = 0;
      }
    } else if (!isspace(c)) {
      accum += 1;
    }
  }
  /* tail */
  if (accum > 0) {
    len += 1;
  }
  if (0 == len) return NULL;

  buf = (char*) cron_malloc(stlen + 1);
  if (!buf) goto return_error;
  memset(buf, 0, stlen + 1);
  res = (char**) cron_malloc(len * sizeof(char*));
  if (!res) goto return_error;
  memset(res, 0, len * sizeof(char*));

  for (i = 0; i < stlen; i++) {
    int c = str[i];
    if (del == str[i]) {
      if (bi > 0) {
        tmp = strdupl(buf, bi);
        if (!tmp) goto return_error;
        res[ri++] = tmp;
        memset(buf, 0, stlen + 1);
        bi = 0;
      }
    } else if (!isspace(c)) {
      buf[bi++] = str[i];
    }
  }
  /* tail */
  if (bi > 0) {
    tmp = strdupl(buf, bi);
    if (!tmp) goto return_error;
    res[ri++] = tmp;
  }
  cron_free(buf);
  *len_out = len;
  return res;

return_error:
  if (buf) {
    cron_free(buf);
  }
  free_splitted(res, len);
  *len_out = 0;
  return NULL;
}

char* replace_ordinals(char* value, const char* const * arr, size_t arr_len) {
  size_t i;
  char* cur = value;
  char* res = NULL;
  int first = 1;
  for (i = 0; i < arr_len; i++) {
    char* strnum = to_string((int) i);
    if (!strnum) {
      if (!first) {
        cron_free(cur);
      }
      return NULL;
    }
    res = str_replace(cur, arr[i], strnum);
    cron_free(strnum);
    if (!first) {
      cron_free(cur);
    }
    if (!res) {
      return NULL;
    }
    cur = res;
    if (first) {
      first = 0;
    }
  }
  return res;
}

int has_char(char* str, char ch) {
  size_t i;
  size_t len = 0;
  if (!str) return 0;
  len = strlen(str);
  for (i = 0; i < len; i++) {
    if (str[i] == ch) return 1;
  }
  return 0;
}
unsigned int* get_range(char* field, unsigned int min, unsigned int max, const char** error) {

  char** parts = NULL;
  size_t len = 0;
  unsigned int* res = (unsigned int*) cron_malloc(2 * sizeof(unsigned int));
  if (!res) goto return_error;

  res[0] = 0;
  res[1] = 0;
  if (1 == strlen(field) && '*' == field[0]) {
    res[0] = min;
    res[1] = max - 1;
  } else if (!has_char(field, '-')) {
    int err = 0;
    unsigned int val = parse_uint(field, &err);
    if (err) {
      *error = "E1";
      goto return_error;
    }

    res[0] = val;
    res[1] = val;
  } else {
    parts = split_str(field, '-', &len);
    if (2 != len) {
      *error = "E2";
      goto return_error;
    }
    int err = 0;
    res[0] = parse_uint(parts[0], &err);
    if (err) {
      *error = "E3";
      goto return_error;
    }
    res[1] = parse_uint(parts[1], &err);
    if (err) {
      *error = "E4";
      goto return_error;
    }
  }
  if (res[0] >= max || res[1] >= max) {
    *error = "E5";
    goto return_error;
  }
  if (res[0] < min || res[1] < min) {
    *error = "E6";
    goto return_error;
  }
  if (res[0] > res[1]) {
    *error = "E7";
    goto return_error;
  }

  free_splitted(parts, len);
  *error = NULL;
  return res;

return_error:
  free_splitted(parts, len);
  if (res) {
    cron_free(res);
  }

  return NULL;
}

void set_months(char* value, uint8_t* targ, const char** error) {
  int err;
  unsigned int i;
  unsigned int max = 12;

  char* replaced = NULL;

  err = to_upper(value);
  if (err) return;
  replaced = replace_ordinals(value, MONTHS_ARR, CRON_MONTHS_ARR_LEN);
  if (!replaced) return;

  set_number_hits(replaced, targ, 1, max + 1, error);
  cron_free(replaced);

  /* ... and then rotate it to the front of the months */
  for (i = 1; i <= max; i++) {
    if (cron_get_bit(targ, i)) {
      cron_set_bit(targ, i - 1);
      cron_del_bit(targ, i);
    }
  }
}

void set_days(char* field, uint8_t* targ, int max, const char** error) {
  if (1 == strlen(field) && '?' == field[0]) {
    field[0] = '*';
  }
  set_number_hits(field, targ, 0, max, error);
}

void set_days_of_month(char* field, uint8_t* targ, const char** error) {
  /* Days of month start with 1 (in Cron and Calendar) so add one */
  if (1 == strlen(field) && '?' == field[0]) {
    field[0] = '*';
  }
  set_number_hits(field, targ, 1, CRON_MAX_DAYS_OF_MONTH, error);
}

void cron_parse_expr(const char* expression, cron_expr* target, const char** error) {
  const char* err_local;
  size_t len = 0;
  char** fields = NULL;
  char* days_replaced = NULL;
  if (!error) {
    error = &err_local;
  }
  *error = NULL;
  if (!expression) {
    *error = "E12";
    goto return_res;
  }

  fields = split_str(expression, ' ', &len);
  if (len != 6) {
    *error = "E13";
    goto return_res;
  }

  set_number_hits(fields[0], target->seconds, 0, 60, error);
  if (*error) goto return_res;
  set_number_hits(fields[1], target->minutes, 0, 60, error);
  if (*error) goto return_res;
  set_number_hits(fields[2], target->hours, 0, 24, error);
  if (*error) goto return_res;

  to_upper(fields[5]);
  days_replaced = replace_ordinals(fields[5], DAYS_ARR, CRON_DAYS_ARR_LEN);
  set_days(days_replaced, target->days_of_week, 8, error);
  cron_free(days_replaced);
  if (*error) goto return_res;
  if (cron_get_bit(target->days_of_week, 7)) {
    /* Sunday can be represented as 0 or 7*/
    cron_set_bit(target->days_of_week, 0);
    cron_del_bit(target->days_of_week, 7);
  }

  set_days_of_month(fields[3], target->days_of_month, error);
  if (*error) goto return_res;
  set_months(fields[4], target->months, error);
  if (*error) goto return_res;
  goto return_res;
return_res:
  free_splitted(fields, len);
}
void set_number_hits(const char* value, uint8_t* target, unsigned int min, unsigned int max, const char** error) {
  size_t i;
  unsigned int i1;
  size_t len = 0;

  char** fields = split_str(value, ',', &len);
  if (!fields) {
    *error = "E8";
    goto return_result;
  }

  for (i = 0; i < len; i++) {
    if (!has_char(fields[i], '/')) {
      /* Not an incrementer so it must be a range (possibly empty) */

      unsigned int* range = get_range(fields[i], min, max, error);

      if (*error) {
        if (range) {
          cron_free(range);
        }
        goto return_result;

      }

      for (i1 = range[0]; i1 <= range[1]; i1++) {
        cron_set_bit(target, i1);

      }
      cron_free(range);

    } else {
      size_t len2 = 0;
      char** split = split_str(fields[i], '/', &len2);
      if (2 != len2) {
        *error = "E9";
        free_splitted(split, len2);
        goto return_result;
      }
      unsigned int* range = get_range(split[0], min, max, error);
      if (*error) {
        if (range) {
          cron_free(range);
        }
        free_splitted(split, len2);
        goto return_result;
      }
      if (!has_char(split[0], '-')) {
        range[1] = max - 1;
      }
      int err = 0;
      unsigned int delta = parse_uint(split[1], &err);
      if (err) {
        *error = "E10";
        cron_free(range);
        free_splitted(split, len2);
        goto return_result;
      }
      if (0 == delta) {
        *error = "E11";
        cron_free(range);
        free_splitted(split, len2);
        goto return_result;
      }
      for (i1 = range[0]; i1 <= range[1]; i1 += delta) {
        cron_set_bit(target, i1);
      }
      free_splitted(split, len2);
      cron_free(range);

    }
  }
  goto return_result;

return_result:
  free_splitted(fields, len);
}
