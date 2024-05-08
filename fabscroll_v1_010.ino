// fabscroll v1_010
//
// by t. b. mosich 2024
//
// fetch text strings online and scroll on p10 led matrices
// using esp32 wifi

// Configuration defines should go before library include
#define PxMATRIX_OE_INVERT 1
#define PxMATRIX_DATA_INVERT 1
#define PxMATRIX_GAMMA_PRESET 3
#define PxMATRIX_DOUBLE_BUFFER 1
#define PxMATRIX_COLOR_DEPTH 2
#define PxMATRIX_DEFAULT_SHOWTIME 60  //millis; try to keep leds active for this between updates
#define PxMATRIX_SPI_FREQUENCY 20000000L

#include "Arduino.h"
#include <PxMatrix.h>
#include <WiFiClientSecure.h>
#include <Ticker.h>

#define MAX_CONNECTION_ATTEMPTS 30  // wifi connect attempts

// Replace with your Wi-Fi network credentials
const char *ssid = "...";
const char *password = "...";

// Replace with the URL of the text file you want to fetch
//file should be plain ASCII text, no special characters, optionally UTF8

const char *url_serv = "gist.githubusercontent.com";  // server base url
const char *url_24h =
  "https://gist.githubusercontent.com/.../.../raw/text24h.txt";  // 24h override text full url
const char *url_def =
  "https://gist.githubusercontent.com/.../.../raw/default.txt";  // default text full url

const int port_number = 443;  // SSL port

const int led_brightness = 255;   // led brightness max 255, 255 also = max power consumption
const int txt_scroll_speed = 97;  // millis; lower = faster; technically, this controls how often the drawing/animation routines are called
const int refresh_rate = 10;      // millis; how often to refresh display (library example default = 1 ms); experiment with higher/lower values if display eg. flickers etc

const int minCharsToFetch = 5;     // min. number of remote chars to be considered a valid text
constexpr size_t maxChars = 1024;  // Maximum number of characters to fetch
char textBuffer[maxChars];         // Buffer to store the fetched text file
char defaultTextBuffer[maxChars] =
  "oOoOoOoOoOo....Copenhagen Fablab....oOoOoOoOoOo\0";  // Default fallback text if remote text cannot be fetched
char previousTextBuffer[maxChars] = "";
char previousDefaultTextBuffer[maxChars] = "";

// timer interval before reverting to default text only (milliseconds)
const unsigned long restore_default_text_interval = 24 * 60 * 60 * 1000;  // 24 hours in milliseconds
// timer remote fetch attempt interval (milliseconds)
const unsigned long remote_fetch_interval = 10 * 60 * 1000;  // 10 minutes
// time to switch between scroll modes
const unsigned long scroll_mode_timer = 3 * 60 * 1000;  // 3mins then switch scroll mode (1line, 2lines, etc)

WiFiClientSecure client;             // use https
Ticker remote_fetch_ticker;          // Create a Ticker object for the main timer
Ticker restore_default_text_ticker;  // Create a Ticker object for the default text timer
Ticker scroll_mode_ticker;           // Ticker object for switching between scroll modes

bool newTextInserted = false;  // keep track of if text has changed

// Define states
enum State {
  CONNECT_WIFI,
  FETCH_FILE,
  DISCONNECT_WIFI,
  WAIT
};

// Define timer states
enum TimerState {
  TIMER_START,
  TIMER_ENDED
};

// Enum to define different drawing modes
enum DrawingMode {
  SINGLE_LINE_SCROLL,
  DOUBLE_LINE_SCROLL,
  OTHER_MODE  // Add more drawing modes as needed
};

// Variables to store current states
State current_state = CONNECT_WIFI;
TimerState remote_fetch_timer_state = TIMER_START;
TimerState default_text_timer_state = TIMER_START;
// Current drawing mode
DrawingMode current_scroll_mode = SINGLE_LINE_SCROLL;  // Initial mode

// physical display properties
const int display_width = 32 * 3;  // standard p10 is 32x16 leds, define total width here -- so here 32*3 is three panels wide
const int display_height = 16;     // define total height

// Define output pins
#define P_A 16    // esp pin 16 -> A on led matrix
#define P_B 17    // -> B
#define P_OE 22   // -> NOE
#define P_LAT 21  // (-> CKL?? CHECK PHYSICAL CONNECTION)
// SPI pins (for the reference)
const uint8_t SPI_MOSI = 23;  // -> R (data in)
const uint8_t SPI_SCK = 18;   // (-> ?? CHECK PHYSICAL CONNECTION)

PxMATRIX display(display_width, display_height, P_LAT, P_OE, P_A, P_B);  // setup the display

// display runs on two timers, one to refresh the display and one for the animation
Ticker display_refresh_timer;  // timer for refreshing the display (like a screen refresh)
Ticker update_leds_timer;      // timer for the animation

// end variables, constants etc


// begin setup

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize textbuffer with default text
  strncpy(textBuffer, defaultTextBuffer, sizeof(textBuffer) - 1);
  textBuffer[sizeof(textBuffer) - 1] = '\0';

  // basic settings
  display.begin(4);  // Scan rate 1/4; decided by led matrix hardware
  display.setBrightness(led_brightness);
  // clear screen(s)
  display.flushDisplay();
  display.showBuffer();
  display.clearDisplay();
  // init/boot message
  display.setTextColor(0xFF);
  display.setCursor(1, 1);
  display.println("fabsc");
  display.setCursor(2, 8);
  display.println("roll1");
  display.showBuffer();

  // attach the display refresh and animation mode timers to automatically trigger
  display_refresh_timer.attach_ms(refresh_rate, display_refresh);       // Update display every display_refresh ms (library example default = 1 ms)
  scroll_mode_ticker.attach_ms(scroll_mode_timer, switch_scroll_mode);  // switch_scroll_mode every scroll_mode_timer minutes
}

//end setup
