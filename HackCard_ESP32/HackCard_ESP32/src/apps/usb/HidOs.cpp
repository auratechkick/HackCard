#include "HidOs.h"



#include <cstring>



const char* hidOsName(HidTargetOs os) {

  switch (os) {

    case HidTargetOs::MacOS:

      return "macOS";

    case HidTargetOs::Linux:

      return "Linux";

    default:

      return "Windows";

  }

}



const char* hidActionName(HidAction action) {

  switch (action) {

    case HidAction::TypeText:

      return "type_text";

    case HidAction::OpenWebsite:

      return "open_website";

    case HidAction::LockScreen:

      return "lock_screen";

    case HidAction::UnlockScreen:

      return "unlock_screen";

    case HidAction::Calculator:

      return "calculator";

    case HidAction::Screenshot:

      return "screenshot";

    case HidAction::Terminal:

      return "terminal";

    case HidAction::TerminalInfo:

      return "terminal_info";

    case HidAction::AppSwitcher:

      return "app_switcher";

    case HidAction::CloseWindow:

      return "close_window";

    case HidAction::VolumeMute:

      return "volume_mute";

    case HidAction::VolumeUp:

      return "volume_up";

    case HidAction::VolumeDown:

      return "volume_down";

    case HidAction::MediaPlayPause:

      return "media_play_pause";

    case HidAction::MediaNext:

      return "media_next";

    case HidAction::MediaPrevious:

      return "media_previous";

    case HidAction::MouseClick:

      return "mouse_click";

    case HidAction::MouseMove:

      return "mouse_move";

    case HidAction::KeyPress:

      return "key_press";

    default:

      return "notepad";

  }

}



HidTargetOs hidOsFromString(const char* name) {

  if (!name || name[0] == '\0') {

    return HidTargetOs::Windows;

  }

  if (strcmp(name, "mac") == 0 || strcmp(name, "macos") == 0 || strcmp(name, "darwin") == 0) {

    return HidTargetOs::MacOS;

  }

  if (strcmp(name, "linux") == 0) {

    return HidTargetOs::Linux;

  }

  return HidTargetOs::Windows;

}



HidAction hidActionFromString(const char* name) {

  if (!name || name[0] == '\0') {

    return HidAction::Notepad;

  }

  if (strcmp(name, "type_text") == 0 || strcmp(name, "type") == 0) {

    return HidAction::TypeText;

  }

  if (strcmp(name, "open_website") == 0 || strcmp(name, "website") == 0 || strcmp(name, "url") == 0) {

    return HidAction::OpenWebsite;

  }

  if (strcmp(name, "lock_screen") == 0 || strcmp(name, "lock") == 0) {

    return HidAction::LockScreen;

  }

  if (strcmp(name, "unlock_screen") == 0 || strcmp(name, "unlock") == 0) {

    return HidAction::UnlockScreen;

  }

  if (strcmp(name, "calculator") == 0 || strcmp(name, "calc") == 0) {

    return HidAction::Calculator;

  }

  if (strcmp(name, "screenshot") == 0) {

    return HidAction::Screenshot;

  }

  if (strcmp(name, "terminal") == 0) {

    return HidAction::Terminal;

  }

  if (strcmp(name, "terminal_info") == 0 || strcmp(name, "sysinfo") == 0) {

    return HidAction::TerminalInfo;

  }

  if (strcmp(name, "app_switcher") == 0 || strcmp(name, "alt_tab") == 0) {

    return HidAction::AppSwitcher;

  }

  if (strcmp(name, "close_window") == 0 || strcmp(name, "close_tab") == 0 ||
      strcmp(name, "alt_f4") == 0) {

    return HidAction::CloseWindow;

  }

  if (strcmp(name, "volume_mute") == 0 || strcmp(name, "mute") == 0) {

    return HidAction::VolumeMute;

  }

  if (strcmp(name, "volume_up") == 0 || strcmp(name, "vol_up") == 0) {

    return HidAction::VolumeUp;

  }

  if (strcmp(name, "volume_down") == 0 || strcmp(name, "vol_down") == 0) {

    return HidAction::VolumeDown;

  }

  if (strcmp(name, "media_play_pause") == 0 || strcmp(name, "play_pause") == 0) {

    return HidAction::MediaPlayPause;

  }

  if (strcmp(name, "media_next") == 0 || strcmp(name, "next") == 0) {

    return HidAction::MediaNext;

  }

  if (strcmp(name, "media_previous") == 0 || strcmp(name, "previous") == 0 || strcmp(name, "prev") == 0) {

    return HidAction::MediaPrevious;

  }

  if (strcmp(name, "mouse_click") == 0 || strcmp(name, "click") == 0) {

    return HidAction::MouseClick;

  }

  if (strcmp(name, "mouse_move") == 0 || strcmp(name, "move") == 0) {

    return HidAction::MouseMove;

  }

  if (strcmp(name, "key_press") == 0 || strcmp(name, "key") == 0) {

    return HidAction::KeyPress;

  }

  if (strcmp(name, "notepad") == 0) {

    return HidAction::Notepad;

  }

  return HidAction::Notepad;

}


