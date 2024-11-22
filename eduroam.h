// configure your connection here
#define USE_EDUROAM 1
#define WIFI_USER "zcabflu@ucl.ac.uk"
#define WIFI_SSID "eduroam"
#define WIFI_PASS "XIchuangzhuyu07"
// end of configuration

#include "WiFi.h"

#if USE_EDUROAM
#include "esp_eap_client.h"
const char* ssid = "eduroam";
const char* user = WIFI_USER;
#else
const char* ssid = WIFI_SSID;
#endif

const char* pass = WIFI_PASS;
wifi_connect ();
// dump some details of the WiFi connection (if any) to the serial line
void print_wifi_info ()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Connected to WiFi network: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway address: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
  }
  else
  {
    Serial.println("WiFi not connected");
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
  }
}

void wifi_connect ( float timeout = 15 )
{
  unsigned long deadline;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

#if USE_EDUROAM
  Serial.printf("Connecting to eduroam as user %s\n", user);
  esp_eap_client_set_identity((uint8_t *)user, strlen(user));
  esp_eap_client_set_username((uint8_t *)user, strlen(user));
  esp_eap_client_set_password((uint8_t *)pass, strlen(pass));
  esp_wifi_sta_enterprise_enable();
  WiFi.begin(ssid);
#else
  Serial.printf("Connecting to %s\n", ssid);
  WiFi.begin(ssid, pass);
#endif
  
  deadline = millis() + (unsigned long)(timeout * 1000);

  while ((WiFi.status() != WL_CONNECTED) && (millis() < deadline))
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  print_wifi_info();
}
