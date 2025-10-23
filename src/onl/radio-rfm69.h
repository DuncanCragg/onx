#ifndef RADIO_RFM69_H
#define RADIO_RFM69_H

#define RFM69_SPI_WRITE_MASK                              0x80

#define RFM69_FIFO_SIZE                                     66

#define RFM69_REG_00_FIFO                                 0x00
#define RFM69_REG_01_OPMODE                               0x01
#define RFM69_REG_02_DATAMODUL                            0x02
#define RFM69_REG_03_BITRATEMSB                           0x03
#define RFM69_REG_04_BITRATELSB                           0x04
#define RFM69_REG_05_FDEVMSB                              0x05
#define RFM69_REG_06_FDEVLSB                              0x06
#define RFM69_REG_07_FRFMSB                               0x07
#define RFM69_REG_08_FRFMID                               0x08
#define RFM69_REG_09_FRFLSB                               0x09
#define RFM69_REG_0A_OSC1                                 0x0a
#define RFM69_REG_0B_AFCCTRL                              0x0b
#define RFM69_REG_0C_RESERVED                             0x0c
#define RFM69_REG_0D_LISTEN1                              0x0d
#define RFM69_REG_0E_LISTEN2                              0x0e
#define RFM69_REG_0F_LISTEN3                              0x0f
#define RFM69_REG_10_VERSION                              0x10
#define RFM69_REG_11_PALEVEL                              0x11
#define RFM69_REG_12_PARAMP                               0x12
#define RFM69_REG_13_OCP                                  0x13
#define RFM69_REG_14_RESERVED                             0x14
#define RFM69_REG_15_RESERVED                             0x15
#define RFM69_REG_16_RESERVED                             0x16
#define RFM69_REG_17_RESERVED                             0x17
#define RFM69_REG_18_LNA                                  0x18
#define RFM69_REG_19_RXBW                                 0x19
#define RFM69_REG_1A_AFCBW                                0x1a
#define RFM69_REG_1B_OOKPEAK                              0x1b
#define RFM69_REG_1C_OOKAVG                               0x1c
#define RFM69_REG_1D_OOKFIX                               0x1d
#define RFM69_REG_1E_AFCFEI                               0x1e
#define RFM69_REG_1F_AFCMSB                               0x1f
#define RFM69_REG_20_AFCLSB                               0x20
#define RFM69_REG_21_FEIMSB                               0x21
#define RFM69_REG_22_FEILSB                               0x22
#define RFM69_REG_23_RSSICONFIG                           0x23
#define RFM69_REG_24_RSSIVALUE                            0x24
#define RFM69_REG_25_DIOMAPPING1                          0x25
#define RFM69_REG_26_DIOMAPPING2                          0x26
#define RFM69_REG_27_IRQFLAGS1                            0x27
#define RFM69_REG_28_IRQFLAGS2                            0x28
#define RFM69_REG_29_RSSITHRESH                           0x29
#define RFM69_REG_2A_RXTIMEOUT1                           0x2a
#define RFM69_REG_2B_RXTIMEOUT2                           0x2b
#define RFM69_REG_2C_PREAMBLEMSB                          0x2c
#define RFM69_REG_2D_PREAMBLELSB                          0x2d
#define RFM69_REG_2E_SYNCCONFIG                           0x2e
#define RFM69_REG_2F_SYNCVALUE1                           0x2f
// another 7 sync word bytes follow, 30 through 36 inclusive
#define RFM69_REG_37_PACKETCONFIG1                        0x37
#define RFM69_REG_38_PAYLOADLENGTH                        0x38
#define RFM69_REG_39_NODEADRS                             0x39
#define RFM69_REG_3A_BROADCASTADRS                        0x3a
#define RFM69_REG_3B_AUTOMODES                            0x3b
#define RFM69_REG_3C_FIFOTHRESH                           0x3c
#define RFM69_REG_3D_PACKETCONFIG2                        0x3d
#define RFM69_REG_3E_AESKEY1                              0x3e
// Another 15 AES key bytes follow
#define RFM69_REG_4E_TEMP1                                0x4e
#define RFM69_REG_4F_TEMP2                                0x4f
#define RFM69_REG_58_TESTLNA                              0x58
#define RFM69_REG_5A_TESTPA1                              0x5a
#define RFM69_REG_5C_TESTPA2                              0x5c
#define RFM69_REG_6F_TESTDAGC                             0x6f
#define RFM69_REG_71_TESTAFC                              0x71

// These register masks etc are named wherever possible
// corresponding to the bit and field names in the RFM69 Manual

// RFM69_REG_01_OPMODE
#define RFM69_OPMODE_SEQUENCEROFF                         0x80
#define RFM69_OPMODE_LISTENON                             0x40
#define RFM69_OPMODE_LISTENABORT                          0x20
#define RFM69_OPMODE_MODE                                 0x1c
#define RFM69_OPMODE_MODE_SLEEP                           0x00
#define RFM69_OPMODE_MODE_STDBY                           0x04
#define RFM69_OPMODE_MODE_FS                              0x08
#define RFM69_OPMODE_MODE_TX                              0x0c
#define RFM69_OPMODE_MODE_RX                              0x10

// RFM69_REG_02_DATAMODUL
#define RFM69_DATAMODUL_DATAMODE                          0x60
#define RFM69_DATAMODUL_DATAMODE_PACKET                   0x00
#define RFM69_DATAMODUL_DATAMODE_CONT_WITH_SYNC           0x40
#define RFM69_DATAMODUL_DATAMODE_CONT_WITHOUT_SYNC        0x60
#define RFM69_DATAMODUL_MODULATIONTYPE                    0x18
#define RFM69_DATAMODUL_MODULATIONTYPE_FSK                0x00
#define RFM69_DATAMODUL_MODULATIONTYPE_OOK                0x08
#define RFM69_DATAMODUL_MODULATIONSHAPING                 0x03
#define RFM69_DATAMODUL_MODULATIONSHAPING_FSK_NONE        0x00
#define RFM69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0       0x01
#define RFM69_DATAMODUL_MODULATIONSHAPING_FSK_BT0_5       0x02
#define RFM69_DATAMODUL_MODULATIONSHAPING_FSK_BT0_3       0x03
#define RFM69_DATAMODUL_MODULATIONSHAPING_OOK_NONE        0x00
#define RFM69_DATAMODUL_MODULATIONSHAPING_OOK_BR          0x01
#define RFM69_DATAMODUL_MODULATIONSHAPING_OOK_2BR         0x02

// RFM69_REG_11_PALEVEL
#define RFM69_PALEVEL_PA0ON                               0x80
#define RFM69_PALEVEL_PA1ON                               0x40
#define RFM69_PALEVEL_PA2ON                               0x20
#define RFM69_PALEVEL_OUTPUTPOWER                         0x1f

// RFM69_REG_23_RSSICONFIG
#define RFM69_RSSICONFIG_RSSIDONE                         0x02
#define RFM69_RSSICONFIG_RSSISTART                        0x01

// RFM69_REG_25_DIOMAPPING1
#define RFM69_DIOMAPPING1_DIO0MAPPING                     0xc0
#define RFM69_DIOMAPPING1_DIO0MAPPING_00                  0x00
#define RFM69_DIOMAPPING1_DIO0MAPPING_01                  0x40
#define RFM69_DIOMAPPING1_DIO0MAPPING_10                  0x80
#define RFM69_DIOMAPPING1_DIO0MAPPING_11                  0xc0

#define RFM69_DIOMAPPING1_DIO1MAPPING                     0x30
#define RFM69_DIOMAPPING1_DIO1MAPPING_00                  0x00
#define RFM69_DIOMAPPING1_DIO1MAPPING_01                  0x10
#define RFM69_DIOMAPPING1_DIO1MAPPING_10                  0x20
#define RFM69_DIOMAPPING1_DIO1MAPPING_11                  0x30

#define RFM69_DIOMAPPING1_DIO2MAPPING                     0x0c
#define RFM69_DIOMAPPING1_DIO2MAPPING_00                  0x00
#define RFM69_DIOMAPPING1_DIO2MAPPING_01                  0x04
#define RFM69_DIOMAPPING1_DIO2MAPPING_10                  0x08
#define RFM69_DIOMAPPING1_DIO2MAPPING_11                  0x0c

#define RFM69_DIOMAPPING1_DIO3MAPPING                     0x03
#define RFM69_DIOMAPPING1_DIO3MAPPING_00                  0x00
#define RFM69_DIOMAPPING1_DIO3MAPPING_01                  0x01
#define RFM69_DIOMAPPING1_DIO3MAPPING_10                  0x02
#define RFM69_DIOMAPPING1_DIO3MAPPING_11                  0x03

// RFM69_REG_26_DIOMAPPING2
#define RFM69_DIOMAPPING2_DIO4MAPPING                     0xc0
#define RFM69_DIOMAPPING2_DIO4MAPPING_00                  0x00
#define RFM69_DIOMAPPING2_DIO4MAPPING_01                  0x40
#define RFM69_DIOMAPPING2_DIO4MAPPING_10                  0x80
#define RFM69_DIOMAPPING2_DIO4MAPPING_11                  0xc0

#define RFM69_DIOMAPPING2_DIO5MAPPING                     0x30
#define RFM69_DIOMAPPING2_DIO5MAPPING_00                  0x00
#define RFM69_DIOMAPPING2_DIO5MAPPING_01                  0x10
#define RFM69_DIOMAPPING2_DIO5MAPPING_10                  0x20
#define RFM69_DIOMAPPING2_DIO5MAPPING_11                  0x30

#define RFM69_DIOMAPPING2_CLKOUT                          0x07
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_                   0x00
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_2                  0x01
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_4                  0x02
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_8                  0x03
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_16                 0x04
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_32                 0x05
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_RC                 0x06
#define RFM69_DIOMAPPING2_CLKOUT_FXOSC_OFF                0x07

// RFM69_REG_27_IRQFLAGS1
#define RFM69_IRQFLAGS1_MODEREADY                         0x80
#define RFM69_IRQFLAGS1_RXREADY                           0x40
#define RFM69_IRQFLAGS1_TXREADY                           0x20
#define RFM69_IRQFLAGS1_PLLLOCK                           0x10
#define RFM69_IRQFLAGS1_RSSI                              0x08
#define RFM69_IRQFLAGS1_TIMEOUT                           0x04
#define RFM69_IRQFLAGS1_AUTOMODE                          0x02
#define RFM69_IRQFLAGS1_SYNADDRESSMATCH                   0x01

// RFM69_REG_28_IRQFLAGS2
#define RFM69_IRQFLAGS2_FIFOFULL                          0x80
#define RFM69_IRQFLAGS2_FIFONOTEMPTY                      0x40
#define RFM69_IRQFLAGS2_FIFOLEVEL                         0x20
#define RFM69_IRQFLAGS2_FIFOOVERRUN                       0x10
#define RFM69_IRQFLAGS2_PACKETSENT                        0x08
#define RFM69_IRQFLAGS2_PAYLOADREADY                      0x04
#define RFM69_IRQFLAGS2_CRCOK                             0x02

// RFM69_REG_2E_SYNCCONFIG
#define RFM69_SYNCCONFIG_SYNCON                           0x80
#define RFM69_SYNCCONFIG_FIFOFILLCONDITION_MANUAL         0x40
#define RFM69_SYNCCONFIG_SYNCSIZE                         0x38
#define RFM69_SYNCCONFIG_SYNCSIZE_1                       0x00
#define RFM69_SYNCCONFIG_SYNCSIZE_2                       0x08
#define RFM69_SYNCCONFIG_SYNCSIZE_3                       0x10
#define RFM69_SYNCCONFIG_SYNCSIZE_4                       0x18
#define RFM69_SYNCCONFIG_SYNCSIZE_5                       0x20
#define RFM69_SYNCCONFIG_SYNCSIZE_6                       0x28
#define RFM69_SYNCCONFIG_SYNCSIZE_7                       0x30
#define RFM69_SYNCCONFIG_SYNCSIZE_8                       0x38
#define RFM69_SYNCCONFIG_SYNCSIZE_SYNCTOL                 0x07

// RFM69_REG_37_PACKETCONFIG1
#define RFM69_PACKETCONFIG1_PACKETFORMAT_VARIABLE         0x80
#define RFM69_PACKETCONFIG1_DCFREE                        0x60
#define RFM69_PACKETCONFIG1_DCFREE_NONE                   0x00
#define RFM69_PACKETCONFIG1_DCFREE_MANCHESTER             0x20
#define RFM69_PACKETCONFIG1_DCFREE_WHITENING              0x40
#define RFM69_PACKETCONFIG1_DCFREE_RESERVED               0x60
#define RFM69_PACKETCONFIG1_CRC_ON                        0x10
#define RFM69_PACKETCONFIG1_CRCAUTOCLEAROFF               0x08
#define RFM69_PACKETCONFIG1_ADDRESSFILTERING              0x06
#define RFM69_PACKETCONFIG1_ADDRESSFILTERING_NONE         0x00
#define RFM69_PACKETCONFIG1_ADDRESSFILTERING_NODE         0x02
#define RFM69_PACKETCONFIG1_ADDRESSFILTERING_NODE_BC      0x04
#define RFM69_PACKETCONFIG1_ADDRESSFILTERING_RESERVED     0x06

// RFM69_REG_3C_FIFOTHRESH
#define RFM69_FIFOTHRESH_TXSTARTCONDITION_NOTEMPTY        0x80
#define RFM69_FIFOTHRESH_FIFOTHRESHOLD                    0x7f

// RFM69_REG_3D_PACKETCONFIG2
#define RFM69_PACKETCONFIG2_INTERPACKETRXDELAY            0xf0
#define RFM69_PACKETCONFIG2_RESTARTRX                     0x04
#define RFM69_PACKETCONFIG2_AUTORXRESTARTON               0x02
#define RFM69_PACKETCONFIG2_AESON                         0x01

// RFM69_REG_4E_TEMP1
#define RFM69_TEMP1_TEMPMEASSTART                         0x08
#define RFM69_TEMP1_TEMPMEASRUNNING                       0x04

// RFM69_REG_5A_TESTPA1
#define RFM69_TESTPA1_NORMAL                              0x55
#define RFM69_TESTPA1_BOOST                               0x5d

// RFM69_REG_5C_TESTPA2
#define RFM69_TESTPA2_NORMAL                              0x70
#define RFM69_TESTPA2_BOOST                               0x7c

// RFM69_REG_6F_TESTDAGC
#define RFM69_TESTDAGC_CONTINUOUSDAGC_NORMAL              0x00
#define RFM69_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAON  0x20
#define RFM69_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAOFF 0x30

typedef enum {

  FSK_Rb2Fd5 = 0,    // FSK, Whitening, Rb = 2kbs,    Fd = 5kHz
  FSK_Rb2_4Fd4_8,    // FSK, Whitening, Rb = 2.4kbs,  Fd = 4.8kHz
  FSK_Rb4_8Fd9_6,    // FSK, Whitening, Rb = 4.8kbs,  Fd = 9.6kHz
  FSK_Rb9_6Fd19_2,   // FSK, Whitening, Rb = 9.6kbs,  Fd = 19.2kHz
  FSK_Rb19_2Fd38_4,  // FSK, Whitening, Rb = 19.2kbs, Fd = 38.4kHz
  FSK_Rb38_4Fd76_8,  // FSK, Whitening, Rb = 38.4kbs, Fd = 76.8kHz
  FSK_Rb57_6Fd120,   // FSK, Whitening, Rb = 57.6kbs, Fd = 120kHz
  FSK_Rb125Fd125,    // FSK, Whitening, Rb = 125kbs,  Fd = 125kHz
  FSK_Rb250Fd250,    // FSK, Whitening, Rb = 250kbs,  Fd = 250kHz
  FSK_Rb55555Fd50,   // FSK, Whitening, Rb = 55555kbs,Fd = 50kHz for RFM69 lib compatibility

  GFSK_Rb2Fd5,        // GFSK, Whitening, Rb = 2kbs,    Fd = 5kHz
  GFSK_Rb2_4Fd4_8,    // GFSK, Whitening, Rb = 2.4kbs,  Fd = 4.8kHz
  GFSK_Rb4_8Fd9_6,    // GFSK, Whitening, Rb = 4.8kbs,  Fd = 9.6kHz
  GFSK_Rb9_6Fd19_2,   // GFSK, Whitening, Rb = 9.6kbs,  Fd = 19.2kHz
  GFSK_Rb19_2Fd38_4,  // GFSK, Whitening, Rb = 19.2kbs, Fd = 38.4kHz
  GFSK_Rb38_4Fd76_8,  // GFSK, Whitening, Rb = 38.4kbs, Fd = 76.8kHz
  GFSK_Rb57_6Fd120,   // GFSK, Whitening, Rb = 57.6kbs, Fd = 120kHz
  GFSK_Rb125Fd125,    // GFSK, Whitening, Rb = 125kbs,  Fd = 125kHz
  GFSK_Rb250Fd250,    // GFSK, Whitening, Rb = 250kbs,  Fd = 250kHz
  GFSK_Rb55555Fd50,   // GFSK, Whitening, Rb = 55555kbs,Fd = 50kHz

  OOK_Rb1Bw1,         // OOK, Whitening, Rb = 1kbs,    Rx Bandwidth = 1kHz.
  OOK_Rb1_2Bw75,      // OOK, Whitening, Rb = 1.2kbs,  Rx Bandwidth = 75kHz.
  OOK_Rb2_4Bw4_8,     // OOK, Whitening, Rb = 2.4kbs,  Rx Bandwidth = 4.8kHz.
  OOK_Rb4_8Bw9_6,     // OOK, Whitening, Rb = 4.8kbs,  Rx Bandwidth = 9.6kHz.
  OOK_Rb9_6Bw19_2,    // OOK, Whitening, Rb = 9.6kbs,  Rx Bandwidth = 19.2kHz.
  OOK_Rb19_2Bw38_4,   // OOK, Whitening, Rb = 19.2kbs, Rx Bandwidth = 38.4kHz.
  OOK_Rb32Bw64,       // OOK, Whitening, Rb = 32kbs,   Rx Bandwidth = 64kHz.

} modem_config_choice;

typedef struct {

  uint8_t reg_02;  // RFM69_REG_02_DATAMODUL
  uint8_t reg_03;  // RFM69_REG_03_BITRATEMSB
  uint8_t reg_04;  // RFM69_REG_04_BITRATELSB
  uint8_t reg_05;  // RFM69_REG_05_FDEVMSB
  uint8_t reg_06;  // RFM69_REG_06_FDEVLSB
  uint8_t reg_19;  // RFM69_REG_19_RXBW
  uint8_t reg_1a;  // RFM69_REG_1A_AFCBW
  uint8_t reg_37;  // RFM69_REG_37_PACKETCONFIG1

} modem_config;

#define CONFIG_FSK  (RFM69_DATAMODUL_DATAMODE_PACKET | RFM69_DATAMODUL_MODULATIONTYPE_FSK | RFM69_DATAMODUL_MODULATIONSHAPING_FSK_NONE)
#define CONFIG_GFSK (RFM69_DATAMODUL_DATAMODE_PACKET | RFM69_DATAMODUL_MODULATIONTYPE_FSK | RFM69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0)
#define CONFIG_OOK  (RFM69_DATAMODUL_DATAMODE_PACKET | RFM69_DATAMODUL_MODULATIONTYPE_OOK | RFM69_DATAMODUL_MODULATIONSHAPING_OOK_NONE)

// Choices for RFM69_REG_37_PACKETCONFIG1:
#define CONFIG_NOWHITE    (RFM69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RFM69_PACKETCONFIG1_DCFREE_NONE       | RFM69_PACKETCONFIG1_CRC_ON | RFM69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
#define CONFIG_WHITE      (RFM69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RFM69_PACKETCONFIG1_DCFREE_WHITENING  | RFM69_PACKETCONFIG1_CRC_ON | RFM69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
#define CONFIG_MANCHESTER (RFM69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RFM69_PACKETCONFIG1_DCFREE_MANCHESTER | RFM69_PACKETCONFIG1_CRC_ON | RFM69_PACKETCONFIG1_ADDRESSFILTERING_NONE)

static const modem_config MODEM_CONFIG_TABLE[] = {

    //  02,        03,   04,   05,   06,   19,   1a,  37
    // FSK, No Manchester, no shaping, whitening, CRC, no address filtering
    // AFC BW == RX BW == 2 x bit rate
    // Low modulation indexes of ~ 1 at slow speeds do not seem to work very well. Choose MI of 2.
    { CONFIG_FSK,  0x3e, 0x80, 0x00, 0x52, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb2Fd5
    { CONFIG_FSK,  0x34, 0x15, 0x00, 0x4f, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb2_4Fd4_8
    { CONFIG_FSK,  0x1a, 0x0b, 0x00, 0x9d, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb4_8Fd9_6

    { CONFIG_FSK,  0x0d, 0x05, 0x01, 0x3b, 0xf4, 0xf4, CONFIG_WHITE}, // FSK_Rb9_6Fd19_2
    { CONFIG_FSK,  0x06, 0x83, 0x02, 0x75, 0xf3, 0xf3, CONFIG_WHITE}, // FSK_Rb19_2Fd38_4
    { CONFIG_FSK,  0x03, 0x41, 0x04, 0xea, 0xf2, 0xf2, CONFIG_WHITE}, // FSK_Rb38_4Fd76_8

    { CONFIG_FSK,  0x02, 0x2c, 0x07, 0xae, 0xe2, 0xe2, CONFIG_WHITE}, // FSK_Rb57_6Fd120
    { CONFIG_FSK,  0x01, 0x00, 0x08, 0x00, 0xe1, 0xe1, CONFIG_WHITE}, // FSK_Rb125Fd125
    { CONFIG_FSK,  0x00, 0x80, 0x10, 0x00, 0xe0, 0xe0, CONFIG_WHITE}, // FSK_Rb250Fd250
    { CONFIG_FSK,  0x02, 0x40, 0x03, 0x33, 0x42, 0x42, CONFIG_WHITE}, // FSK_Rb55555Fd50

    //  02,        03,   04,   05,   06,   19,   1a,  37
    // GFSK (BT=1.0), No Manchester, whitening, CRC, no address filtering
    // AFC BW == RX BW == 2 x bit rate
    { CONFIG_GFSK, 0x3e, 0x80, 0x00, 0x52, 0xf4, 0xf5, CONFIG_WHITE}, // GFSK_Rb2Fd5
    { CONFIG_GFSK, 0x34, 0x15, 0x00, 0x4f, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb2_4Fd4_8
    { CONFIG_GFSK, 0x1a, 0x0b, 0x00, 0x9d, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb4_8Fd9_6

    { CONFIG_GFSK, 0x0d, 0x05, 0x01, 0x3b, 0xf4, 0xf4, CONFIG_WHITE}, // GFSK_Rb9_6Fd19_2
    { CONFIG_GFSK, 0x06, 0x83, 0x02, 0x75, 0xf3, 0xf3, CONFIG_WHITE}, // GFSK_Rb19_2Fd38_4
    { CONFIG_GFSK, 0x03, 0x41, 0x04, 0xea, 0xf2, 0xf2, CONFIG_WHITE}, // GFSK_Rb38_4Fd76_8

    { CONFIG_GFSK, 0x02, 0x2c, 0x07, 0xae, 0xe2, 0xe2, CONFIG_WHITE}, // GFSK_Rb57_6Fd120
    { CONFIG_GFSK, 0x01, 0x00, 0x08, 0x00, 0xe1, 0xe1, CONFIG_WHITE}, // GFSK_Rb125Fd125
    { CONFIG_GFSK, 0x00, 0x80, 0x10, 0x00, 0xe0, 0xe0, CONFIG_WHITE}, // GFSK_Rb250Fd250
    { CONFIG_GFSK, 0x02, 0x40, 0x03, 0x33, 0x42, 0x42, CONFIG_WHITE}, // GFSK_Rb55555Fd50

    //  02,        03,   04,   05,   06,   19,   1a,  37
    // OOK, No Manchester, no shaping, whitening, CRC, no address filtering
    // with the help of the SX1231 configuration program
    // AFC BW == RX BW
    // All OOK configs have the default:
    // Threshold Type: Peak
    // Peak Threshold Step: 0.5dB
    // Peak threshiold dec: ONce per chip
    // Fixed threshold: 6dB
    { CONFIG_OOK,  0x7d, 0x00, 0x00, 0x10, 0x88, 0x88, CONFIG_WHITE}, // OOK_Rb1Bw1
    { CONFIG_OOK,  0x68, 0x2b, 0x00, 0x10, 0xf1, 0xf1, CONFIG_WHITE}, // OOK_Rb1_2Bw75
    { CONFIG_OOK,  0x34, 0x15, 0x00, 0x10, 0xf5, 0xf5, CONFIG_WHITE}, // OOK_Rb2_4Bw4_8
    { CONFIG_OOK,  0x1a, 0x0b, 0x00, 0x10, 0xf4, 0xf4, CONFIG_WHITE}, // OOK_Rb4_8Bw9_6
    { CONFIG_OOK,  0x0d, 0x05, 0x00, 0x10, 0xf3, 0xf3, CONFIG_WHITE}, // OOK_Rb9_6Bw19_2
    { CONFIG_OOK,  0x06, 0x83, 0x00, 0x10, 0xf2, 0xf2, CONFIG_WHITE}, // OOK_Rb19_2Bw38_4
    { CONFIG_OOK,  0x03, 0xe8, 0x00, 0x10, 0xe2, 0xe2, CONFIG_WHITE}, // OOK_Rb32Bw64
};

#endif
