#include "IndoorData.h"

IndoorData& IndoorData::instance() {
  static IndoorData data;
  return data;
}
