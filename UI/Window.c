#include <gtk/gtk.h>
#include <math.h>
#include "../Curl/roster.h"
#include "../Helpers/playerHelper.h"
#include "../Helpers/jsonReader.h"
#include "../UI/login.h"
#include "../UI/dmx.h"
#include "Window.h"
#include <unistd.h>

#define RADIUS 200

typedef struct {
    double x;
    double y;
    double radius;
    double color[3];  // RGB color represented as an array of 3 doubles
} circleStruct;

Player *players;
circleStruct *circles;
int *sorted;
int playerPerPage;
int pageNum;
Settings *settings;
int *numPlayers_pointer;

gdouble distFunction(gdouble x1, gdouble y1, gdouble x2, gdouble y2) {
	return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2),0.5);
}

void cleanup() {
    free(players);
    free(circles);
    free(settings);
    free(sorted);
}

void load_globals(int teamNum) {
    settings = load_settings("settings.json",teamNum);
    
    circles = malloc(settings->playerPerPage * sizeof(circleStruct));
    
    players = getPlayers(settings->baseUrl,settings->username,settings->password, numPlayers_pointer);
    sorted = sort_players(players,*numPlayers_pointer);
    pageNum = 0;
    playerPerPage = settings->playerPerPage;
    
    // Initialize the circles array
    if(settings->playerPerPage == 15) {
        printf("playersPerPage 15\n");
        double xp = 100;
        double yp = 200;
        for (int i = 0; i < 15; i++) {
            circles[i].x = xp;
            circles[i].y = yp;
            circles[i].radius = 70;
            circles[i].color[0] = 0;  // R
            circles[i].color[1] = 255;  // G
            circles[i].color[2] = 0;  // B
            xp += 200;
            if(xp > 1050) {
                xp = 100;
                yp += 200;
            }
        }
    }
    else if(settings->playerPerPage == 60) {
        printf("playersPerPage 60\n");
        double xp = 100;
        double yp = 170;
        for (int i = 0; i < 60; i++) {
            circles[i].x = xp;
            circles[i].y = yp;
            circles[i].radius = 35;
            circles[i].color[0] = 0;  // R
            circles[i].color[1] = 255;  // G
            circles[i].color[2] = 0;  // B
            xp += 100;
            if(xp > 1050) {
                xp = 100;
                yp += 100;
            }
        }
    }
}

void change_team(int teamNum, GtkWidget *widget) {
    cleanup();
    load_globals(teamNum);
    gtk_widget_queue_draw(widget);
}

void show_text_input_dialog(GtkWidget *parent) {
    GtkWidget *dialog, *content_area;
    GtkWidget *entry;
    gint response;

    dialog = gtk_dialog_new_with_buttons("Enter Weight",
                                         GTK_WINDOW(parent),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_OK", GTK_RESPONSE_OK,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);

    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK) {
        g_print("Input: %s\n", gtk_entry_get_text(GTK_ENTRY(entry)));
    }

    gtk_widget_destroy(dialog);
}

int clicks = 0;
gboolean clicked_inside_circle(gdouble event_x, gdouble event_y, GtkWidget *widget) {
	for (int i = 0; i < 15; i++) {
		gdouble dist = distFunction(circles[i].x,circles[i].y,event_x,event_y);
		if(dist < circles[i].radius) {
			circles[i].color[1] = 0;
			circles[i].color[2] = 255;
			gtk_widget_queue_draw(widget);
			show_text_input_dialog(widget);
		}
	}
    return FALSE;
}

gboolean clicked_arrow(gdouble event_x, gdouble event_y, GtkWidget *widget) {
	if(distFunction(970,100,event_x,event_y) < 100) {
        if(pageNum < settings->numPages) {
            pageNum++;
            gtk_widget_queue_draw(widget);
            printf("Next page\n");
        }
	}
	else if(distFunction(850,100,event_x,event_y) < 100) {
        if(pageNum > 0) {
            pageNum--;
            gtk_widget_queue_draw(widget);
            printf("Last page\n");
        }
	}
    return FALSE;
}

gboolean clicked_team(gdouble event_x, gdouble event_y, GtkWidget *widget) {
	if(distFunction(70,100,event_x,event_y) < 100) {
        //change_team(0,widget);
		//show_text_input_dialog(widget);
        gtk_widget_destroy(widget);
        create_weight_window();
	}
    return FALSE;
}

void draw_sprite(cairo_t *cr, int xp, int yp, int radius, const char *filepath) {
    GdkPixbuf *pixbuf;
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    GError *error = NULL;

    // Load the image into a GdkPixbuf
    pixbuf = gdk_pixbuf_new_from_file_at_scale(filepath, -1, -1, TRUE, &error);
    if (!pixbuf) {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return;
    }

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
    g_object_unref(pixbuf);
}

void draw_circle(cairo_t *cr, int i, int player) { //double xp, double yp, double radius, double r, double g, double b) {
    printf("Trying to draw player %i\n",player);
    cairo_set_source_rgb(cr, circles[i].color[0],circles[i].color[0],circles[i].color[0]);
    cairo_arc(cr, circles[i].x,circles[i].y,circles[i].radius, 0, 2 * G_PI);
    cairo_fill(cr);
    draw_sprite(cr,circles[i].x,circles[i].y,circles[i].radius*0.9,players[player].HeadshotFilepath);
    cairo_set_source_rgb(cr, 0, 0, 0); // Sets the color to white for the text
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, circles[i].radius/4); // Sets the text size
    cairo_move_to(cr, circles[i].x-30, circles[i].y+circles[i].radius+10); // Sets the position where the text will start
    cairo_show_text(cr, players[player].Name); // The text string to draw
}

gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int playersDrawn = *numPlayers_pointer;
    int circleNum = 0;
    if(playerPerPage*(pageNum+1) < playersDrawn) //set players drawn to whichever is less, max players or 15 on this page
        playersDrawn = playerPerPage*(pageNum+1);
    printf("Players drawn to: %i\n",playersDrawn);
    for (int i = playerPerPage*pageNum; i < playersDrawn; i++) {
        draw_circle(cr,circleNum, sorted[i]);
        circleNum++;
    }
    draw_sprite(cr,900,100,100,"Resources/arrow");
    draw_sprite(cr,800,100,100,"Resources/arrow");
    return FALSE;
}

gboolean on_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->button == GDK_BUTTON_PRIMARY) {
        clicked_inside_circle(event->x, event->y, widget); //check if circle was clicked
        clicked_arrow(event->x,event->y,widget); //
        clicked_team(event->x,event->y,widget);
    }

    return TRUE;
}

void create_main_window() {
    GtkWidget *window;
    GtkWidget *darea;
    GtkWidget *overlay;
    GtkWidget *fixed_container;
    GtkWidget *image;
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);
    
    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(overlay), darea);

    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL); 
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "button-press-event", G_CALLBACK(on_button_press_event), NULL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 4*RADIUS); 
    
    fixed_container = gtk_fixed_new();
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), fixed_container);

    /*/ Set the relative path to your image file here
    image = gtk_image_new_from_file("Resources/Pineapple.jpg");
    gtk_fixed_put(GTK_FIXED(fixed_container), image, 300, 300);*/

    gtk_widget_show_all(window);

    gtk_main();
}

int main(int argc, char *argv[]) {
    numPlayers_pointer = malloc(sizeof(int));
    printf("Starting window\n");
    gtk_init(&argc, &argv);
    
    if (access("/home/charlie/Desktop/WeightStation/settings.json", F_OK) == -1) {//check if settings.json exists, if not, login
        printf("Doesn't exist\n");
        create_login_window();
    }
    
    load_globals(0);
    
    create_main_window();
    
    cleanup();
    
    printf("Finished main of Window.c\n");
    
    return 0;
}
