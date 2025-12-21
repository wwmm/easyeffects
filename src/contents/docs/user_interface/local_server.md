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
| `load_preset` | Loads a preset. | `pipeline`:`preset_name` |
| `get_last_loaded_preset` | Returns the name of the last loaded preset. | `pipeline` |

---

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

### Get property

**Format:**
`get_property:pipeline:plugin_id:instance_id:property_name`

**Example:**
Get the current output gain of the equalizer:
```bash
echo "get_property:output:equalizer:0:outputGain" | socat - UNIX-CONNECT:$XDG_RUNTIME_DIR/EasyEffectsServer
```
