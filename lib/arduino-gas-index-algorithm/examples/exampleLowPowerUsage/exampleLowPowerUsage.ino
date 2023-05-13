/*
 * Copyright (c) 2022, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <Wire.h>

#include <Arduino.h>
#include <SensirionI2CSgp40.h>
#include <SensirionI2CSht4x.h>
#include <VOCGasIndexAlgorithm.h>

SensirionI2CSht4x sht4x;
SensirionI2CSgp40 sgp40;

// Sampling interval in seconds
// This code uses a fixed heating pulse of ca. 200 ms for the measurement and
// thus, the sampling interval defines the duty cycle
float sampling_interval = 1.f;

VOCGasIndexAlgorithm voc_algorithm(sampling_interval);

char errorMessage[32];

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    Wire.begin();

    sht4x.begin(Wire);
    sgp40.begin(Wire);

    delay(1000);  // needed on some Arduino boards in order to have Serial ready

    Serial.print("Sampling interval (sec):\t");
    Serial.println(voc_algorithm.get_sampling_interval());
    Serial.println("");
}

void sgp40MeasureRawSignalLowPower(uint16_t compensationRh,
                                   uint16_t compensationT, uint16_t* error) {
    uint16_t srawVoc = 0;
    int32_t voc_index = 0;
    // Request a first measurement to heat up the plate (ignoring the result)
    *error = sgp40.measureRawSignal(compensationRh, compensationT, srawVoc);
    if (*error) {
        return;
    }

    // Delaying 170 msec to let the plate heat up.
    // Keeping in mind that the measure command already include a 30ms delay
    delay(140);

    // Request the measurement values
    *error = sgp40.measureRawSignal(compensationRh, compensationT, srawVoc);
    if (*error) {
        return;
    }

    Serial.print("\t");
    Serial.print("srawVOC: ");
    Serial.println(srawVoc);

    // Turn heater off
    *error = sgp40.turnHeaterOff();
    if (*error) {
        return;
    }

    // Process raw signals by Gas Index Algorithm to get the VOC index values
    voc_index = voc_algorithm.process(srawVoc);
    Serial.print("\t");
    Serial.print("VOC Index: ");
    Serial.println(voc_index);
}

void getCompensationValuesFromSHT4x(uint16_t* compensationRh,
                                    uint16_t* compensationT, uint16_t* error) {
    float humidity = 0;     // %RH
    float temperature = 0;  // degreeC
    *error = sht4x.measureHighPrecision(temperature, humidity);
    if (*error) {
        return;
    }
    Serial.print("T:");
    Serial.print(temperature);
    Serial.print("\t");
    Serial.print("RH:");
    Serial.print(humidity);

    // convert temperature and humidity to ticks as defined by SGP40
    // interface
    // NOTE: in case you read RH and T raw signals check out the
    // ticks specification in the datasheet, as they can be different for
    // different sensors
    *compensationT = static_cast<uint16_t>((temperature + 45) * 65535 / 175);
    *compensationRh = static_cast<uint16_t>(humidity * 65535 / 100);
}

void loop() {
    uint16_t error;
    uint16_t compensationRh =
        0x8000;  // initialized to default value in ticks as defined by SGP40
    uint16_t compensationT =
        0x6666;  // initialized to default value in ticks as defined by SGP40

    // 1. Sleep: We need the delay to match the desired sampling interval
    // In low power mode, the SGP40 takes 200ms to acquire values.
    // SHT4X also includes a delay of 10ms
    delay(int(sampling_interval) * 1000 - 210);

    // 2. Measure temperature and humidity for SGP internal compensation
    getCompensationValuesFromSHT4x(&compensationRh, &compensationT, &error);
    if (error) {
        Serial.print(
            "SHT4x - Error trying to execute measureHighPrecision(): ");
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.println(errorMessage);
        Serial.println("Fallback to use default values for humidity and "
                       "temperature compensation for SGP40");
    }

    // 3. Measure SGP40 signals using low power mode
    sgp40MeasureRawSignalLowPower(compensationRh, compensationT, &error);
    if (error) {
        Serial.print(
            "SGP40 - Error trying to acquire data in low power mode: ");
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.println(errorMessage);
    }
}
