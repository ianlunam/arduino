#include <Arduino.h>
#include <time.h>

#include "Clock.h"

const char* tz = "NZST-12NZDT,M9.5.0,M4.1.0/3";

struct tm timeinfo;
time_t lastTime;


Clock::Clock() {
  struct tm t = { 0 };
  t.tm_year = 112;  // This is year-1900, so 112 = 2012
  t.tm_mon = 8;
  t.tm_mday = 15;
  t.tm_hour = 21;
  t.tm_min = 54;
  t.tm_sec = 13;
  lastTime = mktime(&t);
}


void Clock::init() {
  setTime();
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  lastTime = mktime(&timeinfo);
}


void Clock::update() {
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  time_t now = mktime(&timeinfo);
  if (now > (lastTime + (60 * 60 * 12))) {
    setTime();
    lastTime = now;
  }
}


void Clock::setTime() {
  configTime(0, 0, "nz.pool.ntp.org");
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  setenv("TZ", tz, 1);
  tzset();
  Serial.println("NTP updated");
}


String Clock::getTimeString() {
  if (!getLocalTime(&timeinfo)) {
    return "";
  }

  char ptr[20];
  int rc = strftime(ptr, 20, "%H:%M", &timeinfo);
  return ptr;
}

String Clock::getDateString() {
  if (!getLocalTime(&timeinfo)) {
    return "";
  }

  char ptr[20];
  int rc = strftime(ptr, 20, "%a %e %b", &timeinfo);
  return ptr;
}

String Clock::getYearString() {
  if (!getLocalTime(&timeinfo)) {
    return "";
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");

  char ptr[5];
  int rc = strftime(ptr, 5, "%Y", &timeinfo);
  return ptr;
}