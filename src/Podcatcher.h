#pragma once

#include <Arduino.h>
#include <TinyXML.h>

#define MAX_PODCAST_URL_LENGTH 150
#define MAX_PODCAST_GUID_LENGTH 150

typedef std::function<void(const char *url, const char *guid)> EpisodeCallback;

class Podcatcher {
  public:
    void begin(EpisodeCallback &cb);
    void processChar(const char ch);
    void reset();
  private:
    void _xmlCallback(const uint8_t statusflags, char* tagName, const uint16_t tagNameLen, const char* data, const uint16_t dataLen);
    TinyXML _xml;
    EpisodeCallback _cb;
    char _buffer[200];
    char _url[MAX_PODCAST_URL_LENGTH] = "";
    char _guid[MAX_PODCAST_GUID_LENGTH] = "";
    bool _withinItemTag = false;
    bool _withinEnclosureTag = false;
    bool _isMp3 = false;
    bool _isDone = false;
};