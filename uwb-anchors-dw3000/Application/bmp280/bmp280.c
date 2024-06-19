/*
 * bmp280.c
 *
 *  Created on: Jun 18, 2024
 *      Author: nguye
 */

#include "bmp280.h"
#include "i2c.h"
#include "stdbool.h"
#include "bmp280_register_maps.h"
#include "delay_us.h"
#include "timer.h"


MYIIC_HandleTypedef_t myiic;

void bmp280_init_default_params(bmp280_params_t *params);
dev_status_t read_calibration_data(BMP280_HandleTypedef *dev);
dev_status_t read_hum_calibration_data(BMP280_HandleTypedef *dev);

void bmp280_init_default_params(bmp280_params_t *params) {
	params->mode = BMP280_MODE_NORMAL;
	params->filter = BMP280_FILTER_OFF;
	params->oversampling_pressure = BMP280_STANDARD;
	params->oversampling_temperature = BMP280_STANDARD;
	params->oversampling_humidity = BMP280_STANDARD;
	params->standby = BMP280_STANDBY_250;
}

dev_status_t read_calibration_data(BMP280_HandleTypedef *dev) {
		uint8_t buffer[22] = {0};
		dev->register_addr = 0x88;
		if(myiic.read(dev->i2c,dev->addr,dev->register_addr,buffer,22) != IIC_OK) return DEV_ERROR;
		dev->dig_T1 = (uint16_t)((buffer[1] << 8) | buffer[0]);
		dev->dig_T2 = (int16_t)((buffer[3] << 8) | buffer[2]);
		dev->dig_T3 = (int16_t)((buffer[5] << 8) | buffer[4]);
		dev->dig_P1 = (uint16_t)((buffer[7] << 8) | buffer[6]);
		dev->dig_P2 = (uint16_t)((buffer[9] << 8) | buffer[8]);
		dev->dig_P3 = (uint16_t)((buffer[11] << 8) | buffer[10]);
		dev->dig_P4 = (uint16_t)((buffer[13] << 8) | buffer[12]);
		dev->dig_P5 = (int16_t)((buffer[15] << 8) | buffer[14]);
		dev->dig_P6 = (int16_t)((buffer[17] << 8) | buffer[16]);
		dev->dig_P7 = (int16_t)((buffer[19] << 8) | buffer[18]);
		dev->dig_P8 = (int16_t)((buffer[21] << 8) | buffer[20]);
		dev->dig_P9 = (int16_t)((buffer[23] << 8) | buffer[22]);
	return DEV_OK;
}

dev_status_t read_hum_calibration_data(BMP280_HandleTypedef *dev) {
	uint16_t h4, h5;
	uint8_t buffer[2] = {0};
	dev->register_addr = 0xa1;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,&dev->dig_H1,1) != IIC_OK) return DEV_ERROR;

	dev->register_addr = 0xe1;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,buffer,2) != IIC_OK) return DEV_ERROR;
	dev->dig_H2 = (uint16_t)((buffer[1] << 8) | buffer[0]);

	dev->register_addr = 0xe3;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,&dev->dig_H3,1) != IIC_OK) return DEV_ERROR;

	dev->register_addr = 0xe4;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,buffer,2) != IIC_OK) return DEV_ERROR;
	h4 = (uint16_t)((buffer[1] << 8) | buffer[0]);

	dev->register_addr = 0xe5;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,buffer,2) != IIC_OK) return DEV_ERROR;
	h5 = (uint16_t)((buffer[1] << 8) | buffer[0]);
	dev->dig_H4 =(int16_t)((h4 & 0x00ff) << 4 | (h4 & 0x0f00) >> 8);
	dev->dig_H5 =(int16_t)(h5 >> 4);

	return DEV_OK;
}

dev_status_t bmp280_init(BMP280_HandleTypedef *dev, bmp280_params_t *params) {
	bmp280_init_default_params(params);
	dev->addr = BMP280_I2C_ADDRESS_0;
	dev->i2c = &hi2c1;
	if (dev->addr != BMP280_I2C_ADDRESS_0
			&& dev->addr != BMP280_I2C_ADDRESS_1) {
		return DEV_ERROR;
	}
	dev->addr = (BMP280_I2C_ADDRESS_0<<1);
	dev->register_addr = BMP280_REG_ID;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,&dev->id,1) != IIC_OK) return DEV_ERROR;
	if(dev->id != BMP280_CHIP_ID) return DEV_ERROR;
	// Soft reset.
	dev->register_addr = BMP280_REG_RESET;
	dev->tmp = BMP280_RESET_VALUE;
	if(myiic.write(dev->i2c,dev->addr,dev->register_addr,&dev->tmp,1) != IIC_OK) return DEV_ERROR;
	// Wait until finished copying over the NVP data.
	delay_ms(10);
	while (1) {
		dev->register_addr = BMP280_REG_STATUS;
		uint8_t buffer;
		if(myiic.read(dev->i2c,dev->addr,dev->register_addr,&buffer,1) != IIC_OK) return DEV_ERROR;
		if((buffer & 1) == 0) break;
	}
	if(read_calibration_data(dev) != DEV_OK) return DEV_ERROR;
	if(read_hum_calibration_data(dev) != DEV_OK) return DEV_ERROR;

	uint8_t config = (params->standby << 5) | (params->filter << 2);
	dev->register_addr = BMP280_REG_CONFIG;
	if(myiic.write(dev->i2c,dev->addr,dev->register_addr,&config,1) != IIC_OK) return DEV_ERROR;

	if (params->mode == BMP280_MODE_FORCED) {
		params->mode = BMP280_MODE_SLEEP;  // initial mode for forced is sleep
	}

	uint8_t ctrl = (params->oversampling_temperature << 5)
				| (params->oversampling_pressure << 2) | (params->mode);

	if (dev->id == BME280_CHIP_ID) {
		// Write crtl hum reg first, only active after write to BMP280_REG_CTRL.
		uint8_t ctrl_hum = params->oversampling_humidity;
		dev->register_addr = BMP280_REG_CTRL_HUM;
		if(myiic.write(dev->i2c,dev->addr,dev->register_addr,&ctrl_hum,1) != IIC_OK) return DEV_ERROR;
	}
	dev->register_addr = BMP280_REG_CTRL;
	if(myiic.write(dev->i2c,dev->addr,dev->register_addr,&ctrl,1) != IIC_OK) return DEV_ERROR;

	return DEV_OK;
}


dev_status_t bmp280_force_measurement(BMP280_HandleTypedef *dev) {
	uint8_t ctrl;
	dev->register_addr = BMP280_REG_CTRL;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,&ctrl,1) != IIC_OK) return DEV_ERROR;
	ctrl &= ~0b11;  // clear two lower bits
	ctrl |= BMP280_MODE_FORCED;
	if(myiic.write(dev->i2c,dev->addr,dev->register_addr,&ctrl,1) != IIC_OK) return DEV_ERROR;
	return DEV_OK;
}

dev_status_t bmp280_is_measuring(BMP280_HandleTypedef *dev) {
	uint8_t status;
	dev->register_addr = BMP280_REG_CTRL;
	if(myiic.read(dev->i2c,dev->addr,dev->register_addr,&status,1) != IIC_OK) return DEV_ERROR;
	if (!(status & (1 << 3))) return DEV_ERROR;
	return DEV_OK;
}


/**
 * Compensation algorithm is taken from BMP280 datasheet.
 *
 * Return value is in degrees Celsius.
 */

static inline int32_t compensate_temperature(BMP280_HandleTypedef *dev, int32_t adc_temp,
		int32_t *fine_temp) {
	int32_t var1, var2;

	var1 = ((((adc_temp >> 3) - ((int32_t) dev->dig_T1 << 1)))
			* (int32_t) dev->dig_T2) >> 11;
	var2 = (((((adc_temp >> 4) - (int32_t) dev->dig_T1)
			* ((adc_temp >> 4) - (int32_t) dev->dig_T1)) >> 12)
			* (int32_t) dev->dig_T3) >> 14;

	*fine_temp = var1 + var2;
	return (*fine_temp * 5 + 128) >> 8;
}

/**
 * Compensation algorithm is taken from BMP280 datasheet.
 *
 * Return value is in Pa, 24 integer bits and 8 fractional bits.
 */
static inline uint32_t compensate_pressure(BMP280_HandleTypedef *dev, int32_t adc_press,
		int32_t fine_temp) {
	int64_t var1, var2, p;

	var1 = (int64_t) fine_temp - 128000;
	var2 = var1 * var1 * (int64_t) dev->dig_P6;
	var2 = var2 + ((var1 * (int64_t) dev->dig_P5) << 17);
	var2 = var2 + (((int64_t) dev->dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t) dev->dig_P3) >> 8)
			+ ((var1 * (int64_t) dev->dig_P2) << 12);
	var1 = (((int64_t) 1 << 47) + var1) * ((int64_t) dev->dig_P1) >> 33;

	if (var1 == 0) {
		return 0;  // avoid exception caused by division by zero
	}

	p = 1048576 - adc_press;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = ((int64_t) dev->dig_P9 * (p >> 13) * (p >> 13)) >> 25;
	var2 = ((int64_t) dev->dig_P8 * p) >> 19;

	p = ((p + var1 + var2) >> 8) + ((int64_t) dev->dig_P7 << 4);
	return p;
}

/**
 * Compensation algorithm is taken from BME280 datasheet.
 *
 * Return value is in Pa, 24 integer bits and 8 fractional bits.
 */
static inline uint32_t compensate_humidity(BMP280_HandleTypedef *dev, int32_t adc_hum,
		int32_t fine_temp) {
	int32_t v_x1_u32r;

	v_x1_u32r = fine_temp - (int32_t) 76800;
	v_x1_u32r = ((((adc_hum << 14) - ((int32_t) dev->dig_H4 << 20)
			- ((int32_t) dev->dig_H5 * v_x1_u32r)) + (int32_t) 16384) >> 15)
			* (((((((v_x1_u32r * (int32_t) dev->dig_H6) >> 10)
					* (((v_x1_u32r * (int32_t) dev->dig_H3) >> 11)
							+ (int32_t) 32768)) >> 10) + (int32_t) 2097152)
					* (int32_t) dev->dig_H2 + 8192) >> 14);
	v_x1_u32r = v_x1_u32r
			- (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7)
					* (int32_t) dev->dig_H1) >> 4);
	v_x1_u32r = v_x1_u32r < 0 ? 0 : v_x1_u32r;
	v_x1_u32r = v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r;
	return v_x1_u32r >> 12;
}

dev_status_t bmp280_read_fixed(BMP280_HandleTypedef *dev, int32_t *temperature, uint32_t *pressure, uint32_t *humidity) {
	int32_t adc_pressure = 0;
	int32_t adc_temp = 0;
	uint8_t data[8] = {0};

	// Only the BME280 supports reading the humidity.
	if (dev->id != BME280_CHIP_ID) {
		if (humidity)
			*humidity = 0;
		humidity = NULL;
	}

	// Need to read in one sequence to ensure they match.
		size_t size = humidity ? 8 : 6;
		dev->register_addr = 0xf7;
		if(myiic.read(dev->i2c,dev->addr,dev->register_addr,data,size) != IIC_OK) return DEV_ERROR;

		adc_pressure = ((data[0] << 12) | (data[1] << 4) | data[2] >> 4);
		adc_temp = ((data[3] << 12 | data[4] << 4) | data[5] >> 4);

		int32_t fine_temp;
		*temperature = compensate_temperature(dev, adc_temp, &fine_temp);
	    *pressure = compensate_pressure(dev, adc_pressure, fine_temp);

	    if (humidity){
	    		int32_t adc_humidity = data[6] << 8 | data[7];
	    		*humidity = compensate_humidity(dev, adc_humidity, fine_temp);
	    }

	return DEV_OK;
}

dev_status_t bmp280_read_float(BMP280_HandleTypedef *dev, float *temperature, float *pressure, float *humidity) {
	int32_t fixed_temperature;
	uint32_t fixed_pressure;
	uint32_t fixed_humidity;

	if (bmp280_read_fixed(dev, &fixed_temperature, &fixed_pressure,
			humidity ? &fixed_humidity : NULL)) {
		*temperature = (float) fixed_temperature / 100;
		*pressure = (float) fixed_pressure / 256;
		if (humidity)
			*humidity = (float) fixed_humidity / 1024;
		return DEV_OK;
	}

	return DEV_ERROR;
}

