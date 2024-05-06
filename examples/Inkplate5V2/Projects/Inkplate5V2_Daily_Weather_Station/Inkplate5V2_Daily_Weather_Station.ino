/*
    Inkplate5V2_Daily_Weather_Station example for Soldered Inkplate 5 V2
    For this example you will need only USB cable and Inkplate 5 V2.
    Select "Soldered Inkplate5 V2" from Tools -> Board menu.
    Don't have "Soldered Inkplate5 V2" option? Follow our tutorial and add it:
    https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

    This example will show you how you can use Inkplate 5 V2 to display API data,
    e.g. Openweather public weather API. It shows the forecast weather for 4 days 
    at 15 o'clock (3 PM). If it's passed 3 PM, the first rectangle shows the 
    forecast weather for tomorrow at 3 PM. What happens here is basically ESP32 
    connects to WiFi and sends an API call and the server returns data in JSON 
    format containing data about weather, then using the library ArduinoJson we 
    extract only temperature per day from JSON data and show it on Inkplate 5. 
    After displaying the weather, ESP32 goes to sleep and wakes up every DELAY_MS 
    milliseconds to show new weather (you can change the time interval).

    IMPORTANT:
    Make sure to change your desired city and wifi credentials below.
    Also have ArduinoJson installed in your Arduino libraries:
    https://github.com/bblanchon/ArduinoJson

    Want to learn more about Inkplate? Visit www.inkplate.io
    Looking to get support? Write on our forums: https://forum.soldered.com/
    15 April 2024 by Soldered
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE5V2
#error "Wrong board selection for this example, please select Soldered Inkplate5 V2 in the boards menu."
#endif

//---------- CHANGE HERE  -------------:

// City name to de displayed on the bottom
char city[128] = "OSIJEK";

// Coordinates sent to the api
char lat[] = "45.5510548";
char lon[] = "18.695463";

// Change to your wifi ssid and password
char ssid[] = "";
char pass[] = "";

// Change to your api key, if you don't have one, head over to:
// https://openweathermap.org/guide , register and copy the key provided
char apiKey[] = "";

//----------------------------------

// Include Inkplate library to the sketch
#include "Inkplate.h"

// Header file for easier code readability
#include "Network.h"

// Including fonts used
#include "Fonts/Inter16pt7b.h"
#include "Fonts/Inter20pt7b.h"
#include "Fonts/Inter48pt7b.h"

// Including icons generated by the py file
#include "icons.h"

// Delay between API calls, about 1000 per month, which is the free tier limit
#define DELAY_MS 267800L

// Inkplate object
Inkplate display(INKPLATE_1BIT);

// All our network functions are in this object, see Network.h
Network network;

// Contants used for drawing icons
char abbrs[18][4] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d",
                      "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"};
                      
const uint8_t *logos[18] = {
    icon_01d, icon_02d, icon_03d, icon_04d, icon_09d, icon_10d, icon_11d, icon_13d, icon_50d,
    icon_01n, icon_02n, icon_03n, icon_04n, icon_09n, icon_10n, icon_11n, icon_13n, icon_50n,
};

const uint8_t *s_logos[18] = {
    icon_s_01d, icon_s_02d, icon_s_03d, icon_s_04d, icon_s_09d, icon_s_10d, icon_s_11d, icon_s_13d, icon_s_50d,
    icon_s_01n, icon_s_02n, icon_s_03n, icon_s_04n, icon_s_09n, icon_s_10n, icon_s_11n, icon_s_13n, icon_s_50n,
};

// Variable for counting partial refreshes
RTC_DATA_ATTR unsigned refreshes = 0;

// Constant to determine when to full update
const int fullRefresh = 5;

// Variables for storing temperature
RTC_DATA_ATTR char temps[4][8] = {
    "-",
    "-",
    "-",
    "-",
};

// Variables for storing hour strings
RTC_DATA_ATTR uint8_t hours = 0;

// Variables for storing current time and weather info
RTC_DATA_ATTR char currentTemp[16] = "-";
RTC_DATA_ATTR char currentWind[16] = "-";

RTC_DATA_ATTR char currentTime[16] = "--:--";

RTC_DATA_ATTR int timeZone;

RTC_DATA_ATTR char currentWeather[32] = "-";
RTC_DATA_ATTR char currentWeatherAbbr[4] = "01d";

RTC_DATA_ATTR char abbr1[4];
RTC_DATA_ATTR char abbr2[4];
RTC_DATA_ATTR char abbr3[4];
RTC_DATA_ATTR char abbr4[4];

// functions defined below
void drawWeather();
void drawCurrent();
void drawTemps();
void drawCity();
void drawTime();

void setup()
{
    // Begin serial and display
    Serial.begin(115200);
    display.begin();

    // Calling our begin from network.h file
    network.begin(ssid, pass);

    // Get all relevant data, see Network.cpp for info
    if (refreshes % fullRefresh == 0)
    {
        Serial.print("Retrying fetching data");
        while (!network.getData(lat, lon, apiKey, city, temps[0], temps[1], temps[2], temps[3], currentTemp,
                                currentWind, currentTime, currentWeather, currentWeatherAbbr, abbr1, abbr2, abbr3,
                                abbr4, &hours, &timeZone))
        {
            Serial.print('.');
            delay(500);
        }
        Serial.println();
    }
    network.getTime(currentTime, timeZone);

    // Draw data, see functions below for info
    drawWeather();
    drawCurrent();
    drawTemps();
    drawCity();
    drawTime();

    display.display();

    ++refreshes;

    // Go to sleep before checking again
    Serial.println("Going to sleep");
    esp_sleep_enable_timer_wakeup(1000L * DELAY_MS); // Activate wake-up timer
    (void)esp_deep_sleep_start(); // Start deep sleep (this function does not return). Program stops here.
}

void loop()
{
    // Never here! If you are using deep sleep, the whole program should be in setup() because the board restarts each
    // time. loop() must be empty!
}

// Function for drawing weather info
void drawWeather()
{
    // Searching for weather state abbreviation
    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbrs[i], currentWeatherAbbr) == 0)
            display.drawBitmap(200, 90, logos[i], 152, 152, BLACK);
    }

    // Draw weather state
    display.setTextColor(BLACK, WHITE);
    display.setFont(&Inter16pt7b);
    display.setTextSize(1);
    display.setCursor(220, 260);
    display.println(currentWeather);
}

// Function for drawing current time
void drawTime()
{
    // Drawing current time
    display.setTextColor(BLACK, WHITE);
    display.setFont(&Inter16pt7b);
    display.setTextSize(1);

    display.setCursor(display.width() - 20 * strlen(currentTime), 35);
    display.println(currentTime);
}

// Function for drawing city name
void drawCity()
{
    // Drawing city name
    display.setTextColor(BLACK, WHITE);
    display.setFont(&Inter16pt7b);
    display.setTextSize(1);

    display.setCursor(display.width()/2 - 9 * strlen(city), display.height() - 20);
    display.println(city);
}

// Function for drawing temperatures
void drawTemps()
{
    // Drawing 4 black rectangles in which temperatures will be written
    int rectWidth = 150;
    int rectSpacing = ((display.width() - rectWidth * 4) / 5);

    display.fillRect(1 * rectSpacing + 0 * rectWidth, 320, rectWidth, 220, BLACK);
    display.fillRect(2 * rectSpacing + 1 * rectWidth, 320, rectWidth, 220, BLACK);
    display.fillRect(3 * rectSpacing + 2 * rectWidth, 320, rectWidth, 220, BLACK);
    display.fillRect(4 * rectSpacing + 3 * rectWidth, 320, rectWidth, 220, BLACK);

    int textMargin = 6;

    display.setFont(&Inter20pt7b);
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);

    display.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 320 + textMargin + 40);
    display.println("Today");

    display.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 320 + textMargin + 40);
    display.println(wDays[hours + 1 > 6 ? hours + 1 - 6 : hours + 1]);

    display.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 320 + textMargin + 40);
    display.println(wDays[hours + 2 > 6 ? hours + 2 - 6 : hours + 2]);

    display.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 320+ textMargin + 40);
    display.println(wDays[hours + 3 > 6 ? hours + 3 - 6 : hours + 3]);

    // Drawing temperature values into black rectangles
    display.setFont(&Inter20pt7b);
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);

    display.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 320 + textMargin + 120);
    display.print(temps[0]);
    display.println(F("C"));

    display.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 320 + textMargin + 120);
    display.print(temps[1]);
    display.println(F("C"));

    display.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 320 + textMargin + 120);
    display.print(temps[2]);
    display.println(F("C"));

    display.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 320 + textMargin + 120);
    display.print(temps[3]);
    display.println(F("C"));

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr1, abbrs[i]) == 0)
            display.drawBitmap(1 * rectSpacing + 0 * rectWidth + textMargin, 320 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr2, abbrs[i]) == 0)
            display.drawBitmap(2 * rectSpacing + 1 * rectWidth + textMargin, 320 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr3, abbrs[i]) == 0)
            display.drawBitmap(3 * rectSpacing + 2 * rectWidth + textMargin, 320 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr4, abbrs[i]) == 0)
            display.drawBitmap(4 * rectSpacing + 3 * rectWidth + textMargin, 320 + textMargin + 150, s_logos[i], 48, 48,
                               WHITE, BLACK);
    }
}

// Current weather drawing function
void drawCurrent()
{
    // Drawing current information

    // Temperature:
    display.setFont(&Inter48pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);

    display.setCursor(550, 200);
    display.print(currentTemp);

    display.setFont(&Inter20pt7b);
    display.setTextSize(1);
    display.setCursor(665, 200);

    display.println(F("C"));

    // Wind:
    display.setFont(&Inter48pt7b);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);

    display.setCursor(850, 200);
    display.print(currentWind);

    display.setFont(&Inter20pt7b);
    display.setTextSize(1);

    display.setCursor(1000, 200);
    display.println(F("m/s"));

    // Labels underneath
    display.setFont(&Inter16pt7b);
    display.setTextSize(1);

    display.setCursor(500, 260);
    display.println(F("TEMPERATURE"));

    display.setCursor(834, 260);
    display.println(F("WIND SPEED"));
}
