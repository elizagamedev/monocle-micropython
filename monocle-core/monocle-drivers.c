/*
 * This file is part of the MicroPython for Monocle project:
 *      https://github.com/brilliantlabsAR/monocle-micropython
 *
 * Authored by: Josuah Demangeon (me@josuah.net)
 *              Raj Nakarja / Brilliant Labs Ltd. (raj@itsbrilliant.co)
 *
 * ISC Licence
 *
 * Copyright © 2023 Brilliant Labs Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "monocle.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "nrf_gpio.h"
#include "nrfx_spim.h"
#include "nrfx_twim.h"
#include "nrfx_systick.h"

void monocle_set_led(led_t led, bool enable)
{
    if (led == RED_LED)
    {
        if (enable)
        {
            app_err(monocle_i2c_write(PMIC_I2C_ADDRESS, 0x11, 0x2D, 0x00).fail);
        }
        else
        {
            app_err(monocle_i2c_write(PMIC_I2C_ADDRESS, 0x11, 0x2D, 0x08).fail);
        }
    }
    if (led == GREEN_LED)
    {
        if (enable)
        {
            app_err(monocle_i2c_write(PMIC_I2C_ADDRESS, 0x12, 0x2D, 0x00).fail);
        }
        else
        {
            app_err(monocle_i2c_write(PMIC_I2C_ADDRESS, 0x12, 0x2D, 0x08).fail);
        }
    }
}

static const nrfx_twim_t i2c_bus_0 = NRFX_TWIM_INSTANCE(0);
static const nrfx_twim_t i2c_bus_1 = NRFX_TWIM_INSTANCE(1);
static const nrfx_spim_t spi_bus_2 = NRFX_SPIM_INSTANCE(2);

bool not_real_hardware_flag = false;

i2c_response_t monocle_i2c_read(uint8_t device_address_7bit,
                                uint16_t register_address,
                                uint8_t register_mask)
{
    if (not_real_hardware_flag)
    {
        return (i2c_response_t){.fail = false, .value = 0x00};
    }

    // Populate the default response in case of failure
    i2c_response_t i2c_response = {
        .fail = true,
        .value = 0x00,
    };

    // Create the tx payload, bus handle and transfer descriptors
    uint8_t tx_payload[2] = {(uint8_t)(register_address), 0};

    nrfx_twim_t i2c_handle = i2c_bus_0;

    nrfx_twim_xfer_desc_t i2c_tx = NRFX_TWIM_XFER_DESC_TX(device_address_7bit,
                                                          tx_payload,
                                                          1);

    // Switch bus and use 16-bit addressing if the camera is requested
    if (device_address_7bit == CAMERA_I2C_ADDRESS)
    {
        i2c_handle = i2c_bus_1;
        tx_payload[0] = (uint8_t)(register_address >> 8);
        tx_payload[1] = (uint8_t)register_address;
        i2c_tx.primary_length = 2;
    }

    nrfx_twim_xfer_desc_t i2c_rx = NRFX_TWIM_XFER_DESC_RX(device_address_7bit,
                                                          &i2c_response.value,
                                                          1);

    // Try several times
    for (uint8_t i = 0; i < 3; i++)
    {
        nrfx_err_t tx_err = nrfx_twim_xfer(&i2c_handle, &i2c_tx, 0);

        if (tx_err == NRFX_ERROR_BUSY ||
            tx_err == NRFX_ERROR_NOT_SUPPORTED ||
            tx_err == NRFX_ERROR_INTERNAL ||
            tx_err == NRFX_ERROR_INVALID_ADDR ||
            tx_err == NRFX_ERROR_DRV_TWI_ERR_OVERRUN)
        {
            app_err(tx_err);
        }

        nrfx_err_t rx_err = nrfx_twim_xfer(&i2c_handle, &i2c_rx, 0);

        if (rx_err == NRFX_ERROR_BUSY ||
            rx_err == NRFX_ERROR_NOT_SUPPORTED ||
            rx_err == NRFX_ERROR_INTERNAL ||
            rx_err == NRFX_ERROR_INVALID_ADDR ||
            rx_err == NRFX_ERROR_DRV_TWI_ERR_OVERRUN)
        {
            app_err(rx_err);
        }

        if (tx_err == NRFX_SUCCESS && rx_err == NRFX_SUCCESS)
        {
            i2c_response.fail = false;
            break;
        }
    }

    i2c_response.value &= register_mask;

    return i2c_response;
}

i2c_response_t monocle_i2c_write(uint8_t device_address_7bit,
                                 uint16_t register_address,
                                 uint8_t register_mask,
                                 uint8_t set_value)
{
    i2c_response_t resp = {.fail = false, .value = 0x00};

    if (not_real_hardware_flag)
    {
        return resp;
    }

    if (register_mask != 0xFF)
    {
        resp = monocle_i2c_read(device_address_7bit, register_address, 0xFF);

        if (resp.fail)
        {
            return resp;
        }
    }

    // Create a combined value with the existing data and the new value
    uint8_t updated_value = (resp.value & ~register_mask) |
                            (set_value & register_mask);

    // Create the tx payload, bus handle and transfer descriptor
    uint8_t tx_payload[3] = {(uint8_t)register_address, updated_value, 0};

    nrfx_twim_t i2c_handle = i2c_bus_0;

    nrfx_twim_xfer_desc_t i2c_tx = NRFX_TWIM_XFER_DESC_TX(device_address_7bit,
                                                          tx_payload,
                                                          2);

    // Switch bus and use 16-bit addressing if the camera is requested
    if (device_address_7bit == CAMERA_I2C_ADDRESS)
    {
        i2c_handle = i2c_bus_1;
        tx_payload[0] = (uint8_t)(register_address >> 8);
        tx_payload[1] = (uint8_t)register_address;
        tx_payload[2] = updated_value;
        i2c_tx.primary_length = 3;
    }

    // Try several times
    for (uint8_t i = 0; i < 3; i++)
    {
        nrfx_err_t err = nrfx_twim_xfer(&i2c_handle, &i2c_tx, 0);

        if (err == NRFX_ERROR_BUSY ||
            err == NRFX_ERROR_NOT_SUPPORTED ||
            err == NRFX_ERROR_INTERNAL ||
            err == NRFX_ERROR_INVALID_ADDR ||
            err == NRFX_ERROR_DRV_TWI_ERR_OVERRUN)
        {
            app_err(err);
        }

        if (err == NRFX_SUCCESS)
        {
            break;
        }

        // If the last try failed. Don't continue
        if (i == 2)
        {
            resp.fail = true;
            return resp;
        }
    }

    return resp;
}

void monocle_spi_enable(bool enable)
{
    if (enable == false)
    {
        nrfx_spim_uninit(&spi_bus_2);
        return;
    }

    nrfx_spim_config_t config = NRFX_SPIM_DEFAULT_CONFIG(
        FPGA_FLASH_SPI_SCK_PIN,
        FPGA_FLASH_SPI_SDO_PIN,
        FPGA_FLASH_SPI_SDI_PIN,
        NRFX_SPIM_PIN_NOT_USED);

    config.frequency = NRF_SPIM_FREQ_4M;
    config.mode = NRF_SPIM_MODE_3;
    config.bit_order = NRF_SPIM_BIT_ORDER_LSB_FIRST;

    app_err(nrfx_spim_init(&spi_bus_2, &config, NULL, NULL));
}

static uint8_t bit_reverse(uint8_t byte)
{
    byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4;
    byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2;
    byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1;
    return byte;
}

void monocle_spi_read(spi_device_t spi_device, uint8_t *data, size_t length,
                      bool hold_down_cs)
{
    uint8_t cs_pin;

    switch (spi_device)
    {
    case DISPLAY:
        cs_pin = DISPLAY_CS_PIN;
        break;
    case FPGA:
        cs_pin = FPGA_CS_MODE_PIN;
        break;
    case FLASH:
        cs_pin = FLASH_CS_PIN;
        break;
    }

    if (!nrfx_is_in_ram(data))
    {
        nrf_gpio_pin_set(cs_pin);
        mp_raise_TypeError(MP_ERROR_TEXT("buffer must be a bytes object"));
    }

    nrf_gpio_pin_clear(cs_pin);

    // TODO prevent blocking here, and add a mutex
    nrfx_spim_xfer_desc_t xfer = NRFX_SPIM_XFER_RX(data, length);
    app_err(nrfx_spim_xfer(&spi_bus_2, &xfer, 0));

    if (!hold_down_cs)
    {
        nrf_gpio_pin_set(cs_pin);
    }

    // Flash is LSB first, so we need to flip all the bytes before returning
    if (spi_device == FLASH)
    {
        for (size_t i = 0; i < length; i++)
        {
            data[i] = bit_reverse(data[i]);
        }
    }
}

void monocle_spi_write(spi_device_t spi_device, uint8_t *data, size_t length,
                       bool hold_down_cs)
{
    uint8_t cs_pin;

    switch (spi_device)
    {
    case DISPLAY:
        cs_pin = DISPLAY_CS_PIN;
        break;
    case FPGA:
        cs_pin = FPGA_CS_MODE_PIN;
        break;
    case FLASH:
        cs_pin = FLASH_CS_PIN;
        break;
    }

    if (!nrfx_is_in_ram(data))
    {
        nrf_gpio_pin_set(cs_pin);
        mp_raise_TypeError(MP_ERROR_TEXT("buffer must be a bytes object"));
    }

    nrf_gpio_pin_clear(cs_pin);

    // Flash is LSB first, so we need to flip all the bytes before sending
    if (spi_device == FLASH)
    {
        for (size_t i = 0; i < length; i++)
        {
            data[i] = bit_reverse(data[i]);
        }
    }

    // TODO prevent blocking here, and add a mutex
    nrfx_spim_xfer_desc_t xfer = NRFX_SPIM_XFER_TX(data, length);
    app_err(nrfx_spim_xfer(&spi_bus_2, &xfer, 0));

    if (!hold_down_cs)
    {
        nrf_gpio_pin_set(cs_pin);
    }
}