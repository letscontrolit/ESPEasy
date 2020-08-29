

The following PHY connections are required for RMII PHY data connections:

.. list-table:: RMII PHY Wiring
   :widths: 10 25 25 75
   :header-rows: 1

   * - GPIO
     - RMII Signal
     - ESP32 EMAC Function
     - Notes
   * - 0
     - REF_CLK
     - EMAC_TX_CLK
     - See desciption about the clock
   * - 21
     - TX_EN
     - EMAC_TX_EN
     - 
   * - 19
     - TX0
     - EMAC_TXD0
     - 
   * - 22
     - TX1
     - EMAC_TXD1
     -
   * - 25
     - RX0
     - EMAC_RXD0
     -
   * - 26
     - RX1
     - EMAC_RXD1
     -
   * - 27
     - CRS_DV
     - EMAC_RX_DRV
     -

See `ESP32 datasheet <https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf>`_
