#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Streamer CLI
 * Usage: ./s <filename> <rtmp_url>
 */

void launch_ffmpeg(const char *filename, const char *url) {
  char filter_arg[1024];
  snprintf(filter_arg, sizeof(filter_arg), "subtitles=%s", filename);

  const char *args[] = {"ffmpeg",      "-re",        "-i",
                        filename,      "-vf",        filter_arg,
                        "-c:v",        "libx264",    "-preset",
                        "veryfast",    "-profile:v", "high",
                        "-level",      "4.1",        "-b:v",
                        "3000k",       "-maxrate",   "3000k",
                        "-bufsize",    "6000k",      "-pix_fmt",
                        "yuv420p",     "-g",         "60",
                        "-keyint_min", "60",         "-sc_threshold",
                        "0",           "-c:a",       "aac",
                        "-b:a",        "128k",       "-ar",
                        "44100",       "-ac",        "2",
                        "-f",          "flv",        url,
                        NULL};

  printf("Command: ");
  for (int i = 0; args[i] != NULL; i++) {
    printf("%s ", args[i]);
  }
  printf("\n");

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork failed");
    exit(1);
  }

  if (pid == 0) {
    // Child process
    execvp(args[0], (char *const *)args);

    // If execvp returns, it failed
    perror("execvp failed");
    exit(1);
  }
  // Parent process returns immediately, leaving ffmpeg running in the
  // background.
}

int main(int argc, char *argv[]) {
  // Check if we have the correct number of arguments
  if (argc != 3) {
    printf("Streamer CLI\n");
    printf("--------------------\n");
    printf("Usage: %s <filename> rtmp://streamserver.com/live/STREAMKEY\n",
           argv[0]);
    printf("Example: %s movie.mkv rtmp://streamserver.com/live/test123\n",
           argv[0]);
    return 1;
  }

  const char *filename = argv[1];
  const char *url = argv[2];

  printf("Starting stream...\n");
  printf("Input: %s\n", filename);
  printf("Destination: %s\n", url);

  // Execute the command using fork & exec
  launch_ffmpeg(filename, url);

  printf("\nStreaming process launched in background.\n");
  return 0;
}
