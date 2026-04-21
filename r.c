#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * IPTV Stream Recorder CLI
 * Usage: ./recorder <url> <duration(HH:MM:SS)> <filename>
 */

int main(int argc, char *argv[]) {
  // Check if we have the correct number of arguments
  if (argc != 4) {
    printf("IPTV Stream Recorder\n");
    printf("--------------------\n");
    printf("Usage: %s <url> <duration(HH:MM:SS)> <filename>\n", argv[0]);
    printf("Example: %s \"http://example.com/stream.m3u8\" 00:30:00 "
           "\"output.mkv\"\n",
           argv[0]);
    return 1;
  }

  const char *url = argv[1];
  const char *duration = argv[2];
  const char *filename = argv[3];

  // Basic validation of HH:MM:SS format (optional but helpful)
  int h, m, s;
  if (sscanf(duration, "%d:%d:%d", &h, &m, &s) != 3) {
    fprintf(stderr, "Error: Duration must be in HH:MM:SS format.\n");
    return 1;
  }

  // Prepare the ffmpeg command.
  // -i: input URL
  // -t: duration
  // -c copy: stream copy (no transcoding, fast and low CPU)
  // The '&' at the end runs it in the background as requested in previous
  // contexts
  char cmd[2048];
  snprintf(cmd, sizeof(cmd), "ffmpeg -i '%s' -t '%s' -c copy '%s' &", url,
           duration, filename);

  printf("Starting recording...\n");
  printf("Source: %s\n", url);
  printf("Duration: %s\n", duration);
  printf("Output: %s\n", filename);
  printf("Command: %s\n", cmd);

  // Execute the command
  int result = system(cmd);

  if (result == -1) {
    perror("Error executing system command");
    return 1;
  }

  printf("\nRecording process launched in background.\n");
  return 0;
}
