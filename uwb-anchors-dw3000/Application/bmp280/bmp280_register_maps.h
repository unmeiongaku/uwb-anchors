/*
 * bmp280_register_maps.h
 *
 *  Created on: Jun 18, 2024
 *      Author: nguye
 */

#ifndef BMP280_BMP280_REGISTER_MAPS_H_
#define BMP280_BMP280_REGISTER_MAPS_H_


enum bmp_register_page{
	BMP280_REG_TEMP_XLSB  = 0xFC,
	BMP280_REG_TEMP_LSB    =  0xFB,
	BMP280_REG_TEMP_MSB   = 0xFA,
	BMP280_REG_TEMP     		 =   (BMP280_REG_TEMP_MSB),
	BMP280_REG_PRESS_XLSB = 0xF9 /* bits: 7-4 */,
	BMP280_REG_PRESS_LSB   = 0xF8,
	BMP280_REG_PRESS_MSB  = 0xF7,
	BMP280_REG_PRESSURE    = (BMP280_REG_PRESS_MSB),
	BMP280_REG_CONFIG      	 = 0xF5, /* bits: 7-5 t_sb; 4-2 filter; 0 spi3w_en */
	BMP280_REG_CTRL        	 = 0xF4, /* bits: 7-5 osrs_t; 4-2 osrs_p; 1-0 mode */
	BMP280_REG_STATUS        = 0xF3, /* bits: 3 measuring; 0 im_update */
	BMP280_REG_CTRL_HUM   = 0xF2, /* bits: 2-0 osrs_h; */
	BMP280_REG_RESET       	 = 0xE0,
	BMP280_REG_ID          		 =  0xD0,
	BMP280_REG_CALIB       	 = 0x88,
	BMP280_REG_HUM_CALIB  =   0x88,
	BMP280_RESET_VALUE     	 = 0xB6,
};


#endif /* BMP280_BMP280_REGISTER_MAPS_H_ */
