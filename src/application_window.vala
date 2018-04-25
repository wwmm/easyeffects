[GtkTemplate(ui = "/com/github/wwmm/pulseeffects/application.glade")]
public class ApplicationWindow : Gtk.ApplicationWindow {
    [GtkChild]
    Gtk.Label label;

    public ApplicationWindow (Gtk.Application app) {
        Object(application: app);

        this.label.set_text("wwmm");
    }

}
