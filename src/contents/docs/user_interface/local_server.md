# Local Server

Easy Effects runs a local socket server that allows external scripts or applications to control it without a GUI. This can be useful for automating tasks, or integrating with other system tools.

The server listens on a socket named `EasyEffectsServer`.

## Sending commands

You can send commands to the server using `socat` or to the socket directly with the programming language of your choice.

### Example

```bash
echo "load_preset:output:MyPresetName" | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/EasyEffectsServer
```

*Note: In most cases `$XDG_RUNTIME_DIR=/run/user/1000` where `1000` is an id number that will change from one user to another.*

## General commands

| Command | Description | Arguments |
| :--- | :--- | :--- |
| `show_window` | Opens the main window. | None |
| `hide_window` | Hides the main window. | None |
| `quit_app` | Quits Easy Effects. | None |
| `global_bypass` | Toggles effects on/off. | `1` (bypass) or `0` (active) |
| `get_global_bypass` | Returns `1` (enabled) or `2` (disabled). | None |
| `toggle_global_bypass` | Toggles global bypass. | None |
| `microphone_monitoring` | Sets microphone monitoring. | `1` (enabled) or `0` (disabled) |
| `get_microphone_monitoring` | Returns `1` (enabled) or `2` (disabled). | None |
| `toggle_microphone_monitoring` | Toggles microphone monitoring. | None |
| `audio_sharing` | Sets desktop audio sharing through the virtual source. | `1` (enabled) or `0` (disabled) |
| `get_audio_sharing` | Returns `1` (enabled) or `2` (disabled). | None |
| `toggle_audio_sharing` | Toggles audio sharing. | None |
| `load_preset` | Loads a preset. | `pipeline`:`preset_name` |
| `get_last_loaded_preset` | Returns the name of the last loaded preset. | `pipeline` |

---

## Command-line controls

With Easy Effects running, the following options update its live state through
this local server, just like `--bypass` and `--bypass-toggle`:

```bash
easyeffects --microphone-monitoring 1      # Enable microphone monitoring
easyeffects --microphone-monitoring 2      # Disable microphone monitoring
easyeffects --microphone-monitoring 3      # Print 1 (enabled) or 2 (disabled)
easyeffects --microphone-monitoring-toggle

easyeffects --audio-sharing 1              # Enable audio sharing
easyeffects --audio-sharing 2              # Disable audio sharing
easyeffects --audio-sharing 3              # Print 1 (enabled) or 2 (disabled)
easyeffects --audio-sharing-toggle
```

The toggle options can also be bound to custom keyboard shortcuts on desktops
without XDG global shortcut support. Queries print the numeric state followed by
a newline. Unlike CLI state arguments, socket setters use `0` to disable;
for example, send `audio_sharing:0\n`. Socket queries return the numeric state
without a newline. Send each socket command terminated by a newline.

## Plugins

You can modify individual plugin parameters on the fly using the `set_property` and `get_property` commands.


**Arguments:**
* **pipeline**: `output` or `input`.
* **plugin_id**: The unique identifier of the plugin (e.g., `compressor`, `equalizer`).
* **instance_id**: The instance number, starting from `0`.
* **property_name**: The specific setting to change (e.g., `threshold`, `inputGain`).


 See [Plugin Properties](../database/plugins_properties.md) for a full list of properties.

### Set property

**Format:**
`set_property:pipeline:plugin_id:instance_id:property_name:value`

For equalizer channel properties:
`set_property:pipeline:equalizer:instance_id:(left|right):property_name:value`

**Example 1:**
Set the compressor threshold on the output pipeline to -20dB:
```bash
echo "set_property:output:compressor:0:threshold:-20" | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/EasyEffectsServer
```

**Example 2:**
Set the compressor `mode` to `Upward`. If [Plugin Properties](../database/plugins_properties.md) show `1: Upward` in the choices list:
```bash
echo "set_property:output:compressor:0:mode:1" | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/EasyEffectsServer
```

**Example 3:**
Set equalizer left channel band gain:
```bash
echo "set_property:output:equalizer:0:left:band0Gain:1.5" | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/EasyEffectsServer
```

### Get property

**Format:**
`get_property:pipeline:plugin_id:instance_id:property_name`

For equalizer channel properties:
`get_property:pipeline:equalizer:instance_id:(left|right):property_name`

**Example:**
Get the current output gain of the equalizer:
```bash
echo "get_property:output:equalizer:0:outputGain" | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/EasyEffectsServer
```

Get a left channel equalizer band gain:
```bash
echo "get_property:output:equalizer:0:left:band0Gain" | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/EasyEffectsServer
```
