#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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
    if (!bar || !bar->window)
        return;

    gtk_widget_set_visible(GTK_WIDGET(bar->window), FALSE);
}

static void show_bar(MusicBar *bar)
{
    if (!bar || !bar->window)
        return;

    gtk_widget_set_visible(GTK_WIDGET(bar->window), TRUE);
}

static bool json_bool_true(const char *buffer, const char *key)
{
    if (!buffer || !key)
        return false;

    char pattern[128];

    snprintf(
        pattern,
        sizeof(pattern),
        "\"%s\": true",
        key
    );

    return strstr(buffer, pattern) != NULL;
}

static bool get_json_string(
    const char *buffer,
    const char *key,
    char *output,
    size_t output_size
)
{
    if (!buffer || !key || !output || output_size == 0)
        return false;

    output[0] = '\0';

    char pattern[128];

    snprintf(
        pattern,
        sizeof(pattern),
        "\"%s\": \"",
        key
    );

    const char *start = strstr(buffer, pattern);

    if (!start)
        return false;

    start += strlen(pattern);

    const char *end = strchr(start, '"');

    if (!end)
        return false;

    size_t length = (size_t)(end - start);

    if (length >= output_size)
        length = output_size - 1;

    memcpy(output, start, length);
    output[length] = '\0';

    return true;
}

static bool valid_title(const char *title)
{
    if (!title || title[0] == '\0')
        return false;

    for (const char *p = title; *p != '\0'; ++p) {
        if (*p != ' ' && *p != '\n' && *p != '\r' && *p != '\t')
            return true;
    }

    return false;
}

static gboolean update_bar(gpointer data)
{
    MusicBar *bar = data;

    if (!bar)
        return G_SOURCE_CONTINUE;

    FILE *file = fopen(STATE_FILE, "r");

    if (!file) {
        hide_bar(bar);
        return G_SOURCE_CONTINUE;
    }

    char buffer[16384];

    size_t length = fread(
        buffer,
        1,
        sizeof(buffer) - 1,
        file
    );

    fclose(file);

    buffer[length] = '\0';

    /*
     * The userscript is the source of truth.
     * The bar only appears when the state says that a
     * currently active video is playing and classified as music.
     */
    if (!json_bool_true(buffer, "active") ||
        !json_bool_true(buffer, "playing") ||
        !json_bool_true(buffer, "music")) {
        hide_bar(bar);
        return G_SOURCE_CONTINUE;
    }

    char title[4096];

    if (!get_json_string(buffer, "title", title, sizeof(title)) ||
        !valid_title(title)) {
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

    gtk_label_set_text(bar->label, display);
    show_bar(bar);

    return G_SOURCE_CONTINUE;
}

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    MusicBar *bar = g_new0(MusicBar, 1);

    GtkWidget *window = gtk_application_window_new(app);

    bar->window = GTK_WINDOW(window);

    gtk_window_set_decorated(bar->window, FALSE);
    gtk_window_set_resizable(bar->window, FALSE);
    gtk_window_set_default_size(bar->window, 700, BAR_HEIGHT);

    gtk_layer_init_for_window(bar->window);

    gtk_layer_set_layer(
        bar->window,
        GTK_LAYER_SHELL_LAYER_TOP
    );

    gtk_layer_set_anchor(
        bar->window,
        GTK_LAYER_SHELL_EDGE_TOP,
        TRUE
    );

    gtk_layer_set_anchor(
        bar->window,
        GTK_LAYER_SHELL_EDGE_LEFT,
        FALSE
    );

    gtk_layer_set_anchor(
        bar->window,
        GTK_LAYER_SHELL_EDGE_RIGHT,
        FALSE
    );

    gtk_layer_set_anchor(
        bar->window,
        GTK_LAYER_SHELL_EDGE_BOTTOM,
        FALSE
    );

    gtk_layer_set_margin(
        bar->window,
        GTK_LAYER_SHELL_EDGE_TOP,
        BAR_TOP_MARGIN
    );

    gtk_layer_set_exclusive_zone(
        bar->window,
        EXCLUSIVE_ZONE
    );

    gtk_layer_set_keyboard_mode(
        bar->window,
        GTK_LAYER_SHELL_KEYBOARD_MODE_NONE
    );

    GtkCssProvider *css = gtk_css_provider_new();

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
        gtk_widget_get_display(GTK_WIDGET(bar->window)),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(css);

    GtkWidget *label = gtk_label_new("");

    bar->label = GTK_LABEL(label);

    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_widget_set_vexpand(label, TRUE);

    gtk_label_set_ellipsize(
        bar->label,
        PANGO_ELLIPSIZE_END
    );

    gtk_label_set_max_width_chars(
        bar->label,
        90
    );

    GtkWidget *box = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL,
        0
    );

    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_vexpand(box, TRUE);

    gtk_box_append(GTK_BOX(box), label);
    gtk_window_set_child(bar->window, box);

    /*
     * Important: a GTK window must be presented/mapped before
     * changing its visibility. The old code created the window
     * but never presented it, so show_bar() could not reliably
     * make the layer-shell surface appear.
     */
    gtk_window_present(bar->window);

    /* Start hidden after the surface has been mapped. */
    hide_bar(bar);

    g_timeout_add(200, update_bar, bar);
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new(
        "com.yuji.MusicBar",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        NULL
    );

    int status = g_application_run(
        G_APPLICATION(app),
        argc,
        argv
    );

    g_object_unref(app);

    return status;
}
