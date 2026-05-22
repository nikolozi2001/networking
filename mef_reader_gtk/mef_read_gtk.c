#include <gtk/gtk.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mef_format.h"

// ============================
// CSS თემა
// ============================
static const char *APP_CSS =
    "window {"
    "  background-color: #1e1e2e;"
    "}"
    "headerbar {"
    "  background-color: #181825;"
    "  border-bottom: 1px solid #313244;"
    "}"
    "headerbar button {"
    "  background-color: #313244;"
    "  color: #cdd6f4;"
    "  border: none;"
    "  border-radius: 6px;"
    "  padding: 4px 12px;"
    "}"
    "headerbar button:hover {"
    "  background-color: #45475a;"
    "}"
    ".sidebar {"
    "  background-color: #181825;"
    "  border-right: 1px solid #313244;"
    "}"
    ".sidebar row {"
    "  padding: 6px 12px;"
    "  color: #cdd6f4;"
    "  border-radius: 6px;"
    "  margin: 2px 4px;"
    "}"
    ".sidebar row:selected {"
    "  background-color: #313244;"
    "  color: #89b4fa;"
    "}"
    ".sidebar row label {"
    "  font-family: monospace;"
    "  font-size: 13px;"
    "}"
    ".info-panel {"
    "  background-color: #1e1e2e;"
    "  padding: 16px;"
    "}"
    ".info-panel label {"
    "  color: #cdd6f4;"
    "  font-family: monospace;"
    "  font-size: 13px;"
    "}"
    ".field-name {"
    "  color: #89b4fa;"
    "  font-weight: bold;"
    "  font-family: monospace;"
    "  font-size: 13px;"
    "  min-width: 140px;"
    "}"
    ".field-value {"
    "  color: #a6e3a1;"
    "  font-family: monospace;"
    "  font-size: 13px;"
    "}"
    ".section-title {"
    "  color: #cba6f7;"
    "  font-weight: bold;"
    "  font-size: 15px;"
    "  font-family: monospace;"
    "  margin-bottom: 8px;"
    "}"
    ".hex-view {"
    "  background-color: #181825;"
    "  color: #cdd6f4;"
    "  font-family: monospace;"
    "  font-size: 12px;"
    "  padding: 8px;"
    "  border-top: 1px solid #313244;"
    "}"
    ".tag-code {"
    "  color: #a6e3a1;"
    "  font-weight: bold;"
    "}"
    ".tag-data {"
    "  color: #fab387;"
    "  font-weight: bold;"
    "}"
    ".drop-hint {"
    "  color: #6c7086;"
    "  font-size: 16px;"
    "  font-family: sans-serif;"
    "}";

// ============================
// მონაცემები
// ============================
typedef struct {
    GtkWidget    *window;
    GtkListBox   *list_box;
    GtkWidget    *info_box;
    GtkWidget    *hex_view;
    GtkWidget    *right_paned;
    int           fd;
    mef_hdr_t     hdr;
    mef_sect_t   *sects;
    uint32_t      page_size;
} AppData;

// ============================
// page size ჰედერიდან
// ============================
static uint32_t get_page_size(const mef_hdr_t *hdr) {
    if(hdr->os_type == MEF_HDR_OS_TYPE_MAC_OS &&
       hdr->cpu_type == MEF_HDR_CPU_TYPE_AARCH_64)
        return 16384;
    return 4096;
}

static const char *cpu_type_str(uint8_t t) {
    switch(t) {
        case MEF_HDR_CPU_TYPE_UNKNOWN:  return "Unknown";
        case MEF_HDR_CPU_TYPE_X86:      return "x86";
        case MEF_HDR_CPU_TYPE_X86_64:   return "x86_64";
        case MEF_HDR_CPU_TYPE_AARCH:    return "AArch32";
        case MEF_HDR_CPU_TYPE_AARCH_64: return "AArch64";
        default:                        return "Unknown";
    }
}

static const char *os_type_str(uint8_t t) {
    switch(t) {
        case MEF_HDR_OS_TYPE_UNIVERSAL: return "Universal";
        case MEF_HDR_OS_TYPE_LINUX:     return "Linux";
        case MEF_HDR_OS_TYPE_WINDOWS:   return "Windows";
        case MEF_HDR_OS_TYPE_MAC_OS:    return "macOS";
        default:                        return "Unknown";
    }
}

static void protect_str(uint8_t protect, char *buf) {
    buf[0] = (protect & 0x1) ? 'R' : '-';
    buf[1] = (protect & 0x2) ? 'W' : '-';
    buf[2] = (protect & 0x4) ? 'X' : '-';
    buf[3] = '\0';
}

// ============================
// info panel-ში ველის დამატება
// ============================
static void add_field(GtkWidget *box, const char *name, const char *value) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    GtkWidget *lbl_name = gtk_label_new(name);
    gtk_widget_add_css_class(lbl_name, "field-name");
    gtk_label_set_xalign(GTK_LABEL(lbl_name), 0.0f);
    gtk_widget_set_size_request(lbl_name, 160, -1);

    GtkWidget *lbl_val = gtk_label_new(value);
    gtk_widget_add_css_class(lbl_val, "field-value");
    gtk_label_set_xalign(GTK_LABEL(lbl_val), 0.0f);
    gtk_label_set_selectable(GTK_LABEL(lbl_val), TRUE);

    gtk_box_append(GTK_BOX(row), lbl_name);
    gtk_box_append(GTK_BOX(row), lbl_val);
    gtk_box_append(GTK_BOX(box), row);
}

// ============================
// hex dump
// ============================
static void update_hex(AppData *app, uint32_t offset, uint32_t size) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->hex_view));
    GString *str = g_string_new(NULL);

    uint32_t dump_size = size > 512 ? 512 : size;
    uint8_t data[16];

    lseek(app->fd, offset, SEEK_SET);

    for(uint32_t i = 0; i < dump_size; i += 16) {
        ssize_t n = read(app->fd, data, 16);
        if(n <= 0) break;

        g_string_append_printf(str, "%08X  ", offset + i);

        for(int j = 0; j < 16; j++) {
            if(j < n) g_string_append_printf(str, "%02X ", data[j]);
            else       g_string_append(str, "   ");
            if(j == 7) g_string_append(str, " ");
        }

        g_string_append(str, " |");
        for(int j = 0; j < n; j++)
            g_string_append_printf(str, "%c",
                (data[j] >= 32 && data[j] < 127) ? data[j] : '.');
        g_string_append(str, "|\n");
    }

    if(size > 512)
        g_string_append_printf(str, "... (+%u ბაიტი)\n", size - 512);

    gtk_text_buffer_set_text(buf, str->str, -1);
    g_string_free(str, TRUE);
}

// ============================
// info panel-ის გასუფთავება
// ============================
static void clear_info(AppData *app) {
    GtkWidget *child;
    while((child = gtk_widget_get_first_child(app->info_box)) != NULL)
        gtk_box_remove(GTK_BOX(app->info_box), child);
}

// ============================
// ჰედერის ინფო
// ============================
static void show_header_info(AppData *app) {
    clear_info(app);

    GtkWidget *title = gtk_label_new("MEF ჰედერი");
    gtk_widget_add_css_class(title, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(app->info_box), title);

    char val[64];

    snprintf(val, sizeof(val), "0x%08X  ('\\x7f','m','e','f')", app->hdr.magic);
    add_field(app->info_box, "magic:", val);

    snprintf(val, sizeof(val), "%u  →  0x%08X",
             app->hdr.vm_load, app->hdr.vm_load * app->page_size);
    add_field(app->info_box, "vm_load:", val);

    snprintf(val, sizeof(val), "%u გვერდი  (%u ბაიტი)",
             app->hdr.vm_size, app->hdr.vm_size * app->page_size);
    add_field(app->info_box, "vm_size:", val);

    snprintf(val, sizeof(val), "%u  (%s)",
             app->hdr.cpu_type, cpu_type_str(app->hdr.cpu_type));
    add_field(app->info_box, "cpu_type:", val);

    snprintf(val, sizeof(val), "%u  (%s)",
             app->hdr.os_type, os_type_str(app->hdr.os_type));
    add_field(app->info_box, "os_type:", val);

    snprintf(val, sizeof(val), "%u", app->hdr.sect_count);
    add_field(app->info_box, "sect_count:", val);

    snprintf(val, sizeof(val), "%u", app->hdr.code_index);
    add_field(app->info_box, "code_index:", val);

    snprintf(val, sizeof(val), "%u ბაიტი  (page size)", app->page_size);
    add_field(app->info_box, "page_size:", val);

    snprintf(val, sizeof(val), "0x%08X",
             app->hdr.vm_load * app->page_size +
             app->sects[app->hdr.code_index].offset * app->page_size);
    add_field(app->info_box, "entry point:", val);

    update_hex(app, 0, sizeof(mef_hdr_t));
}

// ============================
// სექციის ინფო
// ============================
static void show_section_info(AppData *app, int idx) {
    clear_info(app);

    char title_str[64];
    snprintf(title_str, sizeof(title_str), "სექცია %d%s",
             idx, (idx == app->hdr.code_index) ? "  [კოდი]" : "");

    GtkWidget *title = gtk_label_new(title_str);
    gtk_widget_add_css_class(title, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_box_append(GTK_BOX(app->info_box), title);

    char val[64];
    char prot[4];
    protect_str(app->sects[idx].protect, prot);

    uint32_t file_off = app->sects[idx].offset * app->page_size;
    uint32_t mem_addr = app->hdr.vm_load * app->page_size +
                        app->sects[idx].offset * app->page_size;
    uint32_t size     = app->sects[idx].size * app->page_size;

    snprintf(val, sizeof(val), "%u გვერდი", app->sects[idx].offset);
    add_field(app->info_box, "offset:", val);

    snprintf(val, sizeof(val), "%u გვერდი  (%u ბაიტი)",
             app->sects[idx].size, size);
    add_field(app->info_box, "size:", val);

    snprintf(val, sizeof(val), "0x%08X", file_off);
    add_field(app->info_box, "file offset:", val);

    snprintf(val, sizeof(val), "0x%08X", mem_addr);
    add_field(app->info_box, "mem addr:", val);

    snprintf(val, sizeof(val), "%s  (%s%s%s)",
             prot,
             (app->sects[idx].protect & 0x1) ? "READ " : "",
             (app->sects[idx].protect & 0x2) ? "WRITE " : "",
             (app->sects[idx].protect & 0x4) ? "EXEC" : "");
    add_field(app->info_box, "protect:", val);

    update_hex(app, file_off, size);
}

// ============================
// სელექცია
// ============================
static void on_row_selected(GtkListBox *box, GtkListBoxRow *row, AppData *app) {
    if(!row) return;

    int idx = gtk_list_box_row_get_index(row);
    if(idx == 0)
        show_header_info(app);
    else
        show_section_info(app, idx - 1);
}

// ============================
// sidebar-ის შევსება
// ============================
static void populate_sidebar(AppData *app) {
    // ძველი ელემენტების წაშლა
    GtkWidget *child;
    while((child = gtk_widget_get_first_child(GTK_WIDGET(app->list_box))) != NULL)
        gtk_list_box_remove(app->list_box, child);

    // ჰედერი
    GtkWidget *lbl = gtk_label_new("ჰედერი");
    gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);
    gtk_list_box_append(app->list_box, lbl);

    // სექციები
    for(int i = 0; i < app->hdr.sect_count; i++) {
        char prot[4];
        protect_str(app->sects[i].protect, prot);

        char name[64];
        snprintf(name, sizeof(name), "სექცია %d  [%s]%s",
                 i, prot, (i == app->hdr.code_index) ? " ★" : "");

        GtkWidget *row_lbl = gtk_label_new(name);
        gtk_label_set_xalign(GTK_LABEL(row_lbl), 0.0f);
        gtk_widget_add_css_class(row_lbl,
            (i == app->hdr.code_index) ? "tag-code" : "tag-data");
        gtk_list_box_append(app->list_box, row_lbl);
    }
}

// ============================
// ფაილის ჩატვირთვა
// ============================
static void load_file(AppData *app, const char *path) {
    if(app->fd >= 0) { close(app->fd); app->fd = -1; }
    if(app->sects)   { free(app->sects); app->sects = NULL; }

    int fd = open(path, O_RDONLY);
    if(fd < 0) return;

    mef_hdr_t hdr;
    if(read(fd, &hdr, sizeof(mef_hdr_t)) != sizeof(mef_hdr_t) ||
       hdr.magic != MEF_HDR_MAGIC) {
        close(fd);
        return;
    }

    mef_sect_t *sects = malloc(hdr.sect_count * sizeof(mef_sect_t));
    if(!sects) { close(fd); return; }

    if(read(fd, sects, hdr.sect_count * sizeof(mef_sect_t)) !=
       (ssize_t)(hdr.sect_count * sizeof(mef_sect_t))) {
        free(sects);
        close(fd);
        return;
    }

    app->fd        = fd;
    app->hdr       = hdr;
    app->sects     = sects;
    app->page_size = get_page_size(&hdr);

    gchar *title = g_strdup_printf("MEF Reader  —  %s", path);
    gtk_window_set_title(GTK_WINDOW(app->window), title);
    g_free(title);

    populate_sidebar(app);

    // ავტომატურად ჰედერი
    GtkListBoxRow *first = gtk_list_box_get_row_at_index(app->list_box, 0);
    if(first) gtk_list_box_select_row(app->list_box, first);
}

// ============================
// ფაილის გახსნის callback
// ============================
static void on_open_response(GObject *source, GAsyncResult *res, gpointer user_data) {
    AppData *app = user_data;
    GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source), res, NULL);
    if(!file) return;
    char *path = g_file_get_path(file);
    if(path) { load_file(app, path); g_free(path); }
    g_object_unref(file);
}

static void on_open_clicked(GtkButton *btn, AppData *app) {
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "MEF ფაილის გახსნა");

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "MEF files (*.mef)");
    gtk_file_filter_add_pattern(filter, "*.mef");
    GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

    gtk_file_dialog_open(dialog, GTK_WINDOW(app->window), NULL,
                         on_open_response, app);
    g_object_unref(dialog);
    g_object_unref(filters);
    g_object_unref(filter);
}

// ============================
// drag-and-drop
// ============================
static gboolean on_drop(GtkDropTarget *target, const GValue *value,
                         double x, double y, gpointer user_data) {
    AppData *app = user_data;
    if(G_VALUE_HOLDS(value, G_TYPE_FILE)) {
        GFile *file = g_value_get_object(value);
        char *path = g_file_get_path(file);
        if(path) { load_file(app, path); g_free(path); }
        return TRUE;
    }
    return FALSE;
}

// ============================
// activate
// ============================
static void activate(GtkApplication *gtk_app, gpointer user_data) {
    AppData *app = user_data;

    // CSS
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css, APP_CSS);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    // window
    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "MEF Reader");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 960, 640);

    // header bar
    GtkWidget *hbar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(app->window), hbar);

    GtkWidget *open_btn = gtk_button_new_with_label("  გახსნა");
    g_signal_connect(open_btn, "clicked", G_CALLBACK(on_open_clicked), app);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), open_btn);

    // მთავარი paned
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_window_set_child(GTK_WINDOW(app->window), paned);
    gtk_paned_set_position(GTK_PANED(paned), 240);

    // sidebar
    GtkWidget *sidebar_scroll = gtk_scrolled_window_new();
    gtk_widget_add_css_class(sidebar_scroll, "sidebar");
    gtk_widget_set_size_request(sidebar_scroll, 220, -1);

    app->list_box = GTK_LIST_BOX(gtk_list_box_new());
    gtk_widget_add_css_class(GTK_WIDGET(app->list_box), "sidebar");
    gtk_list_box_set_selection_mode(app->list_box, GTK_SELECTION_SINGLE);
    g_signal_connect(app->list_box, "row-selected", G_CALLBACK(on_row_selected), app);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sidebar_scroll),
                                   GTK_WIDGET(app->list_box));
    gtk_paned_set_start_child(GTK_PANED(paned), sidebar_scroll);

    // მარჯვენა — vertical paned
    GtkWidget *right_paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_position(GTK_PANED(right_paned), 300);
    gtk_paned_set_end_child(GTK_PANED(paned), right_paned);

    // ინფო panel
    GtkWidget *info_scroll = gtk_scrolled_window_new();
    gtk_widget_add_css_class(info_scroll, "info-panel");

    app->info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_add_css_class(app->info_box, "info-panel");
    gtk_widget_set_margin_start(app->info_box, 16);
    gtk_widget_set_margin_top(app->info_box, 16);

    GtkWidget *hint = gtk_label_new("გახსენით MEF ფაილი ან ჩააგდეთ აქ");
    gtk_widget_add_css_class(hint, "drop-hint");
    gtk_box_append(GTK_BOX(app->info_box), hint);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(info_scroll), app->info_box);
    gtk_paned_set_start_child(GTK_PANED(right_paned), info_scroll);

    // hex view
    app->hex_view = gtk_text_view_new();
    gtk_widget_add_css_class(app->hex_view, "hex-view");
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->hex_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(app->hex_view), TRUE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(app->hex_view), 8);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(app->hex_view), 8);

    GtkWidget *hex_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(hex_scroll), app->hex_view);
    gtk_paned_set_end_child(GTK_PANED(right_paned), hex_scroll);

    // drag-and-drop
    GtkDropTarget *drop = gtk_drop_target_new(G_TYPE_FILE, GDK_ACTION_COPY);
    g_signal_connect(drop, "drop", G_CALLBACK(on_drop), app);
    gtk_widget_add_controller(app->window, GTK_EVENT_CONTROLLER(drop));

    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char **argv) {
    AppData app = { .fd = -1 };

    GtkApplication *gtk_app = gtk_application_new(
        "ge.mef.reader", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(activate), &app);

    // argc=1 გადავცეთ GTK-ს, ფაილი ჩვენ გავხსნათ
    int status = g_application_run(G_APPLICATION(gtk_app), 1, argv);

    if(argc >= 2 && app.window)
        load_file(&app, argv[1]);

    if(app.fd >= 0) close(app.fd);
    if(app.sects)   free(app.sects);
    g_object_unref(gtk_app);
    return status;
}
