// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 frank@allwinnertech.com
 */

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "ccu_common.h"
#include "ccu_reset.h"

#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mp.h"
#include "ccu_mult.h"
#include "ccu_nk.h"
#include "ccu_nkm.h"
#include "ccu_nkmp.h"
#include "ccu_nm.h"

#include "ccu-sun50iw9.h"

/*
 * The CPU PLL is actually NP clock, with P being /1, /2 or /4. However
 * P should only be used for output frequencies lower than 288 MHz.
 *
 * For now we can just model it as a multiplier clock, and force P to /1.
 *
 * The M factor is present in the register's description, but not in the
 * frequency formula, and it's documented as "M is only used for backdoor
 * testing", so it's not modelled and then force to 0.
 */
#define SUN50IW9_PLL_CPUX_REG		0x000
static struct ccu_mult pll_cpux_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.mult		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.common		= {
		.reg		= 0x000,
		.hw.init	= CLK_HW_INIT("pll-cpux", "dcxo24M",
					      &ccu_mult_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

/* Some PLLs are input * N / div1 / P. Model them as NKMP with no K */
#define SUN50IW9_PLL_DDR0_REG		0x010
static struct ccu_nkmp pll_ddr0_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.common		= {
		.reg		= 0x010,
		.hw.init	= CLK_HW_INIT("pll-ddr0", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE |
					      CLK_IS_CRITICAL),
	},
};

#define SUN50IW9_PLL_DDR1_REG		0x018
static struct ccu_nkmp pll_ddr1_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.common		= {
		.reg		= 0x018,
		.hw.init	= CLK_HW_INIT("pll-ddr1", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE |
					      CLK_IS_CRITICAL),
	},
};

#define SUN50IW9_PLL_PERIPH0_REG	0x020
static struct ccu_nkmp pll_periph0_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.fixed_post_div	= 2,
	.common		= {
		.reg		= 0x020,
		.features	= CCU_FEATURE_FIXED_POSTDIV,
		.hw.init	= CLK_HW_INIT("pll-periph0", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW9_PLL_PERIPH1_REG	0x028
static struct ccu_nkmp pll_periph1_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.fixed_post_div	= 2,
	.common		= {
		.reg		= 0x028,
		.features	= CCU_FEATURE_FIXED_POSTDIV,
		.hw.init	= CLK_HW_INIT("pll-periph1", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW9_PLL_GPU_REG		0x030
static struct ccu_nkmp pll_gpu_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.common		= {
		.reg		= 0x030,
		.hw.init	= CLK_HW_INIT("pll-gpu", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

/*
 * For Video PLLs, the output divider is described as "used for testing"
 * in the user manual. So it's not modelled and forced to 0.
 */
#define SUN50IW9_PLL_VIDEO0_REG		0x040
static struct ccu_nm pll_video0_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.fixed_post_div	= 4,
	.min_rate	= 288000000,
	.max_rate	= 2400000000UL,
	.common		= {
		.reg		= 0x040,
		.features	= CCU_FEATURE_FIXED_POSTDIV,
		.hw.init	= CLK_HW_INIT("pll-video0", "dcxo24M",
					      &ccu_nm_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW9_PLL_VIDEO1_REG		0x048
static struct ccu_nm pll_video1_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.fixed_post_div	= 4,
	.min_rate	= 288000000,
	.max_rate	= 2400000000UL,
	.common		= {
		.reg		= 0x048,
		.features	= CCU_FEATURE_FIXED_POSTDIV,
		.hw.init	= CLK_HW_INIT("pll-video1", "dcxo24M",
					      &ccu_nm_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW9_PLL_VIDEO2_REG		0x050
static struct ccu_nm pll_video2_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.fixed_post_div	= 4,
	.min_rate	= 288000000,
	.max_rate	= 2400000000UL,
	.common		= {
		.reg		= 0x050,
		.features	= CCU_FEATURE_FIXED_POSTDIV,
		.hw.init	= CLK_HW_INIT("pll-video2", "dcxo24M",
					      &ccu_nm_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW9_PLL_VE_REG		0x058
static struct ccu_nkmp pll_ve_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.common		= {
		.reg		= 0x058,
		.hw.init	= CLK_HW_INIT("pll-ve", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW9_PLL_DE_REG		0x060
static struct ccu_nkmp pll_de_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.common		= {
		.reg		= 0x060,
		.hw.init	= CLK_HW_INIT("pll-de", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

/* for pm resume */
#define SUN50IW9_PLL_PERIPH1_PATTERN0_REG	0x128
static struct ccu_common pll_periph1_pattern0_common = {
	.reg = 0x128,
};

/*
 * The Audio PLL has m0, m1 dividers in addition to the usual N, M
 * factors. Since we only need 4 frequencies from this PLL: 22.5792 MHz,
 * 24.576 MHz, 90.3168MHz and 98.304MHz ignore them for now.
 * Enforce the default for them, which is m0 = 1, m1 = 0.
 */
#define SUN50IW9_PLL_AUDIO_REG		0x078
static struct ccu_sdm_setting pll_audio_sdm_table[] = {
	{ .rate = 45158400, .pattern = 0xc001bcd3, .m = 18, .n = 33 },
	{ .rate = 49152000, .pattern = 0xc001eb85, .m = 20, .n = 40 },
	{ .rate = 180633600, .pattern = 0xc001288d, .m = 3, .n = 22 },
	{ .rate = 196608000, .pattern = 0xc001eb85, .m = 5, .n = 40 },
};

static struct ccu_nm pll_audio_4x_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(16, 6),
	.fixed_post_div	= 2,
	.sdm		= _SUNXI_CCU_SDM(pll_audio_sdm_table, BIT(24),
					 0x178, BIT(31)),
	.common		= {
		.reg		= 0x078,
		.features	= CCU_FEATURE_FIXED_POSTDIV |
				  CCU_FEATURE_SIGMA_DELTA_MOD,
		.hw.init	= CLK_HW_INIT("pll-audio-4x", "dcxo24M",
					      &ccu_nm_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

#define SUN50IW9_PLL_CSI_REG		0x0e0
static struct ccu_nkmp pll_csi_clk = {
	.enable		= BIT(27),
	.lock		= BIT(28),
	.n		= _SUNXI_CCU_MULT_MIN(8, 8, 12),
	.m		= _SUNXI_CCU_DIV(1, 1), /* input divider */
	.p		= _SUNXI_CCU_DIV(0, 1), /* output divider */
	.common		= {
		.reg		= 0x0e0,
		.hw.init	= CLK_HW_INIT("pll-csi", "dcxo24M",
					      &ccu_nkmp_ops,
					      CLK_SET_RATE_UNGATE),
	},
};

static const char * const cpux_parents[] = { "dcxo24M", "osc32k",
					     "iosc", "pll-cpux",
					      "pll-periph0" };
static SUNXI_CCU_MUX(cpux_clk, "cpux", cpux_parents,
		     0x500, 24, 3, CLK_SET_RATE_PARENT | CLK_IS_CRITICAL);
static SUNXI_CCU_M(axi_clk, "axi", "cpux", 0x500, 0, 2, 0);
static SUNXI_CCU_M(cpux_apb_clk, "cpux-apb", "cpux", 0x500, 8, 2, 0);

static const char * const psi_ahb1_ahb2_parents[] = { "dcxo24M", "osc32k",
						      "iosc", "pll-periph0" };
static SUNXI_CCU_MP_WITH_MUX(psi_ahb1_ahb2_clk, "psi-ahb1-ahb2",
			     psi_ahb1_ahb2_parents,
			     0x510,
			     0, 2,	/* M */
			     8, 2,	/* P */
			     24, 2,	/* mux */
			     0);

static const char * const ahb3_apb1_apb2_parents[] = { "dcxo24M", "osc32k",
						       "psi-ahb1-ahb2",
						       "pll-periph0" };
static SUNXI_CCU_MP_WITH_MUX(ahb3_clk, "ahb3", ahb3_apb1_apb2_parents, 0x51c,
			     0, 2,	/* M */
			     8, 2,	/* P */
			     24, 2,	/* mux */
			     0);

static SUNXI_CCU_MP_WITH_MUX(apb1_clk, "apb1", ahb3_apb1_apb2_parents, 0x520,
			     0, 2,	/* M */
			     8, 2,	/* P */
			     24, 2,	/* mux */
			     0);

static SUNXI_CCU_MP_WITH_MUX(apb2_clk, "apb2", ahb3_apb1_apb2_parents, 0x524,
			     0, 2,	/* M */
			     8, 2,	/* P */
			     24, 2,	/* mux */
			     0);

static const char * const mbus_parents[] = { "dcxo24M", "pll-periph0-2x",
					     "pll-ddr0", "pll-ddr1" };
static SUNXI_CCU_M_WITH_MUX_GATE(mbus_clk, "mbus", mbus_parents, 0x540,
				 0, 3,		/* M */
				 24, 2,		/* mux */
				 BIT(31),	/* gate */
				 CLK_IS_CRITICAL);

static const char * const de_di_g2d_parents[] = { "pll-de", "pll-periph0-2x" };
static SUNXI_CCU_M_WITH_MUX_GATE(de_clk, "de", de_di_g2d_parents, 0x600,
				 0, 4,		/* M */
				 24, 1,		/* mux */
				 BIT(31),	/* gate */
				 CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(bus_de_clk, "bus-de", "psi-ahb1-ahb2",
		      0x60c, BIT(0), 0);

static SUNXI_CCU_M_WITH_MUX_GATE(di_clk, "di", de_di_g2d_parents, 0x620,
				 0, 4,		/* M */
				 24, 1,		/* mux */
				 BIT(31),	/* gate */
				 0);

static SUNXI_CCU_GATE(bus_di_clk, "bus-di", "psi-ahb1-ahb2",
		      0x62c, BIT(0), 0);

static SUNXI_CCU_M_WITH_MUX_GATE(g2d_clk, "g2d", de_di_g2d_parents, 0x630,
				 0, 4,		/* M */
				 24, 1,		/* mux */
				 BIT(31),	/* gate */
				 0);

static SUNXI_CCU_GATE(bus_g2d_clk, "bus-g2d", "psi-ahb1-ahb2",
		      0x63c, BIT(0), 0);

static const char * const gpu_parents[] = { "pll-gpu", "gpu1" };
static SUNXI_CCU_M_WITH_MUX_GATE(gpu0_clk, "gpu0", gpu_parents, 0x670,
				       0, 2,	/* M */
				       24, 1,	/* mux */
				       BIT(31),	/* gate */
				       0);

static SUNXI_CCU_M_WITH_GATE(gpu1_clk, "gpu1", "pll-periph0-2x",
			     0x674, 0, 2, BIT(31), 0);

static SUNXI_CCU_GATE(bus_gpu_clk, "bus-gpu", "psi-ahb1-ahb2",
		      0x67c, BIT(0), 0);

static const char * const ce_parents[] = { "dcxo24M", "pll-periph0-2x" };
static SUNXI_CCU_MP_WITH_MUX_GATE(ce_clk, "ce", ce_parents, 0x680,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 1,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_GATE(bus_ce_clk, "bus-ce", "psi-ahb1-ahb2",
		      0x68c, BIT(0), 0);
/*
 * delete the mux because it has only one parent
 */
static SUNXI_CCU_M_WITH_GATE(ve_clk, "ve", "pll-ve",
			     0x690, 0, 3,
			     BIT(31), CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(bus_ve_clk, "bus-ve", "psi-ahb1-ahb2",
		      0x69c, BIT(0), 0);

static SUNXI_CCU_GATE(bus_dma_clk, "bus-dma", "psi-ahb1-ahb2",
		      0x70c, BIT(0), 0);

static SUNXI_CCU_GATE(bus_hstimer_clk, "bus-hstimer", "psi-ahb1-ahb2",
		      0x73c, BIT(0), 0);

static SUNXI_CCU_GATE(avs_clk, "avs", "dcxo24M", 0x740, BIT(31), 0);

static SUNXI_CCU_GATE(bus_dbg_clk, "bus-dbg", "dcxo24M",
		      0x78c, BIT(0), 0);

static SUNXI_CCU_GATE(bus_psi_clk, "bus-psi", "psi-ahb1-ahb2",
		      0x79c, BIT(0), 0);

static SUNXI_CCU_GATE(bus_pwm_clk, "bus-pwm", "apb1", 0x7ac, BIT(0), 0);

static SUNXI_CCU_GATE(bus_iommu_clk, "bus-iommu", "apb1", 0x7bc, BIT(0), 0);

static const char * const dram_parents[] = { "pll-ddr0", "pll-ddr1" };
static SUNXI_CCU_M_WITH_MUX(dram_clk, "dram", dram_parents,
			    0x800, 0, 2, 24, 2, CLK_IS_CRITICAL);

static SUNXI_CCU_GATE(mbus_dma_clk, "mbus-dma", "mbus",
		      0x804, BIT(0), 0);
static SUNXI_CCU_GATE(mbus_ve_clk, "mbus-ve", "mbus",
		      0x804, BIT(1), 0);
static SUNXI_CCU_GATE(mbus_ce_clk, "mbus-ce", "mbus",
		      0x804, BIT(2), 0);
static SUNXI_CCU_GATE(mbus_ts_clk, "mbus-ts", "mbus",
		      0x804, BIT(3), 0);
static SUNXI_CCU_GATE(mbus_nand_clk, "mbus-nand", "mbus",
		      0x804, BIT(5), 0);
static SUNXI_CCU_GATE(mbus_csi_clk, "mbus-csi", "mbus",
		      0x804, BIT(8), 0);
static SUNXI_CCU_GATE(mbus_g2d_clk, "mbus-g2d", "mbus",
		      0x804, BIT(10), 0);

static SUNXI_CCU_GATE(bus_dram_clk, "bus-dram", "psi-ahb1-ahb2",
		      0x80c, BIT(0), CLK_IS_CRITICAL);

static const char * const nand_spi_parents[] = { "dcxo24M",
						 "pll-periph0",
						 "pll-periph1",
						 "pll-periph0-2x",
						 "pll-periph1-2x" };
static SUNXI_CCU_MP_WITH_MUX_GATE(nand0_clk, "nand0", nand_spi_parents, 0x810,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 3,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_MP_WITH_MUX_GATE(nand1_clk, "nand1", nand_spi_parents, 0x814,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 3,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_GATE(bus_nand_clk, "bus-nand", "ahb3", 0x82c, BIT(0), 0);

/* don't use postdiv for bsp kernel */
static const char * const mmc_parents[] = { "dcxo24M", "pll-periph0-2x",
					    "pll-periph1-2x" };
static SUNXI_CCU_MP_WITH_MUX_GATE(mmc0_clk, "mmc0", mmc_parents, 0x830,
					  0, 4,		/* M */
					  8, 2,		/* P */
					  24, 2,	/* mux */
					  BIT(31),	/* gate */
					  CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_MP_WITH_MUX_GATE(mmc1_clk, "mmc1", mmc_parents, 0x834,
					  0, 4,		/* M */
					  8, 2,		/* P */
					  24, 2,	/* mux */
					  BIT(31),	/* gate */
					  CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_MP_WITH_MUX_GATE(mmc2_clk, "mmc2", mmc_parents, 0x838,
					  0, 4,		/* M */
					  8, 2,		/* P */
					  24, 2,	/* mux */
					  BIT(31),	/* gate */
					  CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(bus_mmc0_clk, "bus-mmc0", "ahb3", 0x84c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_mmc1_clk, "bus-mmc1", "ahb3", 0x84c, BIT(1), 0);
static SUNXI_CCU_GATE(bus_mmc2_clk, "bus-mmc2", "ahb3", 0x84c, BIT(2), 0);

static SUNXI_CCU_GATE(bus_uart0_clk, "bus-uart0", "apb2", 0x90c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_uart1_clk, "bus-uart1", "apb2", 0x90c, BIT(1), 0);
static SUNXI_CCU_GATE(bus_uart2_clk, "bus-uart2", "apb2", 0x90c, BIT(2), 0);
static SUNXI_CCU_GATE(bus_uart3_clk, "bus-uart3", "apb2", 0x90c, BIT(3), 0);
static SUNXI_CCU_GATE(bus_uart4_clk, "bus-uart4", "apb2", 0x90c, BIT(4), 0);
static SUNXI_CCU_GATE(bus_uart5_clk, "bus-uart5", "apb2", 0x90c, BIT(5), 0);

static SUNXI_CCU_GATE(bus_i2c0_clk, "bus-i2c0", "apb2", 0x91c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_i2c1_clk, "bus-i2c1", "apb2", 0x91c, BIT(1), 0);
static SUNXI_CCU_GATE(bus_i2c2_clk, "bus-i2c2", "apb2", 0x91c, BIT(2), 0);
static SUNXI_CCU_GATE(bus_i2c3_clk, "bus-i2c3", "apb2", 0x91c, BIT(3), 0);
static SUNXI_CCU_GATE(bus_i2c4_clk, "bus-i2c4", "apb2", 0x91c, BIT(4), 0);

static SUNXI_CCU_GATE(bus_scr_clk, "bus-scr", "apb2", 0x93c, BIT(0), 0);

static SUNXI_CCU_MP_WITH_MUX_GATE(spi0_clk, "spi0", nand_spi_parents, 0x940,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 3,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_MP_WITH_MUX_GATE(spi1_clk, "spi1", nand_spi_parents, 0x944,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 3,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_GATE(bus_spi0_clk, "bus-spi0", "ahb3", 0x96c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_spi1_clk, "bus-spi1", "ahb3", 0x96c, BIT(1), 0);

static SUNXI_CCU_GATE(emac_25m_clk, "emac0-25m", "ahb3", 0x970,
		      BIT(31) | BIT(30), 0);

static SUNXI_CCU_GATE(bus_emac0_clk, "bus-emac0", "ahb3", 0x97c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_emac1_clk, "bus-emac1", "ahb3", 0x97c, BIT(1), 0);

static const char * const ts_parents[] = { "dcxo24M", "pll-periph0" };
static SUNXI_CCU_MP_WITH_MUX_GATE(ts_clk, "ts", ts_parents, 0x9b0,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 1,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_GATE(bus_ts_clk, "bus-ts", "ahb3", 0x9bc, BIT(0), 0);

static SUNXI_CCU_GATE(bus_gpadc_clk, "bus-gpadc", "dcxo24M", 0x9ec, BIT(0), 0);

static SUNXI_CCU_GATE(bus_ths_clk, "bus-ths", "dcxo24M", 0x9fc, BIT(0), 0);

/*
 * No one uses the pll-audio-hs, we do not support this clock.
 */
static const char * const audio_parents[] = { "pll-audio", "pll-audio-2x",
					      "pll-audio-4x" };
static struct ccu_div spdif_clk = {
	.enable		= BIT(31),
	.div		= _SUNXI_CCU_DIV_FLAGS(8, 2, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(24, 2),
	.common		= {
		.reg		= 0xa20,
		.hw.init	= CLK_HW_INIT_PARENTS("spdif",
						      audio_parents,
						      &ccu_div_ops,
						      0),
	},
};

static SUNXI_CCU_GATE(bus_spdif_clk, "bus-spdif", "apb1", 0xa2c, BIT(0), 0);

static struct ccu_div dmic_clk = {
	.enable		= BIT(31),
	.div		= _SUNXI_CCU_DIV_FLAGS(8, 2, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(24, 2),
	.common		= {
		.reg		= 0xa40,
		.hw.init	= CLK_HW_INIT_PARENTS("dmic",
						      audio_parents,
						      &ccu_div_ops,
						      0),
	},
};

static SUNXI_CCU_GATE(bus_dmic_clk, "bus-dmic", "apb1", 0xa4c, BIT(0), 0);

static SUNXI_CCU_M_WITH_MUX_GATE(audio_codec_clk, "audio-codec",
				 audio_parents, 0xa50,
				 0, 4,		/* M */
				 24, 2,		/* mux */
				 BIT(31),	/* gate */
				 0);

static SUNXI_CCU_M_WITH_MUX_GATE(audio_codec_4x_clk, "audio-codec-4x",
				 audio_parents, 0xa54,
				 0, 4,		/* M */
				 24, 2,		/* mux */
				 BIT(31),	/* gate */
				 0);

static SUNXI_CCU_GATE(bus_audio_codec_clk, "bus-audio-codec", "apb1",
		      0xa5c, BIT(0), 0);

static struct ccu_div audio_hub_clk = {
	.enable		= BIT(31),
	.div		= _SUNXI_CCU_DIV_FLAGS(8, 2, CLK_DIVIDER_POWER_OF_TWO),
	.mux		= _SUNXI_CCU_MUX(24, 2),
	.common		= {
		.reg		= 0xa60,
		.hw.init	= CLK_HW_INIT_PARENTS("audio-hub",
						      audio_parents,
						      &ccu_div_ops,
						      0),
	},
};

static SUNXI_CCU_GATE(bus_audio_hub_clk, "bus-audio-hub", "apb1",
		      0xa6c, BIT(0), 0);

/*
 * There are OHCI 12M clock source selection bits for 2 USB 2.0 ports.
 * We will force them to 0 (12M divided from 48M).
 */
#define SUN50IW9_USB0_CLK_REG		0xa70
#define SUN50IW9_USB1_CLK_REG		0xa74
#define SUN50IW9_USB2_CLK_REG		0xa78
#define SUN50IW9_USB3_CLK_REG		0xa7c

static SUNXI_CCU_GATE(usb_ohci0_clk, "usb-ohci0", "osc12M", 0xa70, BIT(31), 0);
static SUNXI_CCU_GATE(usb_phy0_clk, "usb-phy0", "dcxo24M", 0xa70, BIT(29), 0);

static SUNXI_CCU_GATE(usb_ohci1_clk, "usb-ohci1", "osc12M", 0xa74, BIT(31), 0);
static SUNXI_CCU_GATE(usb_phy1_clk, "usb-phy1", "dcxo24M", 0xa74, BIT(29), 0);

static SUNXI_CCU_GATE(usb_ohci2_clk, "usb-ohci2", "osc12M", 0xa78, BIT(31), 0);
static SUNXI_CCU_GATE(usb_phy2_clk, "usb-phy2", "dcxo24M", 0xa78, BIT(29), 0);

static SUNXI_CCU_GATE(usb_ohci3_clk, "usb-ohci3", "osc12M", 0xa7c, BIT(31), 0);
static SUNXI_CCU_GATE(usb_phy3_clk, "usb-phy3", "dcxo24M", 0xa7c, BIT(29), 0);

static SUNXI_CCU_GATE(bus_ohci0_clk, "bus-ohci0", "ahb3", 0xa8c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_ohci1_clk, "bus-ohci1", "ahb3", 0xa8c, BIT(1), 0);
static SUNXI_CCU_GATE(bus_ohci2_clk, "bus-ohci2", "ahb3", 0xa8c, BIT(2), 0);
static SUNXI_CCU_GATE(bus_ohci3_clk, "bus-ohci3", "ahb3", 0xa8c, BIT(3), 0);
static SUNXI_CCU_GATE(bus_ehci0_clk, "bus-ehci0", "ahb3", 0xa8c, BIT(4), 0);
static SUNXI_CCU_GATE(bus_ehci1_clk, "bus-ehci1", "ahb3", 0xa8c, BIT(5), 0);
static SUNXI_CCU_GATE(bus_ehci2_clk, "bus-ehci2", "ahb3", 0xa8c, BIT(6), 0);
static SUNXI_CCU_GATE(bus_ehci3_clk, "bus-ehci3", "ahb3", 0xa8c, BIT(7), 0);
static SUNXI_CCU_GATE(bus_otg_clk, "bus-otg", "ahb3", 0xa8c, BIT(8), 0);

static SUNXI_CCU_GATE(bus_lradc_clk, "bus-lradc", "ahb3", 0xa9c, BIT(0), 0);

static const char * const hdmi_parents[] = { "pll-video0", "pll-video0-4x",
					     "pll-video2", "pll-video2-4x" };
static SUNXI_CCU_M_WITH_MUX_GATE(hdmi_clk, "hdmi", hdmi_parents, 0xb00,
				 0, 4,		/* M */
				 24, 2,		/* mux */
				 BIT(31),	/* gate */
				 CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(hdmi_slow_clk, "hdmi-slow", "dcxo24M", 0xb04, BIT(31), 0);

static struct ccu_gate pll_periph0_2x_div_clk = {
	.enable	= BIT(30),
	.common	= {
		.reg		= 0xb10,
		.prediv		= 36621,
		.features	= CCU_FEATURE_ALL_PREDIV,
		.hw.init	= CLK_HW_INIT("pll-periph0-2x-div",
					      "pll-periph0-2x",
					      &ccu_gate_ops, 0),
	}
};

static const char * const hdmi_cec_parents[] = { "osc32k",
						 "pll-periph0-2x-div" };
static SUNXI_CCU_MUX_WITH_GATE(hdmi_cec_clk, "hdmi-cec", hdmi_cec_parents,
			       0xb10, 24, 2,  BIT(31), 0);

static SUNXI_CCU_GATE(bus_hdmi_clk, "bus-hdmi", "ahb3", 0xb1c, BIT(0), 0);

static SUNXI_CCU_GATE(bus_display_if_top_clk, "bus-display-if-top", "ahb3",
		      0xb5c, BIT(0), 0);

static const char * const video_parents[] = { "pll-video0", "pll-video0-4x",
					      "pll-video1", "pll-video1-4x" };
static SUNXI_CCU_MUX_WITH_GATE(tcon_lcd0_clk, "tcon-lcd0", video_parents,
			       0xb60, 24, 3,  BIT(31), CLK_SET_RATE_PARENT);

static SUNXI_CCU_MUX_WITH_GATE(tcon_lcd1_clk, "tcon-lcd1", video_parents,
			       0xb64, 24, 3,  BIT(31), CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(bus_tcon_lcd0_clk, "bus-tcon-lcd0", "ahb3",
		      0xb7c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_tcon_lcd1_clk, "bus-tcon-lcd1", "ahb3",
		      0xb7c, BIT(1), 0);

/*
 * "pll-video2", "pll-video2-4x" undocemented.
 */
static const char * const tcon_tv_parents[] = { "pll-video0",
						"pll-video0-4x",
						"pll-video1",
						"pll-video1-4x",
						"pll-video2",
						"pll-video2-4x" };
static SUNXI_CCU_MP_WITH_MUX_GATE(tcon_tv0_clk, "tcon-tv0",
				  tcon_tv_parents, 0xb80,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 3,	/* mux */
				  BIT(31),	/* gate */
				  CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_MP_WITH_MUX_GATE(tcon_tv1_clk, "tcon-tv1",
				  tcon_tv_parents, 0xb84,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 3,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_GATE(bus_tcon_tv0_clk, "bus-tcon-tv0", "ahb3",
		      0xb9c, BIT(0), 0);
static SUNXI_CCU_GATE(bus_tcon_tv1_clk, "bus-tcon-tv1", "ahb3",
		      0xb9c, BIT(1), 0);

static SUNXI_CCU_MP_WITH_MUX_GATE(tve_clk, "tve", video_parents, 0xbb0,
				  0, 4,		/* M */
				  8, 2,		/* P */
				  24, 3,	/* mux */
				  BIT(31),	/* gate */
				  0);

static SUNXI_CCU_GATE(bus_tve_clk, "bus-tve", "ahb3", 0xbbc, BIT(1), 0);

static SUNXI_CCU_GATE(bus_tve_top_clk, "bus-tve-top", "ahb3",
		      0xbbc, BIT(0), 0);

static const char * const csi_top_parents[] = { "pll-video0", "pll-ve",
						"pll-periph0", "pll-csi" };
static const u8 csi_top_table[] = { 0, 2, 3, 4 };

static SUNXI_CCU_M_WITH_MUX_TABLE_GATE(csi_top_clk, "csi-top",
				       csi_top_parents, csi_top_table,
				       0xc04, 0, 4,
				       24, 3,
				       BIT(31), 0);

static const char * const csi_mclk_parents[] = { "dcxo24M", "pll-video0",
						 "pll-periph0-2x",
						 "pll-periph1", "pll-csi"};
static const u8 csi_mclk_table[] = { 0, 1, 2, 3, 5};

static SUNXI_CCU_M_WITH_MUX_TABLE_GATE(csi0_mclk_clk, "csi-mclk0",
				       csi_mclk_parents, csi_mclk_table,
				       0xc08, 0, 5,
				       24, 3,
				       BIT(31), 0);

static SUNXI_CCU_M_WITH_MUX_TABLE_GATE(csi1_mclk_clk, "csi-mclk1",
				       csi_mclk_parents, csi_mclk_table,
				       0xc0c, 0, 5,
				       24, 3,
				       BIT(31), 0);

static SUNXI_CCU_GATE(bus_csi_clk, "bus-csi", "ahb3", 0xc2c, BIT(0), 0);

static const char * const hdmi_hdcp_parents[] = { "pll-periph0",
						  "pll-periph1" };
static SUNXI_CCU_M_WITH_MUX_GATE(hdmi_hdcp_clk, "hdmi-hdcp",
				 hdmi_hdcp_parents, 0xc40,
				 0, 4,		/* M */
				 24, 2,		/* mux */
				 BIT(31),	/* gate */
				 0);


static SUNXI_CCU_GATE(bus_hdmi_hdcp_clk, "bus-hdmi-hdcp", "ahb3",
		      0xc4c, BIT(0), 0);

/* Fixed factor clocks */
static CLK_FIXED_FACTOR_FW_NAME(osc12M_clk, "osc12M", "hosc", 2, 1, 0);

static const struct clk_hw *clk_parent_pll_audio[] = {
	&pll_audio_4x_clk.common.hw
};

static CLK_FIXED_FACTOR_HWS(pll_audio_clk, "pll-audio",
			    clk_parent_pll_audio,
			    4, 1, CLK_SET_RATE_PARENT);
static CLK_FIXED_FACTOR_HWS(pll_audio_2x_clk, "pll-audio-2x",
			    clk_parent_pll_audio,
			    2, 1, CLK_SET_RATE_PARENT);

static CLK_FIXED_FACTOR_HW(pll_periph0_2x_clk, "pll-periph0-2x",
			   &pll_periph0_clk.common.hw,
			   1, 2, 0);

static CLK_FIXED_FACTOR_HW(pll_periph1_2x_clk, "pll-periph1-2x",
			   &pll_periph1_clk.common.hw,
			   1, 2, 0);

static CLK_FIXED_FACTOR_HW(pll_video0_4x_clk, "pll-video0-4x",
			   &pll_video0_clk.common.hw,
			   1, 4, CLK_SET_RATE_PARENT);
static CLK_FIXED_FACTOR_HW(pll_video1_4x_clk, "pll-video1-4x",
			   &pll_video1_clk.common.hw,
			   1, 4, CLK_SET_RATE_PARENT);
static CLK_FIXED_FACTOR_HW(pll_video2_4x_clk, "pll-video2-4x",
			   &pll_video2_clk.common.hw,
			   1, 4, CLK_SET_RATE_PARENT);

/* for save sdm register when suspend */
#ifdef CONFIG_PM_SLEEP
static struct ccu_nm pll_audio_sdm_clk = {
	.common		= {
		.reg		= 0x178,
	},
};
#endif

static struct ccu_common *sun50iw9_ccu_clks[] = {
	&pll_cpux_clk.common,
	&pll_ddr0_clk.common,
	&pll_ddr1_clk.common,
	&pll_periph0_clk.common,
	&pll_periph1_pattern0_common, /* for pm resume */
	&pll_periph1_clk.common,
	&pll_gpu_clk.common,
	&pll_video0_clk.common,
	&pll_video1_clk.common,
	&pll_video2_clk.common,
	&pll_ve_clk.common,
	&pll_de_clk.common,
	&pll_audio_4x_clk.common,
	&pll_csi_clk.common,
	&cpux_clk.common,
	&axi_clk.common,
	&cpux_apb_clk.common,
	&psi_ahb1_ahb2_clk.common,
	&ahb3_clk.common,
	&apb1_clk.common,
	&apb2_clk.common,
	&mbus_clk.common,
	&de_clk.common,
	&bus_de_clk.common,
	&di_clk.common,
	&bus_di_clk.common,
	&g2d_clk.common,
	&bus_g2d_clk.common,
	&gpu0_clk.common,
	&gpu1_clk.common,
	&bus_gpu_clk.common,
	&ce_clk.common,
	&bus_ce_clk.common,
	&ve_clk.common,
	&bus_ve_clk.common,
	&bus_dma_clk.common,
	&bus_hstimer_clk.common,
	&avs_clk.common,
	&bus_dbg_clk.common,
	&bus_psi_clk.common,
	&bus_pwm_clk.common,
	&bus_iommu_clk.common,
	&dram_clk.common,
	&mbus_dma_clk.common,
	&mbus_ve_clk.common,
	&mbus_ce_clk.common,
	&mbus_ts_clk.common,
	&mbus_nand_clk.common,
	&mbus_csi_clk.common,
	&mbus_g2d_clk.common,
	&bus_dram_clk.common,
	&nand0_clk.common,
	&nand1_clk.common,
	&bus_nand_clk.common,
	&mmc0_clk.common,
	&mmc1_clk.common,
	&mmc2_clk.common,
	&bus_mmc0_clk.common,
	&bus_mmc1_clk.common,
	&bus_mmc2_clk.common,
	&bus_uart0_clk.common,
	&bus_uart1_clk.common,
	&bus_uart2_clk.common,
	&bus_uart3_clk.common,
	&bus_uart4_clk.common,
	&bus_uart5_clk.common,
	&bus_i2c0_clk.common,
	&bus_i2c1_clk.common,
	&bus_i2c2_clk.common,
	&bus_i2c3_clk.common,
	&bus_i2c4_clk.common,
	&bus_scr_clk.common,
	&spi0_clk.common,
	&spi1_clk.common,
	&bus_spi0_clk.common,
	&bus_spi1_clk.common,
	&emac_25m_clk.common,
	&bus_emac0_clk.common,
	&bus_emac1_clk.common,
	&ts_clk.common,
	&bus_ts_clk.common,
	&bus_gpadc_clk.common,
	&bus_ths_clk.common,
	&spdif_clk.common,
	&bus_spdif_clk.common,
	&dmic_clk.common,
	&bus_dmic_clk.common,
	&audio_codec_clk.common,
	&audio_codec_4x_clk.common,
	&bus_audio_codec_clk.common,
	&audio_hub_clk.common,
	&bus_audio_hub_clk.common,
	&usb_ohci0_clk.common,
	&usb_phy0_clk.common,
	&usb_ohci1_clk.common,
	&usb_phy1_clk.common,
	&usb_ohci2_clk.common,
	&usb_phy2_clk.common,
	&usb_ohci3_clk.common,
	&usb_phy3_clk.common,
	&bus_ohci0_clk.common,
	&bus_ohci1_clk.common,
	&bus_ohci2_clk.common,
	&bus_ohci3_clk.common,
	&bus_ehci0_clk.common,
	&bus_ehci1_clk.common,
	&bus_ehci2_clk.common,
	&bus_ehci3_clk.common,
	&bus_otg_clk.common,
	&bus_lradc_clk.common,
	&hdmi_clk.common,
	&hdmi_slow_clk.common,
	&pll_periph0_2x_div_clk.common,
	&hdmi_cec_clk.common,
	&bus_hdmi_clk.common,
	&bus_display_if_top_clk.common,
	&tcon_lcd0_clk.common,
	&tcon_lcd1_clk.common,
	&bus_tcon_lcd0_clk.common,
	&bus_tcon_lcd1_clk.common,
	&tcon_tv0_clk.common,
	&tcon_tv1_clk.common,
	&bus_tcon_tv0_clk.common,
	&bus_tcon_tv1_clk.common,
	&tve_clk.common,
	&bus_tve_clk.common,
	&bus_tve_top_clk.common,
	&csi_top_clk.common,
	&csi0_mclk_clk.common,
	&csi1_mclk_clk.common,
	&bus_csi_clk.common,
	&hdmi_hdcp_clk.common,
	&bus_hdmi_hdcp_clk.common,
#ifdef CONFIG_PM_SLEEP
	&pll_audio_sdm_clk.common,
#endif
};

static struct clk_hw_onecell_data sun50iw9_hw_clks = {
	.hws	= {
		[CLK_OSC12M]		= &osc12M_clk.hw,
		[CLK_PLL_CPUX]		= &pll_cpux_clk.common.hw,
		[CLK_PLL_DDR0]		= &pll_ddr0_clk.common.hw,
		[CLK_PLL_DDR1]		= &pll_ddr1_clk.common.hw,
		[CLK_PLL_PERIPH0]	= &pll_periph0_clk.common.hw,
		[CLK_PLL_PERIPH0_2X]	= &pll_periph0_2x_clk.hw,
		[CLK_PLL_PERIPH1]	= &pll_periph1_clk.common.hw,
		[CLK_PLL_PERIPH1_2X]	= &pll_periph1_2x_clk.hw,
		[CLK_PLL_GPU]		= &pll_gpu_clk.common.hw,
		[CLK_PLL_VIDEO0]	= &pll_video0_clk.common.hw,
		[CLK_PLL_VIDEO0_4X]	= &pll_video0_4x_clk.hw,
		[CLK_PLL_VIDEO1]	= &pll_video1_clk.common.hw,
		[CLK_PLL_VIDEO1_4X]	= &pll_video1_4x_clk.hw,
		[CLK_PLL_VIDEO2]	= &pll_video2_clk.common.hw,
		[CLK_PLL_VIDEO2_4X]	= &pll_video2_4x_clk.hw,
		[CLK_PLL_VE]		= &pll_ve_clk.common.hw,
		[CLK_PLL_DE]		= &pll_de_clk.common.hw,
		[CLK_PLL_AUDIO]		= &pll_audio_clk.hw,
		[CLK_PLL_AUDIO_2X]	= &pll_audio_2x_clk.hw,
		[CLK_PLL_AUDIO_4X]	= &pll_audio_4x_clk.common.hw,
		[CLK_PLL_CSI]		= &pll_csi_clk.common.hw,
		[CLK_CPUX]		= &cpux_clk.common.hw,
		[CLK_AXI]		= &axi_clk.common.hw,
		[CLK_CPUX_APB]		= &cpux_apb_clk.common.hw,
		[CLK_PSI_AHB1_AHB2]	= &psi_ahb1_ahb2_clk.common.hw,
		[CLK_AHB3]		= &ahb3_clk.common.hw,
		[CLK_APB1]		= &apb1_clk.common.hw,
		[CLK_APB2]		= &apb2_clk.common.hw,
		[CLK_MBUS]		= &mbus_clk.common.hw,
		[CLK_DE]		= &de_clk.common.hw,
		[CLK_BUS_DE]		= &bus_de_clk.common.hw,
		[CLK_DI]		= &di_clk.common.hw,
		[CLK_BUS_DI]		= &bus_di_clk.common.hw,
		[CLK_G2D]		= &g2d_clk.common.hw,
		[CLK_BUS_G2D]		= &bus_g2d_clk.common.hw,
		[CLK_GPU0]		= &gpu0_clk.common.hw,
		[CLK_GPU1]		= &gpu1_clk.common.hw,
		[CLK_BUS_GPU]		= &bus_gpu_clk.common.hw,
		[CLK_CE]		= &ce_clk.common.hw,
		[CLK_BUS_CE]		= &bus_ce_clk.common.hw,
		[CLK_VE]		= &ve_clk.common.hw,
		[CLK_BUS_VE]		= &bus_ve_clk.common.hw,
		[CLK_BUS_DMA]		= &bus_dma_clk.common.hw,
		[CLK_BUS_HSTIMER]	= &bus_hstimer_clk.common.hw,
		[CLK_AVS]		= &avs_clk.common.hw,
		[CLK_BUS_DBG]		= &bus_dbg_clk.common.hw,
		[CLK_BUS_PSI]		= &bus_psi_clk.common.hw,
		[CLK_BUS_PWM]		= &bus_pwm_clk.common.hw,
		[CLK_BUS_IOMMU]		= &bus_iommu_clk.common.hw,
		[CLK_DRAM]		= &dram_clk.common.hw,
		[CLK_MBUS_DMA]		= &mbus_dma_clk.common.hw,
		[CLK_MBUS_VE]		= &mbus_ve_clk.common.hw,
		[CLK_MBUS_CE]		= &mbus_ce_clk.common.hw,
		[CLK_MBUS_TS]		= &mbus_ts_clk.common.hw,
		[CLK_MBUS_NAND]		= &mbus_nand_clk.common.hw,
		[CLK_MBUS_CSI]		= &mbus_csi_clk.common.hw,
		[CLK_MBUS_G2D]		= &mbus_g2d_clk.common.hw,
		[CLK_BUS_DRAM]		= &bus_dram_clk.common.hw,
		[CLK_NAND0]		= &nand0_clk.common.hw,
		[CLK_NAND1]		= &nand1_clk.common.hw,
		[CLK_BUS_NAND]		= &bus_nand_clk.common.hw,
		[CLK_MMC0]		= &mmc0_clk.common.hw,
		[CLK_MMC1]		= &mmc1_clk.common.hw,
		[CLK_MMC2]		= &mmc2_clk.common.hw,
		[CLK_BUS_MMC0]		= &bus_mmc0_clk.common.hw,
		[CLK_BUS_MMC1]		= &bus_mmc1_clk.common.hw,
		[CLK_BUS_MMC2]		= &bus_mmc2_clk.common.hw,
		[CLK_BUS_UART0]		= &bus_uart0_clk.common.hw,
		[CLK_BUS_UART1]		= &bus_uart1_clk.common.hw,
		[CLK_BUS_UART2]		= &bus_uart2_clk.common.hw,
		[CLK_BUS_UART3]		= &bus_uart3_clk.common.hw,
		[CLK_BUS_UART4]		= &bus_uart4_clk.common.hw,
		[CLK_BUS_UART5]		= &bus_uart5_clk.common.hw,
		[CLK_BUS_I2C0]		= &bus_i2c0_clk.common.hw,
		[CLK_BUS_I2C1]		= &bus_i2c1_clk.common.hw,
		[CLK_BUS_I2C2]		= &bus_i2c2_clk.common.hw,
		[CLK_BUS_I2C3]		= &bus_i2c3_clk.common.hw,
		[CLK_BUS_I2C4]		= &bus_i2c4_clk.common.hw,
		[CLK_BUS_SCR]		= &bus_scr_clk.common.hw,
		[CLK_SPI0]		= &spi0_clk.common.hw,
		[CLK_SPI1]		= &spi1_clk.common.hw,
		[CLK_BUS_SPI0]		= &bus_spi0_clk.common.hw,
		[CLK_BUS_SPI1]		= &bus_spi1_clk.common.hw,
		[CLK_EMAC_25M]		= &emac_25m_clk.common.hw,
		[CLK_BUS_EMAC0]		= &bus_emac0_clk.common.hw,
		[CLK_BUS_EMAC1]		= &bus_emac1_clk.common.hw,
		[CLK_TS]		= &ts_clk.common.hw,
		[CLK_BUS_TS]		= &bus_ts_clk.common.hw,
		[CLK_BUS_GPADC]		= &bus_gpadc_clk.common.hw,
		[CLK_BUS_THS]		= &bus_ths_clk.common.hw,
		[CLK_SPDIF]		= &spdif_clk.common.hw,
		[CLK_BUS_SPDIF]		= &bus_spdif_clk.common.hw,
		[CLK_DMIC]		= &dmic_clk.common.hw,
		[CLK_BUS_DMIC]		= &bus_dmic_clk.common.hw,
		[CLK_AUDIO]		= &audio_codec_clk.common.hw,
		[CLK_AUDIO_4X]		= &audio_codec_4x_clk.common.hw,
		[CLK_BUS_AUDIO_CODEC]	= &bus_audio_codec_clk.common.hw,
		[CLK_AUDIO_HUB]		= &audio_hub_clk.common.hw,
		[CLK_BUS_AUDIO_HUB]	= &bus_audio_hub_clk.common.hw,
		[CLK_USB_OHCI0]		= &usb_ohci0_clk.common.hw,
		[CLK_USB_PHY0]		= &usb_phy0_clk.common.hw,
		[CLK_USB_OHCI1]		= &usb_ohci1_clk.common.hw,
		[CLK_USB_PHY1]		= &usb_phy1_clk.common.hw,
		[CLK_USB_OHCI2]		= &usb_ohci2_clk.common.hw,
		[CLK_USB_PHY2]		= &usb_phy2_clk.common.hw,
		[CLK_USB_OHCI3]		= &usb_ohci3_clk.common.hw,
		[CLK_USB_PHY3]		= &usb_phy3_clk.common.hw,
		[CLK_BUS_OHCI0]		= &bus_ohci0_clk.common.hw,
		[CLK_BUS_OHCI1]		= &bus_ohci1_clk.common.hw,
		[CLK_BUS_OHCI2]		= &bus_ohci2_clk.common.hw,
		[CLK_BUS_OHCI3]		= &bus_ohci3_clk.common.hw,
		[CLK_BUS_EHCI0]		= &bus_ehci0_clk.common.hw,
		[CLK_BUS_EHCI1]		= &bus_ehci1_clk.common.hw,
		[CLK_BUS_EHCI2]		= &bus_ehci2_clk.common.hw,
		[CLK_BUS_EHCI3]		= &bus_ehci3_clk.common.hw,
		[CLK_BUS_OTG]		= &bus_otg_clk.common.hw,
		[CLK_BUS_LRADC]		= &bus_lradc_clk.common.hw,
		[CLK_HDMI]		= &hdmi_clk.common.hw,
		[CLK_HDMI_SLOW]		= &hdmi_slow_clk.common.hw,
		[CLK_PLL_PERIPH0_2X_DIV] = &pll_periph0_2x_div_clk.common.hw,
		[CLK_HDMI_CEC]		= &hdmi_cec_clk.common.hw,
		[CLK_BUS_HDMI]		= &bus_hdmi_clk.common.hw,
		[CLK_BUS_DISPLAY_IF_TOP] = &bus_display_if_top_clk.common.hw,
		[CLK_TCON_LCD0]		= &tcon_lcd0_clk.common.hw,
		[CLK_TCON_LCD1]		= &tcon_lcd1_clk.common.hw,
		[CLK_BUS_TCON_LCD0]	= &bus_tcon_lcd0_clk.common.hw,
		[CLK_BUS_TCON_LCD1]	= &bus_tcon_lcd1_clk.common.hw,
		[CLK_TCON_TV0]		= &tcon_tv0_clk.common.hw,
		[CLK_TCON_TV1]		= &tcon_tv1_clk.common.hw,
		[CLK_BUS_TCON_TV0]	= &bus_tcon_tv0_clk.common.hw,
		[CLK_BUS_TCON_TV1]	= &bus_tcon_tv1_clk.common.hw,
		[CLK_TVE]		= &tve_clk.common.hw,
		[CLK_BUS_TVE]		= &bus_tve_clk.common.hw,
		[CLK_BUS_TVE_TOP]	= &bus_tve_top_clk.common.hw,
		[CLK_CSI_TOP]		= &csi_top_clk.common.hw,
		[CLK_CSI0_MCLK]		= &csi0_mclk_clk.common.hw,
		[CLK_CSI1_MCLK]		= &csi1_mclk_clk.common.hw,
		[CLK_BUS_CSI]		= &bus_csi_clk.common.hw,
		[CLK_HDMI_HDCP]		= &hdmi_hdcp_clk.common.hw,
		[CLK_BUS_HDMI_HDCP]	= &bus_hdmi_hdcp_clk.common.hw,
	},
	.num = CLK_NUMBER,
};

static struct ccu_reset_map sun50iw9_ccu_resets[] = {
	[RST_MBUS]		= { 0x540, BIT(30) },

	[RST_BUS_DE]		= { 0x60c, BIT(16) },
	[RST_BUS_DI]		= { 0x62c, BIT(16) },
	[RST_BUS_G2D]		= { 0x63c, BIT(16) },
	[RST_BUS_GPU]		= { 0x67c, BIT(16) },
	[RST_BUS_CE]		= { 0x68c, BIT(16) },
	[RST_BUS_VE]		= { 0x69c, BIT(16) },
	[RST_BUS_DMA]		= { 0x70c, BIT(16) },
	[RST_BUS_HSTIMER]	= { 0x73c, BIT(16) },
	[RST_BUS_DBG]		= { 0x78c, BIT(16) },
	[RST_BUS_PSI]		= { 0x79c, BIT(16) },
	[RST_BUS_PWM]		= { 0x7ac, BIT(16) },
	[RST_BUS_DRAM]		= { 0x80c, BIT(16) },
	[RST_BUS_NAND]		= { 0x82c, BIT(16) },
	[RST_BUS_MMC0]		= { 0x84c, BIT(16) },
	[RST_BUS_MMC1]		= { 0x84c, BIT(17) },
	[RST_BUS_MMC2]		= { 0x84c, BIT(18) },
	[RST_BUS_UART0]		= { 0x90c, BIT(16) },
	[RST_BUS_UART1]		= { 0x90c, BIT(17) },
	[RST_BUS_UART2]		= { 0x90c, BIT(18) },
	[RST_BUS_UART3]		= { 0x90c, BIT(19) },
	[RST_BUS_UART4]		= { 0x90c, BIT(20) },
	[RST_BUS_UART5]		= { 0x90c, BIT(21) },
	[RST_BUS_I2C0]		= { 0x91c, BIT(16) },
	[RST_BUS_I2C1]		= { 0x91c, BIT(17) },
	[RST_BUS_I2C2]		= { 0x91c, BIT(18) },
	[RST_BUS_I2C3]		= { 0x91c, BIT(19) },
	[RST_BUS_I2C4]		= { 0x91c, BIT(20) },
	[RST_BUS_SCR]		= { 0x93c, BIT(16) },
	[RST_BUS_SPI0]		= { 0x96c, BIT(16) },
	[RST_BUS_SPI1]		= { 0x96c, BIT(17) },
	[RST_BUS_EMAC0]		= { 0x97c, BIT(16) },
	[RST_BUS_EMAC1]		= { 0x97c, BIT(17) },
	[RST_BUS_TS]		= { 0x9bc, BIT(16) },
	[RST_BUS_GPADC]		= { 0x9ec, BIT(16) },
	[RST_BUS_THS]		= { 0x9fc, BIT(16) },
	[RST_BUS_SPDIF]		= { 0xa2c, BIT(16) },
	[RST_BUS_DMIC]		= { 0xa4c, BIT(16) },
	[RST_BUS_AUDIO_CODEC]	= { 0xa5c, BIT(16) },
	[RST_BUS_AUDIO_HUB]	= { 0xa6c, BIT(16) },

	[RST_USB_PHY0]		= { 0xa70, BIT(30) },
	[RST_USB_PHY1]		= { 0xa74, BIT(30) },
	[RST_USB_PHY2]		= { 0xa78, BIT(30) },
	[RST_USB_PHY3]		= { 0xa7c, BIT(30) },

	[RST_BUS_OHCI0]		= { 0xa8c, BIT(16) },
	[RST_BUS_OHCI1]		= { 0xa8c, BIT(17) },
	[RST_BUS_OHCI2]		= { 0xa8c, BIT(18) },
	[RST_BUS_OHCI3]		= { 0xa8c, BIT(19) },
	[RST_BUS_EHCI0]		= { 0xa8c, BIT(20) },
	[RST_BUS_EHCI1]		= { 0xa8c, BIT(21) },
	[RST_BUS_EHCI2]		= { 0xa8c, BIT(22) },
	[RST_BUS_EHCI3]		= { 0xa8c, BIT(23) },
	[RST_BUS_OTG]		= { 0xa8c, BIT(24) },

	[RST_BUS_LRADC]		= { 0xa9c, BIT(16) },
	[RST_BUS_HDMI_MAIN]	= { 0xb1c, BIT(16) },
	[RST_BUS_HDMI_SUB]	= { 0xb1c, BIT(17) },
	[RST_BUS_DISPLAY_IF_TOP] = { 0xb5c, BIT(16) },
	[RST_BUS_TCON_LCD0]	= { 0xb7c, BIT(16) },
	[RST_BUS_TCON_LCD1]	= { 0xb7c, BIT(17) },
	[RST_BUS_TCON_TV0]	= { 0xb9c, BIT(16) },
	[RST_BUS_TCON_TV1]	= { 0xb9c, BIT(17) },
	[RST_BUS_LVDS]		= { 0xbac, BIT(16) },
	[RST_BUS_TVE_TOP]	= { 0xbbc, BIT(16) },
	[RST_BUS_TVE]		= { 0xbbc, BIT(17) },
	[RST_BUS_CSI]		= { 0xc2c, BIT(16) },
	[RST_BUS_HDMI_HDCP]	= { 0xc4c, BIT(16) },
};

static const struct sunxi_ccu_desc sun50iw9_ccu_desc = {
	.ccu_clks	= sun50iw9_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun50iw9_ccu_clks),

	.hw_clks	= &sun50iw9_hw_clks,

	.resets		= sun50iw9_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun50iw9_ccu_resets),
};

static const u32 sun50iw9_pll_regs[] = {
	SUN50IW9_PLL_CPUX_REG,
	SUN50IW9_PLL_DDR0_REG,
	SUN50IW9_PLL_DDR1_REG,
	SUN50IW9_PLL_PERIPH0_REG,
	SUN50IW9_PLL_PERIPH1_REG,
	SUN50IW9_PLL_GPU_REG,
	SUN50IW9_PLL_VIDEO0_REG,
	SUN50IW9_PLL_VIDEO1_REG,
	SUN50IW9_PLL_VIDEO2_REG,
	SUN50IW9_PLL_VE_REG,
	SUN50IW9_PLL_DE_REG,
	SUN50IW9_PLL_AUDIO_REG,
	SUN50IW9_PLL_CSI_REG,
};

static const u32 sun50iw9_pll_video_regs[] = {
	SUN50IW9_PLL_VIDEO0_REG,
	SUN50IW9_PLL_VIDEO1_REG,
	SUN50IW9_PLL_VIDEO2_REG,
};

static const u32 sun50iw9_usb_clk_regs[] = {
	SUN50IW9_USB0_CLK_REG,
	SUN50IW9_USB1_CLK_REG,
	SUN50IW9_USB2_CLK_REG,
	SUN50IW9_USB3_CLK_REG,
};

static struct ccu_pll_nb sun50iw9_pll_cpu_nb = {
	.common = &pll_cpux_clk.common,
	/* copy from pll_cpux_clk */
	.enable = BIT(27),
	.lock   = BIT(28),
};

static struct ccu_mux_nb sun50iw9_cpu_nb = {
	.common         = &cpux_clk.common,
	.cm             = &cpux_clk.mux,
	.delay_us       = 1,
	.bypass_index   = 4, /* index of pll periph0 */
};

static int sun50iw9_ccu_probe(struct platform_device *pdev)
{
	void __iomem *reg;
	int ret;
	int i;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	/* Enable the lock bits on all PLLs */
	for (i = 0; i < ARRAY_SIZE(sun50iw9_pll_regs); i++) {
		set_reg(reg + sun50iw9_pll_regs[i], 1, 1, 29);
	}

	/*
	 * In order to pass the EMI certification, the SDM function of
	 * the peripheral 1 bus is enabled, and the frequency is still
	 * calculated using the previous division factor.
	 */
	set_reg(reg + SUN50IW9_PLL_PERIPH1_PATTERN0_REG, 0xd1303333, 32, 0);
	set_reg(reg + SUN50IW9_PLL_PERIPH1_REG, 1, 1, 24);

	/*
	 * Force the output divider of video PLLs to 0.
	 *
	 * See the comment before pll-video0 definition for the reason.
	 */
	for (i = 0; i < ARRAY_SIZE(sun50iw9_pll_video_regs); i++) {
		set_reg(reg + sun50iw9_pll_video_regs[i], 0x0, 1, 0);
	}

	/* Enforce m1 = 0, m0 = 1 for Audio PLL */
	set_reg(reg + SUN50IW9_PLL_AUDIO_REG, 0x1, 2, 0);

	/*
	 * Force OHCI 12M clock sources to 00 (12MHz divided from 48MHz)
	 *
	 * This clock mux is still mysterious, and the code just enforces
	 * it to have a valid clock parent.
	 */
	for (i = 0; i < ARRAY_SIZE(sun50iw9_usb_clk_regs); i++) {
		set_reg(reg + sun50iw9_usb_clk_regs[i], 0x0, 2, 24);
	}

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun50iw9_ccu_desc);
	if (ret)
		return ret;

	/* Gate then ungate PLL CPU after any rate changes */
	ccu_pll_notifier_register(&sun50iw9_pll_cpu_nb);

	/* Reparent CPU during PLL CPU rate changes */
	ccu_mux_notifier_register(pll_cpux_clk.common.hw.clk,
				  &sun50iw9_cpu_nb);

	sunxi_ccu_sleep_init(reg, sun50iw9_ccu_clks,
			     ARRAY_SIZE(sun50iw9_ccu_clks),
			     NULL, 0);

	return 0;
}

static const struct of_device_id sun50iw9_ccu_ids[] = {
	{ .compatible = "allwinner,sun50iw9-ccu" },
	{ }
};

static struct platform_driver sun50iw9_ccu_driver = {
	.probe	= sun50iw9_ccu_probe,
	.driver	= {
		.name	= "sun50iw9-ccu",
		.of_match_table	= sun50iw9_ccu_ids,
	},
};

static int __init sunxi_ccu_sun50iw9_init(void)
{
	return platform_driver_register(&sun50iw9_ccu_driver);
}
core_initcall(sunxi_ccu_sun50iw9_init);

static void __exit sunxi_ccu_sun50iw9_exit(void)
{
	return platform_driver_unregister(&sun50iw9_ccu_driver);
}
module_exit(sunxi_ccu_sun50iw9_exit);

MODULE_DESCRIPTION("Allwinner sun50iw9 clk driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.6");
