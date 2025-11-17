#include <Wire.h> //Needed for I2C to GNSS
#include <SPI.h>
#include <SD.h>

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

File myFile;
long lastTime = 0; //Tracks the passing of 2000ms (2 seconds)
const byte buzzer = 6;
long first_alt = 0;
byte fix_type;
byte number = 0;

void printPVTdata(UBX_NAV_PVT_data_t *ubxDataStruct)
{
  File myFile = SD.open("test.txt", FILE_WRITE);

    Serial.println();

    Serial.print(F("Time: ")); // Print the time
    uint8_t hms = ubxDataStruct->hour; // Print the hours
    if (hms < 10) {
      Serial.print(F("0")); // Print a leading zero if required
      myFile.print(F("0"));
    }
    Serial.print(hms);
    Serial.print(F(":"));
    myFile.print(hms);
    myFile.print(F(":"));
    hms = ubxDataStruct->min; // Print the minutes
    if (hms < 10) {
      Serial.print(F("0")); // Print a leading zero if required
      myFile.print(F("0"));
    }
    myFile.print(hms);
    myFile.print(F(":"));
    hms = ubxDataStruct->sec; // Print the seconds
    if (hms < 10) {
      Serial.print(F("0")); // Print a leading zero if required
      myFile.print(F("0"));
    }
    Serial.print(hms);
    Serial.print(F("."));
    myFile.print(hms);
    myFile.print(F("."));
    uint32_t millisecs = ubxDataStruct->iTOW % 1000; // Print the milliseconds
    if (millisecs < 100) {Serial.print(F("0"));myFile.print(F("0"));} // Print the trailing zeros correctly
    if (millisecs < 10) {Serial.print(F("0")); myFile.print(F("0"));}
    Serial.print(millisecs);
    myFile.print(millisecs);


    int32_t latitude = ubxDataStruct->lat; // Print the latitude
    Serial.print(F(" Lat: "));
    Serial.print(1.0*latitude/10000000, 7);
    myFile.print(F(" Lat: "));
    myFile.print(1.0*latitude/10000000, 7);

    int32_t longitude = ubxDataStruct->lon; // Print the longitude
    Serial.print(F(" Long: "));
    Serial.print(1.0*longitude/10000000, 7);
    myFile.print(F(" Long: "));
    myFile.print(1.0*longitude/10000000, 7);

    int32_t altitude = ubxDataStruct->hMSL; // Print the height above mean sea level
    Serial.print(F(" Height above MSL: "));
    Serial.print(altitude*0.001);
    Serial.println(F(" (m)"));
    myFile.print(F(" Height above MSL: "));
    myFile.print(altitude*0.001);
    myFile.println(F(" (m)"));
  // c write to file
  // fprintf(fptr, "Time: ");
  // //if (hms < 10) fprintf(fptr,"0"); // Print a leading zero if required
  // fprintf(fptr,"%02d:%02d:%02d.%04lu",ubxDataStruct->hour, ubxDataStruct->min, ubxDataStruct->sec, ubxDataStruct->iTOW % 1000);
  // fprintf(fptr, " Lat: %02.0000000f Long: %02.0000000f", 1.0*latitude/10000000, 1.0*longitude/10000000);
  // fprintf(fptr," Height above MSL: %f (m)", altitude*0.001);
  // fclose(fptr); 
}

void setup()
{
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  
  Wire.begin();

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
  else{

        tone(buzzer, 1000, 3000);    // Sounds buzzer for 3 seconds if GPS is connected
        Serial.println("GPS connected");
        delay(4500);
  }

 
  myGNSS.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)
  
  myGNSS.setNavigationFrequency(2); // Produce two solutions per second
  
  myGNSS.setAutoPVTcallbackPtr(&printPVTdata); // Enable automatic NAV PVT messages with callback to printPVTdata
}

void loop()
{
  myGNSS.checkUblox(); // Check for the arrival of new data and process it.
  myGNSS.checkCallbacks(); // Check if any callbacks are waiting to be processed.

  Serial.print(".");
  delay(50);

  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer
    long altitude = myGNSS.getAltitudeMSL();

    fix_type = myGNSS.getFixType();

    if(fix_type == 3 && number == 0)
    {
      tone(buzzer, 1000, 2000);               //checks if GPS has lock on satellites
      Serial.println("Lock on satellites");
      number++;
    }

    if(first_alt == 0)
    {
      first_alt = altitude;       //logs intial altitude
      }

    if(altitude - first_alt == 27432000)
    {
      Serial.println("At 90k ft \n");  //Prints message if at 90k ft
    }

    if(altitude - first_alt == 914400)
    {
      Serial.println("At 3k ft");   //Prints message if at 3k ft
    }
  }
}
