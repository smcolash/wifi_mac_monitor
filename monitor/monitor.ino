//
// time support
//
#include <Time.h>

//
// ESP8266 WiFi support
//
#include <ESP8266WiFi.h>

//
// customize these per your environment
//
//#define ONLINE
#define VERBOSE

#ifdef ONLINE
const char *hostname = "macmonitor";
const char *ssid = "xxxxxxxxx";
const char *password = "yyyyyyyyyy";
#endif

//
// duration of the 'on' state, in seconds
//
const unsigned int duration = 15 * 60;
//const unsigned int duration = 0.5 * 60;

//
// GPIO pin for output control
//
const unsigned int relay = 0;

//
// select the appropriate timestamp mechanism
//
// #define TIMESTAMP (millis () / 1000.0)
#define TIMESTAMP time (NULL)

//
// The length, in bytes, of an Ethernet MAC identifier.
//
#define ETHERNET_MAC_LENGTH 6

//
// The list of MAC identifiers to use as triggers.
//
unsigned char macid[][ETHERNET_MAC_LENGTH] =
{
  { 0x6c, 0x72, 0xe7, 0xaf, 0x89, 0x55 }, // WiFi MAC #1
  { 0x6c, 0x72, 0xe7, 0xaf, 0x89, 0x56 }  // WiFi MAC #2
};

//
// A global timestamp used to debounce the output.
//
time_t timestamp = 0;

//
// A state variable to hold the ON/OFF state of the output.
//
bool on = false;

//
// Print a hex dump of a buffer of data.
//
void dump (unsigned char *data, size_t length)
{
  for (int i = 0; i < length; i++)
  {
    Serial.printf ("%02x", data[i]);
  }
}

//
// Determine if a specific binary pattern exists within a buffer of data.
//
bool find (unsigned char *pattern, size_t plen, unsigned char *buffer, size_t blen)
{
  if (plen > blen)
  {
    return (false);
  }

  for (int i = 0; i < blen - plen; i++)
  {
    int count = 0;

    for (int j = 0; j < plen; j++)
    {
      if (pattern[j] != buffer[j + i])
      {
        break;
      }

      count++;
    }

    if (count == plen)
    {
      return (true);
    }
  }

  return (false);
}

//
// A callback function to examine WiFi packets acquired in promiscuous mode.
//
void callback (uint8_t *data, uint16_t length)
{
  for (int loop = 0; loop < sizeof (macid) / sizeof (macid[0]); loop++)
  {
    if (find ((unsigned char *) &macid[loop], ETHERNET_MAC_LENGTH, data, length))
    {
#ifdef VERBOSE
      Serial.printf ("%d ", time (NULL));
      dump (data, length);
      Serial.println ();

      if (!on)
      {
        Serial.printf ("%d %s", time (NULL), "ON");
        Serial.println ();
        tone (13, 1047, 250);
      }
#endif

      timestamp = TIMESTAMP;
      on = true;
      digitalWrite (0, LOW);
    }
  }
}

//
// Sketch initialization function.
//
void setup()
{
#ifdef VERBOSE
  Serial.begin (115200);
  delay (250);
  Serial.println ();
#endif

  pinMode (relay, OUTPUT);
  digitalWrite (relay, HIGH);

  timestamp = 0;
  on = false;

#ifdef ONLINE
  //
  // connect to the wireless network
  //
  WiFi.mode (WIFI_STA);
  WiFi.hostname (hostname);
  WiFi.begin (ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay (500);
#ifdef VERBOSE
    Serial.print (".");
#endif
  }

#ifdef VERBOSE
  Serial.println ();
  Serial.print ("connected to ");
  Serial.print (ssid);
  Serial.print (" as ");
  Serial.print (hostname);
  Serial.print (" (");
  Serial.print (WiFi.localIP ());
  Serial.print (")");
  Serial.println ();
#endif
#endif

  //
  // set up to listen to WiFi raw packets
  //
  wifi_set_opmode (STATION_MODE);
  wifi_set_channel (1);
  wifi_promiscuous_enable (false);
  wifi_set_promiscuous_rx_cb (callback);
  wifi_promiscuous_enable (true);
}

//
// Sketch main processing loop function.
//
void loop ()
{
  if (on && ((time (NULL) - timestamp) > duration))
  {
    digitalWrite (relay, HIGH);
    on = false;

#ifdef VERBOSE
    Serial.printf ("%d %s", time (NULL), "OFF");
    Serial.println ();
    tone (13, 262, 250);
#endif
  }
}
