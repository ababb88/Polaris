#include <stdarg.h>
#include <Wire.h>
#include <Serial.h>
#include <L3G.h>
#include <LSM303.h>
#include <LPS.h>

// Declare sensors
// TODO: add barometer
L3G gyro;
LSM303 accelmag;
LPS ps;

// housekeeping variables
long timer=0;   //general purpuse timer
unsigned int counter=0;
#define STATUS_LED 13 

void srl_printf(char *fmt, ... ){
        char buf[256]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 256, fmt, args);
        va_end (args);
        Serial.print(buf);
}

void setup()
{
    // set up board comm for debugging
    Serial.begin(115200);
    pinMode (STATUS_LED,OUTPUT);  // Status LED

    // begin I2C
    Wire.begin();

    Serial.println("Pololu MinIMU-10");
    digitalWrite(STATUS_LED,LOW);
    delay(1500);

    // INITIALIZE COMPASS
    accelmag.init();
    accelmag.enableDefault();
    // need all this?
    switch (accelmag.getDeviceType())
    {
      case LSM303::device_D:
        accelmag.writeReg(LSM303::CTRL2, 0x18); // 8 g full scale: AFS = 011
        break;
      case LSM303::device_DLHC:
        accelmag.writeReg(LSM303::CTRL_REG4_A, 0x28); // 8 g full scale: FS = 10; high resolution output mode
        break;
      default: // DLM, DLH
        accelmag.writeReg(LSM303::CTRL_REG4_A, 0x30); // 8 g full scale: FS = 11
    }

    // INITIALIZE GYRO
    gyro.init();
    gyro.writeReg(L3G_CTRL_REG4, 0x20); // 2000 dps full scale
    gyro.writeReg(L3G_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz

    // INITIALIZE BAROMETER
    ps.init();
    ps.enableDefault();

    delay(20);

    Serial.println("Sensors initialized");

    delay(2000);
    digitalWrite(STATUS_LED,HIGH);

    timer=millis();
    delay(20);
    counter=0;
}

void loop()
{
    if((millis()-timer)>=20)  // Main loop runs at 50Hz
    {
        counter++;
        timer=millis();
         
        // READ GYRO
        gyro.read();
        accelmag.readAcc();
        accelmag.readMag();
        float pressure = ps.readPressureMillibars(); // should store locally in LPS for consistency 

        if (counter > 5)  // print some stuff
        {
            counter=0;
            
            // note: gyro data is stored as a float, but actually contains integers.
            srl_printf("gyro: %10i, %10i, %10i", 
                    (int)gyro.g.x, (int)gyro.g.y, (int)gyro.g.z);
            srl_printf(" accel: %10i, %10i, %10i", 
                    accelmag.a.x, accelmag.a.y, accelmag.a.z);
            srl_printf(" compass: %10i, %10i, %10i", 
                    accelmag.m.x, accelmag.m.y, accelmag.m.z);
            //srl_printf(" pressure: %10.4f", pressure);

            // Serial.print("gyro:");
            // Serial.print(" "); Serial.print(gyro.g.x);
            // Serial.print(" "); Serial.print(gyro.g.y);
            // Serial.print(" "); Serial.print(gyro.g.z);

            // Serial.print(" ");
            // Serial.print("accel:");
            // Serial.print(" "); Serial.print(accelmag.a.x);
            // Serial.print(" "); Serial.print(accelmag.a.y);
            // Serial.print(" "); Serial.print(accelmag.a.z);
            // 
            // Serial.print(" ");
            // Serial.print("compass:");
            // Serial.print(" "); Serial.print(accelmag.m.x);
            // Serial.print(" "); Serial.print(accelmag.m.y);
            // Serial.print(" "); Serial.print(accelmag.m.z);

            Serial.print(" ");
            Serial.print("pressure:");
            Serial.print(" "); Serial.print(pressure);
            Serial.println();
        }
    }

}
