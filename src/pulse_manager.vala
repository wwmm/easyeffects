using PulseAudio;

public struct myServerInfo {
    string server_name;
    string server_version;
    string default_sink_name;
    string default_source_name;
}

public struct mySinkInfo {
    string name;
    uint32 index;
    string description;
    uint32 owner_module;
    uint32 monitor_source;
    string monitor_source_name;
    uint32 rate;
    string format;
}

public struct mySourceInfo {
    string name;
    uint32 index;
    string description;
    uint32 rate;
    string format;
}

public class PulseManager : Object {
    private ThreadedMainLoop main_loop;
    private Context ctx;
    private bool ctx_ready;
    private ParseAppInfo pai;

    public bool use_default_sink = true;
    public bool use_default_source = true;

    public myServerInfo server_info;
    public mySinkInfo default_sink_info;
    public mySourceInfo default_source_info;
    public mySinkInfo apps_sink_info;
    public mySinkInfo mic_sink_info;

    public signal void sink_input_added(AppInfo info);
    public signal void sink_input_changed(AppInfo info);
    public signal void sink_input_removed(uint32 idx);
    public signal void source_output_added(AppInfo info);
    public signal void source_output_changed(AppInfo info);
    public signal void source_output_removed(uint32 idx);
    public signal void sink_added(mySinkInfo info);
    public signal void sink_changed(mySinkInfo info);
    public signal void sink_removed(uint32 idx);
    public signal void source_added(mySourceInfo info);
    public signal void source_changed(mySourceInfo info);
    public signal void source_removed(uint32 idx);
    public signal void new_default_sink(string name);
    public signal void new_default_source(string name);
    public signal void stream_level_changed(uint32 app_idx, float value);

    public PulseManager () {
        // Pulseaudio threaded mainloop and context initialization

        this.main_loop = new ThreadedMainLoop();

        this.ctx = new Context(this.main_loop.get_api(), null, null);

        this.ctx.set_state_callback(this.ctx_state_cb);

        var flags = Context.Flags.NOFAIL;

        this.ctx.connect(null, flags, null);

        this.ctx_ready = false;

        this.main_loop.start();

        while(!this.ctx_ready){
        }

        this.get_server_info();
        this.get_default_sink_info();
        this.get_default_source_info();
        this.load_apps_sink();
        this.load_mic_sink();

        this.pai = new ParseAppInfo(this);
    }

    private void ctx_state_cb(Context ctx) {
        var state = this.ctx.get_state();

        if(state == Context.State.UNCONNECTED){
            debug("pulseaudio context is unconnected");
        } else if(state == Context.State.CONNECTING){
            debug("pulseaudio context is connecting");
        } else if(state == Context.State.AUTHORIZING){
            debug("pulseaudio context is authorizing");
        } else if(state == Context.State.SETTING_NAME){
            debug("pulseaudio context is setting name");
        } else if(state == Context.State.READY){
            debug("pulseaudio context is ready");
            debug("connected to server: " + ctx.get_server());
            debug("server protocol version: " +
                  ctx.get_server_protocol_version().to_string());

            ctx.set_subscribe_callback((c, t, idx) => {
                var f = t & Context.SubscriptionEventType.FACILITY_MASK;

                if(f == Context.SubscriptionEventType.SINK_INPUT){
                    var e = t & Context.SubscriptionEventType.TYPE_MASK;

                    if(e == Context.SubscriptionEventType.NEW){
                        c.get_sink_input_info(idx, (cx, info, eol) => {
                            if(eol == 0 && info != null){
                                this.pai.new_sink_input(info);
                            }
                        });
                    } else if(e == Context.SubscriptionEventType.CHANGE){
                        c.get_sink_input_info(idx, (cx, info, eol) => {
                            if(eol == 0 && info != null){
                                this.pai.changed_sink_input(info);
                            }
                        });
                    } else if(e == Context.SubscriptionEventType.REMOVE){
                        Idle.add(() => {
                            this.sink_input_removed(idx);
                            return Source.REMOVE;
                        });
                    }
                } else if(f == Context.SubscriptionEventType.SOURCE_OUTPUT){
                    var e = t & Context.SubscriptionEventType.TYPE_MASK;

                    if(e == Context.SubscriptionEventType.NEW){
                        c.get_source_output_info(idx, (cx, info, eol) => {
                            if(eol == 0 && info != null){
                                this.pai.new_source_output(info);
                            }
                        });
                    } else if(e == Context.SubscriptionEventType.CHANGE){
                        c.get_source_output_info(idx, (cx, info, eol) => {
                            if(eol == 0 && info != null){
                                this.pai.changed_source_output(info);
                            }
                        });
                    } else if(e == Context.SubscriptionEventType.REMOVE){
                        Idle.add(() => {
                            this.source_output_removed(idx);
                            return Source.REMOVE;
                        });
                    }
                } else if(f == Context.SubscriptionEventType.SOURCE){
                    var e = t & Context.SubscriptionEventType.TYPE_MASK;

                    if(e == Context.SubscriptionEventType.NEW){
                        c.get_source_info_by_index(idx, (cx, info, eol) => {
                            if(eol == 0 && info != null){
                                var si = mySourceInfo();

                                si.name = info.name;
                                si.index = info.index;
                                si.description = info.description;
                                si.rate = info.sample_spec.rate;
                                si.format = info.sample_spec.format.to_string();

                                if(si.name != "PulseEffects_apps.monitor" &&
                                   si.name != "PulseEffects_mic.monitor"){
                                    Idle.add(() => {
                                        this.source_added(si);
                                        return Source.REMOVE;
                                    });
                                }
                            }
                        });
                    } else if(e == Context.SubscriptionEventType.CHANGE){

                    } else if(e == Context.SubscriptionEventType.REMOVE){
                        Idle.add(() => {
                            this.source_removed(idx);
                            return Source.REMOVE;
                        });
                    }
                } else if(f == Context.SubscriptionEventType.SINK){
                    var e = t & Context.SubscriptionEventType.TYPE_MASK;

                    if(e == Context.SubscriptionEventType.NEW){
                        c.get_sink_info_by_index(idx, (cx, info, eol) => {
                            if(eol == 0 && info != null){
                                var si = mySinkInfo();

                                si.name = info.name;
                                si.index = info.index;
                                si.description = info.description;
                                si.rate = info.sample_spec.rate;
                                si.format = info.sample_spec.format.to_string();

                                if(si.name != "PulseEffects_apps" &&
                                   si.name != "PulseEffects_mic"){
                                    Idle.add(() => {
                                        this.sink_added(si);
                                        return Source.REMOVE;
                                    });
                                }
                            }
                        });
                    } else if(e == Context.SubscriptionEventType.CHANGE){

                    } else if(e == Context.SubscriptionEventType.REMOVE){
                        Idle.add(() => {
                            this.sink_removed(idx);
                            return Source.REMOVE;
                        });
                    }
                } else if(f == Context.SubscriptionEventType.SERVER){
                    var e = t & Context.SubscriptionEventType.TYPE_MASK;

                    if(e == Context.SubscriptionEventType.CHANGE){
                        c.get_server_info((ctx, info) => {
                            if(info != null){
                                this.server_info.server_name =
                                    info.server_name;
                                this.server_info.server_version =
                                    info.server_version;

                                var sink_name = info.default_sink_name;
                                var source_name = info.default_source_name;

                                this.server_info.default_sink_name = sink_name;
                                this.server_info.default_source_name =
                                    source_name;

                                if(sink_name != "PulseEffects_apps" &&
                                   this.use_default_sink){
                                    Idle.add(() => {
                                        this.new_default_sink(sink_name);
                                        return Source.REMOVE;
                                    });
                                }

                                if(source_name != "PulseEffects_mic.monitor" &&
                                   this.use_default_source){
                                    Idle.add(() => {
                                        this.new_default_source(source_name);
                                        return Source.REMOVE;
                                    });
                                }
                            }
                        });
                    }
                }
            });

            // subscribing to pulseaudio events

            var mask = Context.SubscriptionMask.SINK_INPUT +
                       Context.SubscriptionMask.SOURCE_OUTPUT +
                       Context.SubscriptionMask.SOURCE +
                       Context.SubscriptionMask.SINK +
                       Context.SubscriptionMask.SERVER;

            ctx.subscribe(mask, (ctx, success) => {
                if(success == 0){
                    critical("Pulseaudio context event subscribe failed!");
                }
            });

            this.ctx_ready = true;
        } else if(state == Context.State.FAILED){
            critical("failed to start pulseaudio context");
        } else if(state == Context.State.TERMINATED){
            debug("pulseaudio context terminated");

            this.ctx_ready = false;
        }
    }

    private void get_server_info() {
        var o = this.ctx.get_server_info((ctx, info) => {
            if(info != null){
                this.server_info.server_name = info.server_name;
                this.server_info.server_version = info.server_version;
                this.server_info.default_sink_name = info.default_sink_name;
                this.server_info.default_source_name = info.default_source_name;

                debug("Pulseaudio version: " + info.server_version);
                debug("default pulseaudio source: " + info.default_source_name);
                debug("default pulseaudio sink: " + info.default_sink_name);

                this.main_loop.signal (false);
            }
        });

        this.wait_operation(o);
    }

    private mySinkInfo ? get_sink_info(string name) {
        bool failed = false;
        mySinkInfo si = mySinkInfo();

        var o = this.ctx.get_sink_info_by_name(name, (ctx, info, eol) => {
            if(eol == -1){
                failed = true;

                this.main_loop.signal (false);
            } else if(eol == 0 && info != null){
                si.name = info.name;
                si.index = info.index;
                si.description = info.description;
                si.owner_module = info.owner_module;
                si.monitor_source = info.monitor_source;
                si.monitor_source_name = info.monitor_source_name;
                si.rate = info.sample_spec.rate;
                si.format = info.sample_spec.format.to_string();
            } else if(eol == 1){
                this.main_loop.signal (false);
            }
        });

        this.wait_operation(o);

        if(failed){
            return null;
        } else {
            return si;
        }
    }

    private mySourceInfo ? get_source_info(string name) {
        bool failed = false;
        mySourceInfo si = mySourceInfo();

        var o = this.ctx.get_source_info_by_name(name, (ctx, info, eol) => {
            if(eol == -1){
                failed = true;

                this.main_loop.signal (false);
            } else if(eol == 0 && info != null){
                si.name = info.name;
                si.index = info.index;
                si.description = info.description;
                si.rate = info.sample_spec.rate;
                si.format = info.sample_spec.format.to_string();
            } else if(eol == 1){
                this.main_loop.signal (false);
            }
        });

        this.wait_operation(o);

        if(failed){
            return null;
        } else {
            return si;
        }
    }

    private void get_default_sink_info() {
        var name = this.server_info.default_sink_name;

        mySinkInfo ? info = get_sink_info(name);

        if(info != null){
            this.default_sink_info = info;

            debug("default pulseaudio sink sampling rate: " +
                  this.default_sink_info.rate.to_string());
            debug("default pulseaudio sink audio format: " +
                  this.default_sink_info.format.to_string());
        } else {
            critical("could not get default sink info");
        }
    }

    private void get_default_source_info() {
        var name = this.server_info.default_source_name;

        mySourceInfo ? info = get_source_info(name);

        if(info != null){
            this.default_source_info = info;

            debug("default pulseaudio source sampling rate: " +
                  this.default_source_info.rate.to_string());
            debug("default pulseaudio source audio format: " +
                  this.default_source_info.format.to_string());
        } else {
            critical("could not get default sink info");
        }
    }

    private mySinkInfo ? load_sink(string name, string description,
                                   uint32 rate) {
        mySinkInfo ? si = this.get_sink_info(name);

        if(si == null){ // sink is not loaded
            string argument = "sink_name=" + name + " " +
                              "sink_properties=" + description +
                              "device.class=\"sound\"" + " " +
                              "channels=2" + " " + "rate=" +
                              rate.to_string();

            var o = this.ctx.load_module("module-null-sink", argument,
                                         (ctx, idx) => {
                this.main_loop.signal (false);
            });

            this.wait_operation(o);

            // now that the sink is loaded we get its info
            si = this.get_sink_info(name);
        }

        return si;
    }

    private void load_apps_sink() {
        debug("loading Pulseeffects applications output sink...");

        string name = "PulseEffects_apps";
        string description = "device.description=\"PulseEffects(apps)\"";
        uint32 rate = this.default_sink_info.rate;

        this.apps_sink_info = this.load_sink(name, description, rate);
    }

    private void load_mic_sink() {
        debug("loading Pulseeffects microphone output sink...");

        string name = "PulseEffects_mic";
        string description = "device.description=\"PulseEffects(mic)\"";
        uint32 rate = this.default_source_info.rate;

        this.mic_sink_info = this.load_sink(name, description, rate);
    }

    public void find_sink_inputs() {
        var o = this.ctx.get_sink_input_info_list((ctx, info, eol) => {
            if(eol == -1){
                this.main_loop.signal (false);
            } else if(eol == 0 && info != null){
                this.pai.new_sink_input(info);
            } else if(eol == 1){
                this.main_loop.signal (false);
            }
        });

        this.wait_operation(o);
    }

    public void find_source_outputs() {
        var o = this.ctx.get_source_output_info_list((ctx, info, eol) => {
            if(eol == -1){
                this.main_loop.signal (false);
            } else if(eol == 0 && info != null){
                this.pai.new_source_output(info);
            } else if(eol == 1){
                this.main_loop.signal (false);
            }
        });

        this.wait_operation(o);
    }

    public void find_sinks() {
        var o = this.ctx.get_sink_info_list((ctx, info, eol) => {
            if(eol == -1){
                this.main_loop.signal (false);
            } else if(eol == 0 && info != null){
                var si = mySinkInfo();

                si.name = info.name;
                si.index = info.index;
                si.description = info.description;
                si.rate = info.sample_spec.rate;
                si.format = info.sample_spec.format.to_string();

                if(si.name != "PulseEffects_apps" &&
                   si.name != "PulseEffects_mic"){
                    Idle.add(() => {
                        this.sink_added(si);
                        return Source.REMOVE;
                    });
                }
            } else if(eol == 1){
                this.main_loop.signal (false);
            }
        });

        this.wait_operation(o);
    }

    public void find_sources() {
        var o = this.ctx.get_source_info_list((ctx, info, eol) => {
            if(eol == -1){
                this.main_loop.signal (false);
            } else if(eol == 0 && info != null){
                var si = mySourceInfo();

                si.name = info.name;
                si.index = info.index;
                si.description = info.description;
                si.rate = info.sample_spec.rate;
                si.format = info.sample_spec.format.to_string();

                if(si.name != "PulseEffects_apps.monitor" &&
                   si.name != "PulseEffects_mic.monitor"){
                    Idle.add(() => {
                        this.source_added(si);
                        return Source.REMOVE;
                    });
                }
            } else if(eol == 1){
                this.main_loop.signal (false);
            }
        });

        this.wait_operation(o);
    }

    public void move_sink_input_to_pulseeffects(uint32 idx) {
        this.ctx.move_sink_input_by_index(idx, this.apps_sink_info.index,
                                          (ctx, success) => {
            if(success == 1){
                debug("sink input " + idx.to_string() + " moved to PE");
            } else {
                debug("failed to move sink input " + idx.to_string() +
                      " to PE");
            }
        });
    }

    public void remove_sink_input_from_pulseeffects(uint32 idx) {
        this.ctx.move_sink_input_by_name(idx,
                                         this.server_info.default_sink_name,
                                         (ctx, success) => {
            if(success == 1){
                debug("sink input " + idx.to_string() + " removed from PE");
            } else {
                debug("failed to remove sink input " + idx.to_string() +
                      " from PE");
            }
        });
    }

    public void move_source_output_to_pulseeffects(uint32 idx) {
        this.ctx.move_source_output_by_name(idx,
                                            this.server_info
                                             .default_source_name,
                                            (ctx, success) => {
            if(success == 1){
                debug("source output " + idx.to_string() + " removed from PE");
            } else {
                debug("failed to remove source output " + idx.to_string() +
                      " from PE");
            }
        });
    }

    public void remove_source_output_from_pulseeffects(uint32 idx) {
        this.ctx.move_source_output_by_index(idx,
                                             this.mic_sink_info.monitor_source,
                                             (ctx, success) => {
            if(success == 1){
                debug("source output " + idx.to_string() + " moved to PE");
            } else {
                debug("failed to move source output " + idx.to_string() +
                      " to PE");
            }
        });
    }

    public void set_sink_input_volume(uint32 idx, uint8 channels,
                                      uint32 value) {
        var cvolume = CVolume();

        // cvolume.channels = channels;

        var raw_value = PulseAudio.Volume.NORM * value / 100;

        var cv_ptr = cvolume.set(channels, raw_value);

        if(cv_ptr != null){
            this.ctx.set_sink_input_volume(idx, cv_ptr, (ctx, success) => {
                if(success == 1){
                    debug("changed volume of sink input " + idx.to_string());
                } else {
                    debug("failed to change volume of sink input " +
                          idx.to_string());
                }
            });
        }
    }

    public void set_sink_input_mute(uint32 idx, bool state) {
        this.ctx.set_sink_input_mute(idx, state, (ctx, success) => {
            if(success == 1){
                debug("sink input " + idx.to_string() + " is muted");
            } else {
                debug("failed to mute sink input " + idx.to_string());
            }
        });
    }

    public void set_source_output_volume(uint32 idx, uint8 channels,
                                         uint32 value) {
        var cvolume = CVolume();

        var raw_value = PulseAudio.Volume.NORM * value / 100;

        var cv_ptr = cvolume.set(channels, raw_value);

        if(cv_ptr != null){
            this.ctx.set_source_output_volume(idx, cv_ptr, (ctx, success) => {
                if(success == 1){
                    debug("changed volume of source output " + idx.to_string());
                } else {
                    debug("failed to change volume of source output " +
                          idx.to_string());
                }
            });
        }
    }

    public void set_source_output_mute(uint32 idx, bool state) {
        this.ctx.set_source_output_mute(idx, state, (ctx, success) => {
            if(success == 1){
                debug("source output " + idx.to_string() + " is muted");
            } else {
                debug("failed to mute source output " + idx.to_string());
            }
        });
    }

    public void get_sink_input_info(uint32 idx) {
        this.ctx.get_sink_input_info(idx, (ctx, info, eol) => {
            if(eol == 0 && info != null){
                this.pai.changed_sink_input(info);
            }
        });
    }

    public Stream create_stream(string source_name, uint32 app_idx,
                                string app_name, uint32 monitor_idx) {

        var stream_name = app_name + " - Level Meter Stream";

        var ss = SampleSpec();

        ss.channels = 1;
        ss.format = SampleFormat.FLOAT32LE;
        ss.rate = 10;

        var stream = new Stream(this.ctx, stream_name, ss);

        if(monitor_idx != -1){
            stream.set_monitor_stream(monitor_idx);
        }

        stream.set_state_callback((s) => {
            var state = s.get_state();

            if(state == Stream.State.UNCONNECTED){
                debug(app_name + " volume meter stream is unconnected");
            } else if(state == Stream.State.CREATING){
                debug(app_name + " volume meter stream is being created");
            } else if(state == Stream.State.READY){
                debug(app_name + " volume meter stream is ready");
            } else if(state == Stream.State.FAILED){
                debug(app_name + " volume meter stream has failed. Did you " +
                      " disable this app ?");
            } else if(state == Stream.State.TERMINATED){
                debug(app_name + " volume meter stream was terminated");
            }
        });

        stream.set_read_callback((s, nbytes) => {
            void * data;

            if(s.peek(out data, out nbytes) < 0){
                warning("Failed to read data from " + app_name +
                        " volume meter stream");
                return;
            }

            // according to pulseaudio docs:
            // NULL data means either a hole or empty buffer.
            // Only drop the stream when there is a hole (nbytes > 0)
            if(data == null && nbytes > 0){
                s.drop();
                return;
            }

            float v = ((float[]) data)[nbytes / sizeof (float) - 1];

            s.drop();

            if(v < 0){
                v = 0;
            }

            if(v > 1){
                v = 1;
            }

            Idle.add(() => {
                this.stream_level_changed(app_idx, v);
                return Source.REMOVE;
            });
        });

        var flags = Stream.Flags.PEAK_DETECT | Stream.Flags.DONT_MOVE;

        stream.connect_record(source_name, null, flags);

        return stream;
    }

    private void unload_module(uint32 idx) {
        var o = this.ctx.unload_module(idx, (ctx, success) => {
            if(success == 1){
                debug("module " + idx.to_string() + " unloaded");
            } else {
                debug("failed to unload module " + idx.to_string());
            }

            this.main_loop.signal (false);
        });

        this.wait_operation(o);
    }

    private void unload_sinks() {
        debug("unloading PulseEffects sinks...");

        this.unload_module(this.apps_sink_info.owner_module);
        this.unload_module(this.mic_sink_info.owner_module);
    }

    private void wait_operation(Operation o) {
        this.main_loop.lock();

        while(o.get_state() == Operation.State.RUNNING){
            this.main_loop.wait();
        }

        this.main_loop.unlock();
    }

    private void drain_ctx() {
        var o = this.ctx.drain((ctx) => {
            var state = ctx.get_state();

            if(state == Context.State.READY){
                this.main_loop.signal (false);
            }
        });

        if(o != null){
            this.main_loop.lock();

            while(o.get_state() == Operation.State.RUNNING){
                this.main_loop.wait();
            }

            debug("Context was drained");

            this.main_loop.unlock();
        }
        {
            debug("Context does not need draining");
        }
    }

    public void quit() {
        this.unload_sinks();

        this.drain_ctx();

        this.ctx.disconnect();

        while(this.ctx_ready){
        }

        main_loop.stop();
    }

}
