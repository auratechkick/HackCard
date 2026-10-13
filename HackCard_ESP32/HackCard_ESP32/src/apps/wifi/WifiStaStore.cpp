#include "WifiStaStore.h"

#include "../../core/ConfigStore.h"

void WifiStaStoreSave(AppContext& ctx, const char* ssid, const char* password) {
  if (!ssid || ssid[0] == '\0') {
    return;
  }
  strlcpy(ctx.config.staSsid, ssid, sizeof(ctx.config.staSsid));
  if (password) {
    strlcpy(ctx.config.staPassword, password, sizeof(ctx.config.staPassword));
  }
  ctx.config.staCredentialsSaved = true;
  Config.save(ctx);
}
