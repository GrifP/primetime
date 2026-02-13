/*
 * Primetime
 * 
 * This program illuminates individual LEDs when the current date represents
 * a prime number of days since each family member's birth date.
 * 
 * Each LED stays on for the entire day when it's that person's prime day.
 * Supports up to 4 family members on pins D1, D2, D5, D6.
 * Status LED on D7 provides system feedback.
 * 
 * Hardware: ESP8266 (e.g., NodeMCU, Wemos D1 Mini)
 * 
 * Author: Your Name
 * License: MIT
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

// ==================== CONFIGURATION ====================
// WiFi credentials - CHANGE THESE!
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Timezone offset in seconds (GMT-7 = -7 * 3600, GMT+1 = 1 * 3600)
// Find your offset: https://en.wikipedia.org/wiki/List_of_UTC_offsets
const long TIMEZONE_OFFSET = -7 * 3600;

// Test mode - set to true to cycle through all LEDs for testing
const bool TEST_MODE = false;  // Change to true for testing

// LED brightness configuration (0-1023, where 1023 = 100%)
// Lower values are easier on the eyes and save power
const int LED_BRIGHTNESS = 40;  // ~4% brightness - adjust to preference

// Status LED configuration
const int STATUS_LED_PIN = D7;

// Family member configuration
struct FamilyMember {
  const char* name;
  int birthYear;
  int birthMonth;
  int birthDay;
  int ledPin;
  bool isPrimeDay;
  long daysSinceBirth;
};

// Configure your family members here - CHANGE THESE!
// Add up to 4 family members with their birthdates and assigned LED pins
FamilyMember family[4] = {
  {"Person1", 1990, 1, 15, D1, false, 0},  // Example: Born Jan 15, 1990
  {"Person2", 1992, 6, 22, D2, false, 0},  // Example: Born Jun 22, 1992
  {"Person3", 2015, 9, 8,  D5, false, 0},  // Example: Born Sep 8, 2015
  {"Person4", 2018, 12, 3, D6, false, 0}   // Example: Born Dec 3, 2018
};

const int NUM_FAMILY_MEMBERS = 4;

// NTP update interval (once per hour)
const unsigned long NTP_UPDATE_INTERVAL = 3600000;

// Status LED timing variables
unsigned long lastStatusBlink = 0;
bool statusLedState = false;

// ==================== GLOBAL VARIABLES ====================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", TIMEZONE_OFFSET, NTP_UPDATE_INTERVAL);

int lastCheckedDay = -1;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Primetime: Prime Day of Life ===");
  
  // Initialize status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  analogWrite(STATUS_LED_PIN, 0);
  Serial.println("Status LED: D7 (System status indicator)");
  
  // Initialize LED pins for all family members
  for (int i = 0; i < NUM_FAMILY_MEMBERS; i++) {
    pinMode(family[i].ledPin, OUTPUT);
    analogWrite(family[i].ledPin, 0);
    Serial.printf("%s: %04d-%02d-%02d (Pin D%d)\n", 
                  family[i].name,
                  family[i].birthYear, 
                  family[i].birthMonth, 
                  family[i].birthDay,
                  getPinNumber(family[i].ledPin));
  }
  
  Serial.printf("Timezone: GMT%+d\n", TIMEZONE_OFFSET / 3600);
  Serial.printf("LED Brightness: %d/1023 (%.0f%%)\n", LED_BRIGHTNESS, (LED_BRIGHTNESS / 1023.0) * 100);
  
  // Connect to WiFi with network scanning and timeout
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.println("Found networks:");
  for (int i = 0; i < n; i++) {
    Serial.printf("%d: %s (Signal: %d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }
  Serial.println();
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) { // Timeout after 30 attempts
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connection failed!");
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
  }
  
  // Initialize NTP client
  timeClient.begin();
  Serial.println("NTP client initialized");
  
  // Force initial time sync
  Serial.print("Syncing time");
  while (!timeClient.update()) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  
  // System fully operational - turn on status LED
  analogWrite(STATUS_LED_PIN, LED_BRIGHTNESS);
  Serial.println("Status LED: ON (System operational)");
  
  Serial.println("Setup complete!");
  checkAllFamilyMembers();
  printCurrentStatus();
}

void loop() {
  // Test mode - cycle through LEDs for testing
  if (TEST_MODE) {
    testAllLEDs();
    return;
  }
  
  // Keep status LED on when connected, blink when disconnected
  if (WiFi.status() == WL_CONNECTED) {
    analogWrite(STATUS_LED_PIN, LED_BRIGHTNESS);
  } else {
    updateStatusLED(); // Blink when disconnected
  }
  
  // Update time from NTP server
  timeClient.update();
  
  // Check if we need to recalculate (once per day)
  int currentDay = day(timeClient.getEpochTime());
  if (currentDay != lastCheckedDay) {
    lastCheckedDay = currentDay;
    checkAllFamilyMembers();
    printCurrentStatus();
  }
  
  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.reconnect();
  }
  
  // Small delay to prevent excessive processing
  delay(60000); // Check every minute
}

void startStatusBlink(unsigned long interval) {
  lastStatusBlink = millis();
  statusLedState = false;
  analogWrite(STATUS_LED_PIN, 0);
}

void updateStatusLED() {
  if (millis() - lastStatusBlink >= 500) { // Blink every 500ms when disconnected
    statusLedState = !statusLedState;
    analogWrite(STATUS_LED_PIN, statusLedState ? LED_BRIGHTNESS : 0);
    lastStatusBlink = millis();
  }
}

void checkAllFamilyMembers() {
  unsigned long currentEpochTime = timeClient.getEpochTime();
  
  Serial.println("\n=== Checking all family members ===");
  
  for (int i = 0; i < NUM_FAMILY_MEMBERS; i++) {
    checkFamilyMemberPrimeDay(i, currentEpochTime);
  }
}

void checkFamilyMemberPrimeDay(int memberIndex, unsigned long currentEpochTime) {
  FamilyMember* member = &family[memberIndex];
  
  // Calculate birth date in epoch time
  tmElements_t birthDate;
  birthDate.Year = member->birthYear - 1970;
  birthDate.Month = member->birthMonth;
  birthDate.Day = member->birthDay;
  birthDate.Hour = 0;
  birthDate.Minute = 0;
  birthDate.Second = 0;
  
  unsigned long birthEpoch = makeTime(birthDate);
  
  // Calculate days since birth (birth date = day 0)
  long daysSinceBirth = (currentEpochTime - birthEpoch) / 86400;
  member->daysSinceBirth = daysSinceBirth;
  
  // Ensure we have a positive number (day 0 = birth date)
  if (daysSinceBirth < 0) {
    Serial.printf("%s: Error - birth date is in the future!\n", member->name);
    analogWrite(member->ledPin, 0);
    member->isPrimeDay = false;
    return;
  }
  
  // Check if days since birth is prime
  bool isPrime = isPrimeNumber(daysSinceBirth);
  member->isPrimeDay = isPrime;
  
  // Update LED state
  if (isPrime) {
    analogWrite(member->ledPin, LED_BRIGHTNESS);
    Serial.printf("🎉 %s: PRIME DAY! Day %ld is prime.\n", member->name, daysSinceBirth);
  } else {
    analogWrite(member->ledPin, 0);
    Serial.printf("%s: Day %ld (not prime)\n", member->name, daysSinceBirth);
  }
}

bool isPrimeNumber(long n) {
  if (n <= 1) return false;
  if (n <= 3) return true;
  if (n % 2 == 0 || n % 3 == 0) return false;
  
  // Check for divisors from 5 to sqrt(n)
  for (long i = 5; i * i <= n; i += 6) {
    if (n % i == 0 || n % (i + 2) == 0) {
      return false;
    }
  }
  
  return true;
}

void printCurrentStatus() {
  Serial.println("\n--- Current Status ---");
  Serial.printf("Current time: %s", timeClient.getFormattedTime().c_str());
  Serial.printf(" on %04d-%02d-%02d\n", 
                year(timeClient.getEpochTime()), 
                month(timeClient.getEpochTime()), 
                day(timeClient.getEpochTime()));
  
  Serial.println("Family Prime Day Status:");
  for (int i = 0; i < NUM_FAMILY_MEMBERS; i++) {
    Serial.printf("  %s (Day %ld): %s\n", 
                  family[i].name,
                  family[i].daysSinceBirth,
                  family[i].isPrimeDay ? "🟢 PRIME DAY!" : "⚫ Regular day");
  }
  
  Serial.printf("WiFi status: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.printf("Status LED: %s\n", WiFi.status() == WL_CONNECTED ? "ON (Connected)" : "Blinking (Disconnected)");
  Serial.println("---------------------\n");
}

// Helper function to get pin number for display
int getPinNumber(int pin) {
  switch(pin) {
    case D1: return 1;
    case D2: return 2;
    case D5: return 5;
    case D6: return 6;
    case D7: return 7;
    default: return 0;
  }
}

// Function to print next prime day for each family member
void printUpcomingPrimeDays() {
  Serial.println("\n=== Next Prime Day for Each Family Member ===");
  
  unsigned long currentEpoch = timeClient.getEpochTime();
  
  for (int i = 0; i < NUM_FAMILY_MEMBERS; i++) {
    FamilyMember* member = &family[i];
    
    // Calculate birth epoch
    tmElements_t birthDate;
    birthDate.Year = member->birthYear - 1970;
    birthDate.Month = member->birthMonth;
    birthDate.Day = member->birthDay;
    birthDate.Hour = 0;
    birthDate.Minute = 0;
    birthDate.Second = 0;
    
    unsigned long birthEpoch = makeTime(birthDate);
    long currentDays = (currentEpoch - birthEpoch) / 86400;
    
    // Find next prime day
    for (long day = currentDays + 1; day < currentDays + 1000; day++) {
      if (isPrimeNumber(day)) {
        unsigned long futureEpoch = birthEpoch + (day * 86400);
        Serial.printf("%s: Day %ld on %04d-%02d-%02d\n", 
                      member->name,
                      day,
                      year(futureEpoch), 
                      month(futureEpoch),
                      day(futureEpoch));
        break;
      }
    }
  }
  Serial.println();
}

// Test function to cycle through all LEDs
void testAllLEDs() {
  static unsigned long lastTestTime = 0;
  static int currentTestLED = -1; // Start with -1 to test status LED first
  
  if (millis() - lastTestTime >= 1000) { // Change LED every second
    // Turn off all LEDs
    for (int i = 0; i < NUM_FAMILY_MEMBERS; i++) {
      analogWrite(family[i].ledPin, 0);
    }
    analogWrite(STATUS_LED_PIN, 0);
    
    if (currentTestLED == -1) {
      // Test status LED
      analogWrite(STATUS_LED_PIN, LED_BRIGHTNESS);
      Serial.println("Testing Status LED (Pin D7)");
    } else {
      // Test family member LED
      analogWrite(family[currentTestLED].ledPin, LED_BRIGHTNESS);
      Serial.printf("Testing %s's LED (Pin D%d)\n", 
                    family[currentTestLED].name,
                    getPinNumber(family[currentTestLED].ledPin));
    }
    
    currentTestLED++;
    if (currentTestLED >= NUM_FAMILY_MEMBERS) {
      currentTestLED = -1; // Reset to status LED
    }
    
    lastTestTime = millis();
  }
}
