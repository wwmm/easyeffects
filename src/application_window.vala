[GtkTemplate(ui = "/com/github/wwmm/pulseeffects/application.glade")]
public class ApplicationWindow : Gtk.ApplicationWindow {
    private Application app;
    private List<mySinkInfo ? > sink_list;
    // private List<mySourceInfo ? > source_list;

    [GtkChild]
    Gtk.Switch enable_all_apps;

    [GtkChild]
    Gtk.Switch theme_switch;

    [GtkChild]
    Gtk.Switch show_spectrum;

    [GtkChild]
    Gtk.ToggleButton use_default_sink;

    [GtkChild]
    Gtk.ToggleButton use_default_source;

    [GtkChild]
    Gtk.Adjustment buffer_in;

    [GtkChild]
    Gtk.Adjustment buffer_out;

    [GtkChild]
    Gtk.Adjustment latency_in;

    [GtkChild]
    Gtk.Adjustment latency_out;

    [GtkChild]
    Gtk.Adjustment spectrum_n_points;

    [GtkChild]
    Gtk.ComboBox input_device;

    [GtkChild]
    Gtk.ComboBox output_device;

    [GtkCallback]
    private bool on_enable_autostart_state_set(Gtk.Switch s, bool state) {
        return true;
    }

    [GtkCallback]
    private void on_reset_all_settings_clicked(Gtk.Button b) {
        this.app.settings.reset("buffer-in");
        this.app.settings.reset("buffer-out");
        this.app.settings.reset("latency-in");
        this.app.settings.reset("latency-out");
        this.app.settings.reset("show-spectrum");
        this.app.settings.reset("spectrum-n-points");
        this.app.settings.reset("use-dark-theme");
        this.app.settings.reset("enable-all-apps");
        this.app.settings.reset("use-default-sink");
        this.app.settings.reset("use-default-source");
    }

    [GtkCallback]
    private void on_spectrum_n_points_value_changed(Gtk.Adjustment a) {
    }

    [GtkCallback]
    private bool on_show_spectrum_state_set(Gtk.Switch s, bool state) {
        return true;
    }

    [GtkCallback]
    private void on_input_device_changed(Gtk.ComboBox c) {

    }

    [GtkCallback]
    private void on_output_device_changed(Gtk.ComboBox c) {

    }

    [GtkCallback]
    private void on_use_default_source_toggled(Gtk.ToggleButton t) {

    }

    [GtkCallback]
    private void on_use_default_sink_toggled(Gtk.ToggleButton t) {

    }

    [GtkCallback]
    private void on_test_clicked(Gtk.Button b) {

    }

    private void on_sink_added(mySinkInfo i) {
        var add_to_list = true;

        foreach(var s in this.sink_list){
            if(s.index == i.index){
                add_to_list = false;

                break;
            }
        }

        if(add_to_list){
            this.sink_list.append(i);

            debug("added sink: " + i.name);
        }
    }

    private void on_sink_removed(uint32 idx) {
        foreach(var s in this.sink_list){
            if(s.index == idx){
                debug("removed sink: " + s.name);

                this.sink_list.remove(s);

                break;
            }
        }
    }

    public ApplicationWindow (Application app) {
        Object(application: app);

        this.app = app;

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

        app.settings.bind("use-default-source", this.input_device, "sensitive",
                          flag | flag_invert_boolean);

        app.settings.bind("use-default-sink", this.output_device, "sensitive",
                          flag | flag_invert_boolean);

        app.settings.bind("buffer-out", this.buffer_out, "value", flag);
        app.settings.bind("latency-out", this.latency_out, "value", flag);
        app.settings.bind("buffer-in", this.buffer_in, "value", flag);
        app.settings.bind("latency-in", this.latency_in, "value", flag);

        app.settings.bind("show-spectrum", this.show_spectrum, "active", flag);
        app.settings.bind("spectrum-n-points", this.spectrum_n_points, "value",
                          flag);

        this.app.pm.sink_added.connect(this.on_sink_added);
        this.app.pm.sink_removed.connect(this.on_sink_removed);
    }

}
