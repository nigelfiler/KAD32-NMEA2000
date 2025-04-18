// Uses NMEA2000 library from Timo Lappalainen
// https://github.com/ttlappalainen 


#include <Arduino.h>
#include <Wire.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#include <U8g2lib.h>
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object.
#include <N2kMessages.h>
#include <Ewma.h>

Ewma adcFilter1(0.2);

U8G2_SH1107_SEEED_128X128_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

const int PORT_PRESSURE_PIN = A2;
const int STARBOARD_PRESSURE_PIN = A3;

const int PORT_TEMPERATURE_PIN = A0;
const int STARBOARD_TEMPERATURE_PIN = A1;

double portSensorValue = 0;
double starboardSensorValue = 0;

double portPsiValue = 0.0;
double starboardPsiValue = 0.0;

double pascalValue = 0.0;

double portTemperature = 0;
double starboardTemperature = 0;

double maxPortTemperature = 0;
double maxStarboardTemperature = 0;

double portAlternatorVoltage = 0;
double starboardAlternatorVoltage = 0;

int VoP;              // Integer value of voltage reading Port
int VoS;              // Integer value of voltage reading Starboard

double R = 220.0; // Changed from 1000 to 220 to improve accuracy at higher engine temp.
double num = 0;

// Volvo equivalent earth isolated temperature sender
/*
M18 x1.5
1c = 1795 Ohm
21c = 601 Ohm
75c = 83 Ohm
double c1 = 1.479761336E-03; //Steinhart-Hart Equation coefficients.
double c2 = 3.289077692E-04;
double c3 = -7.049734930E-07;
*/ 


double c1 = 1.479761336E-03;
double c2 = 3.289077692E-04;
double c3 = -7.049734930E-07;

double logRtp, Rtp, logRts, Rts;

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = { 130310L, 130311L, 130312L, 0 };

// Define schedulers for messages.
tN2kSyncScheduler PortEngineFastScheduler(false, 100, 300);
tN2kSyncScheduler StarboardEngineFastScheduler(false, 100, 310);
tN2kSyncScheduler PortEngineSlowScheduler(false, 1000, 320);
tN2kSyncScheduler StarboardEngineSlowScheduler(false, 1000, 330); //1000 = 1 second

void OnN2kOpen() {
  // Start schedulers now.
  PortEngineFastScheduler.UpdateNextTime();
  StarboardEngineFastScheduler.UpdateNextTime();
  PortEngineSlowScheduler.UpdateNextTime();
  StarboardEngineSlowScheduler.UpdateNextTime();
}

void setup() {
  analogReference(AR_DEFAULT);
  pinMode(2, INPUT);  //Port RPM
  pinMode(3, INPUT);  //Starboard RPM

  // Set Product information
  NMEA2000.SetProductInformation("V4.00", 100, "KAD32 to NMEA2000", "EngineBoostNMEA2k_v4", "V4.0");//Model serial code, Product code,  Model ID, Software version code, Model version
  
  // Set device information
  NMEA2000.SetDeviceInformation(112233, 130, 75, 2040);  //ID, function, Class, Manufacturer.

  Serial.begin(115200);
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text);  
  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, 22);
  NMEA2000.EnableForward(false);                      // Disable all msg forwarding to USB (=Serial)
  NMEA2000.ExtendTransmitMessages(TransmitMessages);  // Here we tell library, which PGNs we transmit
  NMEA2000.SetOnOpen(OnN2kOpen);                      // Define OnOpen call back. This will be called, when CAN is open and system starts address claiming.
  NMEA2000.Open();
  readPortTemp();
  readStarboardTemp();
  u8g2.begin();
}


void loop() {
  SendN2kEngines();
  NMEA2000.ParseMessages();
}


void SendN2kEngines() {
  tN2kMsg N2kMsg;

  if (PortEngineFastScheduler.IsTime()) {
    PortEngineFastScheduler.UpdateNextTime();
    SetN2kEngineParamRapid(N2kMsg, 0, 0, ReadPortBoostPressure(), 0);
    NMEA2000.SendMsg(N2kMsg);
  }

  if (StarboardEngineFastScheduler.IsTime()) {
    StarboardEngineFastScheduler.UpdateNextTime();
    SetN2kEngineParamRapid(N2kMsg, 1, 0, ReadStarboardBoostPressure(), 0);
    NMEA2000.SendMsg(N2kMsg);
  }

  if (PortEngineSlowScheduler.IsTime()) {
    PortEngineSlowScheduler.UpdateNextTime();
    SetN2kEngineDynamicParam(N2kMsg, 0, 0, 0, readPortTemp(), readPortAlternatorVoltage(), 0, 0, 0, 0, 0, 0, 0, 0);
    NMEA2000.SendMsg(N2kMsg);
  }

  if (StarboardEngineSlowScheduler.IsTime()) {
    StarboardEngineSlowScheduler.UpdateNextTime();
    SetN2kEngineDynamicParam(N2kMsg, 1, 0, 0, readStarboardTemp(), readStarboardAlternatorVoltage(), 0, 0, 0, 0, 0, 0, 0, 0);
    NMEA2000.SendMsg(N2kMsg);
  }
}


double ReadPortBoostPressure() {
  portSensorValue = analogRead(PORT_PRESSURE_PIN); // Stabilisation read
  delay(5); //Remove blocking delay
  portSensorValue = analogRead(PORT_PRESSURE_PIN);
  portPsiValue = map(portSensorValue, 47, 455, 0, 30);
  UpdateDisplay(portPsiValue, starboardPsiValue, portTemperature, starboardTemperature);
  return portPsiValue * 6894.76;  // Convert PSI to Pascal. Charplotter requires Pascal and converts to PSI
 
}

double ReadStarboardBoostPressure() {
  starboardSensorValue = analogRead(STARBOARD_PRESSURE_PIN); // Stabilisation read
  delay(5); //Remove blocking delay
  starboardSensorValue = analogRead(STARBOARD_PRESSURE_PIN);
  starboardPsiValue = map(starboardSensorValue, 47, 455, 0, 30);
  UpdateDisplay(portPsiValue, starboardPsiValue, portTemperature, starboardTemperature);
  return starboardPsiValue * 6894.76;  // Convert PSI to Pascal. Charplotter requires Pascal and converts to PSI
}

double readPortAlternatorVoltage() {
  portAlternatorVoltage = 0;
  return portAlternatorVoltage;  // Hardcoded value until code is written. Not tested the hardware, so currently unused
}

double readStarboardAlternatorVoltage() {
  starboardAlternatorVoltage = 0;
  return starboardAlternatorVoltage;  // Hardcoded value until code is written.  Not tested the hardware, so currently unused
}

double readPortTemp() {

  VoP = 1024 - analogRead(PORT_TEMPERATURE_PIN);  // Read thermister voltage
  //Serial.println(VoP);
  double VoP_filtered = adcFilter1.filter(VoP);
  Rtp = R * (1024.0 / (double)VoP_filtered - 1.0);           // Calculate thermister resistance
  logRtp = log(Rtp);
  portTemperature = (1.0 / (c1 + c2 * logRtp + c3 * logRtp * logRtp * logRtp));  // Apply Steinhart-Hart equation.

  if (portTemperature > maxPortTemperature) {
    maxPortTemperature = portTemperature;
  }

  double roundedPortTemperature = roundToNearestCustom(portTemperature);
   Serial.print("Port: ");
   Serial.println(roundedPortTemperature-273.15);
  return roundedPortTemperature;  // value is in Kelvin
}

double readStarboardTemp() {

  VoS = 1024 - analogRead(STARBOARD_TEMPERATURE_PIN);  // Read thermister voltage
  double VoS_filtered = adcFilter1.filter(VoS);
  Rts = R * (1024.0 / (double)VoS_filtered - 1.0);                // Calculate thermister resistance
  logRts = log(Rts);
  starboardTemperature = (1.0 / (c1 + c2 * logRts + c3 * logRts * logRts * logRts));  // Apply Steinhart-Hart equation.

  if (starboardTemperature > maxStarboardTemperature) {
    maxStarboardTemperature = starboardTemperature;
  }

  double roundedStarboardTemperature = roundToNearestCustom(starboardTemperature);
  Serial.print("Starboard: ");
  Serial.println(roundedStarboardTemperature-273.15);
  return roundedStarboardTemperature;  // Value is in Kelvin and rounded to xx.15 or xx.65 to provide temperature in Centigrade on display in 1/2 degree values
}

void UpdateDisplay(double portPsiValue, double starboardPsiValue, double roundedPortTemperature, double roundedStarboardTemperature) {
  // Can use a display to show current and max temperature.
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB10_tr);
  u8g2.setCursor(5, 15);
  u8g2.print("PORT:  ");
  u8g2.print (int(portPsiValue));
  u8g2.print(" psi");
  u8g2.setCursor(5, 35);
  u8g2.print("Cur Temp: ");
  //u8g2.print(roundedPortTemperature-273.15);
  u8g2.print(int(roundedPortTemperature-273.15));
  u8g2.print("c");
  u8g2.setCursor(5, 55);
  u8g2.print("Max Temp: ");
 // u8g2.print(maxPortTemperature-273.15);
  u8g2.print(int(maxPortTemperature-273.15));
  u8g2.print("c");
  
  u8g2.setCursor(5, 65);
  u8g2.print("________________");

  u8g2.setFont(u8g2_font_helvB10_tr);
  u8g2.setCursor(5, 85);
  u8g2.print("STBD:  ");
  u8g2.print (int(starboardPsiValue));
  u8g2.print(" psi");
  u8g2.setCursor(5, 105);
  u8g2.print("Cur Temp: ");
  // u8g2.print(roundedStarboardTemperature-273.15);
  u8g2.print(int(roundedStarboardTemperature-273.15));
  u8g2.print("c");
  u8g2.setCursor(5, 125);
  u8g2.print("Max Temp: ");
  u8g2.print(int(maxStarboardTemperature-273.15));
  //  u8g2.print(maxStarboardTemperature-273.15);
  u8g2.print("c");
  u8g2.sendBuffer();

}

double readPortRPM() {
// Hardware not working yet.
  int highTime;   //integer for storing high time
  int lowTime;    //integer for storing low time
  double period;  // integer for storing period
  double freq;    //storing frequency
  int RPM;
  highTime = pulseIn(2, HIGH);  //read high time
  lowTime = pulseIn(2, LOW);    //read low time
  period = highTime + lowTime;  // Period = Ton + Toff
  freq = 1000000 / period;      //getting frequency with totalTime is in Micro seconds
  RPM = (freq / 30) * 60;
  Serial.print("Freq: ");
  Serial.print(freq);
  Serial.println();
  Serial.print("RPM: ");
  Serial.print(RPM);
  Serial.println();
  // delay(0);
  return RPM;
}

double readStarboardRPM() {
//Hardware not working yet.
  int highTime;  //integer for storing high time
  int lowTime;   //integer for storing low time
  double period;  // integer for storing period
  double freq;    //storing frequency
  double RPM;
  highTime = pulseIn(3, HIGH);  //read high time
  lowTime = pulseIn(3, LOW);    //read low time
  period = highTime + lowTime;  // Period = Ton + Toff
  freq = 1000000 / period;      //getting frequency with totalTime is in Micro seconds
  RPM = (freq / 30) * 60;
   Serial.println(freq);
   Serial.println(RPM);
    return RPM;
}

//Rounding to return temerature of xx.0 or xx.5 - makes it look nicer on the display.
double roundToNearestCustom(double num) {
    // Separate the whole part and fractional part of the number
    int wholePart = static_cast<int>(num);  // Extract the whole part
    double fractionPart = num - wholePart;   // Extract the fractional part
    
    // Check the fractional part and round accordingly
    if (fractionPart < 0.4) {
        return wholePart + 0.15;  // Round to .15 if fractional part is less than 0.4
    } else {
        return wholePart + 0.65;  // Round to .65 otherwise
    }
}
