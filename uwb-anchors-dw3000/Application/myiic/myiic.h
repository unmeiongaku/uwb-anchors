/*
 * myiic.h
 *
 *  Created on: Jun 18, 2024
 *      Author: nguye
 */

#ifndef MYIIC_MYIIC_H_
#define MYIIC_MYIIC_H_

#include "stdint.h"
#include "stm32l4xx_hal.h"



typedef enum {
	IIC_E_NULL_PTR = -1,
	IIC_ERROR = 0,
	IIC_OK = 1,
}iic_status_t;

/*Using for write and read multiple bytes*/
typedef iic_status_t (*dev_read_fptr_t)(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg_addr, uint8_t *reg_data, uint32_t length);
typedef iic_status_t (*dev_write_fptr_t)(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length);

/********************************************************* */
/*!               Function Pointers                       */
/********************************************************* */

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 *
 * @param[in]     reg_addr : 8bit register address of the sensor
 * @param[out]    reg_data : Data from the specified address
 * @param[in]     length   : Length of the reg_data array
 * @param[in,out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                           for interface related callbacks
 * @retval 0 for Success
 * @retval Non-zero for Failure
 */

iic_status_t port_iic_read_bytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg_addr, uint8_t *reg_data, uint32_t length);


/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific write functions of the user
 *
 * @param[in]     reg_addr : 8bit register address of the sensor
 * @param[out]    reg_data : Data to the specified address
 * @param[in]     length   : Length of the reg_data array
 * @param[in,out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                           for interface related callbacks
 * @retval 0 for Success
 * @retval Non-zero for Failure
 *
 */

iic_status_t port_iic_write_bytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length);



typedef struct{
	dev_read_fptr_t read;
	dev_write_fptr_t write;
}MYIIC_HandleTypedef_t;


#endif /* MYIIC_MYIIC_H_ */
