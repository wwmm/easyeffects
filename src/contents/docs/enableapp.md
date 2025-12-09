# Enable or Disable an Application

The user can choose which applications have effects applied through the Enable checkbutton. "Process All Inputs/Outputs" option in [General Settings](general.md) can be activated to always apply effects to whichever application.

Note that without the "Process All Inputs/Outputs" option the enabled state may not replicated for an application at the next session. This is because "Process All Inputs/Outputs" makes Easy Effects trying to move the stream to its virtual device every time a new application spawns (unless it is listed in the [Excluded Apps](blocklist.md)). When Easy Effects is not running, the app stream may be redirected to another destination by Pipewire, any third party audio manager or the app itself. This leads Easy Effects to not receive the stream at the next startup if "Process All Inputs/Outputs" option is not active.
