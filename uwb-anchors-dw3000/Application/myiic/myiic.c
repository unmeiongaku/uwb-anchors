/*
 * myiic.c
 *
 *  Created on: Jun 18, 2024
 *      Author: nguye
 */


#include "myiic.h"
#include "i2c.h"

#define I2C_TIMEOUT_MS	100

/*User I2C communication*/

iic_status_t port_iic_read_bytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg_addr, uint8_t *reg_data, uint32_t length){
	if(HAL_I2C_Mem_Read(hi2c,DevAddress, reg_addr , I2C_MEMADD_SIZE_8BIT, reg_data, (uint16_t)length, I2C_TIMEOUT_MS)!= HAL_OK){
		*reg_data = 0;
		return IIC_ERROR;
	}
	return IIC_OK;
}

iic_status_t port_iic_write_bytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length){
	if(HAL_I2C_Mem_Write(hi2c,DevAddress, reg_addr , I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, (uint16_t)length, I2C_TIMEOUT_MS)!= HAL_OK){
		return IIC_ERROR;
	}
	return IIC_OK;
}

int8_t dev_interface_init(MYIIC_HandleTypedef_t *dev){
	int8_t rslt = IIC_OK;
	if (dev != NULL){
		dev->read = port_iic_read_bytes;
		dev->write = port_iic_write_bytes;
	}
	 else{
		 rslt = IIC_E_NULL_PTR;
	 }
	return rslt;
}

