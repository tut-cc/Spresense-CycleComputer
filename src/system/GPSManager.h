#pragma once

#include "../hal/interfaces/IGnssProvider.h"
#include <Arduino.h>
#include <GNSS.h>

namespace application {

class GPSManager : public hal::IGnssProvider {
public:
  bool  begin() override;
  void  update() override;
  float getSpeedKmh() const override;
  void  getTimeJST(char *buffer, size_t size) const override;

private:
  SpGnss    gnss;
  SpNavData navData;
};

} // namespace application
