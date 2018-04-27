public class Application : Gtk.Application {
    private bool running_as_service;

    public Settings settings;
    public PulseManager pm;

    public Application () {
        Object(application_id: "com.github.wwmm.pulseeffects",
               flags : ApplicationFlags.HANDLES_COMMAND_LINE);

        Environment.set_application_name("PulseEffects");

        GLib.Environ.set_variable({ "PULSE_PROP_application.id" },
                                  "com.github.wwmm.pulseeffects", "true");
        GLib.Environ.set_variable({ "PULSE_PROP_application.icon_name" },
                                  "pulseeffects", "true");

        Unix.signal_add(2, () => {
            this.quit();
            return true;
        });

        string help_msg;

        help_msg = _("Quit PulseEffects. Useful when running in service mode.");

        this.add_main_option("quit", 'q', OptionFlags.NONE, OptionArg.NONE,
                             help_msg, null);

        help_msg = _("Show available presets.");

        this.add_main_option("presets", 'p', OptionFlags.NONE, OptionArg.NONE,
                             help_msg, null);

        help_msg = _("Load a preset. Example: pulseeffects -l music");

        this.add_main_option("load-preset", 'l', OptionFlags.NONE,
                             OptionArg.STRING, help_msg, null);
    }

    public override void startup() {
        base.startup();

        this.running_as_service = false;
        this.settings = new Settings("com.github.wwmm.pulseeffects");

        try {
            var presets_directory = Environment.get_user_config_dir() +
                                    "/PulseEffects";

            File file = File.new_for_path(presets_directory);

            file.make_directory();

            debug("user presets directory created: " + presets_directory);
        } catch (Error e){
            debug("user preset directory already exists");
        }

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

        var flags = GLib.SettingsBindFlags.DEFAULT;

        this.settings.bind("use-default-sink", this.pm, "use_default_sink",
                           flags);
        this.settings.bind("use-default-source", this.pm, "use_default_source",
                           flags);


        pm.find_sinks();
        pm.find_sources();
        pm.find_sink_inputs();
        pm.find_source_outputs();

        if((this.flags & ApplicationFlags.IS_SERVICE) != 0){
            this.running_as_service = true;

            pm.find_sink_inputs();
            pm.find_source_outputs();

            debug(_("Running in Background"));

            this.hold();
        }
    }

    public override void activate() {
        var win = this.active_window;

        if(win == null){
            win = new ApplicationWindow(this);
        }

        win.present();
    }

    public override int command_line(ApplicationCommandLine command_line) {
        var options = command_line.get_options_dict();

        if(options.contains("quit")){
            this.quit();
        } else if(options.contains("presets")){
        } else if(options.contains("load-presets")){
            // var value = options.lookup_value("load-preset",
            // new VariantType("s"));
        } else {
            this.activate();
        }

        return base.command_line(command_line);
    }

    public override void shutdown() {
        base.shutdown();

        pm.quit();
    }
}
