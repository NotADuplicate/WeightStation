#include <gtk/gtk.h>
#include "../Curl/survey.h"

typedef struct {
    GtkWidget *button_box;
    int question_index;
    int option_index;
} ButtonIndices;

int selected_options[6];

void submit_survey(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog = GTK_WIDGET(user_data);
    
    printf("Submitted\n");

    // Your code here...
    
    
    gtk_widget_destroy(dialog);
}

void on_button_toggled(GtkToggleButton *button, ButtonIndices *data) {
    if (gtk_toggle_button_get_active(button)) {
        printf("Question %d option %d clicked\n", data->question_index+1, data->option_index+1);
        selected_options[question_index] = option_index+1;

        // Iterate over all children of the box and turn them off
        GList *children, *iter;
        children = gtk_container_get_children(GTK_CONTAINER(data->button_box));

        for (iter = children; iter != NULL; iter = g_list_next(iter)) {
            GtkWidget *child_button = gtk_bin_get_child(GTK_BIN(iter->data));

            if (GTK_TOGGLE_BUTTON(child_button) != button) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child_button), FALSE);
            }
        }

        g_list_free(children);
    }
}


GtkWidget* create_question_box(int val, char* question_text, int question_index) {
    GtkWidget *box;
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create label with larger text
    char label_text[256];
    sprintf(label_text, "<span font='20'>%s</span>", question_text);
    GtkWidget *label;
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), label_text);
    gtk_container_add(GTK_CONTAINER(box), label);

    GtkWidget *center_box;
    center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
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
        gtk_style_context_add_class(gtk_widget_get_style_context(button), "my-toggle-button");

        // Allocate and fill the ButtonIndices structure
        ButtonIndices *data = malloc(sizeof(ButtonIndices));
        data->button_box = button_box;
        data->question_index = question_index;
        data->option_index = i;

        g_signal_connect_data(button, "toggled", G_CALLBACK(on_button_toggled), data, (GClosureNotify)free, 0);

        GtkWidget *event_box = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(event_box), button);
        gtk_box_pack_start(GTK_BOX(button_box), event_box, FALSE, FALSE, 0);
    }
    return box;
}

void create_survey_window(surveyQuestion *survey) {
    printf("Created survey\n");
    GtkCssProvider *provider;
    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (provider,
                                     "button:checked { font: 35px Arial, sans-serif; color: green}"
                                     "button { font: 35px Arial, sans-serif; }",
                                     -1, NULL);
    gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                               GTK_STYLE_PROVIDER (provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);


    GtkWidget *dialog;
    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Survey");
    gtk_widget_set_size_request(dialog, 1000, 800);
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

    GtkWidget *ok_button = gtk_button_new_with_label("_OK");
    gtk_widget_set_size_request(ok_button, 100, 50);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    g_signal_connect(ok_button, "clicked", G_CALLBACK(submit_survey), dialog);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), ok_button, GTK_RESPONSE_OK);

    for (int i = 0; i < 5; i++) {
        if (survey[i].responseType > 0 && survey[i].responseType < 5) {
            GtkWidget *question_box = create_question_box(survey[i].responseType, survey[i].Question,i);
            gtk_container_add(GTK_CONTAINER(content_area), question_box);
        }
    }

    printf("About to show dialog\n");
    gtk_widget_show_all(dialog);
    printf("About to run dialog\n");
    gtk_dialog_run(GTK_DIALOG(dialog));
    printf("About to destroy dialog\n");
    //gtk_widget_destroy(dialog);
}
