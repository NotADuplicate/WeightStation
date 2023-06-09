#include <gtk/gtk.h>
#include <time.h>

static gboolean time_handler(GtkLabel *label) {
    time_t rawtime;
    struct tm *time_info;
    char buffer[80];

    time (&rawtime);
    time_info = localtime(&rawtime);

    strftime(buffer, sizeof(buffer),"%H:%M:%S", time_info);
    gtk_label_set_text(label, buffer);

    return G_SOURCE_CONTINUE; // return TRUE here to ensure the timeout continues
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *label;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Time");

    label = gtk_label_new(NULL);
    gtk_container_add(GTK_CONTAINER(window), label);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    g_timeout_add_seconds(60, (GSourceFunc) time_handler, (gpointer) label);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
