#include <Arduino.h>
#include <math.h>

// UART7
// PB3  UART7_RX
// PB4  UART7_TX

// 1. Data Structure: NAV-POSLLH (Geodetic Position Solution)
#pragma pack(push, 1)
struct UBX_NAV_POSLLH {
  uint32_t iTOW;    // GPS Time of Week (ms)
  int32_t lon;      // Longitude (Scale: 1e-7 deg)
  int32_t lat;      // Latitude (Scale: 1e-7 deg)
  int32_t height;   // Height above ellipsoid (mm)
  int32_t hMSL;     // Height above mean sea level / Altitude (mm)
  uint32_t hAcc;    // Horizontal accuracy estimate (mm)
  uint32_t vAcc;    // Vertical accuracy estimate (mm)
};

// 2. Data Structure: NAV-SOL (Navigation Solution - Satellites & Fix Status)
struct UBX_NAV_SOL {
  uint32_t iTOW; int32_t fTOW; int16_t week;
  uint8_t gpsFix;   // GPS Fix Type (3=3D Fix, 2=2D Fix, 0/1=No Fix)
  uint8_t flags;
  int32_t ecefX; int32_t ecefY; int32_t ecefZ;
  uint32_t pAcc;
  int32_t ecefVX; int32_t ecefVY; int32_t ecefVZ;
  uint32_t sAcc; uint16_t pDOP; uint8_t res1;
  uint8_t numSV;    // Number of satellites used in Nav Solution
  uint32_t res2;
};

// 3. Data Structure: NAV-VELNED (Velocity Solution in North-East-Down)
struct UBX_NAV_VELNED {
  uint32_t iTOW;    // GPS Time of Week (ms)
  int32_t velN;     // North velocity component (cm/s)
  int32_t velE;     // East velocity component (cm/s)
  int32_t velD;     // Down velocity component (cm/s)
  uint32_t speed;   // 3D Speed (cm/s)
  uint32_t gSpeed;  // 2D Ground Speed (cm/s) -> Used for vehicle speed
  int32_t heading;  // Heading of motion (1e-5 deg)
  uint32_t sAcc;    // Speed accuracy estimate (cm/s)
  uint32_t cAcc;    // Heading accuracy estimate (deg)
};
#pragma pack(pop)

// Global Telemetry Variables
uint8_t g_satelliteCount = 0;
uint8_t g_fixStatus = 0;
float g_speedKmh = 0.0;

// Home Location Variables for Distance Calculation
bool homeSaved = false;
double homeLat = 0.0;
double homeLon = 0.0;

/**
 * Calculates the great-circle distance between two points on the Earth's surface 
 * using the Haversine Formula.
 * 
 * @param lat1 Latitude of first point (decimal degrees)
 * @param lon1 Longitude of first point (decimal degrees)
 * @param lat2 Latitude of second point (decimal degrees)
 * @param lon2 Longitude of second point (decimal degrees)
 * @return Distance between the two points in meters
 */
float calculateDistance(double lat1, double lon1, double lat2, double lon2) {
  float r = 6371000.0; // Earth's radius in meters
  float dLat = (lat2 - lat1) * M_PI / 180.0;
  float dLon = (lon2 - lon1) * M_PI / 180.0;
  float a = sin(dLat / 2) * sin(dLat / 2) +
            cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
            sin(dLon / 2) * sin(dLon / 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return r * c; // Returns distance in meters
}

void setup() {
  Serial.begin(115200);   // Hardware Serial Link to PC (Serial Monitor)
  Serial7.begin(115200);  // Hardware Serial Link to u-blox GPS Module
  delay(1000);
  Serial.println("Advanced Autopilot UBX Parser Initialized...");
}

void loop() {
  static int state = 0;
  static uint8_t msgClass, msgID;
  static uint16_t msgLength;
  static uint8_t payload[60]; // Common payload buffer allocated for the largest packet
  static uint16_t payloadCounter = 0;

  // Read raw byte stream from the GPS module
  while (Serial7.available() > 0) {
    uint8_t b = Serial7.read();

    // UBX Protocol State Machine
    if (state == 0 && b == 181) { state = 1; }       // Sync Char 1 (0xB5)
    else if (state == 1 && b == 98)  { state = 2; }   // Sync Char 2 (0x62)
    else if (state == 2) { msgClass = b; state = 3; } // Message Class
    else if (state == 3) { msgID = b; state = 4; }    // Message ID
    else if (state == 4) { msgLength = b; state = 5; }// Length Low Byte
    else if (state == 5) { 
      msgLength |= ((uint16_t)b << 8); // Length High Byte
      payloadCounter = 0; 
      // Safe guard against buffer overflow
      state = (msgLength > sizeof(payload)) ? 0 : 6; 
    }
    else if (state == 6) {
      payload[payloadCounter++] = b; // Store packet payload
      if (payloadCounter >= msgLength) state = 7;
    }
    else if (state == 7) { state = 8; } // Skip Checksum A
    else if (state == 8) {              // Skip Checksum B & Process Packet
      state = 0;
      
      // A) IDENTIFY MESSAGE: NAV-SOL (Class: 0x01, ID: 0x06)
      if (msgClass == 1 && msgID == 6) {
        UBX_NAV_SOL *nav = (UBX_NAV_SOL*)payload;
        g_satelliteCount = nav->numSV;
        g_fixStatus = nav->gpsFix;
      }
      
      // B) IDENTIFY MESSAGE: NAV-VELNED (Class: 0x01, ID: 0x12)
      else if (msgClass == 1 && msgID == 18) {
        UBX_NAV_VELNED *vel = (UBX_NAV_VELNED*)payload;
        // Convert cm/s to km/h (cm/s * 0.036 = km/h)
        g_speedKmh = vel->gSpeed * 0.036; 
      }
      
      // C) IDENTIFY MESSAGE: NAV-POSLLH (Class: 0x01, ID: 0x02)
      else if (msgClass == 1 && msgID == 2) {
        UBX_NAV_POSLLH *pos = (UBX_NAV_POSLLH*)payload;
        
        // u-blox sends data scaled by 10,000,000 to avoid floats. Converting back to double.
        double latitude = pos->lat / 10000000.0;
        double longitude = pos->lon / 10000000.0;
        double altitude = pos->hMSL / 1000.0; // Convert mm to meters

        Serial.print("Sats: "); Serial.print(g_satelliteCount);
        
        // Check if GPS has a stable 2D or 3D Fix
        if (g_fixStatus == 3 || g_fixStatus == 2) {
          // Lock the first valid coordinate as Home Location
          if (!homeSaved) {
            homeLat = latitude;
            homeLon = longitude;
            homeSaved = true;
            Serial.println("\n[SUCCESS] Home Position Saved!");
          }

          // Calculate real-time distance to the home position
          float distanceToHome = calculateDistance(latitude, longitude, homeLat, homeLon);

          // Stream final parsed telemetry over Serial
          Serial.print(" | Lat: ");       Serial.print(latitude, 6);
          Serial.print(" | Lon: ");       Serial.print(longitude, 6);
          Serial.print(" | Alt: ");       Serial.print(altitude, 1);  Serial.print("m");
          Serial.print(" | Speed: ");     Serial.print(g_speedKmh, 1); Serial.print(" km/h");
          Serial.print(" | DistToHome: "); Serial.print(distanceToHome, 1); Serial.println(" m");
          
        } else {
          Serial.println(" | Searching Sigs... (Home not locked yet)");
          homeSaved = false; // Reset home if signal is completely dropped
        }
      }
    }
  }
}
