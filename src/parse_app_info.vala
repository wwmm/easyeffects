using PulseAudio;

public struct AppInfo {
    uint32 index;
    string name;
    string icon_name;
    uint8 channels;
    float volume;
    uint32 rate;
    string resampler;
    string format;
    int mute;
    bool connected;
    uint32 buffer;
    uint32 latency;
    int corked;
}

public class ParseAppInfo : Object {
    private PulseManager pm;
    private string[] blacklist_apps;
    private string[] blacklist_media_name;
    private string[] blacklist_media_role;

    public ParseAppInfo (PulseManager pm) {
        this.pm = pm;

        // it does not make sense to show these apps

        this.blacklist_apps = { "PulseEffects", "pulseeffects",
                                "PulseEffectsWebrtcProbe", "gsd-media-keys",
                                "GNOME Shell", "libcanberra", "gnome-pomodoro",
                                "PulseAudio Volume Control", "Screenshot",
                                "speech-dispatcher" };

        this.blacklist_media_name = { "pulsesink probe", "bell-window-system",
                                      "audio-volume-change", "Peak detect",
                                      "screen-capture" };

        this.blacklist_media_role = { "event" };
    }

    public void new_sink_input(SinkInputInfo info) {
        var app_info = this.parse_sink_input(info);

        if(app_info != null){
            Idle.add(() => {
                this.pm.sink_input_added(app_info);
                return Source.REMOVE;
            });
        }
    }

    public void changed_sink_input(SinkInputInfo info) {
        var app_info = this.parse_sink_input(info);

        if(app_info != null){
            Idle.add(() => {
                this.pm.sink_input_changed(app_info);
                return Source.REMOVE;
            });
        }
    }

    public void new_source_output(SourceOutputInfo info) {
        var app_info = this.parse_source_output(info);

        if(app_info != null){
            Idle.add(() => {
                this.pm.source_output_added(app_info);
                return Source.REMOVE;
            });
        }
    }

    public void changed_source_output(SourceOutputInfo info) {
        var app_info = this.parse_source_output(info);

        if(app_info != null){
            Idle.add(() => {
                this.pm.source_output_changed(app_info);
                return Source.REMOVE;
            });
        }
    }

    private AppInfo ? parse_sink_input(SinkInputInfo info) {
        var ai = AppInfo();

        var proplist = info.proplist.copy();

        var app_name = proplist.gets("application.name");
        var media_name = proplist.gets("media.name");
        var media_role = proplist.gets("media.role");

        if(app_name == null){
            app_name = "";
        }

        if(media_name == null){
            media_name = "";
        }

        if(media_role == null){
            media_role = "";
        }

        var forbidden_app = app_name in this.blacklist_apps;
        var forbidden_media_name = media_name in this.blacklist_media_name;
        var forbidden_media_role = media_role in this.blacklist_media_role;

        if(forbidden_app || forbidden_media_name || forbidden_media_role){
            return null;
        } else {
            string icon_name = proplist.gets("application.icon_name");

            if(icon_name == null){
                icon_name = "audio-x-generic-symbolic";
            }

            ai.connected = false;

            if(info.sink == this.pm.apps_sink_info.index){
                ai.connected = true;
            }

            // linear volume
            ai.volume = 100 * info.volume.max() / PulseAudio.Volume.NORM;

            ai.format = info.sample_spec.format.to_string();

            if(ai.format == null){
                ai.format = "invalid";
            }

            ai.index = info.index;
            ai.name = app_name;
            ai.icon_name = icon_name;
            ai.channels = info.volume.channels;
            ai.rate = info.sample_spec.rate;
            ai.resampler = info.resample_method;
            ai.mute = info.mute;
            ai.buffer = info.buffer_usec;
            ai.latency = info.sink_usec;
            ai.corked = info.corked;

            return ai;
        }
    }

    private AppInfo ? parse_source_output(SourceOutputInfo info) {
        var ai = AppInfo();

        var proplist = info.proplist.copy();

        var app_name = proplist.gets("application.name");
        var media_name = proplist.gets("media.name");
        var media_role = proplist.gets("media.role");

        if(app_name == null){
            app_name = "";
        }

        if(media_name == null){
            media_name = "";
        }

        if(media_role == null){
            media_role = "";
        }

        var forbidden_app = app_name in this.blacklist_apps;
        var forbidden_media_name = media_name in this.blacklist_media_name;
        var forbidden_media_role = media_role in this.blacklist_media_role;

        if(forbidden_app || forbidden_media_name || forbidden_media_role){
            return null;
        } else {
            string icon_name = proplist.gets("application.icon_name");

            if(icon_name == null){
                icon_name = "audio-x-generic-symbolic";
            }

            ai.connected = false;

            if(info.source == this.pm.mic_sink_info.index){
                ai.connected = true;
            }

            // linear volume
            ai.volume = 100 * info.volume.max() / PulseAudio.Volume.NORM;

            ai.format = info.sample_spec.format.to_string();

            if(ai.format == null){
                ai.format = "invalid";
            }

            ai.index = info.index;
            ai.name = app_name;
            ai.icon_name = icon_name;
            ai.channels = info.volume.channels;
            ai.rate = info.sample_spec.rate;
            ai.resampler = info.resample_method;
            ai.mute = info.mute;
            ai.buffer = info.buffer_usec;
            ai.latency = info.source_usec;
            ai.corked = info.corked;
        }

        return ai;
    }

}
