using Gtk;

[GtkTemplate(ui = "/com/github/wwmm/pulseeffects/application.glade")]
public class ApplicationWindow : Gtk.ApplicationWindow {
    [GtkChild]
    Switch enable_all_apps;

    [GtkChild]
    Switch theme_switch;

    [GtkChild]
    Switch show_spectrum;

    [GtkChild]
    ToggleButton use_default_sink;

    [GtkChild]
    ToggleButton use_default_source;

    [GtkChild]
    Adjustment buffer_in;

    [GtkChild]
    Adjustment buffer_out;

    [GtkChild]
    Adjustment latency_in;

    [GtkChild]
    Adjustment latency_out;

    [GtkChild]
    Adjustment spectrum_n_points;

    [GtkChild]
    ComboBox input_device;

    [GtkChild]
    ComboBox output_device;

    [GtkCallback]
    private bool on_enable_autostart_state_set(Switch s, bool state) {
        return true;
    }

    [GtkCallback]
    private void on_reset_all_settings_clicked(Button b) {

    }

    [GtkCallback]
    private void on_spectrum_n_points_value_changed(Adjustment a) {
    }

    [GtkCallback]
    private bool on_show_spectrum_state_set(Switch s, bool state) {
        return true;
    }

    [GtkCallback]
    private void on_input_device_changed(ComboBox c) {

    }

    [GtkCallback]
    private void on_output_device_changed(ComboBox c) {

    }

    [GtkCallback]
    private void on_use_default_source_toggled(ToggleButton t) {

    }

    [GtkCallback]
    private void on_use_default_sink_toggled(ToggleButton t) {

    }

    [GtkCallback]
    private void on_test_clicked(Button b) {

    }

    public ApplicationWindow (Application app) {
        Object(application: app);

        var gtk_settings = Gtk.Settings.get_default();

        var flag = GLib.SettingsBindFlags.DEFAULT;
        var flag_invert_boolean = GLib.SettingsBindFlags.INVERT_BOOLEAN;

        app.settings.bind("use-dark-theme", this.theme_switch, "active", flag);

        app.settings.bind("use-dark-theme", gtk_settings,
                          "gtk_application_prefer_dark_theme", flag);

        app.settings.bind("enable-all-apps", this.enable_all_apps, "active",
                          flag);

        app.settings.bind("use-default-sink", this.use_default_sink, "active",
                          flag);

        app.settings.bind("use-default-sink", this.output_device, "sensitive",
                          flag | flag_invert_boolean);

        app.settings.bind("use-default-source", this.use_default_source,
                          "active", flag);

        app.settings.bind("use-default-source", this.input_device, "sensitive",
                          flag | flag_invert_boolean);

        app.settings.bind("buffer-out", this.buffer_out, "value", flag);
        app.settings.bind("latency-out", this.latency_out, "value", flag);
        app.settings.bind("buffer-in", this.buffer_in, "value", flag);
        app.settings.bind("latency-in", this.latency_in, "value", flag);

        app.settings.bind("show-spectrum", this.show_spectrum, "active", flag);
    }

}
