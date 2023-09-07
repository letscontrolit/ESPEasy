/**
* Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file       bmp3.c
* @date       2020-07-20
* @version    v2.0.1
*
*/

/*! @file bmp3.c
 * @brief Sensor driver for BMP3 sensor */
#include "bmp3.h"

/***************** Static function declarations ******************************/

/*!
 * @brief This internal API reads the calibration data from the sensor, parse
 * it then compensates it and store in the device structure.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t get_calib_data(struct bmp3_dev *dev);

/*!
 * @brief This internal API is used to parse the calibration data, compensates
 * it and store it in device structure.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 * @param[out] reg_data : Contains calibration data to be parsed.
 *
 */
static void parse_calib_data(const uint8_t *reg_data, struct bmp3_dev *dev);

/*!
 * @brief This internal API gets the over sampling, ODR and filter settings
 * from the sensor.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t get_odr_filter_settings(struct bmp3_dev *dev);

/*!
 * @brief This internal API is used to parse the pressure and temperature data
 * and store it in the bmp3_uncomp_data structure instance.
 *
 * @param[in] reg_data : Contains the register data which needs to be parsed.
 * @param[out] uncomp_data : Contains the uncompensated press and temp data.
 *
 */
static void parse_sensor_data(const uint8_t *reg_data, struct bmp3_uncomp_data *uncomp_data);

/*!
 * @brief This internal API is used to compensate the pressure or temperature
 * or both the data according to the component selected by the user.
 *
 * @param[in] sensor_comp : Used to select pressure or temperature.
 * @param[in] uncomp_data : Contains the uncompensated pressure and
 * temperature data.
 * @param[out] comp_data : Contains the compensated pressure and
 * temperature data.
 * @param[in] calib_data : Pointer to the calibration data structure.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t compensate_data(uint8_t sensor_comp,
                              const struct bmp3_uncomp_data *uncomp_data,
                              struct bmp3_data *comp_data,
                              struct bmp3_calib_data *calib_data);

#ifdef BMP3_DOUBLE_PRECISION_COMPENSATION

/*!
 * @brief This internal API is used to compensate the raw temperature data and
 * return the compensated temperature data.
 *
 * @param[in] uncomp_data : Contains the uncompensated temperature data.
 * @param[in] calib_data : Pointer to calibration data structure.
 *
 * @return Compensated temperature data.
 * @retval Compensated temperature data in double.
 */
static double compensate_temperature(const struct bmp3_uncomp_data *uncomp_data, struct bmp3_calib_data *calib_data);

/*!
 * @brief This internal API is used to compensate the pressure data and return
 * the compensated pressure data.
 *
 * @param[in] uncomp_data : Contains the uncompensated pressure data.
 * @param[in] calib_data : Pointer to the calibration data structure.
 *
 * @return Compensated pressure data.
 * @retval Compensated pressure data in double.
 */
static double compensate_pressure(const struct bmp3_uncomp_data *uncomp_data, const struct bmp3_calib_data *calib_data);

/*!
 * @brief This internal API is used to calculate the power functionality for
 * double precision floating point values.
 *
 * @param[in] base : Contains the base value.
 * @param[in] power : Contains the power value.
 *
 * @return Output of power function.
 * @retval Calculated power function output in float.
 */
static float pow_bmp3(double base, uint8_t power);

#else

/*!
 * @brief This internal API is used to compensate the raw temperature data and
 * return the compensated temperature data in integer data type.
 *
 * @param[in] uncomp_data : Contains the uncompensated temperature data.
 * @param[in] calib_data : Pointer to calibration data structure.
 *
 * @return Compensated temperature data.
 * @retval Compensated temperature data in integer.
 */
static int64_t compensate_temperature(const struct bmp3_uncomp_data *uncomp_data, struct bmp3_calib_data *calib_data);

/*!
 * @brief This internal API is used to compensate the pressure data and return
 * the compensated pressure data in integer data type.
 *
 * @param[in] uncomp_data : Contains the uncompensated pressure data.
 * @param[in] calib_data : Pointer to the calibration data structure.
 *
 * @return Compensated pressure data.
 * @retval Compensated pressure data in integer.
 */
static uint64_t compensate_pressure(const struct bmp3_uncomp_data *uncomp_data,
                                    const struct bmp3_calib_data *calib_data);

/*!
 * @brief This internal API is used to calculate the power functionality.
 *
 * @param[in] base : Contains the base value.
 * @param[in] power : Contains the power value.
 *
 * @return Output of power function.
 * @retval Calculated power function output in integer.
 */
static uint32_t pow_bmp3(uint8_t base, uint8_t power);

#endif /* BMP3_DOUBLE_PRECISION_COMPENSATION */

/*!
 * @brief This internal API is used to identify the settings which the user
 * wants to modify in the sensor.
 *
 * @param[in] sub_settings : Contains the settings subset to identify particular
 * group of settings which the user is interested to change.
 * @param[in] settings : Contains the user specified settings.
 *
 * @return Indicates whether user is interested to modify the settings which
 * are related to sub_settings.
 * @retval True -> User wants to modify this group of settings
 * @retval False -> User does not want to modify this group of settings
 */
static uint8_t are_settings_changed(uint32_t sub_settings, uint32_t settings);

/*!
 * @brief This internal API interleaves the register address between the
 * register data buffer for burst write operation.
 *
 * @param[in] reg_addr : Contains the register address array.
 * @param[out] temp_buff : Contains the temporary buffer to store the
 * register data and register address.
 * @param[in] reg_data : Contains the register data to be written in the
 * temporary buffer.
 * @param[in] len : No of bytes of data to be written for burst write.
 *
 */
static void interleave_reg_addr(const uint8_t *reg_addr, uint8_t *temp_buff, const uint8_t *reg_data, uint32_t len);

/*!
 * @brief This internal API sets the pressure enable and
 * temperature enable settings of the sensor.
 *
 * @param[in] desired_settings : Contains the settings which user wants to
 * change.
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t set_pwr_ctrl_settings(uint32_t desired_settings, struct bmp3_dev *dev);

/*!
 * @brief This internal API sets the over sampling, ODR and filter settings of
 * the sensor based on the settings selected by the user.
 *
 * @param[in] desired_settings : Variable used to select the settings which
 * are to be set.
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t set_odr_filter_settings(uint32_t desired_settings, struct bmp3_dev *dev);

/*!
 * @brief This internal API sets the interrupt control (output mode, level,
 * latch and data ready) settings of the sensor based on the settings
 * selected by the user.
 *
 * @param[in] desired_settings : Variable used to select the settings which
 * are to be set.
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t set_int_ctrl_settings(uint32_t desired_settings, struct bmp3_dev *dev);

/*!
 * @brief This internal API sets the advance (i2c_wdt_en, i2c_wdt_sel)
 * settings of the sensor based on the settings selected by the user.
 *
 * @param[in] desired_settings : Variable used to select the settings which
 * are to be set.
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t set_advance_settings(uint32_t desired_settings, struct bmp3_dev *dev);

/*!
 * @brief This internal API fills the register address and register data of the
 * the over sampling settings for burst write operation.
 *
 * @param[in] desired_settings : Variable which specifies the settings which
 * are to be set in the sensor.
 * @param[out] addr : To store the address to fill in register buffer.
 * @param[out] reg_data : To store the osr register data.
 * @param[out] len : To store the len for burst write.
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 */
static void fill_osr_data(uint32_t desired_settings,
                          uint8_t *addr,
                          uint8_t *reg_data,
                          uint8_t *len,
                          const struct bmp3_dev *dev);

/*!
 * @brief This internal API fills the register address and register data of the
 * the ODR settings for burst write operation.
 *
 * @param[out] addr : To store the address to fill in register buffer.
 * @param[out] reg_data : To store the register data to set the odr data.
 * @param[out] len : To store the len for burst write.
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 */
static void fill_odr_data(uint8_t *addr, uint8_t *reg_data, uint8_t *len, struct bmp3_dev *dev);

/*!
 * @brief This internal API fills the register address and register data of the
 * the filter settings for burst write operation.
 *
 * @param[out] addr : To store the address to fill in register buffer.
 * @param[out] reg_data : To store the register data to set the filter.
 * @param[out] len : To store the len for burst write.
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 */
static void fill_filter_data(uint8_t *addr, uint8_t *reg_data, uint8_t *len, const struct bmp3_dev *dev);

/*!
 * @brief This internal API is used to validate the device pointer for
 * null conditions.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t null_ptr_check(const struct bmp3_dev *dev);

/*!
 * @brief This internal API parse the power control(power mode, pressure enable
 * and temperature enable), over sampling, ODR, filter and interrupt control
 * settings and store in the device structure.
 *
 * @param[in] reg_data : Register data to be parsed.
 * @param[out] dev : Structure instance of bmp3_dev.
 */
static void parse_sett_data(const uint8_t *reg_data, struct bmp3_dev *dev);

/*!
 * @brief This internal API parse the power control(power mode, pressure enable
 * and temperature enable) settings and store in the device structure.
 *
 * @param[in] reg_data : Pointer variable which stores the register data to
 * parse.
 * @param[out] settings : Structure instance of bmp3_settings.
 */
static void parse_pwr_ctrl_settings(const uint8_t *reg_data, struct bmp3_settings *settings);

/*!
 * @brief This internal API parse the over sampling, ODR and filter
 * settings and store in the device structure.
 *
 * @param[in] reg_data : Pointer variable which stores the register data to
 * parse.
 * @param[out] settings : Structure instance of bmp3_odr_filter_settings.
 */
static void parse_odr_filter_settings(const uint8_t *reg_data, struct bmp3_odr_filter_settings *settings);

/*!
 * @brief This internal API parse the interrupt control(output mode, level,
 * latch and data ready) settings and store in the device structure.
 *
 * @param[in] reg_data : Pointer variable which stores the register data to
 * parse.
 * @param[out] settings : Structure instance of bmp3_int_ctrl_settings.
 */
static void parse_int_ctrl_settings(const uint8_t *reg_data, struct bmp3_int_ctrl_settings *settings);

/*!
 * @brief This internal API parse the advance (i2c_wdt_en, i2c_wdt_sel)
 * settings and store in the device structure.
 *
 * @param[in] reg_data : Pointer variable which stores the register data to
 * parse.
 * @param[out] settings : Structure instance of bmp3_adv_settings.
 */
static void parse_advance_settings(const uint8_t *reg_data, struct bmp3_adv_settings *settings);

/*!
 * @brief This internal API validate the normal mode settings of the sensor.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t validate_normal_mode_settings(struct bmp3_dev *dev);

/*!
 * @brief This internal API validate the over sampling, ODR settings of the
 * sensor.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Indicates whether ODR and OSR settings are valid or not.
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t validate_osr_and_odr_settings(const struct bmp3_dev *dev);

/*!
 * @brief This internal API calculates the pressure measurement duration of the
 * sensor.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Pressure measurement time
 * @retval Pressure measurement time in microseconds
 */
static uint32_t calculate_press_meas_time(const struct bmp3_dev *dev);

/*!
 * @brief This internal API calculates the temperature measurement duration of
 * the sensor.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Temperature measurement time
 * @retval Temperature measurement time in microseconds
 */
static uint32_t calculate_temp_meas_time(const struct bmp3_dev *dev);

/*!
 * @brief This internal API checks whether the measurement time and ODR duration
 * of the sensor are proper.
 *
 * @param[in] meas_t : Pressure and temperature measurement time in microseconds.
 * @param[in] odr_duration : Duration in microseconds corresponding to the ODR
 *                           value.
 *
 * @return Indicates whether ODR and OSR settings are valid or not.
 * @retval 0 -> Success
 * @retval >0 -> Warning
 */
static int8_t verify_meas_time_and_odr_duration(uint32_t meas_t, uint32_t odr_duration);

/*!
 * @brief This internal API puts the device to sleep mode.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t put_device_to_sleep(struct bmp3_dev *dev);

/*!
 * @brief This internal API sets the normal mode in the sensor.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t set_normal_mode(struct bmp3_dev *dev);

/*!
 * @brief This internal API writes the power mode in the sensor.
 *
 * @param[in] dev : Structure instance of bmp3_dev.
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t write_power_mode(struct bmp3_dev *dev);

/*!
 * @brief This internal API fills the fifo_config_1(fifo_mode,
 * fifo_stop_on_full, fifo_time_en, fifo_press_en, fifo_temp_en) settings in the
 * reg_data variable so as to burst write in the sensor.
 *
 * @param[in] desired_settings : Variable which specifies the settings which
 * are to be set in the sensor.
 * @param[out] reg_data : Pointer variable where the fifo_config_1 register
 * data will be stored so as to burst write in the register.
 * @param[in] dev_fifo : Structure instance of bmp3_fifo_settings which
 * contains the fifo_config_1 values set by the user.
 */
static int8_t fill_fifo_config_1(uint16_t desired_settings, struct bmp3_dev *dev);

/*!
 * @brief This internal API fills the fifo_config_2(fifo_sub_sampling,
 * data_select) settings in the reg_data variable so as to burst write
 * in the sensor.
 *
 * @param[in] desired_settings : Variable which specifies the settings which
 * are to be set in the sensor.
 * @param[out] reg_data : Pointer variable where the fifo_config_2 register
 * data will be stored so as to burst write in the register.
 * @param[in] dev_fifo : Structure instance of bmp3_fifo_settings which
 * contains the fifo_config_2 values set by the user.
 */
static int8_t fill_fifo_config_2(uint16_t desired_settings, struct bmp3_dev *dev);

/*!
 * @brief This internal API fills the fifo interrupt control(fwtm_en, ffull_en)
 * settings in the reg_data variable so as to burst write in the sensor.
 *
 * @param[in] desired_settings : Variable which specifies the settings which
 * are to be set in the sensor.
 * @param[out] reg_data : Pointer variable where the fifo interrupt control
 * register data will be stored so as to burst write in the register.
 * @param[in] dev_fifo : Structure instance of bmp3_fifo_settings which
 * contains the fifo interrupt control values set by the user.
 */
static int8_t fill_fifo_int_ctrl(uint16_t desired_settings, struct bmp3_dev *dev);

/*!
 * @brief This internal API is used to parse the fifo_config_1(fifo_mode,
 * fifo_stop_on_full, fifo_time_en, fifo_press_en, fifo_temp_en),
 * fifo_config_2(fifo_subsampling, data_select) and int_ctrl(fwtm_en, ffull_en)
 * settings and store it in device structure
 *
 * @param[in] reg_data : Pointer variable which stores the fifo settings data
 * read from the sensor.
 * @param[out] dev_fifo : Structure instance of bmp3_fifo_settings which
 * contains the fifo settings after parsing.
 */
static void parse_fifo_settings(const uint8_t *reg_data, struct bmp3_fifo_settings *dev_fifo);

/*!
 * @brief This internal API parse the FIFO data frame from the fifo buffer and
 * fills the byte count, uncompensated pressure and/or temperature data and no
 * of parsed frames.
 *
 * @param[in] header : Pointer variable which stores the fifo settings data
 * read from the sensor.
 * @param[in,out] fifo : Structure instance of bmp3_fifo which stores the
 * read fifo data.
 * @param[out] byte_index : Byte count which is incremented according to the
 * of data.
 * @param[out] uncomp_data : Uncompensated pressure and/or temperature data
 * which is stored after parsing fifo buffer data.
 * @param[out] parsed_frames : Total number of parsed frames.
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static uint8_t parse_fifo_data_frame(uint8_t header,
                                     struct bmp3_fifo *fifo,
                                     uint16_t *byte_index,
                                     struct bmp3_uncomp_data *uncomp_data,
                                     uint8_t *parsed_frames);

/*!
 * @brief This internal API unpacks the FIFO data frame from the fifo buffer and
 * fills the byte count, uncompensated pressure and/or temperature data.
 *
 * @param[out] byte_index : Byte count of fifo buffer.
 * @param[in] fifo_buffer : FIFO buffer from where the temperature and pressure
 * frames are unpacked.
 * @param[out] uncomp_data : Uncompensated temperature and pressure data after
 * unpacking from fifo buffer.
 */
static void unpack_temp_press_frame(uint16_t *byte_index,
                                    const uint8_t *fifo_buffer,
                                    struct bmp3_uncomp_data *uncomp_data);

/*!
 * @brief This internal API unpacks the FIFO data frame from the fifo buffer and
 * fills the byte count and uncompensated pressure data.
 *
 * @param[out] byte_index : Byte count of fifo buffer.
 * @param[in] fifo_buffer : FIFO buffer from where the pressure frames are
 * unpacked.
 * @param[out] uncomp_data : Uncompensated pressure data after unpacking from
 * fifo buffer.
 */
static void unpack_press_frame(uint16_t *byte_index, const uint8_t *fifo_buffer, struct bmp3_uncomp_data *uncomp_data);

/*!
 * @brief This internal API unpacks the FIFO data frame from the fifo buffer and
 * fills the byte count and uncompensated temperature data.
 *
 * @param[out] byte_index : Byte count of fifo buffer.
 * @param[in] fifo_buffer : FIFO buffer from where the temperature frames
 * are unpacked.
 * @param[out] uncomp_data : Uncompensated temperature data after unpacking from
 * fifo buffer.
 */
static void unpack_temp_frame(uint16_t *byte_index, const uint8_t *fifo_buffer, struct bmp3_uncomp_data *uncomp_data);

/*!
 * @brief This internal API unpacks the time frame from the fifo data buffer and
 * fills the byte count and update the sensor time variable.
 *
 * @param[out] byte_index : Byte count of fifo buffer.
 * @param[in] fifo_buffer : FIFO buffer from where the sensor time frames
 * are unpacked.
 * @param[out] sensor_time : Variable used to store the sensor time.
 */
static void unpack_time_frame(uint16_t *byte_index, const uint8_t *fifo_buffer, uint32_t *sensor_time);

/*!
 * @brief This internal API parses the FIFO buffer and gets the header info.
 *
 * @param[out] header : Variable used to store the fifo header data.
 * @param[in] fifo_buffer : FIFO buffer from where the header data is retrieved.
 * @param[out] byte_index : Byte count of fifo buffer.
 */
static void get_header_info(uint8_t *header, const uint8_t *fifo_buffer, uint16_t *byte_index);

/*!
 * @brief This internal API parses the FIFO data frame from the fifo buffer and
 * fills uncompensated temperature and/or pressure data.
 *
 * @param[in] sensor_comp : Variable used to select either temperature or
 * pressure or both while parsing the fifo frames.
 * @param[in] fifo_buffer : FIFO buffer where the temperature or pressure or
 * both the data to be parsed.
 * @param[out] uncomp_data : Uncompensated temperature or pressure or both the
 * data after unpacking from fifo buffer.
 */
static void parse_fifo_sensor_data(uint8_t sensor_comp, const uint8_t *fifo_buffer,
                                   struct bmp3_uncomp_data *uncomp_data);

/*!
 * @brief This internal API resets the FIFO buffer, start index,
 * parsed frame count, configuration change, configuration error and
 * frame_not_available variables.
 *
 * @param[out] fifo : FIFO structure instance where the fifo related variables
 * are reset.
 */
static void reset_fifo_index(struct bmp3_fifo *fifo);

/*!
 * @brief This API gets the command ready, data ready for pressure and
 * temperature, power on reset status from the sensor.
 *
 * @param[in,out] dev : Structure instance of bmp3_dev
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t get_sensor_status(struct bmp3_dev *dev);

/*!
 * @brief This API gets the interrupt (fifo watermark, fifo full, data ready)
 * status from the sensor.
 *
 * @param[in,out] dev : Structure instance of bmp3_dev
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t get_int_status(struct bmp3_dev *dev);

/*!
 * @brief This API gets the fatal, command and configuration error
 * from the sensor.
 *
 * @param[in,out] dev : Structure instance of bmp3_dev
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t get_err_status(struct bmp3_dev *dev);

/*!
 * @brief This internal API converts the no. of frames required by the user to
 * bytes so as to write in the watermark length register.
 *
 * @param[in] dev : Structure instance of bmp3_dev
 * @param[out] watermark_len : Pointer variable which contains the watermark
 * length.
 *
 * @return Result of API execution status.
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t convert_frames_to_bytes(uint16_t *watermark_len, const struct bmp3_dev *dev);

/****************** Global Function Definitions *******************************/

/*!
 *  @brief This API is the entry point.
 *  It performs the selection of I2C/SPI read mechanism according to the
 *  selected interface and reads the chip-id and calibration data of the sensor.
 */
int8_t bmp3_init(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t chip_id = 0;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if (rslt == BMP3_OK)
    {
        /* Read mechanism according to selected interface */
        if (dev->intf != BMP3_I2C_INTF)
        {
            /* If SPI interface is selected, read extra byte */
            dev->dummy_byte = 1;
        }
        else
        {
            /* If I2C interface is selected, no need to read
             * extra byte */
            dev->dummy_byte = 0;
        }

        /* Read the chip-id of bmp3 sensor */
        rslt = bmp3_get_regs(BMP3_REG_CHIP_ID, &chip_id, 1, dev);

        /* Proceed if everything is fine until now */
        if (rslt == BMP3_OK)
        {
            /* Check for chip id validity */
            if ((chip_id == BMP3_CHIP_ID) || (chip_id == BMP390_CHIP_ID))
            {
                dev->chip_id = chip_id;

                /* Reset the sensor */
                rslt = bmp3_soft_reset(dev);
                if (rslt == BMP3_OK)
                {
                    /* Read the calibration data */
                    rslt = get_calib_data(dev);
                }
            }
            else
            {
                rslt = BMP3_E_DEV_NOT_FOUND;
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API reads the data from the given register address of the sensor.
 */
int8_t bmp3_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint32_t idx;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if ((rslt == BMP3_OK) && (reg_data != NULL))
    {
        uint32_t temp_len = len + dev->dummy_byte;
        uint8_t temp_buff[len + dev->dummy_byte];

        /* If interface selected is SPI */
        if (dev->intf != BMP3_I2C_INTF)
        {
            reg_addr = reg_addr | 0x80;

            /* Read the data from the register */
            dev->intf_rslt = dev->read(reg_addr, temp_buff, temp_len, dev->intf_ptr);
            for (idx = 0; idx < len; idx++)
            {
                reg_data[idx] = temp_buff[idx + dev->dummy_byte];
            }
        }
        else
        {
            /* Read the data using I2C */
            dev->intf_rslt = dev->read(reg_addr, reg_data, len, dev->intf_ptr);
        }

        /* Check for communication error */
        if (dev->intf_rslt != BMP3_INTF_RET_SUCCESS)
        {
            rslt = BMP3_E_COMM_FAIL;
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API writes the given data to the register address
 * of the sensor.
 */
int8_t bmp3_set_regs(uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t temp_buff[len * 2];
    uint32_t temp_len;
    uint8_t reg_addr_cnt;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Check for arguments validity */
    if ((rslt == BMP3_OK) && (reg_addr != NULL) && (reg_data != NULL))
    {
        if (len != 0)
        {
            temp_buff[0] = reg_data[0];

            /* If interface selected is SPI */
            if (dev->intf == BMP3_SPI_INTF)
            {
                for (reg_addr_cnt = 0; reg_addr_cnt < len; reg_addr_cnt++)
                {
                    reg_addr[reg_addr_cnt] = reg_addr[reg_addr_cnt] & 0x7F;
                }
            }

            /* Burst write mode */
            if (len > 1)
            {
                /* Interleave register address w.r.t data for
                 * burst write*/
                interleave_reg_addr(reg_addr, temp_buff, reg_data, len);
                temp_len = len * 2;
            }
            else
            {
                temp_len = len;
            }

            dev->intf_rslt = dev->write(reg_addr[0], temp_buff, temp_len, dev->intf_ptr);

            /* Check for communication error */
            if (dev->intf_rslt != BMP3_INTF_RET_SUCCESS)
            {
                rslt = BMP3_E_COMM_FAIL;
            }
        }
        else
        {
            rslt = BMP3_E_INVALID_LEN;
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API sets the power control(pressure enable and
 * temperature enable), over sampling, ODR and filter
 * settings in the sensor.
 */
int8_t bmp3_set_sensor_settings(uint32_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if (rslt == BMP3_OK)
    {
        if (are_settings_changed(BMP3_POWER_CNTL, desired_settings))
        {
            /* Set the power control settings */
            rslt = set_pwr_ctrl_settings(desired_settings, dev);
        }

        if (are_settings_changed(BMP3_ODR_FILTER, desired_settings) && (!rslt))
        {
            /* Set the over sampling, ODR and filter settings*/
            rslt = set_odr_filter_settings(desired_settings, dev);
        }

        if (are_settings_changed(BMP3_INT_CTRL, desired_settings) && (!rslt))
        {
            /* Set the interrupt control settings */
            rslt = set_int_ctrl_settings(desired_settings, dev);
        }

        if (are_settings_changed(BMP3_ADV_SETT, desired_settings) && (!rslt))
        {
            /* Set the advance settings */
            rslt = set_advance_settings(desired_settings, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API gets the power control(power mode, pressure enable and
 * temperature enable), over sampling, ODR, filter, interrupt control and
 * advance settings from the sensor.
 */
int8_t bmp3_get_sensor_settings(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t settings_data[BMP3_LEN_GEN_SETT];

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if (rslt == BMP3_OK)
    {
        rslt = bmp3_get_regs(BMP3_REG_INT_CTRL, settings_data, BMP3_LEN_GEN_SETT, dev);

        if (rslt == BMP3_OK)
        {
            /* Parse the settings data */
            parse_sett_data(settings_data, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the fifo_config_1(fifo_mode,
 * fifo_stop_on_full, fifo_time_en, fifo_press_en, fifo_temp_en),
 * fifo_config_2(fifo_subsampling, data_select) and int_ctrl(fwtm_en, ffull_en)
 * settings in the sensor.
 */
int8_t bmp3_set_fifo_settings(uint16_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if ((rslt == BMP3_OK) && (dev->fifo != NULL))
    {
        if (are_settings_changed(BMP3_FIFO_CONFIG_1, desired_settings))
        {
            /* Fill the FIFO config 1 register data */
            rslt = fill_fifo_config_1(desired_settings, dev);
        }

        if (are_settings_changed(desired_settings, BMP3_FIFO_CONFIG_2))
        {
            /* Fill the FIFO config 2 register data */
            rslt = fill_fifo_config_2(desired_settings, dev);
        }

        if (are_settings_changed(desired_settings, BMP3_FIFO_INT_CTRL))
        {
            /* Fill the FIFO interrupt ctrl register data */
            rslt = fill_fifo_int_ctrl(desired_settings, dev);
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API gets the fifo_config_1(fifo_mode,
 * fifo_stop_on_full, fifo_time_en, fifo_press_en, fifo_temp_en),
 * fifo_config_2(fifo_subsampling, data_select) and int_ctrl(fwtm_en, ffull_en)
 * settings from the sensor.
 */
int8_t bmp3_get_fifo_settings(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t fifo_sett[3];
    uint8_t len = 3;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if ((rslt == BMP3_OK) && (dev->fifo != NULL))
    {
        rslt = bmp3_get_regs(BMP3_REG_FIFO_CONFIG_1, fifo_sett, len, dev);

        /* Parse the fifo settings */
        parse_fifo_settings(fifo_sett, &dev->fifo->settings);
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API gets the fifo data from the sensor.
 */
int8_t bmp3_get_fifo_data(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint16_t fifo_len;

    rslt = null_ptr_check(dev);

    if ((rslt == BMP3_OK) && (dev->fifo != NULL))
    {
        struct bmp3_fifo *fifo = dev->fifo;

        reset_fifo_index(dev->fifo);

        /* Get the total no of bytes available in FIFO */
        rslt = bmp3_get_fifo_length(&fifo_len, dev);

        /* For sensor time frame */
        if (dev->fifo->settings.time_en == TRUE)
        {
            fifo_len = fifo_len + 4;
        }

        /* Update the fifo length in the fifo structure */
        dev->fifo->data.byte_count = fifo_len;

        if (rslt == BMP3_OK)
        {
            /* Read the fifo data */
            rslt = bmp3_get_regs(BMP3_REG_FIFO_DATA, fifo->data.buffer, fifo_len, dev);
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API sets the fifo watermark length according to the frames count
 * set by the user in the device structure. Refer below for usage.
 */
int8_t bmp3_set_fifo_watermark(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data[2];
    uint8_t reg_addr[2] = { BMP3_REG_FIFO_WM, BMP3_REG_FIFO_WM + 1 };
    uint16_t watermark_len;

    rslt = null_ptr_check(dev);

    if ((rslt == BMP3_OK) && (dev->fifo != NULL))
    {
        rslt = convert_frames_to_bytes(&watermark_len, dev);

        if (rslt == BMP3_OK)
        {
            reg_data[0] = BMP3_GET_LSB(watermark_len);
            reg_data[1] = BMP3_GET_MSB(watermark_len) & 0x01;
            rslt = bmp3_set_regs(reg_addr, reg_data, 2, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This API extracts the temperature and/or pressure data from the FIFO
 * data which is already read from the fifo.
 */
int8_t bmp3_extract_fifo_data(struct bmp3_data *data, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t header;
    uint8_t parsed_frames = 0;
    uint8_t t_p_frame;
    struct bmp3_uncomp_data uncomp_data;

    rslt = null_ptr_check(dev);

    if ((rslt == BMP3_OK) && (dev->fifo != NULL) && (data != NULL))
    {
        uint16_t byte_index = dev->fifo->data.start_idx;

        while ((parsed_frames < (dev->fifo->data.req_frames)) && (byte_index < dev->fifo->data.byte_count))
        {
            get_header_info(&header, dev->fifo->data.buffer, &byte_index);
            t_p_frame = parse_fifo_data_frame(header, dev->fifo, &byte_index, &uncomp_data, &parsed_frames);

            /* If the frame is pressure and/or temperature data */
            if (t_p_frame != FALSE)
            {
                /* Compensate temperature and pressure data */
                rslt = compensate_data(t_p_frame, &uncomp_data, &data[parsed_frames - 1], &dev->calib_data);
            }
        }

        /* Check if any frames are parsed in FIFO */
        if (parsed_frames != 0)
        {
            /* Update the bytes parsed in the device structure */
            dev->fifo->data.start_idx = byte_index;
            dev->fifo->data.parsed_frames += parsed_frames;
        }
        else
        {
            /* No frames are there to parse. It is time to
             * read the FIFO, if more frames are needed */
            dev->fifo->data.frame_not_available = TRUE;
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API gets the command ready, data ready for pressure and
 * temperature and interrupt (fifo watermark, fifo full, data ready) and
 * error status from the sensor.
 */
int8_t bmp3_get_status(struct bmp3_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    if (rslt == BMP3_OK)
    {
        rslt = get_sensor_status(dev);

        /* Proceed further if the earlier operation is fine */
        if (rslt == BMP3_OK)
        {
            rslt = get_int_status(dev);

            /* Proceed further if the earlier operation is fine */
            if (rslt == BMP3_OK)
            {
                /* Get the error status */
                rslt = get_err_status(dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API gets the fifo length from the sensor.
 */
int8_t bmp3_get_fifo_length(uint16_t *fifo_length, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data[2];

    rslt = null_ptr_check(dev);

    if ((rslt == BMP3_OK) && (fifo_length != NULL))
    {
        rslt = bmp3_get_regs(BMP3_REG_FIFO_LENGTH, reg_data, 2, dev);

        /* Proceed if read from register is fine */
        if (rslt == BMP3_OK)
        {
            /* Update the fifo length */
            *fifo_length = BMP3_CONCAT_BYTES(reg_data[1], reg_data[0]);
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API performs the soft reset of the sensor.
 */
int8_t bmp3_soft_reset(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_CMD;

    /* 0xB6 is the soft reset command */
    uint8_t soft_rst_cmd = BMP3_SOFT_RESET;
    uint8_t cmd_rdy_status;
    uint8_t cmd_err_status;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if (rslt == BMP3_OK)
    {
        /* Check for command ready status */
        rslt = bmp3_get_regs(BMP3_REG_SENS_STATUS, &cmd_rdy_status, 1, dev);

        /* Device is ready to accept new command */
        if ((cmd_rdy_status & BMP3_CMD_RDY) && (rslt == BMP3_OK))
        {
            /* Write the soft reset command in the sensor */
            rslt = bmp3_set_regs(&reg_addr, &soft_rst_cmd, 1, dev);

            /* Proceed if everything is fine until now */
            if (rslt == BMP3_OK)
            {
                /* Wait for 2 ms */
                dev->delay_us(2000, dev->intf_ptr);

                /* Read for command error status */
                rslt = bmp3_get_regs(BMP3_REG_ERR, &cmd_err_status, 1, dev);

                /* check for command error status */
                if ((cmd_err_status & BMP3_REG_CMD) || (rslt != BMP3_OK))
                {
                    /* Command not written hence return
                     * error */
                    rslt = BMP3_E_CMD_EXEC_FAILED;
                }
            }
        }
        else
        {
            rslt = BMP3_E_CMD_EXEC_FAILED;
        }
    }

    return rslt;
}

/*!
 * @brief This API performs the soft reset of the sensor.
 */
int8_t bmp3_fifo_flush(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_CMD;

    uint8_t fifo_flush_cmd = BMP3_FIFO_FLUSH;
    uint8_t cmd_rdy_status;
    uint8_t cmd_err_status;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    /* Proceed if null check is fine */
    if (rslt == BMP3_OK)
    {
        /* Check for command ready status */
        rslt = bmp3_get_regs(BMP3_REG_SENS_STATUS, &cmd_rdy_status, 1, dev);

        /* Device is ready to accept new command */
        if ((cmd_rdy_status & BMP3_CMD_RDY) && (rslt == BMP3_OK))
        {
            /* Write the soft reset command in the sensor */
            rslt = bmp3_set_regs(&reg_addr, &fifo_flush_cmd, 1, dev);

            /* Proceed if everything is fine until now */
            if (rslt == BMP3_OK)
            {
                /* Wait for 2 ms */
                dev->delay_us(2000, dev->intf_ptr);

                /* Read for command error status */
                rslt = bmp3_get_regs(BMP3_REG_ERR, &cmd_err_status, 1, dev);

                /* check for command error status */
                if ((cmd_err_status & BMP3_REG_CMD) || (rslt != BMP3_OK))
                {
                    /* Command not written hence return
                     * error */
                    rslt = BMP3_E_CMD_EXEC_FAILED;
                }
            }
        }
        else
        {
            rslt = BMP3_E_CMD_EXEC_FAILED;
        }
    }

    return rslt;
}

/*!
 * @brief This API sets the power mode of the sensor.
 */
int8_t bmp3_set_op_mode(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t last_set_mode;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    if (rslt == BMP3_OK)
    {
        uint8_t curr_mode = dev->settings.op_mode;

        rslt = bmp3_get_op_mode(&last_set_mode, dev);

        /* If the sensor is not in sleep mode put the device to sleep
         * mode */
        if (last_set_mode != BMP3_MODE_SLEEP)
        {
            /* Device should be put to sleep before transiting to
             * forced mode or normal mode */
            rslt = put_device_to_sleep(dev);

            /* Give some time for device to go into sleep mode */
            dev->delay_us(5000, dev->intf_ptr);
        }

        /* Set the power mode */
        if (rslt == BMP3_OK)
        {
            if (curr_mode == BMP3_MODE_NORMAL)
            {
                /* Set normal mode and validate
                 * necessary settings */
                rslt = set_normal_mode(dev);
            }
            else if (curr_mode == BMP3_MODE_FORCED)
            {
                /* Set forced mode */
                rslt = write_power_mode(dev);
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API gets the power mode of the sensor.
 */
int8_t bmp3_get_op_mode(uint8_t *op_mode, struct bmp3_dev *dev)
{
    int8_t rslt;

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    if ((rslt == BMP3_OK) && (op_mode != NULL))
    {
        /* Read the power mode register */
        rslt = bmp3_get_regs(BMP3_REG_PWR_CTRL, op_mode, 1, dev);

        /* Assign the power mode in the device structure */
        *op_mode = BMP3_GET_BITS(*op_mode, BMP3_OP_MODE);
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API reads the pressure, temperature or both data from the
 * sensor, compensates the data and store it in the bmp3_data structure
 * instance passed by the user.
 */
int8_t bmp3_get_sensor_data(uint8_t sensor_comp, struct bmp3_data *comp_data, struct bmp3_dev *dev)
{
    int8_t rslt;

    /* Array to store the pressure and temperature data read from
     * the sensor */
    uint8_t reg_data[BMP3_LEN_P_T_DATA] = { 0 };
    struct bmp3_uncomp_data uncomp_data = { 0 };

    /* Check for null pointer in the device structure*/
    rslt = null_ptr_check(dev);

    if ((rslt == BMP3_OK) && (comp_data != NULL))
    {
        /* Read the pressure and temperature data from the sensor */
        rslt = bmp3_get_regs(BMP3_REG_DATA, reg_data, BMP3_LEN_P_T_DATA, dev);

        if (rslt == BMP3_OK)
        {
            /* Parse the read data from the sensor */
            parse_sensor_data(reg_data, &uncomp_data);

            /* Compensate the pressure/temperature/both data read
             * from the sensor */
            rslt = compensate_data(sensor_comp, &uncomp_data, comp_data, &dev->calib_data);
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

/****************** Static Function Definitions *******************************/

/*!
 * @brief This internal API converts the no. of frames required by the user to
 * bytes so as to write in the watermark length register.
 */
static int8_t convert_frames_to_bytes(uint16_t *watermark_len, const struct bmp3_dev *dev)
{
    int8_t rslt = BMP3_OK;

    if ((dev->fifo->data.req_frames > 0) && (dev->fifo->data.req_frames <= BMP3_FIFO_MAX_FRAMES))
    {
        if (dev->fifo->settings.press_en && dev->fifo->settings.temp_en)
        {
            /* Multiply with pressure and temperature header len */
            *watermark_len = dev->fifo->data.req_frames * BMP3_LEN_P_AND_T_HEADER_DATA;
        }
        else if (dev->fifo->settings.temp_en || dev->fifo->settings.press_en)
        {
            /* Multiply with pressure or temperature header len */
            *watermark_len = dev->fifo->data.req_frames * BMP3_LEN_P_OR_T_HEADER_DATA;
        }
        else
        {
            /* No sensor is enabled */
            rslt = BMP3_W_SENSOR_NOT_ENABLED;
        }
    }
    else
    {
        /* Required frame count is zero, which is invalid */
        rslt = BMP3_W_INVALID_FIFO_REQ_FRAME_CNT;
    }

    return rslt;
}

/*!
 * @brief This internal API resets the FIFO buffer, start index,
 * parsed frame count, configuration change, configuration error and
 * frame_not_available variables.
 */
static void reset_fifo_index(struct bmp3_fifo *fifo)
{
    /* Loop variable */
    uint16_t index;

    /* Variable for FIFO size */
    uint16_t fifo_size = 512;

    /* The size of FIFO in BMP3 is 512 bytes */
    for (index = 0; index < fifo_size; index++)
    {
        /* Initialize data buffer to zero */
        fifo->data.buffer[index] = 0;
    }

    fifo->data.byte_count = 0;
    fifo->data.start_idx = 0;
    fifo->data.parsed_frames = 0;
    fifo->data.config_change = 0;
    fifo->data.config_err = 0;
    fifo->data.frame_not_available = 0;
}

/*!
 * @brief This internal API parse the FIFO data frame from the fifo buffer and
 * fills the byte count, uncompensated pressure and/or temperature data and no
 * of parsed frames.
 */
static uint8_t parse_fifo_data_frame(uint8_t header,
                                     struct bmp3_fifo *fifo,
                                     uint16_t *byte_index,
                                     struct bmp3_uncomp_data *uncomp_data,
                                     uint8_t *parsed_frames)
{
    uint8_t t_p_frame = 0;

    switch (header)
    {
        case BMP3_FIFO_TEMP_PRESS_FRAME:
            unpack_temp_press_frame(byte_index, fifo->data.buffer, uncomp_data);
            *parsed_frames = *parsed_frames + 1;
            t_p_frame = BMP3_PRESS | BMP3_TEMP;
            break;
        case BMP3_FIFO_TEMP_FRAME:
            unpack_temp_frame(byte_index, fifo->data.buffer, uncomp_data);
            *parsed_frames = *parsed_frames + 1;
            t_p_frame = BMP3_TEMP;
            break;
        case BMP3_FIFO_PRESS_FRAME:
            unpack_press_frame(byte_index, fifo->data.buffer, uncomp_data);
            *parsed_frames = *parsed_frames + 1;
            t_p_frame = BMP3_PRESS;
            break;
        case BMP3_FIFO_TIME_FRAME:
            unpack_time_frame(byte_index, fifo->data.buffer, &fifo->data.sensor_time);
            break;
        case BMP3_FIFO_CONFIG_CHANGE:
            fifo->data.config_change = 1;
            *byte_index = *byte_index + 1;
            break;
        case BMP3_FIFO_ERROR_FRAME:
            fifo->data.config_err = 1;
            *byte_index = *byte_index + 1;
            break;
        default:
            fifo->data.config_err = 1;
            *byte_index = *byte_index + 1;
            break;
    }

    return t_p_frame;
}

/*!
 * @brief This internal API unpacks the FIFO data frame from the fifo buffer and
 * fills the byte count, uncompensated pressure and/or temperature data.
 */
static void unpack_temp_press_frame(uint16_t *byte_index,
                                    const uint8_t *fifo_buffer,
                                    struct bmp3_uncomp_data *uncomp_data)
{
    parse_fifo_sensor_data((BMP3_PRESS | BMP3_TEMP), &fifo_buffer[*byte_index], uncomp_data);
    *byte_index = *byte_index + BMP3_LEN_P_T_DATA;
}

/*!
 * @brief This internal API unpacks the FIFO data frame from the fifo buffer and
 * fills the byte count and uncompensated temperature data.
 */
static void unpack_temp_frame(uint16_t *byte_index, const uint8_t *fifo_buffer, struct bmp3_uncomp_data *uncomp_data)
{
    parse_fifo_sensor_data(BMP3_TEMP, &fifo_buffer[*byte_index], uncomp_data);
    *byte_index = *byte_index + BMP3_LEN_T_DATA;
}

/*!
 * @brief This internal API unpacks the FIFO data frame from the fifo buffer and
 * fills the byte count and uncompensated pressure data.
 */
static void unpack_press_frame(uint16_t *byte_index, const uint8_t *fifo_buffer, struct bmp3_uncomp_data *uncomp_data)
{
    parse_fifo_sensor_data(BMP3_PRESS, &fifo_buffer[*byte_index], uncomp_data);
    *byte_index = *byte_index + BMP3_LEN_P_DATA;
}

/*!
 * @brief This internal API unpacks the time frame from the fifo data buffer and
 * fills the byte count and update the sensor time variable.
 */
static void unpack_time_frame(uint16_t *byte_index, const uint8_t *fifo_buffer, uint32_t *sensor_time)
{
    uint16_t index = *byte_index;
    uint32_t xlsb = fifo_buffer[index];
    uint32_t lsb = ((uint32_t)fifo_buffer[index + 1]) << 8;
    uint32_t msb = ((uint32_t)fifo_buffer[index + 2]) << 16;

    *sensor_time = msb | lsb | xlsb;
    *byte_index = *byte_index + BMP3_LEN_SENSOR_TIME;
}

/*!
 * @brief This internal API parses the FIFO data frame from the fifo buffer and
 * fills uncompensated temperature and/or pressure data.
 */
static void parse_fifo_sensor_data(uint8_t sensor_comp, const uint8_t *fifo_buffer,
                                   struct bmp3_uncomp_data *uncomp_data)
{
    /* Temporary variables to store the sensor data */
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;

    /* Store the parsed register values for temperature data */
    data_xlsb = (uint32_t)fifo_buffer[0];
    data_lsb = (uint32_t)fifo_buffer[1] << 8;
    data_msb = (uint32_t)fifo_buffer[2] << 16;

    if (sensor_comp == BMP3_TEMP)
    {
        /* Update uncompensated temperature */
        uncomp_data->temperature = data_msb | data_lsb | data_xlsb;
    }

    if (sensor_comp == BMP3_PRESS)
    {
        /* Update uncompensated pressure */
        uncomp_data->pressure = data_msb | data_lsb | data_xlsb;
    }

    if (sensor_comp == (BMP3_TEMP | BMP3_PRESS))
    {
        uncomp_data->temperature = data_msb | data_lsb | data_xlsb;

        /* Store the parsed register values for pressure data */
        data_xlsb = (uint32_t)fifo_buffer[3];
        data_lsb = (uint32_t)fifo_buffer[4] << 8;
        data_msb = (uint32_t)fifo_buffer[5] << 16;
        uncomp_data->pressure = data_msb | data_lsb | data_xlsb;
    }
}

/*!
 * @brief This internal API parses the FIFO buffer and gets the header info.
 */
static void get_header_info(uint8_t *header, const uint8_t *fifo_buffer, uint16_t *byte_index)
{
    *header = fifo_buffer[*byte_index];
    *byte_index = *byte_index + 1;
}

/*!
 * @brief This internal API sets the normal mode in the sensor.
 */
static int8_t set_normal_mode(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t conf_err_status;

    rslt = validate_normal_mode_settings(dev);

    /* If OSR and ODR settings are proper then write the power mode */
    if (rslt == BMP3_OK)
    {
        rslt = write_power_mode(dev);

        /* check for configuration error */
        if (rslt == BMP3_OK)
        {
            /* Read the configuration error status */
            rslt = bmp3_get_regs(BMP3_REG_ERR, &conf_err_status, 1, dev);

            /* Check if conf. error flag is set */
            if (rslt == BMP3_OK)
            {
                if (conf_err_status & BMP3_ERR_CONF)
                {
                    /* OSR and ODR configuration is not proper */
                    rslt = BMP3_E_CONFIGURATION_ERR;
                }
            }
        }
    }

    return rslt;
}

/*!
 * @brief This internal API writes the power mode in the sensor.
 */
static int8_t write_power_mode(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_PWR_CTRL;
    uint8_t op_mode = dev->settings.op_mode;

    /* Temporary variable to store the value read from op-mode register */
    uint8_t op_mode_reg_val;

    /* Read the power mode register */
    rslt = bmp3_get_regs(reg_addr, &op_mode_reg_val, 1, dev);

    /* Set the power mode */
    if (rslt == BMP3_OK)
    {
        op_mode_reg_val = BMP3_SET_BITS(op_mode_reg_val, BMP3_OP_MODE, op_mode);

        /* Write the power mode in the register */
        rslt = bmp3_set_regs(&reg_addr, &op_mode_reg_val, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API puts the device to sleep mode.
 */
static int8_t put_device_to_sleep(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_PWR_CTRL;

    /* Temporary variable to store the value read from op-mode register */
    uint8_t op_mode_reg_val;

    rslt = bmp3_get_regs(BMP3_REG_PWR_CTRL, &op_mode_reg_val, 1, dev);

    if (rslt == BMP3_OK)
    {
        /* Set the power mode */
        op_mode_reg_val = op_mode_reg_val & (~(BMP3_OP_MODE_MSK));

        /* Write the power mode in the register */
        rslt = bmp3_set_regs(&reg_addr, &op_mode_reg_val, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API validate the normal mode settings of the sensor.
 */
static int8_t validate_normal_mode_settings(struct bmp3_dev *dev)
{
    int8_t rslt;

    rslt = get_odr_filter_settings(dev);

    if (rslt == BMP3_OK)
    {
        rslt = validate_osr_and_odr_settings(dev);
    }

    return rslt;
}

/*!
 * @brief This internal API reads the calibration data from the sensor, parse
 * it then compensates it and store in the device structure.
 */
static int8_t get_calib_data(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_CALIB_DATA;

    /* Array to store calibration data */
    uint8_t calib_data[BMP3_LEN_CALIB_DATA] = { 0 };

    /* Read the calibration data from the sensor */
    rslt = bmp3_get_regs(reg_addr, calib_data, BMP3_LEN_CALIB_DATA, dev);

    /* Parse calibration data and store it in device structure */
    parse_calib_data(calib_data, dev);

    return rslt;
}

/*!
 * @brief This internal API interleaves the register address between the
 * register data buffer for burst write operation.
 */
static void interleave_reg_addr(const uint8_t *reg_addr, uint8_t *temp_buff, const uint8_t *reg_data, uint32_t len)
{
    uint32_t index;

    for (index = 1; index < len; index++)
    {
        temp_buff[(index * 2) - 1] = reg_addr[index];
        temp_buff[index * 2] = reg_data[index];
    }
}

/*!
 * @brief This internal API parse the power control(power mode, pressure enable
 * and temperature enable), over sampling, ODR, filter, interrupt control and
 * advance settings and store in the device structure.
 */
static void parse_sett_data(const uint8_t *reg_data, struct bmp3_dev *dev)
{
    /* Parse interrupt control settings and store in device structure */
    parse_int_ctrl_settings(&reg_data[0], &dev->settings.int_settings);

    /* Parse advance settings and store in device structure */
    parse_advance_settings(&reg_data[1], &dev->settings.adv_settings);

    /* Parse power control settings and store in device structure */
    parse_pwr_ctrl_settings(&reg_data[2], &dev->settings);

    /* Parse ODR and filter settings and store in device structure */
    parse_odr_filter_settings(&reg_data[3], &dev->settings.odr_filter);
}

/*!
 * @brief This internal API parse the interrupt control(output mode, level,
 * latch and data ready) settings and store in the device structure.
 */
static void parse_int_ctrl_settings(const uint8_t *reg_data, struct bmp3_int_ctrl_settings *settings)
{
    settings->output_mode = BMP3_GET_BITS_POS_0(*reg_data, BMP3_INT_OUTPUT_MODE);
    settings->level = BMP3_GET_BITS(*reg_data, BMP3_INT_LEVEL);
    settings->latch = BMP3_GET_BITS(*reg_data, BMP3_INT_LATCH);
    settings->drdy_en = BMP3_GET_BITS(*reg_data, BMP3_INT_DRDY_EN);
}
static void parse_advance_settings(const uint8_t *reg_data, struct bmp3_adv_settings *settings)
{
    settings->i2c_wdt_en = BMP3_GET_BITS(*reg_data, BMP3_I2C_WDT_EN);
    settings->i2c_wdt_sel = BMP3_GET_BITS(*reg_data, BMP3_I2C_WDT_SEL);
}

/*!
 * @brief This internal API parse the power control(power mode, pressure enable
 * and temperature enable) settings and store in the device structure.
 */
static void  parse_pwr_ctrl_settings(const uint8_t *reg_data, struct bmp3_settings *settings)
{
    settings->op_mode = BMP3_GET_BITS(*reg_data, BMP3_OP_MODE);
    settings->press_en = BMP3_GET_BITS_POS_0(*reg_data, BMP3_PRESS_EN);
    settings->temp_en = BMP3_GET_BITS(*reg_data, BMP3_TEMP_EN);
}

/*!
 * @brief This internal API parse the over sampling, ODR and filter
 * settings and store in the device structure.
 */
static void  parse_odr_filter_settings(const uint8_t *reg_data, struct bmp3_odr_filter_settings *settings)
{
    uint8_t index = 0;

    /* ODR and filter settings index starts from one (0x1C register) */
    settings->press_os = BMP3_GET_BITS_POS_0(reg_data[index], BMP3_PRESS_OS);
    settings->temp_os = BMP3_GET_BITS(reg_data[index], BMP3_TEMP_OS);

    /* Move index to 0x1D register */
    index++;
    settings->odr = BMP3_GET_BITS_POS_0(reg_data[index], BMP3_ODR);

    /* Move index to 0x1F register */
    index = index + 2;
    settings->iir_filter = BMP3_GET_BITS(reg_data[index], BMP3_IIR_FILTER);
}

/*!
 * @brief This API sets the pressure enable and temperature enable
 * settings of the sensor.
 */
static int8_t set_pwr_ctrl_settings(uint32_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_PWR_CTRL;
    uint8_t reg_data;

    rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        if (desired_settings & BMP3_SEL_PRESS_EN)
        {
            /* Set the pressure enable settings in the
             * register variable */
            reg_data = BMP3_SET_BITS_POS_0(reg_data, BMP3_PRESS_EN, dev->settings.press_en);
        }

        if (desired_settings & BMP3_SEL_TEMP_EN)
        {
            /* Set the temperature enable settings in the
             * register variable */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_TEMP_EN, dev->settings.temp_en);
        }

        /* Write the power control settings in the register */
        rslt = bmp3_set_regs(&reg_addr, &reg_data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API sets the over sampling, ODR and filter settings
 * of the sensor based on the settings selected by the user.
 */
static int8_t set_odr_filter_settings(uint32_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;

    /* No of registers to be configured is 3*/
    uint8_t reg_addr[3] = { 0 };

    /* No of register data to be read is 4 */
    uint8_t reg_data[4];
    uint8_t len = 0;

    rslt = bmp3_get_regs(BMP3_REG_OSR, reg_data, 4, dev);

    if (rslt == BMP3_OK)
    {
        if (are_settings_changed((BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS), desired_settings))
        {
            /* Fill the over sampling register address and
            * register data to be written in the sensor */
            fill_osr_data(desired_settings, reg_addr, reg_data, &len, dev);
        }

        if (are_settings_changed(BMP3_SEL_ODR, desired_settings))
        {
            /* Fill the output data rate register address and
             * register data to be written in the sensor */
            fill_odr_data(reg_addr, reg_data, &len, dev);
        }

        if (are_settings_changed(BMP3_SEL_IIR_FILTER, desired_settings))
        {
            /* Fill the iir filter register address and
             * register data to be written in the sensor */
            fill_filter_data(reg_addr, reg_data, &len, dev);
        }

        if (dev->settings.op_mode == BMP3_MODE_NORMAL)
        {
            /* For normal mode, OSR and ODR settings should
             * be proper */
            rslt = validate_osr_and_odr_settings(dev);
        }

        if (rslt == BMP3_OK)
        {
            /* Burst write the over sampling, ODR and filter
             * settings in the register */
            rslt = bmp3_set_regs(reg_addr, reg_data, len, dev);
        }
    }

    return rslt;
}

/*!
 * @brief This internal API sets the interrupt control (output mode, level,
 * latch and data ready) settings of the sensor based on the settings
 * selected by the user.
 */
static int8_t set_int_ctrl_settings(uint32_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data;
    uint8_t reg_addr;
    struct bmp3_int_ctrl_settings int_settings;

    reg_addr = BMP3_REG_INT_CTRL;
    rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        int_settings = dev->settings.int_settings;

        if (desired_settings & BMP3_SEL_OUTPUT_MODE)
        {
            /* Set the interrupt output mode bits */
            reg_data = BMP3_SET_BITS_POS_0(reg_data, BMP3_INT_OUTPUT_MODE, int_settings.output_mode);
        }

        if (desired_settings & BMP3_SEL_LEVEL)
        {
            /* Set the interrupt level bits */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_INT_LEVEL, int_settings.level);
        }

        if (desired_settings & BMP3_SEL_LATCH)
        {
            /* Set the interrupt latch bits */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_INT_LATCH, int_settings.latch);
        }

        if (desired_settings & BMP3_SEL_DRDY_EN)
        {
            /* Set the interrupt data ready bits */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_INT_DRDY_EN, int_settings.drdy_en);
        }

        rslt = bmp3_set_regs(&reg_addr, &reg_data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API sets the advance (i2c_wdt_en, i2c_wdt_sel)
 * settings of the sensor based on the settings selected by the user.
 */
static int8_t set_advance_settings(uint32_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr;
    uint8_t reg_data;
    struct bmp3_adv_settings adv_settings = dev->settings.adv_settings;

    reg_addr = BMP3_REG_IF_CONF;
    rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        if (desired_settings & BMP3_SEL_I2C_WDT_EN)
        {
            /* Set the i2c watch dog enable bits */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_I2C_WDT_EN, adv_settings.i2c_wdt_en);
        }

        if (desired_settings & BMP3_SEL_I2C_WDT)
        {
            /* Set the i2c watch dog select bits */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_I2C_WDT_SEL, adv_settings.i2c_wdt_sel);
        }

        rslt = bmp3_set_regs(&reg_addr, &reg_data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API gets the over sampling, ODR and filter settings
 * of the sensor.
 */
static int8_t get_odr_filter_settings(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data[4];

    /* Read data beginning from 0x1C register */
    rslt = bmp3_get_regs(BMP3_REG_OSR, reg_data, 4, dev);

    /* Parse the read data and store it in dev structure */
    parse_odr_filter_settings(reg_data, &dev->settings.odr_filter);

    return rslt;
}

/*!
 * @brief This internal API validate the over sampling, ODR settings of the
 * sensor.
 */
static int8_t validate_osr_and_odr_settings(const struct bmp3_dev *dev)
{
    int8_t rslt;

    /* According to BMP388 datasheet at Section 3.9.2. "Measurement rate in
     * forced mode and normal mode" there is also the constant of 234us also to
     * be considered in the sum. */
    uint32_t meas_t = 234;
    uint32_t meas_t_p = 0;

    /* Sampling period corresponding to ODR in microseconds  */
    uint32_t odr[18] = {
        5000, 10000, 20000, 40000, 80000, 160000, 320000, 640000, 1280000, 2560000, 5120000, 10240000, 20480000,
        40960000, 81920000, 163840000, 327680000, 655360000
    };

    if (dev->settings.press_en)
    {
        /* Calculate the pressure measurement duration */
        meas_t_p += calculate_press_meas_time(dev);
    }

    if (dev->settings.temp_en)
    {
        /* Calculate the temperature measurement duration */
        meas_t_p += calculate_temp_meas_time(dev);
    }

    /* Constant 234us added to the summation of temperature and pressure measurement duration */
    meas_t += meas_t_p;

    rslt = verify_meas_time_and_odr_duration(meas_t, odr[dev->settings.odr_filter.odr]);

    return rslt;
}

/*!
 * @brief This internal API checks whether the measurement time and ODR duration
 * of the sensor are proper.
 */
static int8_t verify_meas_time_and_odr_duration(uint32_t meas_t, uint32_t odr_duration)
{
    int8_t rslt;

    if (meas_t < odr_duration)
    {
        /* If measurement duration is less than ODR duration
         * then OSR and ODR settings are fine */
        rslt = BMP3_OK;
    }
    else
    {
        /* OSR and ODR settings are not proper */
        rslt = BMP3_E_INVALID_ODR_OSR_SETTINGS;
    }

    return rslt;
}

/*!
 * @brief This internal API calculates the pressure measurement duration of the
 * sensor.
 */
static uint32_t calculate_press_meas_time(const struct bmp3_dev *dev)
{
    uint32_t press_meas_t;
    struct bmp3_odr_filter_settings odr_filter = dev->settings.odr_filter;

#ifdef BMP3_DOUBLE_PRECISION_COMPENSATION
    double base = 2.0;
    float partial_out;
#else
    uint8_t base = 2;
    uint32_t partial_out;
#endif /* BMP3_DOUBLE_PRECISION_COMPENSATION */
    partial_out = pow_bmp3(base, odr_filter.press_os);
    press_meas_t = (BMP3_SETTLE_TIME_PRESS + partial_out * BMP3_ADC_CONV_TIME);

    /* Output in microseconds */
    return press_meas_t;
}

/*!
 * @brief This internal API calculates the temperature measurement duration of
 * the sensor.
 */
static uint32_t calculate_temp_meas_time(const struct bmp3_dev *dev)
{
    uint32_t temp_meas_t;
    struct bmp3_odr_filter_settings odr_filter = dev->settings.odr_filter;

#ifdef BMP3_DOUBLE_PRECISION_COMPENSATION
    double base = 2.0;
    float partial_out;
#else
    uint8_t base = 2;
    uint32_t partial_out;
#endif /* BMP3_DOUBLE_PRECISION_COMPENSATION */
    partial_out = pow_bmp3(base, odr_filter.temp_os);
    temp_meas_t = (BMP3_SETTLE_TIME_TEMP + partial_out * BMP3_ADC_CONV_TIME);

    /* Output in uint32_t */
    return temp_meas_t;
}

/*!
 * @brief This internal API fills the register address and register data of
 * the over sampling settings for burst write operation.
 */
static void fill_osr_data(uint32_t settings, uint8_t *addr, uint8_t *reg_data, uint8_t *len, const struct bmp3_dev *dev)
{
    struct bmp3_odr_filter_settings osr_settings = dev->settings.odr_filter;

    if (settings & (BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS))
    {
        /* Pressure over sampling settings check */
        if (settings & BMP3_SEL_PRESS_OS)
        {
            /* Set the pressure over sampling settings in the
             * register variable */
            reg_data[*len] = BMP3_SET_BITS_POS_0(reg_data[0], BMP3_PRESS_OS, osr_settings.press_os);
        }

        /* Temperature over sampling settings check */
        if (settings & BMP3_SEL_TEMP_OS)
        {
            /* Set the temperature over sampling settings in the
             * register variable */
            reg_data[*len] = BMP3_SET_BITS(reg_data[0], BMP3_TEMP_OS, osr_settings.temp_os);
        }

        /* 0x1C is the register address of over sampling register */
        addr[*len] = BMP3_REG_OSR;
        (*len)++;
    }
}

/*!
 * @brief This internal API fills the register address and register data of
 * the ODR settings for burst write operation.
 */
static void fill_odr_data(uint8_t *addr, uint8_t *reg_data, uint8_t *len, struct bmp3_dev *dev)
{
    struct bmp3_odr_filter_settings *osr_settings = &dev->settings.odr_filter;

    /* Limit the ODR to 0.001525879 Hz*/
    if (osr_settings->odr > BMP3_ODR_0_001_HZ)
    {
        osr_settings->odr = BMP3_ODR_0_001_HZ;
    }

    /* Set the ODR settings in the register variable */
    reg_data[*len] = BMP3_SET_BITS_POS_0(reg_data[1], BMP3_ODR, osr_settings->odr);

    /* 0x1D is the register address of output data rate register */
    addr[*len] = BMP3_REG_ODR;
    (*len)++;
}

/*!
 * @brief This internal API fills the register address and register data of
 * the filter settings for burst write operation.
 */
static void fill_filter_data(uint8_t *addr, uint8_t *reg_data, uint8_t *len, const struct bmp3_dev *dev)
{
    struct bmp3_odr_filter_settings osr_settings = dev->settings.odr_filter;

    /* Set the iir settings in the register variable */
    reg_data[*len] = BMP3_SET_BITS(reg_data[3], BMP3_IIR_FILTER, osr_settings.iir_filter);

    /* 0x1F is the register address of iir filter register */
    addr[*len] = BMP3_REG_CONFIG;
    (*len)++;
}

/*!
 *  @brief This internal API is used to parse the pressure or temperature or
 *  both the data and store it in the bmp3_uncomp_data structure instance.
 */
static void parse_sensor_data(const uint8_t *reg_data, struct bmp3_uncomp_data *uncomp_data)
{
    /* Temporary variables to store the sensor data */
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;

    /* Store the parsed register values for pressure data */
    data_xlsb = (uint32_t)reg_data[0];
    data_lsb = (uint32_t)reg_data[1] << 8;
    data_msb = (uint32_t)reg_data[2] << 16;
    uncomp_data->pressure = data_msb | data_lsb | data_xlsb;

    /* Store the parsed register values for temperature data */
    data_xlsb = (uint32_t)reg_data[3];
    data_lsb = (uint32_t)reg_data[4] << 8;
    data_msb = (uint32_t)reg_data[5] << 16;
    uncomp_data->temperature = data_msb | data_lsb | data_xlsb;
}

/*!
 * @brief This internal API is used to compensate the pressure or temperature
 * or both the data according to the component selected by the user.
 */
static int8_t compensate_data(uint8_t sensor_comp,
                              const struct bmp3_uncomp_data *uncomp_data,
                              struct bmp3_data *comp_data,
                              struct bmp3_calib_data *calib_data)
{
    int8_t rslt = BMP3_OK;

    if ((uncomp_data != NULL) && (comp_data != NULL) && (calib_data != NULL))
    {
        /* If pressure or temperature component is selected */
        if (sensor_comp & (BMP3_PRESS | BMP3_TEMP))
        {
            /* Compensate the temperature data */
            comp_data->temperature = compensate_temperature(uncomp_data, calib_data);
        }

        if (sensor_comp & BMP3_PRESS)
        {
            /* Compensate the pressure data */
            comp_data->pressure = compensate_pressure(uncomp_data, calib_data);
        }
    }
    else
    {
        rslt = BMP3_E_NULL_PTR;
    }

    return rslt;
}

#ifdef BMP3_DOUBLE_PRECISION_COMPENSATION

/*!
 *  @brief This internal API is used to parse the calibration data, compensates
 *  it and store it in device structure
 */
static void parse_calib_data(const uint8_t *reg_data, struct bmp3_dev *dev)
{
    /* Temporary variable to store the aligned trim data */
    struct bmp3_reg_calib_data *reg_calib_data = &dev->calib_data.reg_calib_data;
    struct bmp3_quantized_calib_data *quantized_calib_data = &dev->calib_data.quantized_calib_data;

    /* Temporary variable */
    double temp_var;

    /* 1 / 2^8 */
    temp_var = 0.00390625f;
    reg_calib_data->par_t1 = BMP3_CONCAT_BYTES(reg_data[1], reg_data[0]);
    quantized_calib_data->par_t1 = ((double)reg_calib_data->par_t1 / temp_var);
    reg_calib_data->par_t2 = BMP3_CONCAT_BYTES(reg_data[3], reg_data[2]);
    temp_var = 1073741824.0f;
    quantized_calib_data->par_t2 = ((double)reg_calib_data->par_t2 / temp_var);
    reg_calib_data->par_t3 = (int8_t)reg_data[4];
    temp_var = 281474976710656.0f;
    quantized_calib_data->par_t3 = ((double)reg_calib_data->par_t3 / temp_var);
    reg_calib_data->par_p1 = (int16_t)BMP3_CONCAT_BYTES(reg_data[6], reg_data[5]);
    temp_var = 1048576.0f;
    quantized_calib_data->par_p1 = ((double)(reg_calib_data->par_p1 - (16384)) / temp_var);
    reg_calib_data->par_p2 = (int16_t)BMP3_CONCAT_BYTES(reg_data[8], reg_data[7]);
    temp_var = 536870912.0f;
    quantized_calib_data->par_p2 = ((double)(reg_calib_data->par_p2 - (16384)) / temp_var);
    reg_calib_data->par_p3 = (int8_t)reg_data[9];
    temp_var = 4294967296.0f;
    quantized_calib_data->par_p3 = ((double)reg_calib_data->par_p3 / temp_var);
    reg_calib_data->par_p4 = (int8_t)reg_data[10];
    temp_var = 137438953472.0f;
    quantized_calib_data->par_p4 = ((double)reg_calib_data->par_p4 / temp_var);
    reg_calib_data->par_p5 = BMP3_CONCAT_BYTES(reg_data[12], reg_data[11]);

    /* 1 / 2^3 */
    temp_var = 0.125f;
    quantized_calib_data->par_p5 = ((double)reg_calib_data->par_p5 / temp_var);
    reg_calib_data->par_p6 = BMP3_CONCAT_BYTES(reg_data[14], reg_data[13]);
    temp_var = 64.0f;
    quantized_calib_data->par_p6 = ((double)reg_calib_data->par_p6 / temp_var);
    reg_calib_data->par_p7 = (int8_t)reg_data[15];
    temp_var = 256.0f;
    quantized_calib_data->par_p7 = ((double)reg_calib_data->par_p7 / temp_var);
    reg_calib_data->par_p8 = (int8_t)reg_data[16];
    temp_var = 32768.0f;
    quantized_calib_data->par_p8 = ((double)reg_calib_data->par_p8 / temp_var);
    reg_calib_data->par_p9 = (int16_t)BMP3_CONCAT_BYTES(reg_data[18], reg_data[17]);
    temp_var = 281474976710656.0f;
    quantized_calib_data->par_p9 = ((double)reg_calib_data->par_p9 / temp_var);
    reg_calib_data->par_p10 = (int8_t)reg_data[19];
    temp_var = 281474976710656.0f;
    quantized_calib_data->par_p10 = ((double)reg_calib_data->par_p10 / temp_var);
    reg_calib_data->par_p11 = (int8_t)reg_data[20];
    temp_var = 36893488147419103232.0f;
    quantized_calib_data->par_p11 = ((double)reg_calib_data->par_p11 / temp_var);
}

/*!
 * @brief This internal API is used to compensate the raw temperature data and
 * return the compensated temperature data in double data type.
 * for e.g. returns temperature 24.26 deg Celsius
 */
static double compensate_temperature(const struct bmp3_uncomp_data *uncomp_data, struct bmp3_calib_data *calib_data)
{
    uint32_t uncomp_temp = uncomp_data->temperature;
    double partial_data1;
    double partial_data2;

    partial_data1 = (double)(uncomp_temp - calib_data->quantized_calib_data.par_t1);
    partial_data2 = (double)(partial_data1 * calib_data->quantized_calib_data.par_t2);

    /* Update the compensated temperature in calib structure since this is
     * needed for pressure calculation */
    calib_data->quantized_calib_data.t_lin = partial_data2 + (partial_data1 * partial_data1) *
                                             calib_data->quantized_calib_data.par_t3;

    /* Returns compensated temperature */
    return calib_data->quantized_calib_data.t_lin;
}

/*!
 * @brief This internal API is used to compensate the raw pressure data and
 * return the compensated pressure data in double data type.
 * For e.g. returns pressure in Pascal p = 95305.295 which is 953.05295 hecto pascal
 */
static double compensate_pressure(const struct bmp3_uncomp_data *uncomp_data, const struct bmp3_calib_data *calib_data)
{
    const struct bmp3_quantized_calib_data *quantized_calib_data = &calib_data->quantized_calib_data;

    /* Variable to store the compensated pressure */
    double comp_press;

    /* Temporary variables used for compensation */
    double partial_data1;
    double partial_data2;
    double partial_data3;
    double partial_data4;
    double partial_out1;
    double partial_out2;

    partial_data1 = quantized_calib_data->par_p6 * quantized_calib_data->t_lin;
    partial_data2 = quantized_calib_data->par_p7 * pow_bmp3(quantized_calib_data->t_lin, 2);
    partial_data3 = quantized_calib_data->par_p8 * pow_bmp3(quantized_calib_data->t_lin, 3);
    partial_out1 = quantized_calib_data->par_p5 + partial_data1 + partial_data2 + partial_data3;
    partial_data1 = quantized_calib_data->par_p2 * quantized_calib_data->t_lin;
    partial_data2 = quantized_calib_data->par_p3 * pow_bmp3(quantized_calib_data->t_lin, 2);
    partial_data3 = quantized_calib_data->par_p4 * pow_bmp3(quantized_calib_data->t_lin, 3);
    partial_out2 = uncomp_data->pressure *
                   (quantized_calib_data->par_p1 + partial_data1 + partial_data2 + partial_data3);
    partial_data1 = pow_bmp3((double)uncomp_data->pressure, 2);
    partial_data2 = quantized_calib_data->par_p9 + quantized_calib_data->par_p10 * quantized_calib_data->t_lin;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + pow_bmp3((double)uncomp_data->pressure, 3) * quantized_calib_data->par_p11;
    comp_press = partial_out1 + partial_out2 + partial_data4;

    return comp_press;
}

/*!
 * @brief This internal API is used to calculate the power functionality for
 *  floating point values.
 */
static float pow_bmp3(double base, uint8_t power)
{
    float pow_output = 1;

    while (power != 0)
    {
        pow_output = (float) base * pow_output;
        power--;
    }

    return pow_output;
}
#else

/*!
 *  @brief This internal API is used to parse the calibration data, compensates
 *  it and store it in device structure
 */
static void parse_calib_data(const uint8_t *reg_data, struct bmp3_dev *dev)
{
    /* Temporary variable to store the aligned trim data */
    struct bmp3_reg_calib_data *reg_calib_data = &dev->calib_data.reg_calib_data;

    reg_calib_data->par_t1 = BMP3_CONCAT_BYTES(reg_data[1], reg_data[0]);
    reg_calib_data->par_t2 = BMP3_CONCAT_BYTES(reg_data[3], reg_data[2]);
    reg_calib_data->par_t3 = (int8_t)reg_data[4];
    reg_calib_data->par_p1 = (int16_t)BMP3_CONCAT_BYTES(reg_data[6], reg_data[5]);
    reg_calib_data->par_p2 = (int16_t)BMP3_CONCAT_BYTES(reg_data[8], reg_data[7]);
    reg_calib_data->par_p3 = (int8_t)reg_data[9];
    reg_calib_data->par_p4 = (int8_t)reg_data[10];
    reg_calib_data->par_p5 = BMP3_CONCAT_BYTES(reg_data[12], reg_data[11]);
    reg_calib_data->par_p6 = BMP3_CONCAT_BYTES(reg_data[14], reg_data[13]);
    reg_calib_data->par_p7 = (int8_t)reg_data[15];
    reg_calib_data->par_p8 = (int8_t)reg_data[16];
    reg_calib_data->par_p9 = (int16_t)BMP3_CONCAT_BYTES(reg_data[18], reg_data[17]);
    reg_calib_data->par_p10 = (int8_t)reg_data[19];
    reg_calib_data->par_p11 = (int8_t)reg_data[20];
}

/*!
 * @brief This internal API is used to compensate the raw temperature data and
 * return the compensated temperature data in integer data type.
 * For eg if returned temperature is 2426 then it is 2426/100 = 24.26 deg Celsius
 */
static int64_t compensate_temperature(const struct bmp3_uncomp_data *uncomp_data, struct bmp3_calib_data *calib_data)
{
    int64_t partial_data1;
    int64_t partial_data2;
    int64_t partial_data3;
    int64_t partial_data4;
    int64_t partial_data5;
    int64_t partial_data6;
    int64_t comp_temp;

    partial_data1 = ((int64_t)uncomp_data->temperature - (256 * calib_data->reg_calib_data.par_t1));
    partial_data2 = calib_data->reg_calib_data.par_t2 * partial_data1;
    partial_data3 = (partial_data1 * partial_data1);
    partial_data4 = (int64_t)partial_data3 * calib_data->reg_calib_data.par_t3;
    partial_data5 = ((int64_t)(partial_data2 * 262144) + partial_data4);
    partial_data6 = partial_data5 / 4294967296;

    /* Store t_lin in dev. structure for pressure calculation */
    calib_data->reg_calib_data.t_lin = partial_data6;
    comp_temp = (int64_t)((partial_data6 * 25) / 16384);

    return comp_temp;
}

/*!
 * @brief This internal API is used to compensate the raw pressure data and
 * return the compensated pressure data in integer data type.
 * for eg return if pressure is 9528709 which is 9528709/100 = 95287.09 Pascal or 952.8709 hecto Pascal
 */
static uint64_t compensate_pressure(const struct bmp3_uncomp_data *uncomp_data,
                                    const struct bmp3_calib_data *calib_data)
{
    const struct bmp3_reg_calib_data *reg_calib_data = &calib_data->reg_calib_data;
    int64_t partial_data1;
    int64_t partial_data2;
    int64_t partial_data3;
    int64_t partial_data4;
    int64_t partial_data5;
    int64_t partial_data6;
    int64_t offset;
    int64_t sensitivity;
    uint64_t comp_press;

    partial_data1 = reg_calib_data->t_lin * reg_calib_data->t_lin;
    partial_data2 = partial_data1 / 64;
    partial_data3 = (partial_data2 * reg_calib_data->t_lin) / 256;
    partial_data4 = (reg_calib_data->par_p8 * partial_data3) / 32;
    partial_data5 = (reg_calib_data->par_p7 * partial_data1) * 16;
    partial_data6 = (reg_calib_data->par_p6 * reg_calib_data->t_lin) * 4194304;
    offset = (reg_calib_data->par_p5 * 140737488355328) + partial_data4 + partial_data5 + partial_data6;
    partial_data2 = (reg_calib_data->par_p4 * partial_data3) / 32;
    partial_data4 = (reg_calib_data->par_p3 * partial_data1) * 4;
    partial_data5 = (reg_calib_data->par_p2 - 16384) * reg_calib_data->t_lin * 2097152;
    sensitivity = ((reg_calib_data->par_p1 - 16384) * 70368744177664) + partial_data2 + partial_data4 + partial_data5;
    partial_data1 = (sensitivity / 16777216) * uncomp_data->pressure;
    partial_data2 = reg_calib_data->par_p10 * reg_calib_data->t_lin;
    partial_data3 = partial_data2 + (65536 * reg_calib_data->par_p9);
    partial_data4 = (partial_data3 * uncomp_data->pressure) / 8192;

    /* dividing by 10 followed by multiplying by 10
     * To avoid overflow caused by (uncomp_data->pressure * partial_data4)
     */
    partial_data5 = (uncomp_data->pressure * (partial_data4 / 10)) / 512;
    partial_data5 = partial_data5 * 10;
    partial_data6 = (int64_t)((uint64_t)uncomp_data->pressure * (uint64_t)uncomp_data->pressure);
    partial_data2 = (reg_calib_data->par_p11 * partial_data6) / 65536;
    partial_data3 = (partial_data2 * uncomp_data->pressure) / 128;
    partial_data4 = (offset / 4) + partial_data1 + partial_data5 + partial_data3;
    comp_press = (((uint64_t)partial_data4 * 25) / (uint64_t)1099511627776);

    return comp_press;
}

/*!
 * @brief This internal API is used to calculate the power functionality.
 */
static uint32_t pow_bmp3(uint8_t base, uint8_t power)
{
    uint32_t pow_output = 1;

    while (power != 0)
    {
        pow_output = base * pow_output;
        power--;
    }

    return pow_output;
}

#endif

/*!
 * @brief This internal API is used to identify the settings which the user
 * wants to modify in the sensor.
 */
static uint8_t are_settings_changed(uint32_t sub_settings, uint32_t desired_settings)
{
    uint8_t settings_changed = FALSE;

    if (sub_settings & desired_settings)
    {
        /* User wants to modify this particular settings */
        settings_changed = TRUE;
    }
    else
    {
        /* User don't want to modify this particular settings */
        settings_changed = FALSE;
    }

    return settings_changed;
}

/*!
 * @brief This internal API is used to validate the device structure pointer for
 * null conditions.
 */
static int8_t null_ptr_check(const struct bmp3_dev *dev)
{
    int8_t rslt;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL) ||
        (dev->intf_ptr == NULL))
    {
        /* Device structure pointer is not valid */
        rslt = BMP3_E_NULL_PTR;
    }
    else
    {
        /* Device structure is fine */
        rslt = BMP3_OK;
    }

    return rslt;
}

/*!
 * @brief This internal API is used to parse the fifo_config_1(fifo_mode,
 * fifo_stop_on_full, fifo_time_en, fifo_press_en, fifo_temp_en),
 * fifo_config_2(fifo_subsampling, data_select) and int_ctrl(fwtm_en, ffull_en)
 * settings and store it in device structure
 */
static void parse_fifo_settings(const uint8_t *reg_data, struct bmp3_fifo_settings *dev_fifo)
{
    uint8_t fifo_config_1_data = reg_data[0];
    uint8_t fifo_config_2_data = reg_data[1];
    uint8_t fifo_int_ctrl_data = reg_data[2];

    /* Parse fifo config 1 register data */
    dev_fifo->mode = BMP3_GET_BITS_POS_0(fifo_config_1_data, BMP3_FIFO_MODE);
    dev_fifo->stop_on_full_en = BMP3_GET_BITS(fifo_config_1_data, BMP3_FIFO_STOP_ON_FULL);
    dev_fifo->time_en = BMP3_GET_BITS(fifo_config_1_data, BMP3_FIFO_TIME_EN);
    dev_fifo->press_en = BMP3_GET_BITS(fifo_config_1_data, BMP3_FIFO_PRESS_EN);
    dev_fifo->temp_en = BMP3_GET_BITS(fifo_config_1_data, BMP3_FIFO_TEMP_EN);

    /* Parse fifo config 2 register data */
    dev_fifo->down_sampling = BMP3_GET_BITS_POS_0(fifo_config_2_data, BMP3_FIFO_DOWN_SAMPLING);
    dev_fifo->filter_en = BMP3_GET_BITS(fifo_config_2_data, BMP3_FIFO_FILTER_EN);

    /* Parse fifo related interrupt control data */
    dev_fifo->ffull_en = BMP3_GET_BITS(fifo_int_ctrl_data, BMP3_FIFO_FULL_EN);
    dev_fifo->fwtm_en = BMP3_GET_BITS(fifo_int_ctrl_data, BMP3_FIFO_FWTM_EN);
}

/*!
 * @brief This internal API fills the fifo_config_1(fifo_mode,
 * fifo_stop_on_full, fifo_time_en, fifo_press_en, fifo_temp_en) settings in the
 * reg_data variable so as to burst write in the sensor.
 */
static int8_t fill_fifo_config_1(uint16_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_FIFO_CONFIG_1;
    uint8_t reg_data;

    rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        if (desired_settings & BMP3_SEL_FIFO_MODE)
        {
            /* Fill the FIFO mode register data */
            reg_data = BMP3_SET_BITS_POS_0(reg_data, BMP3_FIFO_MODE, dev->fifo->settings.mode);
        }

        if (desired_settings & BMP3_SEL_FIFO_STOP_ON_FULL_EN)
        {
            /* Fill the stop on full data */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_FIFO_STOP_ON_FULL, dev->fifo->settings.stop_on_full_en);
        }

        if (desired_settings & BMP3_SEL_FIFO_TIME_EN)
        {
            /* Fill the time enable data */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_FIFO_TIME_EN, dev->fifo->settings.time_en);
        }

        if (desired_settings & (BMP3_SEL_FIFO_PRESS_EN | BMP3_SEL_FIFO_TEMP_EN))
        {
            /* Fill the FIFO pressure enable */
            if (desired_settings & BMP3_SEL_FIFO_PRESS_EN)
            {
                if ((dev->fifo->settings.temp_en == BMP3_DISABLE) && (dev->fifo->settings.press_en == BMP3_ENABLE))
                {
                    /* Set the temperature sensor to be enabled */
                    dev->fifo->settings.temp_en = BMP3_ENABLE;
                }

                /* Fill the pressure enable data */
                reg_data = BMP3_SET_BITS(reg_data, BMP3_FIFO_PRESS_EN, dev->fifo->settings.press_en);
            }

            /* Temperature should be enabled to get the pressure data */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_FIFO_TEMP_EN, dev->fifo->settings.temp_en);
        }

        /* Write the power control settings in the register */
        rslt = bmp3_set_regs(&reg_addr, &reg_data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API fills the fifo_config_2(fifo_subsampling,
 * data_select) settings in the reg_data variable so as to burst write
 * in the sensor.
 */
static int8_t fill_fifo_config_2(uint16_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_FIFO_CONFIG_2;
    uint8_t reg_data;

    rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        if (desired_settings & BMP3_SEL_FIFO_DOWN_SAMPLING)
        {
            /* To do check Normal mode */
            /* Fill the down-sampling data */
            reg_data = BMP3_SET_BITS_POS_0(reg_data, BMP3_FIFO_DOWN_SAMPLING, dev->fifo->settings.down_sampling);
        }

        if (desired_settings & BMP3_SEL_FIFO_FILTER_EN)
        {
            /* Fill the filter enable data */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_FIFO_FILTER_EN, dev->fifo->settings.filter_en);
        }

        /* Write the power control settings in the register */
        rslt = bmp3_set_regs(&reg_addr, &reg_data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This internal API fills the fifo interrupt control(fwtm_en, ffull_en)
 * settings in the reg_data variable so as to burst write in the sensor.
 */
static int8_t fill_fifo_int_ctrl(uint16_t desired_settings, struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP3_REG_INT_CTRL;
    uint8_t reg_data;

    rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        if (desired_settings & BMP3_SEL_FIFO_FWTM_EN)
        {
            /* Fill the FIFO watermark interrupt enable data */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_FIFO_FWTM_EN, dev->fifo->settings.fwtm_en);
        }

        if (desired_settings & BMP3_SEL_FIFO_FULL_EN)
        {
            /* Fill the FIFO full interrupt enable data */
            reg_data = BMP3_SET_BITS(reg_data, BMP3_FIFO_FULL_EN, dev->fifo->settings.ffull_en);
        }

        /* Write the power control settings in the register */
        rslt = bmp3_set_regs(&reg_addr, &reg_data, 1, dev);
    }

    return rslt;
}

/*!
 * @brief This API gets the command ready, data ready for pressure and
 * temperature, power on reset status from the sensor.
 */
static int8_t get_sensor_status(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr;
    uint8_t reg_data;

    reg_addr = BMP3_REG_SENS_STATUS;
    rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        dev->status.sensor.cmd_rdy = BMP3_GET_BITS(reg_data, BMP3_STATUS_CMD_RDY);
        dev->status.sensor.drdy_press = BMP3_GET_BITS(reg_data, BMP3_STATUS_DRDY_PRESS);
        dev->status.sensor.drdy_temp = BMP3_GET_BITS(reg_data, BMP3_STATUS_DRDY_TEMP);
        reg_addr = BMP3_REG_EVENT;
        rslt = bmp3_get_regs(reg_addr, &reg_data, 1, dev);
        dev->status.pwr_on_rst = reg_data & 0x01;
    }

    return rslt;
}

/*!
 * @brief This API gets the interrupt (fifo watermark, fifo full, data ready)
 * status from the sensor.
 */
static int8_t get_int_status(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data;

    rslt = bmp3_get_regs(BMP3_REG_INT_STATUS, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        dev->status.intr.fifo_wm = BMP3_GET_BITS_POS_0(reg_data, BMP3_INT_STATUS_FWTM);
        dev->status.intr.fifo_full = BMP3_GET_BITS(reg_data, BMP3_INT_STATUS_FFULL);
        dev->status.intr.drdy = BMP3_GET_BITS(reg_data, BMP3_INT_STATUS_DRDY);
    }

    return rslt;
}

/*!
 * @brief This API gets the fatal, command and configuration error
 * from the sensor.
 */
static int8_t get_err_status(struct bmp3_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data;

    rslt = bmp3_get_regs(BMP3_REG_ERR, &reg_data, 1, dev);

    if (rslt == BMP3_OK)
    {
        dev->status.err.cmd = BMP3_GET_BITS_POS_0(reg_data, BMP3_ERR_FATAL);
        dev->status.err.conf = BMP3_GET_BITS(reg_data, BMP3_ERR_CMD);
        dev->status.err.fatal = BMP3_GET_BITS(reg_data, BMP3_ERR_CONF);
    }

    return rslt;
}
