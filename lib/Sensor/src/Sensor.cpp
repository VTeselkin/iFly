#include <Arduino.h>
#include <Sensor.h>
#include <SparkFunMPU9250-DMP.h>
#include <Adafruit_BMP280.h>
#include <SimpleKalmanFilter.h>

MPU9250_DMP imu;
Adafruit_BMP280 bmp;
SimpleKalmanFilter pressureKalmanFilter(1, 1, 0.01);

float base_pressure = 1013.25F;
const long SERIAL_REFRESH_TIME = 100;
long refresh_time;

void Sensor::setup(void)
{
    // Call imu.begin() to verify communication with and
    // initialize the MPU-9250 to it's default values.
    // Most functions return an error code - INV_SUCCESS (0)
    // indicates the IMU was present and successfully set up
    if (imu.begin() != INV_SUCCESS)
    {
        while (1)
        {
            Serial.println("Unable to communicate with MPU-9250");
            Serial.println("Check connections, and try again.");
            Serial.println();
            delay(5000);
        }
        // инициализируем датчик давления по адресу 0x76
        bmp.begin(0x76);

        // Use setSensors to turn on or off MPU-9250 sensors.
        // Any of the following defines can be combined:
        // INV_XYZ_GYRO, INV_XYZ_ACCEL, INV_XYZ_COMPASS,
        // INV_X_GYRO, INV_Y_GYRO, or INV_Z_GYRO
        // Enable all sensors:
        imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);

        // Use setGyroFSR() and setAccelFSR() to configure the
        // gyroscope and accelerometer full scale ranges.
        // Gyro options are +/- 250, 500, 1000, or 2000 dps
        imu.setGyroFSR(2000); // Set gyro to 2000 dps

        // Accel options are +/- 2, 4, 8, or 16 g
        imu.setAccelFSR(2); // Set accel to +/-2g

        // Note: the MPU-9250's magnetometer FSR is set at
        // +/- 4912 uT (micro-tesla's)

        // setLPF() can be used to set the digital low-pass filter
        // of the accelerometer and gyroscope.
        // Can be any of the following: 188, 98, 42, 20, 10, 5
        // (values are in Hz).
        imu.setLPF(5); // Set LPF corner frequency to 5Hz

        // The sample rate of the accel/gyro can be set using
        // setSampleRate. Acceptable values range from 4Hz to 1kHz
        imu.setSampleRate(10); // Set sample rate to 10Hz

        // Likewise, the compass (magnetometer) sample rate can be
        // set using the setCompassSampleRate() function.
        // This value can range between: 1-100Hz
        imu.setCompassSampleRate(10);

        // Set mag rate to 10Hz
        // DMP_FEATURE_LP_QUAT can also be used. It uses the
        // accelerometer in low-power mode to estimate quat's.
        // DMP_FEATURE_LP_QUAT and 6X_LP_QUAT are mutually exclusive
        imu.dmpBegin(DMP_FEATURE_LP_QUAT | DMP_FEATURE_GYRO_CAL, 10); // Enable 6-axis quat  Use gyro calibration
        base_pressure = bmp.readPressure() / 100.0F;
    }
}
void Sensor::loop(DataFly data)
{

    // Check for new data in the FIFO
    if (imu.fifoAvailable())
    {
        // Use dmpUpdateFifo to update the ax, gx, mx, etc. values
        if (imu.dmpUpdateFifo() == INV_SUCCESS)
        {
            // computeEulerAngles can be used -- after updating the
            // quaternion values -- to estimate roll, pitch, and yaw
            imu.computeEulerAngles();
            // After calling dmpUpdateFifo() the ax, gx, mx, etc. values
            // are all updated.
            // Quaternion values are, by default, stored in Q30 long
            // format. calcQuat turns them into a float between -1 and 1
            data.Q1 = imu.calcQuat(imu.qw);
            data.Q2 = imu.calcQuat(imu.qx);
            data.Q3 = imu.calcQuat(imu.qy);
            data.Q4 = imu.calcQuat(imu.qz);

            data.Pitch = imu.pitch;
            data.Roll = imu.roll;
            data.Yaw = imu.yaw;

            Serial.println("Q: " + String(data.Q1, 4) + ", " + String(data.Q2, 4) + ", " + String(data.Q3, 4) + ", " + String(data.Q4, 4));
            Serial.println("R/P/Y: " + String(data.Roll) + ", " + String(data.Pitch) + ", " + String(data.Yaw));
            Serial.println("Time: " + String(imu.time) + " ms");
            Serial.println();
        }
    }
    getAltitude(data);
}

void Sensor::getAltitude(DataFly data)
{
    float pressure = bmp.readPressure() / 100.0F; // давление в гектопаскалях
    float altitude = bmp.seaLevelForAltitude(pressure, base_pressure);
    float estimated_altitude = pressureKalmanFilter.updateEstimate(altitude);
    if (millis() > refresh_time)
    {
        Serial.print("Altitude = ");
        Serial.print(altitude, 6);
        Serial.print(" , ");
        Serial.print("Estimated altitude = ");
        Serial.print(estimated_altitude, 6);
        Serial.println();
        data.Altitude = estimated_altitude;
        refresh_time = millis() + SERIAL_REFRESH_TIME;
    }
}