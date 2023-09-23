// SPDX-License-Identifier: (GPL-2.0+ or MIT)
/*
 * Copyright (C) 2020 huangzhenwei@allwinnertech.com
 */

#ifndef _DT_BINDINGS_CLK_SUN8IW20_H_
#define _DT_BINDINGS_CLK_SUN8IW20_H_

#define CLK_OSC12M		0
#define CLK_PLL_CPUX		1
#define CLK_PLL_DDR0		2
#define CLK_PLL_PERIPH0_PARENT	3
#define CLK_PLL_PERIPH0		4
#define CLK_PLL_PERIPH0_2X	5
#define CLK_PLL_PERIPH0_800M	6
#define CLK_PLL_PERIPH0_DIV3	7
#define CLK_PLL_VIDEO0		8
#define CLK_PLL_VIDEO0_2X	9
#define CLK_PLL_VIDEO0_4X	10
#define CLK_PLL_VIDEO1		11
#define CLK_PLL_VIDEO1_2X	12
#define CLK_PLL_VIDEO1_4X	13
#define CLK_PLL_VE		14
#define CLK_PLL_AUDIO0		15
#define CLK_PLL_AUDIO0_2X	16
#define CLK_PLL_AUDIO0_4X	17
#define CLK_PLL_AUDIO1		18
#define CLK_PLL_AUDIO1_DIV2	19
#define CLK_PLL_AUDIO1_DIV5	20
#define CLK_PLL_CPUX_DIV	21
#define CLK_CPUX		22
#define CLK_AXI			23
#define CLK_APB			24
#define CLK_PSI_AHB		25
#define CLK_APB0		26
#define CLK_APB1		27
#define CLK_MBUS		28
#define CLK_DE0			29
#define CLK_BUS_DE0		30
#define CLK_DI			31
#define CLK_BUS_DI		32
#define CLK_G2D			33
#define CLK_BUS_G2D		34
#define CLK_CE			35
#define CLK_BUS_CE		36
#define CLK_VE			37
#define CLK_BUS_VE		38
#define CLK_BUS_DMA		39
#define CLK_BUS_MSGBOX0		40
#define CLK_BUS_MSGBOX1		41
#define CLK_BUS_MSGBOX2		42
#define CLK_BUS_SPINLOCK	43
#define CLK_BUS_HSTIMER		44
#define CLK_AVS			45
#define CLK_BUS_DBG		46
#define CLK_BUS_PWM		47
#define CLK_BUS_IOMMU		48
#define CLK_DRAM		49
#define CLK_MBUS_DMA		50
#define CLK_MBUS_VE		51
#define CLK_MBUS_CE		52
#define CLK_MBUS_TVIN		53
#define CLK_MBUS_CSI		54
#define CLK_MBUS_G2D		55
#define CLK_BUS_DRAM		56
#define CLK_MMC0		57
#define CLK_MMC1		58
#define CLK_MMC2		59
#define CLK_BUS_MMC0		60
#define CLK_BUS_MMC1		61
#define CLK_BUS_MMC2		62
#define CLK_BUS_UART0		63
#define CLK_BUS_UART1		64
#define CLK_BUS_UART2		65
#define CLK_BUS_UART3		66
#define CLK_BUS_UART4		67
#define CLK_BUS_UART5		68
#define CLK_BUS_I2C0		69
#define CLK_BUS_I2C1		70
#define CLK_BUS_I2C2		71
#define CLK_BUS_I2C3		72
#define CLK_SPI0		75
#define CLK_SPI1		76
#define CLK_BUS_SPI0		77
#define CLK_BUS_SPI1		78
#define CLK_EMAC0_25M		79
#define CLK_BUS_EMAC0		80
#define CLK_IR_TX		81
#define CLK_BUS_IR_TX		82
#define CLK_BUS_GPADC		83
#define CLK_BUS_THS		84
#define CLK_I2S0		85
#define CLK_I2S1		86
#define CLK_I2S2		87
#define CLK_I2S2_ASRC		88
#define CLK_BUS_I2S0		89
#define CLK_BUS_I2S1		90
#define CLK_BUS_I2S2		91
#define CLK_SPDIF_TX		92
#define CLK_SPDIF_RX		93
#define CLK_BUS_SPDIF		94
#define CLK_DMIC		95
#define CLK_BUS_DMIC		96
#define CLK_AUDIO_DAC		97
#define CLK_AUDIO_ADC		98
#define CLK_BUS_AUDIO_CODEC	99
#define CLK_USB_OHCI0		100
#define CLK_USB_OHCI1		101
#define CLK_BUS_OHCI0		102
#define CLK_BUS_OHCI1		103
#define CLK_BUS_EHCI0		104
#define CLK_BUS_EHCI1		105
#define CLK_BUS_OTG		106
#define CLK_BUS_LRADC		107
#define CLK_BUS_DPSS_TOP0	108
#define CLK_HDMI_24M		109
#define CLK_HDMI_CEC		110
#define CLK_HDMI_CEC_32K	111
#define CLK_BUS_HDMI		112
#define CLK_MIPI_DSI		113
#define CLK_BUS_MIPI_DSI	114
#define CLK_TCON_LCD0		115
#define CLK_BUS_TCON_LCD0	116
#define CLK_TCON_TV		117
#define CLK_BUS_TCON_TV		118
#define CLK_TVE			119
#define CLK_BUS_TVE		120
#define CLK_BUS_TVE_TOP		121
#define CLK_TVD			122
#define CLK_BUS_TVD		123
#define CLK_BUS_TVD_TOP		124
#define CLK_LEDC		125
#define CLK_BUS_LEDC		126
#define CLK_CSI_TOP		127
#define CLK_CSI0_MCLK		128
#define CLK_BUS_CSI		129
#define CLK_TPADC		130
#define CLK_BUS_TPADC		131
#define CLK_BUS_TZMA		132
#define CLK_DSP			133
#define CLK_BUS_DSP_CFG		134
#define CLK_RISCV		135
#define CLK_RISCV_AXI		136
#define CLK_BUS_RISCV_CFG	137
#define CLK_FANOUT_24M		138
#define CLK_FANOUT_12M		139
#define CLK_FANOUT_16M		140
#define CLK_FANOUT_25M		141
#define CLK_FANOUT_32K		142
#define CLK_FANOUT_27M		143
#define CLK_FANOUT_PCLK		144
#define CLK_FANOUT0_OUT		145
#define CLK_FANOUT1_OUT		146
#define CLK_FANOUT2_OUT		147

#define CLK_MAX_NO		CLK_FANOUT2_OUT

#endif /* _DT_BINDINGS_CLK_SUN8IW20_H_ */
