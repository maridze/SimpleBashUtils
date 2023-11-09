#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const char SHORT_OPTIONS[] = "beEnstTv";

typedef struct Options {
  bool b;
  bool e;
  bool n;
  bool s;
  bool t;
  bool v;
} Options;

void print_file(FILE *file, const Options *const options);

static const struct option LONG_OPTIONS[] = {{"number-nonblank", 0, NULL, 'b'},
                                             {"number", 0, NULL, 'n'},
                                             {"squeeze-blank", 0, NULL, 's'},
                                             {0, 0, 0, 0}};

static void set_option(const char option, Options *const options, int *status) {
  switch (option) {
    case 'b':
      options->b = true;
      break;
    case 'e':
      options->v = true;
      options->e = true;
      break;
    case 'E':
      options->e = true;
      break;
    case 'n':
      options->n = true;
      break;
    case 's':
      options->s = true;
      break;
    case 't':
      options->v = true;
      options->t = true;
      break;
    case 'T':
      options->t = true;
      break;
    case 'v':
      options->v = true;
      break;
    case '?':
    default:
      *status = 0;
      break;
  }
}

int init_flags(int argc, char *argv[], Options *const options) {
  int status = 1;
  int long_options_index = 0;
  char current_option =
      getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &long_options_index);
  while (current_option != -1) {
    set_option(current_option, options, &status);
    current_option = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS,
                                 &long_options_index);
  }
  if (options->b) {
    options->n = false;
  }
  return status;
}

void process_files(int file_count, char *const file_path[],
                   const Options *const options) {
  FILE *current_file = NULL;
  while (file_count > 0) {
    current_file = fopen(*file_path, "r");
    if (current_file == NULL) {
      fprintf(stderr, "no such file");
      break;
    } else {
      print_file(current_file, options);
      file_count--;
      file_path++;
    }
  }
}

void print_file(FILE *file, const Options *const options) {
  int ch, prev;
  int current_line = 1;
  int squeeze = 0;
  int s_status = 1;
  for (prev = '\n'; (ch = getc(file)) != EOF; prev = ch) {
    if (options->s == 1) {
      if (ch == '\n' && prev == '\n') {
        if (squeeze == 1) {
          s_status = 0;
        }
        squeeze++;
      } else {
        squeeze = 0;
        s_status = 1;
      }
    }

    if (options->b == 1 && s_status == 1) {
      if (prev == '\n' && ch != '\n') {
        printf("%*d\t", 6, current_line);
        current_line++;
      }
    }

    if (options->n == 1 && options->b == 0 && prev == '\n' && s_status == 1) {
      printf("%*d\t", 6, current_line);
      current_line++;
    }

    if (options->v == 1 && s_status == 1) {
      if ((unsigned char)ch > 127 && (unsigned char)ch < 160) {
        printf("M-");
      }
      if (((unsigned char)ch < 32 ||
           ((unsigned char)ch > 126 && (unsigned char)ch < 160)) &&
          (unsigned char)ch != '\n' && (unsigned char)ch != '\t') {
        printf("^");
        if ((unsigned char)ch > 126) {
          ch = (unsigned char)ch - 64;
        } else {
          ch = (unsigned char)ch + 64;
        }
      }
    }
    if (options->t == 1 && ch == '\t' && s_status == 1) {
      printf("^");
      ch = 'I';
    }

    if (options->e == 1 && ch == '\n' && s_status == 1) {
      printf("$");
    }
    if (s_status == 1) fputc(ch, stdout);
  }
  fclose(file);
}

int main(int argc, char *argv[]) {
  Options options = {0};
  if (init_flags(argc, argv, &options)) {
    process_files(argc - optind, argv + optind, &options);
  }
  return 0;
}