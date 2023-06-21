#include <gtk/gtk.h>
#include "../Curl/survey.h"
#include "surveyUI.h"

typedef struct {
    GtkWidget *button_box;
    int question_index;
    int option_index;
} ButtonIndices;

int selected_options[6];
GdkPixbuf *image1;
GdkPixbuf *clicked1;

void submit_survey(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog = GTK_WIDGET(user_data);
    
    printf("Submitted\n");

    // Your code here...
    
    
    gtk_widget_destroy(dialog);
}

void on_button_toggled(GtkToggleButton *button, ButtonIndices *data) {
    GtkWidget *image;
    if (gtk_toggle_button_get_active(button)) {
        printf("Question %d option %d clicked\n", data->question_index+1, data->option_index+1);
        selected_options[data->question_index] = data->option_index+1;

        // Iterate over all children of the box and turn them off
        GList *children, *iter;
        children = gtk_container_get_children(GTK_CONTAINER(data->button_box));

        for (iter = children; iter != NULL; iter = g_list_next(iter)) {
            GtkWidget *child_button = gtk_bin_get_child(GTK_BIN(iter->data));

            if (GTK_TOGGLE_BUTTON(child_button) != button) {
                //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child_button), FALSE);
                image = gtk_image_new_from_pixbuf(image1);
                gtk_button_set_image(GTK_BUTTON(child_button), image);
            }
        }
        image = gtk_image_new_from_pixbuf(clicked1);
        gtk_button_set_image(GTK_BUTTON(button), image);

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
    // Create an image
    GtkWidget *image = gtk_image_new_from_file("Resources/default_1.png");

    GtkWidget *button = gtk_toggle_button_new();
    gtk_button_set_image(GTK_BUTTON(button), image); // Set the image
    gtk_widget_set_size_request(button, 150, 100);

    // Remove button border and background
    GtkStyleContext *style_context = gtk_widget_get_style_context(button);
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
                                    "* {"
                                    "background-color: rgba(0, 0, 0, 0);"
                                    "border: 0;"
                                    "}",
                                    -1, NULL);
    gtk_style_context_add_provider(style_context, 
                                   GTK_STYLE_PROVIDER(provider), 
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

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

gboolean survey_timeout(gpointer window) {
    gtk_widget_destroy(GTK_WIDGET(window));
    return G_SOURCE_REMOVE;
}

void create_survey_window(surveyQuestion *survey, int numQuestions, int timer) {
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

    for (int i = 0; i < numQuestions; i++) {
        if (survey[i].responseType > 0 && survey[i].responseType < 5) {
            GtkWidget *question_box = create_question_box(survey[i].responseType, survey[i].Question,i);
            gtk_container_add(GTK_CONTAINER(content_area), question_box);
        }
    }
    
    g_timeout_add_seconds(timer/1000, (GSourceFunc)survey_timeout, (gpointer)dialog); //time out

    printf("About to show dialog\n");
    gtk_widget_show_all(dialog);
    printf("About to run dialog\n");
    gtk_dialog_run(GTK_DIALOG(dialog));
    printf("About to destroy dialog\n");
    //gtk_widget_destroy(dialog);
}

gboolean survey_draw_event(GtkWidget *widget, cairo_t *cr, SurveyDrawData *data) {
    int yp = 50;
    for (int i = 0; i < data->numQuestions; i++) {
        if (data->survey[i].responseType > 0 && data->survey[i].responseType < 5) {
            cairo_set_source_rgb(cr, 0, 0, 0); // Sets the color to black for the text
            cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(cr, 30); // Sets the text size

            cairo_text_extents_t extents;
            cairo_text_extents(cr, data->survey[i].Question, &extents); 

            // calculate the x position to start drawing the text such that it is centered
            double x_position = 400 - extents.width/2;

            cairo_move_to(cr, x_position, yp); // Sets the position where the text will start
            cairo_show_text(cr, data->survey[i].Question); // The text string to draw
            yp += 100;
        }
    }
    gdk_cairo_set_source_pixbuf(cr, image1, 0, 20);
    cairo_paint(cr);
}

/*void create_survey_window(surveyQuestion *survey, int numQuestions, int timer) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *darea;
    GtkWidget *overlay;
    //submit_weight_func = submit_func;

    // create new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Survey Window");

    // Change dimensions here for a larger window
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // Create overlay
    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);

    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(overlay), darea);

    SurveyDrawData *draw_data = g_new(SurveyDrawData,1);
    draw_data->numQuestions = numQuestions;
    draw_data->survey = survey;

    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(survey_draw_event), draw_data);

    //g_signal_connect(window, "button-press-event", G_CALLBACK(weigh_click), NULL);

    g_timeout_add_seconds(timer/1000, (GSourceFunc)survey_timeout, (gpointer)window); //time out

    // show all widgets in the window
    gtk_widget_show_all(window);
    printf("Finished survey window\n");
}*/
