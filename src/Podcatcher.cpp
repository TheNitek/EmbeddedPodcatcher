#include <Podcatcher.h>

void Podcatcher::begin(EpisodeCallback &cb) {
  _cb = cb;
  XMLcallback xmlCb = std::bind(&Podcatcher::_xmlCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
  _xml.init((uint8_t *)_buffer, sizeof(_buffer), xmlCb);
}

void Podcatcher::_xmlCallback(const uint8_t statusflags, char* tagName,
 const uint16_t tagNameLen, const char* data, const uint16_t dataLen) {
  if(statusflags & STATUS_ERROR) {
    tagName[tagNameLen] = '\0';
    /*Serial.print(tagName);
    Serial.print(" ");
    Serial.println(data);*/
    _isDone = true;
    return;
  }

  if((statusflags & STATUS_END_TAG) && (strcasecmp(tagName, "/rss") == 0)) {
    _isDone = true;
    return;
  }

  if((statusflags & STATUS_END_TAG) && strcasecmp(tagName, "/rss/channel/item") == 0) {
    _withinItemTag = false;
    return;
  }

  if(statusflags & STATUS_START_TAG) {
    if (strcasecmp(tagName, "/rss/channel/item") == 0) {
      _withinItemTag = true;
      _withinEnclosureTag = false;
      _isMp3 = false;
      _url[0] = '\0';
      _guid[0] = '\0';
    }
    _withinEnclosureTag = (strcasecmp(tagName, "/rss/channel/item/enclosure") == 0);
    //Serial.println(tagName);
    return;
  }

  if(!_withinItemTag || dataLen == 0) {
    return;
  }

  if(_withinEnclosureTag && (statusflags & STATUS_ATTR_TEXT)) {
    if(strcasecmp(tagName, "url") == 0) {
      strncpy(_url, data, MAX_PODCAST_URL_LENGTH);
    } else if(strcasecmp(tagName, "type") == 0 && strcasecmp(data, "audio/mpeg") == 0) {
      _isMp3 = true;
    }
  }

  if((statusflags & STATUS_TAG_TEXT) && (strcasecmp(tagName, "/rss/channel/item/guid") == 0)) {
    strncpy(_guid, data, MAX_PODCAST_GUID_LENGTH);
  }


  if(_isMp3 && (strlen(_url) > 0) && (strlen(_guid) > 0)) {
    // We are done with this tag
    _withinItemTag = false;
    
    _cb(_url, _guid);
  }
}

void Podcatcher::processChar(const char ch) {
  if(_isDone) {
    return;
  }
  _xml.processChar(ch);
}

void Podcatcher::reset() {
  _xml.reset();
  _url[0] = '\0';
  _guid[0] = '\0';
  _withinItemTag = false;
  _withinEnclosureTag = false;
  _isMp3 = false;
  _isDone = false;
}