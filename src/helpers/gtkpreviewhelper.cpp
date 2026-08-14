#include <gtk/gtk.h>

#include <cairo.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef MAUI_SETTINGS_GTK4
#include <gsk/gsk.h>

typedef struct _PreviewRoot
{
    GtkWidget parent_instance;
    GtkWidget *child;
} PreviewRoot;

typedef struct _PreviewRootClass
{
    GtkWidgetClass parent_class;
} PreviewRootClass;

G_DEFINE_TYPE(PreviewRoot, preview_root, GTK_TYPE_WIDGET)

static void preview_root_measure(GtkWidget *widget,
                                 GtkOrientation orientation,
                                 int for_size,
                                 int *minimum,
                                 int *natural,
                                 int *minimum_baseline,
                                 int *natural_baseline)
{
    PreviewRoot *root = reinterpret_cast<PreviewRoot *>(widget);
    if (root->child)
        gtk_widget_measure(root->child, orientation, for_size, minimum, natural, minimum_baseline, natural_baseline);
    else
    {
        *minimum = 0;
        *natural = 0;
        *minimum_baseline = -1;
        *natural_baseline = -1;
    }
}

static void preview_root_size_allocate(GtkWidget *widget,
                                       int width,
                                       int height,
                                       int baseline)
{
    PreviewRoot *root = reinterpret_cast<PreviewRoot *>(widget);
    if (root->child)
        gtk_widget_allocate(root->child, width, height, baseline, nullptr);
}

static void preview_root_snapshot(GtkWidget *widget, GtkSnapshot *snapshot)
{
    PreviewRoot *root = reinterpret_cast<PreviewRoot *>(widget);
    const float width = static_cast<float>(gtk_widget_get_width(widget));
    const float height = static_cast<float>(gtk_widget_get_height(widget));
    const GdkRGBA background = {0.12, 0.12, 0.18, 1.0};
    const graphene_rect_t bounds = GRAPHENE_RECT_INIT(0, 0, width, height);
    gtk_snapshot_append_color(snapshot, &background, &bounds);
    if (root->child)
        gtk_widget_snapshot_child(widget, root->child, snapshot);
}

static void preview_root_class_init(PreviewRootClass *klass)
{
    GtkWidgetClass *widgetClass = GTK_WIDGET_CLASS(klass);
    widgetClass->measure = preview_root_measure;
    widgetClass->size_allocate = preview_root_size_allocate;
    widgetClass->snapshot = preview_root_snapshot;
}

static void preview_root_init(PreviewRoot *root)
{
    root->child = nullptr;
}

static PreviewRoot *preview_root_new(GtkWidget *child)
{
    PreviewRoot *root = reinterpret_cast<PreviewRoot *>(g_object_new(preview_root_get_type(), nullptr));
    root->child = child;
    gtk_widget_set_parent(child, GTK_WIDGET(root));
    return root;
}

static cairo_status_t writeCairoPng(void *, const unsigned char *data, unsigned int length)
{
    return std::fwrite(data, 1, length, stdout) == length ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR;
}

static GtkWidget *createPreview()
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(box, 16);
    gtk_widget_set_margin_end(box, 16);
    gtk_widget_set_margin_top(box, 16);
    gtk_widget_set_margin_bottom(box, 16);

    GtkWidget *title = gtk_label_new("GTK 4 preview");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), title);

    GtkWidget *entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry), "Text field");
    gtk_box_append(GTK_BOX(box), entry);

    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *button = gtk_button_new_with_label("Button");
    GtkWidget *check = gtk_check_button_new_with_label("Check box");
    const char *options[] = {"Option one", "Option two", nullptr};
    GtkWidget *dropDown = gtk_drop_down_new_from_strings(options);
    gtk_widget_set_hexpand(dropDown, TRUE);
    gtk_box_append(GTK_BOX(controls), button);
    gtk_box_append(GTK_BOX(controls), check);
    gtk_box_append(GTK_BOX(controls), dropDown);
    gtk_box_append(GTK_BOX(box), controls);

    GtkWidget *slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(slider), 60);
    gtk_widget_set_hexpand(slider, TRUE);
    gtk_box_append(GTK_BOX(box), slider);

    return box;
}

static int renderPreview(const char *theme, int width, int height)
{
    g_setenv("GTK_THEME", theme, TRUE);
    gtk_init();

    GtkSettings *settings = gtk_settings_get_default();
    if (settings)
        g_object_set(settings, "gtk-theme-name", theme, nullptr);

    GtkWidget *window = gtk_window_new();
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);

    GtkWidget *content = createPreview();
    PreviewRoot *root = preview_root_new(content);
    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(root));
    gtk_window_present(GTK_WINDOW(window));

    for (int i = 0; i < 30; ++i)
    {
        while (g_main_context_pending(nullptr))
            g_main_context_iteration(nullptr, FALSE);
    }

    GtkSnapshot *snapshot = gtk_snapshot_new();
    GTK_WIDGET_GET_CLASS(root)->snapshot(GTK_WIDGET(root), snapshot);
    GskRenderNode *node = gtk_snapshot_free_to_node(snapshot);
    if (!node)
        return 1;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cairo = cairo_create(surface);
    gsk_render_node_draw(node, cairo);
    const cairo_status_t status = cairo_surface_write_to_png_stream(surface, writeCairoPng, nullptr);

    cairo_destroy(cairo);
    cairo_surface_destroy(surface);
    gsk_render_node_unref(node);
    gtk_window_destroy(GTK_WINDOW(window));
    return status == CAIRO_STATUS_SUCCESS ? 0 : 1;
}

#else

static gboolean writePixbuf(const gchar *data, gsize length, GError **, gpointer)
{
    return std::fwrite(data, 1, length, stdout) == length;
}

static void setMargins(GtkWidget *widget, int margin)
{
    gtk_widget_set_margin_start(widget, margin);
    gtk_widget_set_margin_end(widget, margin);
    gtk_widget_set_margin_top(widget, margin);
    gtk_widget_set_margin_bottom(widget, margin);
}

static GtkWidget *createPreview()
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    setMargins(box, 16);

    GtkWidget *title = gtk_label_new("GTK 3 preview");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "Text field");
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);

    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *button = gtk_button_new_with_label("Button");
    GtkWidget *check = gtk_check_button_new_with_label("Check box");
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Option one");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "Option two");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_box_pack_start(GTK_BOX(controls), button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(controls), check, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(controls), combo, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), controls, FALSE, FALSE, 0);

    GtkWidget *slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(slider), 60);
    gtk_widget_set_hexpand(slider, TRUE);
    gtk_box_pack_start(GTK_BOX(box), slider, FALSE, FALSE, 0);

    return box;
}

static int renderPreview(const char *theme, int width, int height)
{
    g_setenv("GTK_THEME", theme, TRUE);

    int argc = 1;
    char applicationName[] = "maui-settings-gtk3-preview-helper";
    char *argvData[] = {applicationName, nullptr};
    char **argv = argvData;
    if (!gtk_init_check(&argc, &argv))
        return 1;

    GtkSettings *settings = gtk_settings_get_default();
    if (settings)
        g_object_set(settings, "gtk-theme-name", theme, nullptr);

    GtkWidget *window = gtk_offscreen_window_new();
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_widget_set_size_request(window, width, height);

    GtkWidget *content = createPreview();
    gtk_container_add(GTK_CONTAINER(window), content);
    gtk_widget_show_all(window);

    for (int i = 0; i < 30; ++i)
    {
        while (gtk_events_pending())
            gtk_main_iteration_do(FALSE);
    }

    GdkPixbuf *pixbuf = gtk_offscreen_window_get_pixbuf(GTK_OFFSCREEN_WINDOW(window));
    if (!pixbuf)
        return 1;

    GError *error = nullptr;
    const gboolean saved = gdk_pixbuf_save_to_callback(pixbuf, writePixbuf, nullptr, "png", &error, nullptr);
    if (error)
        g_error_free(error);
    gtk_widget_destroy(window);
    return saved ? 0 : 1;
}

#endif

int main(int argc, char **argv)
{
    const char *theme = "Adwaita";
    int width = 460;
    int height = 190;

    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--theme") == 0 && i + 1 < argc)
            theme = argv[++i];
        else if (std::strcmp(argv[i], "--width") == 0 && i + 1 < argc)
            width = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--height") == 0 && i + 1 < argc)
            height = std::atoi(argv[++i]);
    }

    if (theme[0] == 0 || width <= 0 || height <= 0)
        return 2;

    return renderPreview(theme, width, height);
}
