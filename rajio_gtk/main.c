#include <stdlib.h>
#include <gtk/gtk.h>

//i need to learn how to make headers
struct station {
    int station_number;
    char station_name[100];
    char thumbnail[400];
    int num_of_addresses;
    char address[400];
    char secondary_address[400];
};

//prototypes
void add_station(GtkWidget* flowbox, char* station_name, char* image_file);
extern struct station read_file(char* file_name, int station_number, long eof);
extern long eof_finder(char* file_name);

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

    //station_number = 0;
    //add_station(flow, "j sakura", "/home/seth/c/C/rajio_gtk/imgs/jsakura2016_thumb.jpg");

    float max_stations;

    long eof = eof_finder("/home/seth/c/C/rajio_gtk/stations");

    //get the max number of stations in file
    //this is done by deviding the number of bytes in the file by the size of the stuct that is stored in the file
    max_stations = sizeof(struct station) / eof;

    struct station read;

    for (station_number=0; station_number < max_stations;) {
            read = read_file("/home/seth/c/C/rajio_gtk/stations", station_number, eof);
            add_station(flow, read.station_name, read.thumbnail);
    }

    gtk_widget_show_all(window);
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
