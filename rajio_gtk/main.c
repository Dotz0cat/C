#include <stdlib.h>
#include <gtk/gtk.h>

//prototypes
void add_station(GtkWidget* flowbox, char* station_name, char* image_file);

int station_number;

int main(int argc, char* argv[]) {

    GtkBuilder* builder;
    GtkWidget* window;
    GtkWidget* flow;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "rajio_gtk_v2.glade", NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "gWindow"));
    flow = GTK_WIDGET(gtk_builder_get_object(builder, "flowbox1"));

    station_number = 0;
    char* name = "jsakura2016_thumb.jpg";
    add_station(flow, "j sakura", name);

    gtk_widget_show(window);
    gtk_main();

    return 0;
}

void add_station(GtkWidget* flowbox, char* station_name, char* image_file) {
    GtkGrid* grid;

    grid = (GtkGrid*) gtk_grid_new();

    gtk_grid_set_row_homogeneous(grid, TRUE);
    gtk_grid_set_column_homogeneous(grid, TRUE);

    //make the gtkImage
    GtkImage* image;
    image = (GtkImage*) gtk_image_new_from_file(image_file);

    gtk_grid_attach(grid, (GtkWidget*) image, 0, 0, 3, 2);

    //make the label
    GtkWidget* label;
    label = gtk_label_new(station_name);

    gtk_grid_attach(grid, label, 0, 2, 3, 1);

    //add the grid to the flowbox
    gtk_container_add((GtkContainer*) flowbox, (GtkWidget*) grid);

    //icremnt the station number
    station_number++;
}
