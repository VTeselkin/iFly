#include <SparkFunMPU9250-DMP.h>

MPU9250_DMP imu;
void MPU9250::setup()
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
    }

    void MPU9250::loop()
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
                printIMUData();
            }
        }
    }

    void printIMUData(void)
    {
        // After calling dmpUpdateFifo() the ax, gx, mx, etc. values
        // are all updated.
        // Quaternion values are, by default, stored in Q30 long
        // format. calcQuat turns them into a float between -1 and 1
        float q0 = imu.calcQuat(imu.qw);
        float q1 = imu.calcQuat(imu.qx);
        float q2 = imu.calcQuat(imu.qy);
        float q3 = imu.calcQuat(imu.qz);

        SerialPort.println("Q: " + String(q0, 4) + ", " +
                           String(q1, 4) + ", " + String(q2, 4) +
                           ", " + String(q3, 4));
                           //roll - крен pitch - наклон yaw - рысканье
        SerialPort.println("R/P/Y: " + String(imu.roll) + ", " + String(imu.pitch) + ", " + String(imu.yaw));
        SerialPort.println("Time: " + String(imu.time) + " ms");
        SerialPort.println();
    }
}