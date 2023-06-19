#include <gtk/gtk.h>
#include <math.h>
#include "../Curl/roster.h"
#include "../Helpers/playerHelper.h"
#include "../Helpers/jsonReader.h"
#include "../UI/login.h"
#include "../UI/dmx.h"
#include "../Curl/schedule.h"
#include "../Curl/survey.h"
#include "../UI/surveyUI.h"
#include "../ServerClient/client.h"
#include "../ServerClient/server.h"
#include "Window.h"
#include <unistd.h>
#include <time.h>

#define RADIUS 130
#define RADIUS_60 50
#define FORWARD_ARROW_X 1050
#define BACK_ARROW_X 950
#define FORWARD_ARROW_Y 60
#define BACK_ARROW_Y 60
#define LOGO_X 70
#define LOGO_Y 60
#define TEAM_X 170
#define TEAM_Y 60
#define MAX_TEAMS 5

Player *players;
circleStruct *circles;
int *sorted;
int playerPerPage;
int pageNum;
Settings *settings;
int *numPlayers_pointer;
enum Mode mode = WAITING;
GdkPixbuf **images;
int drawn = 1;
int logoClicks = 0;
practiceStruct *practices;
int numTeams;
int teamIndex = 0;
int overridden = 0; //0 if regular, 1 if manual override
teamStruct teams[MAX_TEAMS];
enum View view = MAIN_WINDOW;
GtkWidget *main_window;

//Images
GdkPixbuf *forwardArrow;
GdkPixbuf *backArrow;
GdkPixbuf *oasisLogo;
GdkPixbuf *changeTeam;
GdkPixbuf *emptyImage;

gdouble distFunction(gdouble x1, gdouble y1, gdouble x2, gdouble y2) {
	return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2),0.5);
}

void update_weighed(int team_index, int player_index) {
    players[player_index].weighed = 1;
    gtk_widget_queue_draw(main_window);
    printf("UPDATED %s\n",players[player_index].UniformNumber);
}

void cleanup() {
    free(players);
    free(practices);
    free(circles);
    free(sorted);
    int i = 0;
    //printf("Num players:%i\n",*numPlayers_pointer);
    while(i < *numPlayers_pointer) {
        //printf("Freeing image %i\n",i);
        if (images[i]) {
            g_object_unref(images[i]);
        }
        i++;
    }
    i = 0;
    free(images);
    free(settings);
    g_object_unref(forwardArrow);
    //printf("Freed forward arrow\n");
    g_object_unref(backArrow);
    g_object_unref(oasisLogo);
    
}

void load_image(GdkPixbuf **pixbuf, const char *filepath, double scale) {
    //printf("Loading %s\n", filepath);
    GError *error = NULL;
    *pixbuf = gdk_pixbuf_new_from_file_at_scale(filepath, scale, scale, TRUE, &error);
    if (!*pixbuf) {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        *pixbuf = emptyImage;
    }
}

void load_global_images() {
    load_image(&forwardArrow,"Resources/ForwardGray.png",80);
    load_image(&backArrow,"Resources/BackGray.png",80);
    load_image(&oasisLogo,"Resources/OASIS_logo.png",100);
    load_image(&changeTeam,"Resources/Choose_Team.png",100);
    load_image(&emptyImage,"Resources/Empty.png",260);
}

void load_globals(int teamNum) {
    //Set team stuff
    printf("Settings teams\n");
    teams[teamNum].teamSettings = load_settings("settings.json",teamNum);
    teams[teamNum].teamCircles = malloc(teams[teamNum].teamSettings->playerPerPage * sizeof(circleStruct));
    teams[teamNum].teamPlayers = getPlayers(teams[teamNum].teamSettings->baseUrl,teams[teamNum].teamSettings->username,teams[teamNum].teamSettings->password, numPlayers_pointer, teamNum);
    teams[teamNum].teamSorted = sort_players(teams[teamNum].teamPlayers,*numPlayers_pointer);
    teams[teamNum].teamHeadshots = malloc(*numPlayers_pointer * sizeof(GdkPixbuf *));
    teams[teamNum].teamPractices = get_next_practices(teams[teamNum].teamSettings->baseUrl,teamNum);
    
    printf("Finished setting team stuff\n");
    //Load images
    for (int i = 0; i < *numPlayers_pointer; i++) {
        //printf("%s\n",teams[teamNum].teamPlayers[i].Name);
        load_image(&teams[teamNum].teamHeadshots[i], teams[teamNum].teamPlayers[i].HeadshotFilepath,-1);
    }
    
    pageNum = 0;
    playerPerPage = teams[teamNum].teamSettings->playerPerPage;
    
    // Initialize the circles array
    if(teams[teamNum].teamSettings->playerPerPage == 15) {
        //printf("playersPerPage 15\n");
        double xp = 200;
        double yp = 270;
        for (int i = 0; i < 15; i++) {
            teams[teamNum].teamCircles[i].x = xp;
            teams[teamNum].teamCircles[i].y = yp;
            teams[teamNum].teamCircles[i].radius = RADIUS;
            yp += 300;
            if(yp > 1450) {
                xp += 340;
                yp = 270;
            }
        }
    }
    else if(teams[teamNum].teamSettings->playerPerPage == 60) {
        //printf("playersPerPage 60\n");
        double xp = 200;
        double yp = 200;
        for (int i = 0; i < 60; i++) {
            teams[teamNum].teamCircles[i].x = xp;
            teams[teamNum].teamCircles[i].y = yp;
            teams[teamNum].teamCircles[i].radius = RADIUS_60;
            yp += 150;
            if(yp > 1900) {
                yp = 200;
                xp += 200;
            }
        }
    }
    teams[teamNum].survey = get_survey(teams[teamNum].teamSettings->baseUrl,teamNum);
    //create_survey_window(teams[teamNum].survey);
}

void set_team(int teamNum) {
    //Set global vars to current team
    printf("Settings global vars to team\n");
    images = teams[teamNum].teamHeadshots;
    circles = teams[teamNum].teamCircles;
    sorted = teams[teamNum].teamSorted;
    settings = teams[teamNum].teamSettings;
    players = teams[teamNum].teamPlayers;
    practices = teams[teamNum].teamPractices;
}

void clear_practice() {
    int i = 0; //add later
}

static gboolean time_handler(GtkWidget *widget) {
    if(overridden == 0 && view == MAIN_WINDOW) { //only works when not in manual override and looking at main window
        //printf("Handling time\n");
        time_t rawtime_int;
        double rawtime_double;
        rawtime_int = time(NULL);
        rawtime_double = (double)rawtime_int;
        if(rawtime_double < practices[0].utcStartTime - settings->weighInOffset*60) //before the next practice
            mode = WAITING;
        else if(rawtime_double < (practices[0].utcStartTime+practices[0].utcEndTime)/2) //weigh in until the half way mark
            mode = WEIGH_IN;
        else if(rawtime_double > practices[0].utcEndTime+settings->weighOutOffset*60) //finish weigh out
            clear_practice();
        else
            mode = WEIGH_OUT;
            
        int connected = check_connection();
        printf("Connected to client: %i\n",connected);
            
        // Invalidate the widget so it will be redrawn
        gtk_widget_queue_draw(widget);
    }
    //printf("Tried to handle time: %i\n", overridden);

    return G_SOURCE_CONTINUE;
}

void change_team(int teamNum, GtkWidget *widget) {
    set_team(teamNum);
    gtk_widget_queue_draw(widget);
}

void destroy() { //when window is closed
    printf("Closing window\n");
    view = DMX_WINDOW;
    GtkCssProvider* Provider = gtk_css_provider_new();
    GdkDisplay* Display = gdk_display_get_default();
    GdkScreen* Screen = gdk_display_get_default_screen(Display);

    gtk_style_context_add_provider_for_screen(Screen, GTK_STYLE_PROVIDER(Provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(Provider), "* { background-color: rgb(255,255,255); }", -1, NULL);
}

void submit_weight(SubmitData *data) {
    const char *entry = gtk_entry_get_text(GTK_ENTRY(data->entry));
    int playerIndex = data->playerIndex;
    g_print("Input: %s\n", entry);
    int type;
    time_t rawtime_int;
    double rawtime_double;
    rawtime_int = time(NULL);
    rawtime_double = (double)rawtime_int;
    if(mode == WEIGH_IN)
        type = 1;
    else if(mode == WEIGH_OUT)
        type = 2;
    free(send_weight(settings->baseUrl,entry,players[sorted[playerIndex]].Id,type,rawtime_double,teamIndex));
    players[playerIndex].weight = atoi(entry);
    printf("%i\n",settings->survey);
    gtk_widget_destroy(data->window);
    if(settings->survey == 1) {
        create_survey_window(teams[teamIndex].survey);
        view = MAIN_WINDOW;
    }
    else
        view = MAIN_WINDOW;
    if(settings->server == 0)
        send_weighed(teamIndex, playerIndex);
}

void create_weigh_input(GdkPixbuf *pixbuf, const char *text, int playerIndex) {
    view = WEIGH_WINDOW;
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *entry;
    GtkWidget *button;
    view = WEIGH_WINDOW;

    // create new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "popup_window");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);


    // when the window is given the "delete-event" signal (this is given by the window manager, 
    // usually by the "close" option, or on the titlebar), we ask it to call the delete_event () 
    // function as defined above. The data passed to the callback function is NULL and is ignored 
    // in the callback function.
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    // create vbox to hold image, label, entry and button
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // create label with text
    label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2);

    // create image from pixbuf
    image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 2);

    // create text entry
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 2);

    // create submit button
    button = gtk_button_new_with_label("Send weight");
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 2);
    
    SubmitData *submit_data = g_new(SubmitData, 1);
    submit_data->playerIndex = playerIndex;
    submit_data->entry = entry;
    submit_data->window = window;

    // connect button click event with callback that closes the window and call the submit function
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(submit_weight), submit_data);
    //g_signal_connect(button, "clicked", G_CALLBACK(submit_weight), NULL);

    // show all widgets in the window
    gtk_widget_show_all(window);

    // enter the main loop and start processing events
    //gtk_main();
    printf("Finished weigh window\n");
}

int clicks = 0;
gboolean clicked_inside_circle(gdouble event_x, gdouble event_y, GtkWidget *widget) { //get weight
	if(mode != WAITING && view == MAIN_WINDOW) {
        for (int i = 0; i < 15; i++) {
            gdouble dist = distFunction(circles[i].x,circles[i].y,event_x,event_y);
            if(dist < circles[i].radius) {
                gtk_widget_queue_draw(widget);
                //show_text_input_dialog(widget,sorted[i]);
                create_weigh_input(images[sorted[i]],players[sorted[i]].Name, sorted[i]);
            }
        }
    }
    return FALSE;
}

gboolean clicked_arrow(gdouble event_x, gdouble event_y, GtkWidget *widget) {
	if(distFunction(FORWARD_ARROW_X,FORWARD_ARROW_Y,event_x,event_y) < 60) {
        if(pageNum < settings->numPages) {
            pageNum++;
            gtk_widget_queue_draw(widget);
            printf("Next page, %i our of %i",pageNum,settings->numPages);
        }
	}
	else if(distFunction(BACK_ARROW_X,BACK_ARROW_Y,event_x,event_y) < 60) {
        if(pageNum > 0) {
            pageNum--;
            gtk_widget_queue_draw(widget);
            printf("Previous page\n");
        }
	}
    return FALSE;
}

gboolean clicked_logo(gdouble event_x, gdouble event_y, GtkWidget *widget) {
	if(distFunction(LOGO_X,LOGO_Y,event_x,event_y) < 70) {
        if(logoClicks == 1) {
            logoClicks = 0;
            destroy();
            gtk_widget_destroy(widget);
            create_weight_window(settings);
        }
        else {
            logoClicks = 1;
        }
	}
    else {
        logoClicks = 0;
    }
    return FALSE;
}

gboolean clicked_team(gdouble event_x, gdouble event_y, GtkWidget *widget) {
	if(distFunction(TEAM_X,TEAM_Y,event_x,event_y) < 70) {
        teamIndex++;
        if(teamIndex >= numTeams)
            teamIndex = 0;
        change_team(teamIndex,widget);
    }
    return FALSE;
}

void draw_sprite(cairo_t *cr, int xp, int yp, int radius, GdkPixbuf *pixbuf) {
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    GError *error = NULL;

    // Create a Cairo surface from the GdkPixbuf
    surface = gdk_cairo_surface_create_from_pixbuf(pixbuf, 0, NULL);

    // Create a Cairo pattern from the surface
    pattern = cairo_pattern_create_for_surface(surface);

    // Create a circular clipping path
    cairo_arc(cr, xp, yp, radius, 0, 2 * G_PI);
    cairo_clip(cr);

    // Move the pattern to the desired position
    cairo_matrix_t matrix;
    cairo_matrix_init_translate(&matrix, -xp + radius, -yp + radius);
    cairo_pattern_set_matrix(pattern, &matrix);

    // Paint the pattern onto the Cairo context
    cairo_set_source(cr, pattern);
    cairo_paint(cr);

    // Reset clipping path for the next draw call
    cairo_reset_clip(cr);

    // Clean up
    cairo_pattern_destroy(pattern);
    cairo_surface_destroy(surface);
}

void draw_image(cairo_t *cr, int x, int y, GdkPixbuf *pixbuf) {
    if (pixbuf != NULL) {
        cairo_surface_t *surface = gdk_cairo_surface_create_from_pixbuf(pixbuf, 0, NULL);
        // Get image dimensions
        int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        // Compute top left position to center image at (x, y)
        int top_left_x = x - width / 2;
        int top_left_y = y - height / 2;
        // Draw the image
        cairo_set_source_surface(cr, surface, top_left_x, top_left_y);
        cairo_paint(cr);
        // Clean up
        cairo_surface_destroy(surface);
    }
}


void draw_circle(cairo_t *cr, int i, int player) { //double xp, double yp, double radius, double r, double g, double b) {
    //printf("Trying to draw player %i\n",player);
    if(players[player].weighed == 1) //if player has weighed in
        cairo_set_source_rgb(cr, settings->completedRGB[0],settings->completedRGB[1],settings->completedRGB[2]); //they have been weighed so display complete RGB
    else
        cairo_set_source_rgb(cr, settings->incompletedRGB[0],settings->incompletedRGB[1],settings->incompletedRGB[2]); //they have not been weighed so display other RGB
        
    cairo_arc(cr, circles[i].x,circles[i].y,circles[i].radius, 0, 2 * G_PI);
    cairo_fill(cr);
    draw_sprite(cr,circles[i].x,circles[i].y,circles[i].radius*0.9,images[player]);
    cairo_set_source_rgb(cr, 255, 255, 255); // Sets the color to white for the text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, circles[i].radius/4); // Sets the text size
    cairo_move_to(cr, circles[i].x+circles[i].radius, circles[i].y+circles[i].radius); // Sets the position where the text will start
    cairo_show_text(cr, players[player].UniformNumber); // The text string to draw
}

gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data) {
    //Draw text
    cairo_set_source_rgb(cr, 255, 255, 255); // Sets the color to white for the text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 65); // Sets the text size
    cairo_move_to(cr, 280,80); // Sets the position where the text will start
    cairo_show_text(cr, "WEIGHT STATION"); // The text string to draw
    cairo_set_font_size(cr, 15); // Sets the text size
    cairo_move_to(cr, 340,110); // Sets the position where the text will start
    if(mode == WEIGH_IN)
        cairo_show_text(cr, "Click your headshot to weigh in"); // The text string to draw
    else if(mode == WEIGH_OUT)
        cairo_show_text(cr, "Click your headshot to weigh out");
    else if(mode == WAITING) { //when waiting draw the time till weigh in
        printf("Showing time till weigh in\n");
        time_t rawtime_int;
        double rawtime_double;
        rawtime_int = time(NULL);
        rawtime_double = (double)rawtime_int;
        double waitTime = practices[0].utcStartTime-rawtime_double;
        char time_string[120];
        seconds_to_readable_time(waitTime,time_string);
        printf("%d\n",waitTime);
        cairo_show_text(cr, time_string);
        
        g_timeout_add_seconds(10, (GSourceFunc) time_handler, (gpointer) widget); // every 60 seconds recheck if in timeout
    }
    if(mode != WAITING) {
        g_timeout_add_seconds(60, (GSourceFunc) time_handler, (gpointer) widget); // every 60 seconds recheck if in timeout
        int playersDrawn = *numPlayers_pointer;
        int circleNum = 0;
        if(playerPerPage*(pageNum+1) < playersDrawn) //set players drawn to whichever is less, max players or 15 on this page
            playersDrawn = playerPerPage*(pageNum+1);
        if(drawn == 1) {
        printf("Players drawn to: %i\n",playersDrawn);
        for (int i = playerPerPage*pageNum; i < playersDrawn; i++) {
            draw_circle(cr,circleNum, sorted[i]);
            circleNum++;
        }
        //drawn = 0;
        }
        if(pageNum < settings->numPages)
            draw_image(cr,FORWARD_ARROW_X,FORWARD_ARROW_Y,forwardArrow);
        if(pageNum > 0)
            draw_image(cr,BACK_ARROW_X,BACK_ARROW_Y,backArrow);
        }
        draw_image(cr,LOGO_X,LOGO_Y,oasisLogo);
        draw_image(cr,TEAM_X,LOGO_Y,changeTeam);
            
    return FALSE;
}

gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->button == GDK_BUTTON_PRIMARY) {
        clicked_inside_circle(event->x, event->y, widget); //check if circle was clicked
        clicked_arrow(event->x,event->y,widget); //
        clicked_logo(event->x,event->y,widget);
        clicked_team(event->x,event->y,widget);
    }

    return TRUE;
}

void create_main_window() {
    view = MAIN_WINDOW;
    GtkWidget *darea;
    GtkWidget *overlay;
    GtkWidget *fixed_container;
    GtkWidget *image;
    
    /*window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);*/
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    overlay = gtk_overlay_new();

    GdkDisplay* Display = gdk_display_get_default();
    GdkScreen* Screen = gdk_display_get_default_screen(Display);

    //gtk_style_context_add_provider_for_screen(Screen, GTK_STYLE_PROVIDER(Provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    //gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(Provider), "* { background-color: rgb(44,67,102); }", -1, NULL); //create background color

    gtk_widget_set_name(main_window, "main_window");

    GtkCssProvider *Provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(Provider),
        "#main_window { background-color: rgb(44,67,102); }"
        "window { background-color: rgb(255,255,255); }", 
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(Provider), 
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_container_add(GTK_CONTAINER(main_window), overlay);
    
    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(overlay), darea);

    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL); 
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(main_window, "button-press-event", G_CALLBACK(on_button_press_event), NULL);

    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(main_window), 1080, 1920); 
    
    fixed_container = gtk_fixed_new();
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), fixed_container);

    /*/ Set the relative path to your image file here
    image = gtk_image_new_from_file("Resources/Pineapple.jpg");
    gtk_fixed_put(GTK_FIXED(fixed_container), image, 300, 300);*/

    gtk_widget_show_all(main_window);
    
    time_handler(main_window);

    gtk_main();
}

void set_mode(enum Mode setMode, int manualOverride) {
    overridden = manualOverride;
	mode = setMode;
}

int main(int argc, char *argv[]) {
    numPlayers_pointer = malloc(sizeof(int));
    printf("Starting window\n");
    gtk_init(&argc, &argv);
    
    if (access("/home/charlie/Desktop/WeightStation/settings.json", F_OK) == -1) {//check if settings.json exists, if not, login
        printf("Doesn't exist\n");
        create_login_window();
    }
    
    load_global_images();
    
    numTeams = get_num_teams("/home/charlie/Desktop/WeightStation/settings.json");
    for(int i = 0; i < numTeams; i++) {
        load_globals(i);
    }
    set_team(0);
    
    printf("Num teams: %i\n",numTeams);
    
    if(settings->server == 0) //if client, create client
        create_client(settings->ip); 
    else {
        update_weighed_ptr = update_weighed;
        initialize_server(); //if server, create server
    }
    
    create_main_window();
    
    cleanup();
    
    printf("Finished main of Window.c\n"); 
    
    return 0;
}
