#include <libadwaita-1/adwaita.h>
// #include <gtk/gtk.h>
#include <stdio.h>

// Struct to hold our application state and widget pointers
typedef struct {
  GtkWidget *status_label;
  GtkWidget *clock_label;
  GtkWidget *calendar;
  GtkWidget *spin_minutes;
  GtkWidget *spin_duration;
  GtkWidget *url_entry;
  GtkWidget *start_button;
  GtkWidget *stop_button;
  guint timer_id;
  guint clock_timer_id;
  int seconds;
  int duration_seconds;
  int hour;
  int day;
  int minute;
} IptvTimerData;

// Update the current time display
static gboolean on_clock_tick(gpointer user_data) {
  IptvTimerData *data = (IptvTimerData *)user_data;
  GDateTime *now = g_date_time_new_now_local();
  char *time_str = g_date_time_format(now, "\n%Y-%m-%d %H:%M:%S");

  gtk_label_set_text(GTK_LABEL(data->clock_label), time_str);

  g_free(time_str);
  g_date_time_unref(now);
  return G_SOURCE_CONTINUE;
}

// The function that runs every second
static gboolean on_timer_tick(gpointer user_data) {
  IptvTimerData *data = (IptvTimerData *)user_data;
  data->seconds--;

  // When the timer hits zero, trigger the recording
  if (data->seconds <= 0) {
    const char *url = gtk_editable_get_text(GTK_EDITABLE(data->url_entry));

    // Example of how you might use the URL in a command:
    // char *cmd = g_strdup_printf("ffmpeg -i '%s' -c copy output_%ld.ts &",
    // url, time(NULL)); system(cmd); g_free(cmd);

    char *cmd = g_strdup_printf(
        "ffmpeg -reconnect 1 -reconnect_at_eof 0 -reconnect_streamed 1 "
        "-reconnect_delay_max 4294 -i '%s' -t %d -c copy output%u.ts &",
        url, data->spin_duration, data->timer_id);
    system(cmd);
    g_free(cmd);

    char *msg =
        g_strdup_printf("⏺ Recording Started!\nSource: %s\nDuration left: %d",
                        url, data->spin_duration - data->duration_seconds);
    gtk_label_set_text(GTK_LABEL(data->status_label), msg);
    g_free(msg);

    gtk_widget_remove_css_class(data->status_label, "accent");
    gtk_widget_add_css_class(data->status_label,
                             "error"); // Make it red for recording

    data->timer_id = 0; // Reset timer ID
    gtk_widget_set_sensitive(data->start_button, TRUE);

    // TODO: Insert your actual IPTV recording system call here
    // e.g., system("ffmpeg -i http://iptv-stream.m3u8 -c copy output.ts &");

    return G_SOURCE_REMOVE; // Stop the GLib timer loop
  }

  // Update the UI with the remaining time
  char buf[64];
  snprintf(buf, sizeof(buf), "Starting in: %02d:%02d", data->seconds / 60,
           data->seconds % 60);
  gtk_label_set_text(GTK_LABEL(data->status_label), buf);

  return G_SOURCE_CONTINUE; // Keep the timer running
}

// Triggered when the user clicks the "Stop" button
static void on_stop_clicked(GtkButton *btn, IptvTimerData *data) {
  if (data->timer_id != 0) {
    g_source_remove(data->timer_id);
    data->timer_id = 0;
  }

  gtk_label_set_text(GTK_LABEL(data->status_label), "Recording Stopped");
  gtk_widget_remove_css_class(data->status_label, "error");
  gtk_widget_remove_css_class(data->status_label, "accent");

  system("killall ffmpeg");

  gtk_widget_set_sensitive(data->start_button, TRUE);
  gtk_widget_set_sensitive(data->stop_button, FALSE);
}

// Triggered when the user clicks the "Start" button
static void on_start_clicked(GtkButton *btn, IptvTimerData *data) {
  if (data->timer_id != 0)
    return; // Prevent multiple timers

  // Get the minutes from the UI and convert to seconds
  int mins =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->spin_minutes));
  data->seconds = mins * 60;

  if (data->seconds <= 0)
    return;

  // Get recording duration
  int duration_mins =
      gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->spin_duration));
  data->duration_seconds = duration_mins * 60;

  // Lock the button and style the label
  gtk_widget_set_sensitive(data->start_button, FALSE);
  gtk_widget_set_sensitive(data->stop_button, TRUE);
  gtk_widget_remove_css_class(data->status_label, "error");
  gtk_widget_add_css_class(data->status_label, "accent");

  // Start the asynchronous timer (1 interval = 1 second)
  data->timer_id = g_timeout_add_seconds(1, on_timer_tick, data);
  // on_timer_tick(data); // Force an immediate UI update
  char buf[64];
  snprintf(buf, sizeof(buf), "Starting in: %02d:%02d", data->seconds / 60,
           data->seconds % 60);
  gtk_label_set_text(GTK_LABEL(data->status_label), buf);
}

// Build the UI
static void on_activate(GtkApplication *app, IptvTimerData *data) {
  GtkWidget *window = adw_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "IPTV Recorder");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 600);

  // Adwaita Toolbar View (Modern standard for windows with headerbars)
  GtkWidget *toolbar_view = adw_toolbar_view_new();
  adw_application_window_set_content(ADW_APPLICATION_WINDOW(window),
                                     toolbar_view);

  GtkWidget *header = adw_header_bar_new();
  adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar_view), header);

  // Main layout container
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_start(vbox, 20);
  gtk_widget_set_margin_end(vbox, 20);
  gtk_widget_set_margin_top(vbox, 20);
  gtk_widget_set_margin_bottom(vbox, 20);
  adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar_view), vbox);

  // Clock Label
  data->clock_label = gtk_label_new(NULL);
  gtk_widget_add_css_class(data->clock_label, "title-1");
  gtk_box_append(GTK_BOX(vbox), data->clock_label);

  // Start the clock timer
  on_clock_tick(data);
  data->clock_timer_id = g_timeout_add_seconds(1, on_clock_tick, data);

  // Calendar
  data->calendar = gtk_calendar_new();
  gtk_box_append(GTK_BOX(vbox), data->calendar);

  // Status Label
  data->status_label = gtk_label_new("Set Recording Time");
  gtk_widget_add_css_class(data->status_label, "title-2"); // Large font
  gtk_box_append(GTK_BOX(vbox), data->status_label);

  // Input area for time
  GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign(input_box, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(vbox), input_box);

  GtkWidget *min_label = gtk_label_new("Delay (Minutes):");
  gtk_box_append(GTK_BOX(input_box), min_label);

  data->spin_minutes =
      gtk_spin_button_new_with_range(0, 1440, 1); // Up to 24 hours
  gtk_box_append(GTK_BOX(input_box), data->spin_minutes);

  // Duration input area
  GtkWidget *duration_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign(duration_box, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(vbox), duration_box);

  GtkWidget *dur_label = gtk_label_new("Duration (Minutes):");
  gtk_box_append(GTK_BOX(duration_box), dur_label);

  data->spin_duration =
      gtk_spin_button_new_with_range(0, 1440, 1); // Default 1 min
  gtk_box_append(GTK_BOX(duration_box), data->spin_duration);

  // Stream URL Input
  data->url_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(data->url_entry),
                                 "Enter Stream URL (e.g. http://...)");
  gtk_widget_set_margin_top(data->url_entry, 10);
  gtk_box_append(GTK_BOX(vbox), data->url_entry);

  // Buttons container
  GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
  gtk_box_append(GTK_BOX(vbox), button_box);

  // Start Button
  data->start_button = gtk_button_new_with_label("Start Timer");
  gtk_widget_add_css_class(data->start_button, "suggested-action");
  gtk_widget_add_css_class(data->start_button, "pill");
  g_signal_connect(data->start_button, "clicked", G_CALLBACK(on_start_clicked),
                   data);
  gtk_box_append(GTK_BOX(button_box), data->start_button);

  // Stop Button
  data->stop_button = gtk_button_new_with_label("Stop Recording");
  gtk_widget_add_css_class(data->stop_button, "destructive-action");
  gtk_widget_add_css_class(data->stop_button, "pill");
  gtk_widget_set_sensitive(data->stop_button, FALSE);
  g_signal_connect(data->stop_button, "clicked", G_CALLBACK(on_stop_clicked),
                   data);
  gtk_box_append(GTK_BOX(button_box), data->stop_button);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  // Initialize our struct with zeros
  IptvTimerData data = {0};

  // Create the Adwaita application
  AdwApplication *app = adw_application_new("org.example.IptvRecorder",
                                            G_APPLICATION_DEFAULT_FLAGS);

  // Connect the activation signal and pass our struct to it
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), &data);

  // Run the main loop
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}