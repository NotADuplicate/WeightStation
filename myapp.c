#include <gtk/gtk.h>

void on_button_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    GList *children, *iter;

    children = gtk_container_get_children(GTK_CONTAINER(user_data));
    for(iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *button = gtk_bin_get_child(GTK_BIN(iter->data));
        if(GTK_TOGGLE_BUTTON(button) != togglebutton) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        }
    }
    g_list_free(children);
}

GtkWidget* create_question_box(int val, char* question_text) {
    GtkWidget *box;
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *label;
    label = gtk_label_new(question_text);
    gtk_container_add(GTK_CONTAINER(box), label);

    GtkWidget *center_box;
    center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(box), center_box);

    GtkWidget *button_box;
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(center_box), button_box);

    for (int i = 0; i < (val == 4 ? 5 : val); i++) {
        char buffer[10];
        sprintf(buffer, "Option %d", i+1);

        GtkWidget *button = gtk_toggle_button_new_with_label(buffer);
        gtk_widget_set_size_request(button, 150, 100);
        g_signal_connect(button, "toggled", G_CALLBACK(on_button_toggled), button_box);

        GtkWidget *event_box = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(event_box), button);
        gtk_box_pack_start(GTK_BOX(button_box), event_box, FALSE, FALSE, 0);
        gtk_style_context_add_class(gtk_widget_get_style_context(event_box), "my-toggle-button");
    }

    return box;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
                                    ".my-toggle-button { background-color: #FFFFFF; }"
                                    ".my-toggle-button:checked { background-color: #008000; color: #FFFFFF; }"
                                    "button { font: 20px Arial, sans-serif; }",
                                    -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *dialog;
    dialog = gtk_dialog_new_with_buttons("Survey", NULL, GTK_DIALOG_MODAL, "_OK", GTK_RESPONSE_OK, NULL);
    gtk_widget_set_size_request(dialog, 1000, 800);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    int survey_values[3] = {2, 3, 4};
    char *questions[3] = {"Question 1", "Question 2", "Question 3"};

    for (int i = 0; i < 3; i++) {
        if (survey_values[i] == 2 || survey_values[i] == 3 || survey_values[i] == 4) {
            GtkWidget *question_box = create_question_box(survey_values[i], questions[i]);
            gtk_container_add(GTK_CONTAINER(content_area), question_box);
        }
    }

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    gtk_main();
    return 0;
}
