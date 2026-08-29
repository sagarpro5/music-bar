#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STATE_FILE "/home/yuji/.local/share/youtube-song/state.json"

#define WAYBAR_HEIGHT 40
#define GAP_ABOVE_BAR 2
#define BAR_HEIGHT 40
#define GAP_BELOW_BAR 4

#define BAR_TOP_MARGIN (WAYBAR_HEIGHT + GAP_ABOVE_BAR)
#define EXCLUSIVE_ZONE (BAR_TOP_MARGIN + BAR_HEIGHT + GAP_BELOW_BAR)

typedef struct {
    GtkWindow *window;
    GtkLabel *label;
} MusicBar;

static void hide_bar(MusicBar *bar)
{
    gtk_widget_set_visible(
        GTK_WIDGET(bar->window),
        FALSE
    );
}

static void show_bar(MusicBar *bar)
{
    gtk_widget_set_visible(
        GTK_WIDGET(bar->window),
        TRUE
    );
}

static gboolean update_bar(gpointer data)
{
    MusicBar *bar = data;

    FILE *file = fopen(
        STATE_FILE,
        "r"
    );

    if (!file) {
        hide_bar(bar);
        return G_SOURCE_CONTINUE;
    }

    char buffer[8192];

    size_t length = fread(
        buffer,
        1,
        sizeof(buffer) - 1,
        file
    );

    fclose(file);

    buffer[length] = '\0';

    if (!strstr(
        buffer,
        "\"active\": true"
    )) {
        hide_bar(bar);
        return G_SOURCE_CONTINUE;
    }

    char title[4096] = "";

    char *start = strstr(
        buffer,
        "\"title\": \""
    );

    if (!start) {
        hide_bar(bar);
        return G_SOURCE_CONTINUE;
    }

    start += strlen(
        "\"title\": \""
    );

    char *end = strchr(
        start,
        '"'
    );

    if (!end) {
        hide_bar(bar);
        return G_SOURCE_CONTINUE;
    }

    size_t title_length =
        (size_t)(end - start);

    if (title_length >= sizeof(title))
        title_length = sizeof(title) - 1;

    memcpy(
        title,
        start,
        title_length
    );

    title[title_length] = '\0';

    if (title[0] == '\0') {
        hide_bar(bar);
        return G_SOURCE_CONTINUE;
    }

    if (strlen(title) > 100) {
        title[97] = '\0';
        strcat(title, "...");
    }

    char display[4200];

    snprintf(
        display,
        sizeof(display),
        "♪  %s",
        title
    );

    gtk_label_set_text(
        bar->label,
        display
    );

    show_bar(bar);

    return G_SOURCE_CONTINUE;
}

static void activate(
    GtkApplication *app,
    gpointer user_data
)
{
    GtkWidget *window;
    GtkWidget *label;
    GtkWidget *box;

    window = gtk_application_window_new(
        app
    );

    gtk_layer_init_for_window(
        GTK_WINDOW(window)
    );

    gtk_layer_set_layer(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_LAYER_TOP
    );

    gtk_layer_set_anchor(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_EDGE_TOP,
        TRUE
    );

    gtk_layer_set_anchor(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_EDGE_LEFT,
        FALSE
    );

    gtk_layer_set_anchor(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_EDGE_RIGHT,
        FALSE
    );

    gtk_layer_set_margin(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_EDGE_TOP,
        BAR_TOP_MARGIN
    );

    gtk_layer_set_exclusive_zone(
        GTK_WINDOW(window),
        EXCLUSIVE_ZONE
    );

    gtk_layer_set_keyboard_mode(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_KEYBOARD_MODE_NONE
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        700,
        BAR_HEIGHT
    );

    GtkCssProvider *css =
        gtk_css_provider_new();

    gtk_css_provider_load_from_string(
        css,

        "window {"
        "background: #e7d8b1;"
        "border-radius: 10px;"
        "}"

        "box {"
        "background: transparent;"
        "}"

        "label {"
        "color: #000000;"
        "font-size: 16px;"
        "font-weight: 500;"
        "padding-left: 20px;"
        "padding-right: 20px;"
        "}"
    );

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(css);

    label = gtk_label_new("");

    gtk_widget_set_halign(
        label,
        GTK_ALIGN_CENTER
    );

    gtk_widget_set_valign(
        label,
        GTK_ALIGN_CENTER
    );

    gtk_widget_set_hexpand(
        label,
        TRUE
    );

    gtk_widget_set_vexpand(
        label,
        TRUE
    );

    gtk_label_set_ellipsize(
        GTK_LABEL(label),
        PANGO_ELLIPSIZE_END
    );

    gtk_label_set_max_width_chars(
        GTK_LABEL(label),
        90
    );

    box = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL,
        0
    );

    gtk_widget_set_halign(
        box,
        GTK_ALIGN_CENTER
    );

    gtk_widget_set_valign(
        box,
        GTK_ALIGN_CENTER
    );

    gtk_widget_set_hexpand(
        box,
        TRUE
    );

    gtk_widget_set_vexpand(
        box,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        label
    );

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    MusicBar *bar = g_new0(
        MusicBar,
        1
    );

    bar->window =
        GTK_WINDOW(window);

    bar->label =
        GTK_LABEL(label);

    hide_bar(bar);

    g_timeout_add(
        200,
        update_bar,
        bar
    );
}

int main(
    int argc,
    char **argv
)
{
    GtkApplication *app =
        gtk_application_new(
            "com.yuji.MusicBar",
            G_APPLICATION_DEFAULT_FLAGS
        );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        NULL
    );

    int status =
        g_application_run(
            G_APPLICATION(app),
            argc,
            argv
        );

    g_object_unref(app);

    return status;
}