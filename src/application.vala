public class Application : Gtk.Application {
    private bool ui_initialized;
    private bool window_activated;
    private bool running_as_service;
    // private List<mySinkInfo ? > sink_list;
    // private List<mySourceInfo ? > source_list;

    private PulseManager pm;

    public Settings settings;

    public Application () {
        Object(application_id: "com.github.wwmm.pulseeffects",
               flags : ApplicationFlags.FLAGS_NONE);

        Environment.set_application_name("PulseEffects");

        Unix.signal_add(2, () => {
            this.quit();
            return true;
        });
    }

    public override void startup() {
        base.startup();

        this.ui_initialized = false;
        this.window_activated = false;
        this.running_as_service = false;
        this.settings = new Settings("com.github.wwmm.pulseeffects");

        // creating user presets folder
        try {
            var presets_directory = Environment.get_user_config_dir() +
                                    "/PulseEffects";

            File file = File.new_for_path(presets_directory);

            file.make_directory();

            debug("user presets directory created: " + presets_directory);
        } catch (Error e){
            debug("user preset directory already exists");
        }

        this.create_appmenu();

        pm = new PulseManager();

        pm.sink_input_added.connect((i) => { debug("si added: " + i.name); });
        pm.sink_input_changed.connect((i) => {
            debug("si changed: " + i.name);
        });
        pm.sink_input_removed.connect((i) => {
            debug("si removed: " + i.to_string());
        });

        pm.source_output_added.connect((i) => {
            debug("so added: " + i.name);
        });
        pm.source_output_changed.connect((i) => {
            debug("so changed: " + i.name);
        });
        pm.source_output_removed.connect((i) => {
            debug("so removed: " + i.to_string());
        });
        pm.new_default_sink.connect((n) => {
            debug("new default sink: " + n);
        });
        pm.new_default_source.connect((n) => {
            debug("new default source: " + n);
        });
        pm.source_added.connect((i) => { debug("source added: " + i.name); });
        pm.sink_added.connect((i) => { debug("sink added: " + i.name); });

        var flags = GLib.SettingsBindFlags.DEFAULT;

        this.settings.bind("use-default-sink", this.pm, "use_default_sink",
                           flags);
        this.settings.bind("use-default-source", this.pm, "use_default_source",
                           flags);

        pm.find_sink_inputs();
        pm.find_source_outputs();
        pm.find_sinks();
        pm.find_sources();
    }

    public override void activate() {
        var win = this.active_window;

        if(win == null){
            win = new ApplicationWindow(this);
        }

        win.present();
    }

    public override void shutdown() {
        base.shutdown();

        pm.quit();
    }

    private void create_appmenu() {
        var menu = new Menu();

        menu.append("About", "app.about");
        menu.append("Quit", "app.quit");

        this.set_app_menu(menu);

        var about_action = new SimpleAction("about", null);

        about_action.activate.connect(() => {
            this.hold();

            var builder = new Gtk.Builder.from_resource(
                "/com/github/wwmm/pulseeffects/about.glade");

            var dialog = builder.get_object("about_dialog") as Gtk.Dialog;

            dialog.set_transient_for(this.active_window);

            dialog.run();

            dialog.destroy();

            this.release();
        });

        this.add_action(about_action);

        var quit_action = new SimpleAction("quit", null);

        quit_action.activate.connect(() => {
            this.hold();
            this.quit();
            this.release();
        });

        this.add_action(quit_action);
    }

}
