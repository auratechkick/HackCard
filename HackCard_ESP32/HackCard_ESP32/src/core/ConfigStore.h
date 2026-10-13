#pragma once

#include "AppContext.h"

class ConfigStore {
public:
  bool begin(AppContext& ctx);
  bool load(AppContext& ctx);
  bool save(const AppContext& ctx);

  const char* getSourceLabel() const { return _sourceLabel; }

private:
  bool loadFromCompiledDefaults(AppContext& ctx);
  bool loadFromJson(AppContext& ctx, const String& json);
  String serialize(const AppContext& ctx) const;
  bool loadJsonFromNvs(String& out) const;
  bool saveJsonToNvs(const String& json) const;

  char _sourceLabel[32] = "compiled";
};

extern ConfigStore Config;
