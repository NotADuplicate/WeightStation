#include <gtk/gtk.h>
#include "../Curl/survey.h"
#include "surveyUI.h"

typedef struct {
    GtkWidget *button_box;
    int question_index;
    int option_index;
    int num_options;
} ButtonIndices;

int selected_options[10];
int num_of_questions;
GdkPixbuf *unclicked[6];
GdkPixbuf *clicked[6];
GdkPixbuf *cancel_pixbuf;
GtkWidget *ok_button;

GtkWidget *options[10][5];

GtkWidget *clickedImages[10][6];
GtkWidget *unclickedImages[10][6];

void submit_survey(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog = GTK_WIDGET(user_data);
    
    printf("Submitted\n");

    // Your code here...
    
    
    gtk_widget_destroy(dialog);
}

void exit_survey(GtkWidget *widget, gpointer user_data) {
    GtkWidget *dialog = GTK_WIDGET(user_data);
    gtk_widget_destroy(dialog);
}

/*void on_button_toggled(GtkToggleButton *button, ButtonIndices *data) {
    GtkWidget *image;
    GdkPixbuf *swapping;
    if (selected_options[data->question_index] != data->option_index+1){//if you have clicked an unselected button
        printf("Question %d option %d clicked\n", data->question_index+1, data->option_index+1);

        // Iterate over all children of the box and turn them off
        GList *children, *iter;
        children = gtk_container_get_children(GTK_CONTAINER(data->button_box));
        int i = 0;
        int clickedI;

        for (iter = children; iter != NULL; iter = g_list_next(iter)) { //loop through all of the buttons of a given question
            g_print("Widget type: %s\n", G_OBJECT_TYPE_NAME(iter->data));
            GtkWidget *child_button = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(iter->data))->data);
            //printf("Welp didnt crash\n");
            
            if (i == selected_options[data->question_index]-1) { //unclick the already highlighted thing
                //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child_button), FALSE);
                printf("Unselecting: %i\n",i);
                swapping = gtk_image_get_pixbuf(GTK_IMAGE(clickedImages[data->question_index][i])); //store the existing image of the clicked button
                image = unclickedImages[data->question_index][i];
                gtk_button_set_image(GTK_BUTTON(child_button), image);
                clickedImages[data->question_index][i] = gtk_image_new_from_pixbuf(swapping); //set the stored clicked image to what it had saved
            }
            else if(i == data->option_index){ //set clicked button to highlighted
                swapping = gtk_image_get_pixbuf(GTK_IMAGE(unclickedImages[data->question_index][i])); //store the existing image of the unclicked button before it gets highlighted
                image = clickedImages[data->question_index][i];
                gtk_button_set_image(GTK_BUTTON(child_button), image);
                unclickedImages[data->question_index][i] = gtk_image_new_from_pixbuf(swapping);
            } 
            i++;
        }
        selected_options[data->question_index] = data->option_index+1;
        int allSelected = 1;
        for(int i = 0; i < num_of_questions; i++) {
            if(selected_options[i] == 0)
                allSelected = 0;
        }
        if(allSelected == 1) {
            printf("Showing submit button\n");
            gtk_widget_show(ok_button);
        }

        g_list_free(children);
    }
    else {printf("Button already selected\n");}
}*/

void toggle_buttons(ButtonIndices *data, int i, GtkWidget *child_button) {
    GtkWidget *image;
    GdkPixbuf *swapping;
    printf("Button toggled = %i, selected button = %i\n", i, selected_options[data->question_index]);
    if (i == selected_options[data->question_index]-1) { //unclick the already highlighted thing
        printf("Unselecting: %i\n",i);
        swapping = gtk_image_get_pixbuf(GTK_IMAGE(clickedImages[data->question_index][i])); //store the existing image of the clicked button
        image = unclickedImages[data->question_index][i];
        gtk_button_set_image(GTK_BUTTON(child_button), image);
        clickedImages[data->question_index][i] = gtk_image_new_from_pixbuf(swapping); //set the stored clicked image to what it had saved
    }
    else if(i == data->option_index){ //set clicked button to highlighted
        swapping = gtk_image_get_pixbuf(GTK_IMAGE(unclickedImages[data->question_index][i])); //store the existing image of the unclicked button before it gets highlighted
        image = clickedImages[data->question_index][i];
        gtk_button_set_image(GTK_BUTTON(child_button), image);
        unclickedImages[data->question_index][i] = gtk_image_new_from_pixbuf(swapping);
        selected_options[data->question_index] = i+1;
    } 
}

void on_button_toggled(GtkToggleButton *button, ButtonIndices *data) {
    if (selected_options[data->question_index] != data->option_index+1){//if you have clicked an unselected button
        printf("Question %d option %d clicked\n", data->question_index+1, data->option_index+1);
        if(selected_options[data->question_index] > 0)
            toggle_buttons(data,selected_options[data->question_index]-1,options[data->question_index][selected_options[data->question_index]-1]);
        toggle_buttons(data,data->option_index,options[data->question_index][data->option_index]);
    }
    else {printf("Already clicked\n");}
}

GtkWidget* create_question_box(int val, char* question_text, int question_index, int defaultAnswer, char answerLabels[6][30]) {
    GtkWidget *box;
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *frame = gtk_frame_new(NULL); // create a new frame
    gtk_container_add(GTK_CONTAINER(frame), box); // add the box into the frame
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);  //set frame shadow

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
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 25);
    gtk_container_add(GTK_CONTAINER(center_box), button_box);

for (int i = 0; i < (val == 4 ? 5 : val); i++) {
    // Create an image
    GtkWidget *image = gtk_image_new_from_pixbuf(unclicked[i]);

    GtkWidget *button = gtk_toggle_button_new();
    options[question_index][i] = button;
    gtk_button_set_image(GTK_BUTTON(button), image); // Set the image

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
    data->num_options = val;

    g_signal_connect_data(button, "toggled", G_CALLBACK(on_button_toggled), data, (GClosureNotify)free, 0);
    
    if (defaultAnswer == i + 1) {// store default answer
        //on_button_toggled(GTK_TOGGLE_BUTTON(button), data);
        printf("Default answer\n\n\n");
    }

    GtkWidget *event_box = gtk_event_box_new();

    // Create a label
    if(answerLabels[i+1] != NULL) {
        GtkWidget *label = gtk_label_new(answerLabels[i+1]);

        // Create event boxes for the label and the button
        GtkWidget *event_box_label = gtk_event_box_new();
        GtkWidget *event_box_button = gtk_event_box_new();

        // Add the label and the button to the respective event boxes
        gtk_container_add(GTK_CONTAINER(event_box_label), label);
        gtk_container_add(GTK_CONTAINER(event_box_button), button);

        // Create a new vertical box for the label and button
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        // Add the event boxes to the vbox
        gtk_box_pack_start(GTK_BOX(vbox), event_box_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), event_box_button, FALSE, FALSE, 0);

        // Add the vbox to the button_box
        gtk_box_pack_start(GTK_BOX(button_box), vbox, FALSE, FALSE, 0);
    }
    else {gtk_container_add(GTK_CONTAINER(event_box), button);}

    // Add the button (inside an event box) to the button box
    gtk_box_pack_start(GTK_BOX(button_box), event_box, FALSE, FALSE, 0);
}
    return frame;
}

gboolean survey_timeout(gpointer window) {
    gtk_widget_destroy(GTK_WIDGET(window));
    for(int i = 0; i < 10; i++) {
        selected_options[i] = 0;
    }
    return G_SOURCE_REMOVE;
}

void load_button_images(int num) {
    for (int j = 0; j < num; j++) {
        for(int i = 0; i < 6; i++) {
            printf("Loading button: %i\n",i);
            clickedImages[j][i] = gtk_image_new_from_pixbuf(clicked[i]);
            unclickedImages[j][i] = gtk_image_new_from_pixbuf(unclicked[i]);
        }
    }
}

void create_title_grid(GtkWidget *content_area, GdkPixbuf *headshot, Player *player, char *weighMode, GtkWidget *dialog) {
    // Create grid
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10); // adjust as needed
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10); // adjust as needed
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    // Create widgets
    GtkWidget *title = gtk_label_new(NULL);
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    GtkWidget *subtitle = gtk_label_new(NULL);
    gtk_widget_set_halign(subtitle, GTK_ALIGN_CENTER);
    
    GdkPixbuf *scaled_headshot = gdk_pixbuf_scale_simple(headshot, 90, 120, GDK_INTERP_BILINEAR); // load your GdkPixbuf here
    GtkWidget *image = gtk_image_new_from_pixbuf(scaled_headshot);
    
    //Create the exit button
    GtkWidget *cancel_image = gtk_image_new_from_pixbuf(cancel_pixbuf);

        GtkWidget *button = gtk_toggle_button_new();
        gtk_button_set_image(GTK_BUTTON(button), cancel_image); // Set the image

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

        g_signal_connect(button, "toggled", G_CALLBACK(exit_survey), dialog);
        //g_signal_connect(ok_button, "clicked", G_CALLBACK(submit_survey), dialog);
    
    
    GtkWidget *left_text = gtk_label_new("Weigh In");
    //GtkWidget *right_text = gtk_label_new("Your Right Text Here");
    GtkWidget *weight_text = gtk_label_new(NULL);
    //Make text look fancy
    char *label_subtitle_text = g_strdup_printf("<span font='14' foreground='gray'>%s</span>", concat(
        concat("#",player->UniformNumber), concat(" ", player->Position)
        ));
    char *label_name_text = g_strdup_printf("<span font='30' foreground='black'>%s</span>", player->Name);
    char *label_left_text = g_strdup_printf("<span font='25' foreground='black'>%s</span>", weighMode);
    gtk_label_set_markup(GTK_LABEL(left_text), label_left_text);
    gtk_label_set_markup(GTK_LABEL(subtitle), label_subtitle_text);
    gtk_label_set_markup(GTK_LABEL(title), label_name_text);
    
    char *weight_with_unit = g_strdup_printf("%s <span font='20'>lbs</span>", player->weight);
    char *label_weight_text = g_strdup_printf("<span font='30' foreground='black'>%s</span>", weight_with_unit);
    gtk_label_set_markup(GTK_LABEL(weight_text), label_weight_text);
    g_free(weight_with_unit);
        
    // Create new grid for the right_text and subtitle
    GtkWidget *small_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(small_grid), 5); // adjust as needed
    gtk_grid_set_column_spacing(GTK_GRID(small_grid), 5); // adjust as needed

    // Create subtitle and right_text
    GtkWidget *right_text = gtk_label_new(player->LockerNumber);

    // Create frame for the right_text
    GtkWidget *frame = gtk_frame_new(NULL);
    GtkCssProvider *provider_frame = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider_frame,
        "frame {"
        "border: 1px solid black;" // Adjust border thickness and color as needed
        "}",
        -1, NULL);
    GtkStyleContext *context_frame = gtk_widget_get_style_context(frame);
    gtk_style_context_add_provider(context_frame,
                                   GTK_STYLE_PROVIDER(provider_frame),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider_frame);

    // Add right_text to frame
    gtk_container_add(GTK_CONTAINER(frame), right_text);

    // Add widgets to the small grid
    gtk_grid_attach(GTK_GRID(small_grid), subtitle, 0, 0, 1, 1); // at (0,0), spanning 1 column and 1 row
    gtk_grid_attach(GTK_GRID(small_grid), frame, 0, 1, 1, 1); // at (0,1), spanning 1 column and 1 row

    // Add small grid to the main grid
    gtk_grid_attach(GTK_GRID(grid), small_grid, 1, 1, 1, 1); // at (2,0), spanning 1 column and 1 row



    // Add widgets to grid
    gtk_grid_attach(GTK_GRID(grid), image, 0, 0, 1, 2); // at (0,0), spanning 1 column and 2 rows
    gtk_grid_attach(GTK_GRID(grid), title, 1, 0, 1, 1); // at (1,0), spanning 1 column and 1 row
    gtk_grid_attach(GTK_GRID(grid), left_text, 0, 2, 1, 1); // at (0,2), spanning 1 column and 1 row
    //gtk_grid_attach(GTK_GRID(grid), subtitle, 1, 1, 1, 1); // at (1,1), spanning 1 column and 1 row
    //gtk_grid_attach(GTK_GRID(grid), right_text, 2, 0, 1, 1); // at (2,0), spanning 1 column and 1 row
    gtk_grid_attach(GTK_GRID(grid), weight_text, 2, 2, 1, 1);
    
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_widget_set_valign(button, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), button, 4, 0, 1, 2); // at (0,0), spanning 1 column and 2 rows
}

void create_survey_window(surveyQuestion *survey, int numQuestions, int timer, GdkPixbuf *headshot, Player *player, char *weighMode) {
    num_of_questions = numQuestions;
    load_button_images(numQuestions);
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
    gtk_window_set_default_size(GTK_WINDOW(dialog), 200, 450);
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWindow *window = GTK_WINDOW(dialog);
    gtk_window_set_decorated(window, FALSE);

    ok_button = gtk_button_new_with_label("Submit");
    gtk_widget_hide(ok_button);
    gtk_widget_set_size_request(ok_button, 100, 50);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    g_signal_connect(ok_button, "clicked", G_CALLBACK(submit_survey), dialog);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), ok_button, GTK_RESPONSE_OK);
    
    create_title_grid(content_area, headshot, player, weighMode, dialog);

    for (int i = 0; i < numQuestions; i++) {
        if (survey[i].responseType > 0 && survey[i].responseType < 5) {
            GtkWidget *question_box = create_question_box(survey[i].responseType, survey[i].Question,i, survey[i].defaultAnswer, survey[i].answerLabels);
            gtk_container_add(GTK_CONTAINER(content_area), question_box);
        }
    }
    
    g_timeout_add_seconds(timer/1000, (GSourceFunc)survey_timeout, (gpointer)dialog); //time out

    printf("About to show dialog\n");
    gtk_widget_show_all(dialog);
    gtk_widget_hide(ok_button);
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
    gdk_cairo_set_source_pixbuf(cr, clicked[0], 0, 20);
    cairo_paint(cr);
}
