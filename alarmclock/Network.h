#ifndef Network_h
#define Network_h
#include <Arduino.h>

class Network {
public:
  Network();
  void connect();
  String ipAddress();
};

#endif
