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

MYIIC_HandleTypedef_t myiic;

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
