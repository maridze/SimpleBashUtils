#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define INPUT_STRING 4096

typedef struct Options {
  bool e;
  bool i;
  bool v;
  bool c;
  bool l;
  bool n;
  bool h;
  bool s;
  bool f;
  bool o;
} Options;

void o_flag(int count_files, char *filename, regex_t regular_expression,
            regmatch_t matches[10], const Options *const options,
            char input[INPUT_STRING], int line_number) {
  int match_count = 0;
  int offset = 0;
  while (regexec(&regular_expression, input + offset, 10, matches, 0) == 0) {
    if (matches[0].rm_so != -1 && matches[0].rm_eo != -1) {
      if (count_files == 1 || options->h == 1) {
        for (int i = matches[0].rm_so; i < matches[0].rm_eo; i++) {
          putchar(input[i + offset]);
          if (i == matches[0].rm_eo - 1) putchar('\n');
        }
      } else {
        if (match_count == 0) printf("%s:", filename);
        if (options->n && match_count == 0) printf("%d:", line_number);
        for (int i = matches[0].rm_so; i < matches[0].rm_eo; i++) {
          putchar(input[i + offset]);
          if (i == matches[0].rm_eo - 1) putchar('\n');
        }
      }
      offset += matches[0].rm_eo;
      match_count++;
    } else {
      break;
    }
  }
}

void print_file(int count_files, char *filename, regex_t regular_expression,
                FILE *file, const Options *const options) {
  int line_number = 0;
  int file_match = 0;
  char input[INPUT_STRING];
  regmatch_t matches[10];
  while (fgets(input, sizeof(input), file) != NULL) {
    line_number++;
    int status = regexec(&regular_expression, input, 1, matches, 0);
    if ((status == 0 && options->v == 0) ||
        (status == REG_NOMATCH && options->v == 1)) {
      if (options->c == 1)
        file_match++;
      else if (options->l == 1) {
        printf("%s\n", filename);
        break;
      } else if (options->o == 1) {
        if (options->v == 1) {
          printf("%s:%s", filename, input);
        } else {
          o_flag(count_files, filename, regular_expression, matches, options,
                 input, line_number);
        }
      } else if (options->n == 1) {
        if (count_files == 1 || options->h == 1)
          printf("%d:%s", line_number, input);
        else
          printf("%s:%d:%s", filename, line_number, input);
      } else {
        if (count_files == 1 || options->h == 1)
          printf("%s", input);
        else
          printf("%s:%s", filename, input);
      }
      if (feof(file) && (options->v == 1) && (options->c == 0)) {
        printf("\n");
      }
    }
  }
  if (options->c == 1) {
    if (count_files == 1 || options->h == 1)
      printf("%d\n", file_match);
    else if (options->l == 1 && file_match > 0) {
      printf("%s:1\n", filename);
    } else
      printf("%s:%d\n", filename, file_match);
  }
  if (options->l == 1 && file_match > 0) {
    printf("%s\n", filename);
  }
}

int get_regex_flags(const Options *const options) {
  int flags = REG_EXTENDED;

  if (options->i == 1) {
    flags |= REG_ICASE;
  }
  return flags;
}

void patterns_init(char *temp, const char *optarg) {
  if (temp[0] == 0) {
    strcpy(temp, optarg);
  } else if (strcmp(temp, ".*") != 0) {
    strcat(temp, "|");
    strcat(temp, optarg);
  }
}

void flag_f(char *patterns, const char *optarg, const Options *const options) {
  FILE *file;
  file = fopen(optarg, "r");
  char temp_pattern[INPUT_STRING] = {0};
  if (file == NULL) {
    if ((options->s) == 0) fprintf(stderr, "no such file");
  } else {
    while (fgets(temp_pattern, INPUT_STRING, file) != NULL) {
      patterns_init(patterns, temp_pattern);
    }
  }
  fclose(file);
}

int set_option(Options *const options, int argc, char **argv, char *patterns) {
  int result = 1;
  int symbol;
  while ((symbol = getopt_long(argc, argv, "e:ivclnhsf:o", NULL, NULL)) != -1) {
    switch (symbol) {
      case 'e': {
        options->e = 1;
        patterns_init(patterns, optarg);
        break;
      }
      case 'i': {
        options->i = 1;
        break;
      }
      case 'v': {
        options->v = 1;
        break;
      }
      case 'c': {
        options->c = 1;
        break;
      }
      case 'l': {
        options->l = 1;
        break;
      }
      case 'n': {
        options->n = 1;
        break;
      }
      case 'h': {
        options->h = 1;
        break;
      }
      case 's': {
        options->s = 1;
        break;
      }
      case 'f': {
        options->f = 1;
        flag_f(patterns, optarg, options);
        break;
      }
      case 'o': {
        options->o = 1;
        break;
      }
      default: {
        result = 0;
        break;
      }
    }
  }
  return result;
}

void print_result(char *temp, Options *const options, int argc, char **argv) {
  FILE *file;
  int count_files = 0;
  int flags = get_regex_flags(options);

  if (options->f == 0 && options->e == 0) {
    strcpy(temp, argv[optind]);
    optind += 1;
  }

  for (int i = optind; i < argc; i++) {
    if ((file = fopen(argv[i], "r")) != NULL) {
      count_files++;
      fclose(file);
    } else {
      if ((options->s) == 0)
        fprintf(stderr, "grep: %s No such fileor directory\n", argv[i]);
      count_files++;
    }
  }

  for (int i = optind; i < argc; i++) {
    if ((file = fopen(argv[i], "r")) != NULL) {
      regex_t regex;
      regcomp(&regex, temp, flags);
      print_file(count_files, argv[i], regex, file, options);
      regfree(&regex);
      fclose(file);
    }
  }
}

int main(int argc, char *argv[]) {
  Options options = {0};
  char patterns[INPUT_STRING] = {0};
  int check = set_option(&options, argc, argv, patterns);
  if (check && argc >= 3) {
    print_result(patterns, &options, argc, argv);
  }
  return 1;
}