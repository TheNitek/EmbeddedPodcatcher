#include <Arduino.h>
#include <Podcatcher.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>

Podcatcher catcher;

const char podcastUrl[] = "https://audioboom.com/channels/4910899.rss";

void episodeCallback(const char *url, const char *guid) {
  Serial.printf("%s %s\n", url, guid);
}

void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(3 * 60);
  if (!wifiManager.autoConnect("PODCAST", "podpodpod")) {
    Serial.println(F("Setup timed out, starting AP"));
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP("MP3-SHELF", "lacklack")) {
      Serial.println(F("Soft-AP is set up"));
    } else {
      Serial.println(F("Soft-AP setup failed"));
    }
  }

  if(WiFi.isConnected()) {
    Serial.print(F("Connected! IP address: ")); Serial.println(WiFi.localIP());
  }

  catcher.begin(episodeCallback);

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  class: public HTTPClient {
    public:
      transferEncoding_t getTransferEncoding() {
        return _transferEncoding;
      }
  } httpClient;
  httpClient.begin(*client, podcastUrl);
  int httpCode = httpClient.GET();
  if (httpCode == HTTP_CODE_OK) {
    int len = httpClient.getSize();
    WiFiClient* stream = httpClient.getStreamPtr();

    if(httpClient.getTransferEncoding() == HTTPC_TE_CHUNKED) {
      Serial.println("Chunked");
    }

    while(len > 0 || httpClient.getTransferEncoding() == HTTPC_TE_CHUNKED){ 
      if(httpClient.getTransferEncoding() == HTTPC_TE_CHUNKED) {

        char chunkSizeBuffer[20] = "";

        size_t headerLength = stream->readBytesUntil('\n', chunkSizeBuffer, sizeof(chunkSizeBuffer));

        if(headerLength == 0) {
          Serial.println("Last Chunk");
          break;
        } else {
          // Terminate string and get rid of \r
          chunkSizeBuffer[headerLength - 1] = '\0';

          // read size of chunk
          len = (uint32_t) strtol((const char *) chunkSizeBuffer, NULL, 16);
        }
      }

      while(len > 0) {
        if(stream->available()) {
          catcher.processChar(stream->read());
          if (len > 0) {
            len--;
          }
        }
      }

      if(httpClient.getTransferEncoding() == HTTPC_TE_CHUNKED) {
        char buf[2];
        stream->readBytes((uint8_t*)buf, 2);
      }
    }
  } else {
    Serial.printf("HTTP Error: %d\n", httpCode);
  }
  httpClient.end();
  Serial.println("Fetched podcast");
}

void loop() {

}
