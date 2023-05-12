/*
 * Copyright (c) 2021, Sensirion AG
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

#ifndef SENSIRIONGASINDEXALGORITHM_H
#define SENSIRIONGASINDEXALGORITHM_H

extern "C" {
#include "algorithm/sensirion_gas_index_algorithm.h"
}

class SensirionGasIndexAlgorithm {

  public:
    SensirionGasIndexAlgorithm(const SensirionGasIndexAlgorithm&) = delete;
    SensirionGasIndexAlgorithm&
    operator=(const SensirionGasIndexAlgorithm&) = delete;

    explicit SensirionGasIndexAlgorithm(int32_t algorithm_type);
    explicit SensirionGasIndexAlgorithm(int32_t algorithm_type,
                                        float sampling_interval);
    ~SensirionGasIndexAlgorithm();

    /**
     * Set parameters to customize the gas index algorithm. Call this once after
     * creating a new instance of GasIndexAlgorithm if desired.
     * Otherwise, the default values will be used.
     *
     * @param index_offset                Gas index representing typical
     * (average) conditions. Range 1..250, default 100 for VOC and 1 for NOx
     * @param learning_time_offset_hours  Time constant of long-term estimator
     * for offset. Past events will be forgotten after about twice the learning
     * time. Range 1..1000 [hours], default 12 [hours]
     * @param learning_time_gain_hours    Time constant of long-term estimator
     * for gain. Past events will be forgotten after about twice the learning
     * time. Range 1..1000 [hours], default 12 [hours] NOTE: This value is not
     * relevant for NOx algorithm type
     * @param gating_max_duration_minutes Maximum duration of gating (freeze of
     *                                    estimator during high gas index
     * signal). 0 (no gating) or range 1..3000 [minutes], default 180 [minutes]
     * for VOC and 720 [minutes] for NOx
     * @param std_initial                 Initial estimate for standard
     * deviation. Lower value boosts events during initial learning period, but
     * may result in larger device-to-device variations. Range 10..5000, default
     * 50 NOTE: This value is not relevant for NOx algorithm type
     * @param gain_factor                 Factor used to scale applied gain
     * value when calculating gas index. Range 1..1000, default 230
     */
    void set_tuning_parameters(int32_t index_offset,
                               int32_t learning_time_offset_hours,
                               int32_t learning_time_gain_hours,
                               int32_t gating_max_duration_minutes,
                               int32_t std_initial, int32_t gain_factor);

    /**
     * Get current parameters to customize the gas index algorithm.
     * Refer to set_tuning_parameters() for description of the
     * parameters.
     */
    void get_tuning_parameters(int32_t& index_offset,
                               int32_t& learning_time_offset_hours,
                               int32_t& learning_time_gain_hours,
                               int32_t& gating_max_duration_minutes,
                               int32_t& std_initial, int32_t& gain_factor);

    /**
     * Get the sampling interval used by the algorithm
     */
    float get_sampling_interval();

    /**
     * Calculate the gas index value from the raw sensor value.
     *
     * @param sraw        Raw value from the SGP4x sensor
     * @return            Calculated gas index value from the raw sensor value.
     * Zero during initial blackout period and 1..500 afterwards
     */
    int32_t process(int32_t sraw);

    /**
     * Reset the internal states of the gas index algorithm. Previously set
     * tuning parameters are preserved. Call this when resuming operation after
     * a measurement interruption.
     */
    void reset();

    static const int32_t ALGORITHM_TYPE_VOC =
        GasIndexAlgorithm_ALGORITHM_TYPE_VOC;
    static const int32_t ALGORITHM_TYPE_NOX =
        GasIndexAlgorithm_ALGORITHM_TYPE_NOX;

  protected:
    GasIndexAlgorithmParams* params = nullptr;
};

#endif /* SENSIRIONGASINDEXALGORITHM_H */
