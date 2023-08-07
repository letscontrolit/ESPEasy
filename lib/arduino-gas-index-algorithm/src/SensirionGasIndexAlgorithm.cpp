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
#include "SensirionGasIndexAlgorithm.h"
#include <stdlib.h>

SensirionGasIndexAlgorithm::SensirionGasIndexAlgorithm(int32_t algorithm_type) {
    params = new GasIndexAlgorithmParams();
    GasIndexAlgorithm_init(params, algorithm_type);
}

SensirionGasIndexAlgorithm::SensirionGasIndexAlgorithm(
    int32_t algorithm_type, float sampling_interval) {
    params = new GasIndexAlgorithmParams();
    GasIndexAlgorithm_init_with_sampling_interval(params, algorithm_type,
                                                  sampling_interval);
}

SensirionGasIndexAlgorithm::~SensirionGasIndexAlgorithm() {
    delete params;
}

void SensirionGasIndexAlgorithm::set_tuning_parameters(
    int32_t index_offset, int32_t learning_time_offset_hours,
    int32_t learning_time_gain_hours, int32_t gating_max_duration_minutes,
    int32_t std_initial, int32_t gain_factor) {
    GasIndexAlgorithm_set_tuning_parameters(
        params, index_offset, learning_time_offset_hours,
        learning_time_gain_hours, gating_max_duration_minutes, std_initial,
        gain_factor);
}

void SensirionGasIndexAlgorithm::get_tuning_parameters(
    int32_t& index_offset, int32_t& learning_time_offset_hours,
    int32_t& learning_time_gain_hours, int32_t& gating_max_duration_minutes,
    int32_t& std_initial, int32_t& gain_factor) {
    GasIndexAlgorithm_get_tuning_parameters(
        params, &index_offset, &learning_time_offset_hours,
        &learning_time_gain_hours, &gating_max_duration_minutes, &std_initial,
        &gain_factor);
}

float SensirionGasIndexAlgorithm::get_sampling_interval() {
    float sampling_interval = 0;
    GasIndexAlgorithm_get_sampling_interval(params, &sampling_interval);
    return sampling_interval;
}

void SensirionGasIndexAlgorithm::reset() {
    GasIndexAlgorithm_reset(params);
}

int32_t SensirionGasIndexAlgorithm::process(int32_t sraw) {
    int32_t index_value = 0;
    GasIndexAlgorithm_process(params, sraw, &index_value);
    return index_value;
}
