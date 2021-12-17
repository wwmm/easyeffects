#!/bin/bash

# this checks the PipeWire version on the host, and then compares it to what we need.
# current baseline to compile EasyEffects is 0.3.31, thus the host needs minimum that as well.

# get host's PipeWire version:

# https://stackoverflow.com/a/48066872
# we can't use jq to parse JSON as that will fail to run with flatpak run or from desktop file; gives "command not found", even though jq is in the runtime/sdk.
found_pipewire_version="$(pw-dump | grep -Po '"version":.*?[^\\]",' |  sed 's/"//g' | sed 's/://g' | sed 's/,//g' | cut -c9-18)"

required_pipewire_version="0.3.31" # from EasyEffects' build requirements

# compare versions
# from https://unix.stackexchange.com/a/285928

if [ "$(printf '%s\n' "$required_pipewire_version" "$found_pipewire_version" | sort -V | head -n1)" = "$required_pipewire_version" ]; then 
      echo "You have PipeWire ${found_pipewire_version} installed"
      echo "This is newer or the same as PipeWire ${required_pipewire_version} required to run EasyEffects"

else

  if pw-dump >/dev/null ; then # TRY
      # https://stackoverflow.com/a/6852427
      # https://stackoverflow.com/a/428118
      echo "pw-dump succeeded; system likely has PipeWire 0.3.x or later."
      echo "You have PipeWire ${found_pipewire_version} installed"
      echo "This is less than PipeWire ${required_pipewire_version} recommended to run EasyEffects"

      # put all the text here so we can use variable as part of output, it is helpful to the user.
      zenity --info --title="PipeWire version warning" --no-wrap \
        --text="Could not find compatible PipeWire.
It is recommended to install PipeWire ${required_pipewire_version} or newer for EasyEffects to work correctly. \n
More info:
PipeWire ${found_pipewire_version} was found, which is not considered recent enough for EasyEffects. 
If on Ubuntu or an Ubuntu-based system you may install newer PipeWire from: https://pipewire-debian.github.io
You may try PulseEffects which does not require PipeWire.
You may dismiss this warning by clicking \"OK\", please note PipeWire ${found_pipewire_version} is not tested to work with EasyEffects. \n
If you believe this message was shown in error, please report it to: https://github.com/wwmm/easyeffects"
      
  else # CATCH
      echo 'pw-dump failed; system likely has PipeWire 0.2.x or older, or no PipeWire package exists.'
      echo "Your system does not appear to have PipeWire 0.3.0 or later installed"
      echo "This is less than PipeWire ${required_pipewire_version} required to run EasyEffects"

      # put all the text here so we can use variable as part of output, it is helpful to the user.
      zenity --info --title="PipeWire version warning" --no-wrap \
        --text="Could not find compatible PipeWire.
Install PipeWire ${required_pipewire_version} or newer for EasyEffects to work correctly. \n
More info:
Either PipeWire is not installed or is not recent enough for EasyEffects. 
If on Ubuntu or an Ubuntu-based system you may install newer PipeWire from: https://pipewire-debian.github.io
You may try PulseEffects which does not require PipeWire. \n
If you believe this message was shown in error, please report it to: https://github.com/wwmm/easyeffects"
      
  fi

fi

# launch the normal app without checking again
exec easyeffects "$@"