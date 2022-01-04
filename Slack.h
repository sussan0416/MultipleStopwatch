#include <WiFiClientSecure.h>
#include "certs.h"
#include "SlackEnv.h"

const char reportFormat[] = "{\"blocks\":[{\"type\":\"section\",\"text\":{\"type\":\"plain_text\",\"text\":\"%s現在の稼働レポート\",\"emoji\":true}},{\"type\":\"divider\"},{\"type\":\"section\",\"fields\":[{\"type\":\"mrkdwn\",\"text\":\"*A* %s\"},{\"type\":\"mrkdwn\",\"text\":\"*B* %s\"},{\"type\":\"mrkdwn\",\"text\":\"*C* %s\"},{\"type\":\"mrkdwn\",\"text\":\"*D* %s\"},{\"type\":\"mrkdwn\",\"text\":\"*E* %s\"}]},{\"type\":\"section\",\"text\":{\"type\":\"mrkdwn\",\"text\":\"*合計* %s\"}}]}";
int retryCount = 0;

bool sendToSlack(char *currentTime, char *a, char *b, char *c, char *d, char *e, char *total) {
  WiFiClientSecure client;
  client.setCACert(cert_DigiCert_Global_Root_CA);
  if (!client.connect(slack_host, slack_port)) {
    return false;
  }
  char reportChar[600];
  sprintf(reportChar, reportFormat, currentTime, a, b, c, d, e, total);
  String data = String(reportChar);

  client.print(String("POST ") + SLACK_PATH + " HTTP/1.1\r\n" +
               "User-Agent: Stopwatch5\r\n" +
               "Host: " + slack_host + "\r\n" +
               "Connection: close\r\n" +
               "Content-Type: application/json; charset=utf-8\r\n" +
               "Content-Length: " + data.length() + "\r\n\r\n" +
                data +
               "\r\n"
               );
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println(line);
  client.stop();
  return true;
}
