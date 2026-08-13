#include <errno.h>
#include <error.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_SIZE 1024

char msg[256];

void reverse_string(char *str, int len) {
  char temp;
  int i;

  for (i = 0; i < len / 2; i++) {
    temp = str[i];
    str[i] = str[len - i - 1];
    str[len - i - 1] = temp;
  }
}

void write_in_reverse(int fd_in, int fd_out) {
  char buf[MAX_SIZE];
  ssize_t bytesRead, totalRead = 0;

  // calculte size of input file to track progress
  ssize_t filesize = lseek(fd_in, 0, SEEK_END);
  lseek(fd_in, 0, SEEK_SET);

  int last_progress = -1;
  while ((bytesRead = read(fd_in, buf, MAX_SIZE)) > 0) {
    totalRead += bytesRead;

    reverse_string(buf, bytesRead);
    lseek(fd_out, filesize - totalRead, SEEK_SET);

    ssize_t n = write(fd_out, buf, bytesRead);
    if (n == -1) {
      perror("failed to write");
      exit(1);
    }

    int progress = (int)(totalRead * 100 / filesize);
    if (progress != last_progress) {
      last_progress = progress;
      sprintf(msg, "writing in progress: %d %% written\r", progress);
      write(1, msg, strlen(msg));
    }
  }
  if (bytesRead == -1) {
    perror("failed to read");
    exit(1);
  }
  write(1, "\n", 1);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    sprintf(msg, "Usage: %s <input-file-path> <output-file-path>\n", argv[0]);
    write(STDERR_FILENO, msg, strlen(msg));
    exit(1);
  }

  char output_path[256];
  int fd_in, fd_out;

  // Open input file read-only; fail if doesn't exist or isn't readable
  fd_in = open(argv[1], O_RDONLY);
  if (fd_in == -1) {
    perror(argv[1]);
    exit(1);
  }

  // Create directory "Assignment"; provide RWX permissions to user
  if (mkdir("./Assignment", 0700)) {
    if (errno == EEXIST) {
      // Directory already exists, do nothing
    } else {
      perror("mkdir");
      exit(1);
    }
  }

  // Open output file write-only; overwrite if file exists already
  // Provide Read and Write permissions to User
  sprintf(output_path, "./Assignment/%s", argv[2]);
  fd_out = open(output_path, O_WRONLY | O_TRUNC | O_CREAT, 0600);
  if (fd_out == -1) {
    perror(argv[2]);
    exit(1);
  }

  write_in_reverse(fd_in, fd_out);

  close(fd_in);
  close(fd_out);

  return 0;
}
