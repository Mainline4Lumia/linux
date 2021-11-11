// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>

#include <dt-bindings/clock/qcom,mmcc-msm8974.h>
#include <dt-bindings/reset/qcom,mmcc-msm8974.h>

#include "common.h"
#include "clk-regmap.h"
#include "clk-pll.h"
#include "clk-rcg.h"
#include "clk-branch.h"
#include "reset.h"
#include "gdsc.h"

enum {
	P_XO,
	P_MMPLL0,
	P_EDPLINK,
	P_MMPLL1,
	P_HDMIPLL,
	P_GPLL0,
	P_EDPVCO,
	P_GPLL1,
	P_DSI0PLL,
	P_DSI0PLL_BYTE,
	P_MMPLL2,
	P_MMPLL3,
	P_DSI1PLL,
	P_DSI1PLL_BYTE,
};

static const struct parent_map mmcc_xo_mmpll0_mmpll1_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL1, 2 },
	{ P_GPLL0, 5 }
};

static const char * const mmcc_xo_mmpll0_mmpll1_gpll0[] = {
	"xo",
	"mmpll0_vote",
	"mmpll1_vote",
	"mmss_gpll0_vote",
};

static const struct parent_map mmcc_xo_mmpll0_dsi_hdmi_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_HDMIPLL, 4 },
	{ P_GPLL0, 5 },
	{ P_DSI0PLL, 2 },
	{ P_DSI1PLL, 3 }
};

static const char * const mmcc_xo_mmpll0_dsi_hdmi_gpll0[] = {
	"xo",
	"mmpll0_vote",
	"hdmipll",
	"mmss_gpll0_vote",
	"dsi0pll",
	"dsi1pll",
};

static const struct parent_map mmcc_xo_mmpll0_1_3_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL1, 2 },
	{ P_GPLL0, 5 },
	{ P_MMPLL3, 3 }
};

static const char * const mmcc_xo_mmpll0_1_3_gpll0[] = {
	"xo",
	"mmpll0_vote",
	"mmpll1_vote",
	"mmss_gpll0_vote",
	"mmpll3",
};

static const struct parent_map mmcc_xo_mmpll0_1_gpll1_0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL1, 2 },
	{ P_GPLL0, 5 },
	{ P_GPLL1, 4 }
};

static const char * const mmcc_xo_mmpll0_1_gpll1_0[] = {
	"xo",
	"mmpll0_vote",
	"mmpll1_vote",
	"mmss_gpll0_vote",
	"gpll1_vote",
};

static const struct parent_map mmcc_xo_dsi_hdmi_edp_map[] = {
	{ P_XO, 0 },
	{ P_EDPLINK, 4 },
	{ P_HDMIPLL, 3 },
	{ P_EDPVCO, 5 },
	{ P_DSI0PLL, 1 },
	{ P_DSI1PLL, 2 }
};

static const char * const mmcc_xo_dsi_hdmi_edp[] = {
	"xo",
	"edp_link_clk",
	"hdmipll",
	"edp_vco_div",
	"dsi0pll",
	"dsi1pll",
};

static const struct parent_map mmcc_xo_dsi_hdmi_edp_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_EDPLINK, 4 },
	{ P_HDMIPLL, 3 },
	{ P_GPLL0, 5 },
	{ P_DSI0PLL, 1 },
	{ P_DSI1PLL, 2 }
};

static const char * const mmcc_xo_dsi_hdmi_edp_gpll0[] = {
	"xo",
	"edp_link_clk",
	"hdmipll",
	"gpll0_vote",
	"dsi0pll",
	"dsi1pll",
};

static const struct parent_map mmcc_xo_dsibyte_hdmi_edp_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_EDPLINK, 4 },
	{ P_HDMIPLL, 3 },
	{ P_GPLL0, 5 },
	{ P_DSI0PLL_BYTE, 1 },
	{ P_DSI1PLL_BYTE, 2 }
};

static const char * const mmcc_xo_dsibyte_hdmi_edp_gpll0[] = {
	"xo",
	"edp_link_clk",
	"hdmipll",
	"gpll0_vote",
	"dsi0pllbyte",
	"dsi1pllbyte",
};

static struct clk_pll mmpll0 = {
	.l_reg = 0x0004,
	.m_reg = 0x0008,
	.n_reg = 0x000c,
	.config_reg = 0x0014,
	.mode_reg = 0x0000,
	.status_reg = 0x001c,
	.status_bit = 17,
        .clkr.hw.init = &(struct clk_init_data){
                .name = "mmpll0",
                .parent_names = (const char *[]){ "xo" },
                .num_parents = 1,
                .ops = &clk_pll_ops,
        },
};

static struct clk_regmap mmpll0_vote = {
	.enable_reg = 0x0100,
	.enable_mask = BIT(0),
	.hw.init = &(struct clk_init_data){
		.name = "mmpll0_vote",
		.parent_names = (const char *[]){ "mmpll0" },
		.num_parents = 1,
		.ops = &clk_pll_vote_ops,
	},
};

static struct clk_pll mmpll1 = {
	.l_reg = 0x0044,
	.m_reg = 0x0048,
	.n_reg = 0x004c,
	.config_reg = 0x0050,
	.mode_reg = 0x0040,
	.status_reg = 0x005c,
	.status_bit = 17,
        .clkr.hw.init = &(struct clk_init_data){
                .name = "mmpll1",
                .parent_names = (const char *[]){ "xo" },
                .num_parents = 1,
                .ops = &clk_pll_ops,
        },
};

static struct clk_regmap mmpll1_vote = {
	.enable_reg = 0x0100,
	.enable_mask = BIT(1),
	.hw.init = &(struct clk_init_data){
		.name = "mmpll1_vote",
		.parent_names = (const char *[]){ "mmpll1" },
		.num_parents = 1,
		.ops = &clk_pll_vote_ops,
	},
};

static struct clk_pll mmpll2 = {
	.l_reg = 0x4104,
	.m_reg = 0x4108,
	.n_reg = 0x410c,
	.config_reg = 0x4110,
	.mode_reg = 0x4100,
	.status_reg = 0x411c,
        .clkr.hw.init = &(struct clk_init_data){
                .name = "mmpll2",
                .parent_names = (const char *[]){ "xo" },
                .num_parents = 1,
                .ops = &clk_pll_ops,
        },
};

static struct clk_rcg2 mmss_ahb_clk_src = {
	.cmd_rcgr = 0x5000,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmss_ahb_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_mmss_axi_clk[] = {
	F( 19200000, P_XO, 1, 0, 0),
	F( 37500000, P_GPLL0, 16, 0, 0),
	F( 50000000, P_GPLL0, 12, 0, 0),
	F( 75000000, P_GPLL0, 8, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(150000000, P_GPLL0, 4, 0, 0),
	F(291750000, P_MMPLL1, 4, 0, 0),
	F(400000000, P_MMPLL0, 2, 0, 0),
	F(466800000, P_MMPLL1, 2.5, 0, 0),
};

static struct clk_rcg2 mmss_axi_clk_src = {
	.cmd_rcgr = 0x5040,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_mmss_axi_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmss_axi_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_csi0_3_clk[] = {
	F(100000000, P_GPLL0, 6, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
	{ }
};

static struct clk_rcg2 csi0_clk_src = {
	.cmd_rcgr = 0x3090,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_csi0_3_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi0_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_rcg2 csi1_clk_src = {
	.cmd_rcgr = 0x3100,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_csi0_3_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi1_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_vfe_vfe0_1_clk[] = {
	F(37500000, P_GPLL0, 16, 0, 0),
	F(50000000, P_GPLL0, 12, 0, 0),
	F(60000000, P_GPLL0, 10, 0, 0),
	F(80000000, P_GPLL0, 7.5, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(109090000, P_GPLL0, 5.5, 0, 0),
	F(133330000, P_GPLL0, 4.5, 0, 0),
	F(200000000, P_GPLL0, 3, 0, 0),
	F(228570000, P_MMPLL0, 3.5, 0, 0),
	F(266670000, P_MMPLL0, 3, 0, 0),
	F(320000000, P_MMPLL0, 2.5, 0, 0),
	F(400000000, P_MMPLL0, 2, 0, 0),
	F(465000000, P_MMPLL3, 2, 0, 0),
	{ }
};

static struct clk_rcg2 vfe0_clk_src = {
	.cmd_rcgr = 0x3600,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_vfe_vfe0_1_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vfe0_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_mdss_mdp_clk[] = {
	F(37500000, P_GPLL0, 16, 0, 0),
	F(60000000, P_GPLL0, 10, 0, 0),
	F(75000000, P_GPLL0, 8, 0, 0),
	F(85710000, P_GPLL0, 7, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(133330000, P_MMPLL0, 6, 0, 0),
	F(160000000, P_MMPLL0, 5, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
	F(228570000, P_MMPLL0, 3.5, 0, 0),
	F(240000000, P_GPLL0, 2.5, 0, 0),
	F(266670000, P_MMPLL0, 3, 0, 0),
	F(320000000, P_MMPLL0, 2.5, 0, 0),
	{ }
};

static struct clk_rcg2 mdp_clk_src = {
	.cmd_rcgr = 0x2040,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_dsi_hdmi_gpll0_map,
	.freq_tbl = ftbl_mdss_mdp_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mdp_clk_src",
		.parent_names = mmcc_xo_mmpll0_dsi_hdmi_gpll0,
		.num_parents = 6,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_jpeg_jpeg0_2_clk[] = {
	F(75000000, P_GPLL0, 8, 0, 0),
	F(133330000, P_GPLL0, 4.5, 0, 0),
	F(200000000, P_GPLL0, 3, 0, 0),
	F(228570000, P_MMPLL0, 3.5, 0, 0),
	F(266670000, P_MMPLL0, 3, 0, 0),
	F(320000000, P_MMPLL0, 2.5, 0, 0),
	{ }
};

static struct clk_rcg2 jpeg0_clk_src = {
	.cmd_rcgr = 0x3500,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_jpeg_jpeg0_2_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "jpeg0_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_rcg2 pclk0_clk_src = {
	.cmd_rcgr = 0x2000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_xo_dsi_hdmi_edp_gpll0_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "pclk0_clk_src",
		.parent_names = mmcc_xo_dsi_hdmi_edp_gpll0,
		.num_parents = 6,
		.ops = &clk_pixel_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct freq_tbl ftbl_venus0_vcodec0_clk[] = {
	F(50000000, P_GPLL0, 12, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(133330000, P_MMPLL0, 6, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
	F(266670000, P_MMPLL0, 3, 0, 0),
	F(465000000, P_MMPLL3, 2, 0, 0),
	{ }
};

static struct clk_rcg2 vcodec0_clk_src = {
	.cmd_rcgr = 0x1000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_1_3_gpll0_map,
	.freq_tbl = ftbl_venus0_vcodec0_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vcodec0_clk_src",
		.parent_names = mmcc_xo_mmpll0_1_3_gpll0,
		.num_parents = 5,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_cci_cci_clk[] = {
	F(19200000, P_XO, 1, 0, 0),
	{ }
};

static struct clk_rcg2 cci_clk_src = {
	.cmd_rcgr = 0x3300,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_cci_cci_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "cci_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_mclk0_3_clk[] = {
	F(4800000, P_XO, 4, 0, 0),
	F(6000000, P_GPLL0, 10, 1, 10),
	F(8000000, P_GPLL0, 15, 1, 5),
	F(9600000, P_XO, 2, 0, 0),
	F(16000000, P_GPLL0, 12.5, 1, 3),
	F(19200000, P_XO, 1, 0, 0),
	F(24000000, P_GPLL0, 5, 1, 5),
	F(32000000, P_MMPLL0, 5, 1, 5),
	F(48000000, P_GPLL0, 12.5, 0, 0),
	F(64000000, P_MMPLL0, 12.5, 0, 0),
	F(66670000, P_GPLL0, 9, 0, 0),
	{ }
};

static struct clk_rcg2 mclk0_clk_src = {
	.cmd_rcgr = 0x3360,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_mclk0_3_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mclk0_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_rcg2 mclk1_clk_src = {
	.cmd_rcgr = 0x3390,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_mclk0_3_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mclk1_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_phy0_2_csi0_2phytimer_clk[] = {
	F(100000000, P_GPLL0, 6, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
	{ }
};

static struct clk_rcg2 csi0phytimer_clk_src = {
	.cmd_rcgr = 0x3000,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_phy0_2_csi0_2phytimer_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi0phytimer_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_rcg2 csi1phytimer_clk_src = {
	.cmd_rcgr = 0x3030,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_phy0_2_csi0_2phytimer_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi1phytimer_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_vfe_cpp_clk[] = {
	F(133330000, P_GPLL0, 4.5, 0, 0),
	F(266670000, P_MMPLL0, 3, 0, 0),
	F(320000000, P_MMPLL0, 2.5, 0, 0),
	F(400000000, P_MMPLL0, 2, 0, 0),
	F(465000000, P_MMPLL3, 2, 0, 0),
	{ }
};

static struct clk_rcg2 cpp_clk_src = {
	.cmd_rcgr = 0x3640,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_vfe_cpp_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "cpp_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl byte_freq_tbl[] = {
	{ .src = P_DSI0PLL_BYTE },
	{ }
};

static struct clk_rcg2 byte0_clk_src = {
	.cmd_rcgr = 0x2120,
	.hid_width = 5,
	.parent_map = mmcc_xo_dsibyte_hdmi_edp_gpll0_map,
	.freq_tbl = byte_freq_tbl,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "byte0_clk_src",
		.parent_names = mmcc_xo_dsibyte_hdmi_edp_gpll0,
		.num_parents = 6,
		.ops = &clk_byte2_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct freq_tbl ftbl_mdss_esc0_1_clk[] = {
	F(19200000, P_XO, 1, 0, 0),
	{ }
};

static struct clk_rcg2 esc0_clk_src = {
	.cmd_rcgr = 0x2160,
	.hid_width = 5,
	.parent_map = mmcc_xo_dsibyte_hdmi_edp_gpll0_map,
	.freq_tbl = ftbl_mdss_esc0_1_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "esc0_clk_src",
		.parent_names = mmcc_xo_dsibyte_hdmi_edp_gpll0,
		.num_parents = 6,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_mdss_vsync_clk[] = {
	F(19200000, P_XO, 1, 0, 0),
	{ }
};

static struct clk_rcg2 vsync_clk_src = {
	.cmd_rcgr = 0x2080,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_mdss_vsync_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vsync_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch mmss_misc_ahb_clk = {
	.halt_reg = 0x502c,
	.clkr = {
		.enable_reg = 0x502c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_misc_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mmssnoc_ahb_clk = {
	.halt_reg = 0x5024,
	.clkr = {
		.enable_reg = 0x5024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mmssnoc_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch mmss_mmssnoc_bto_ahb_clk = {
	.halt_reg = 0x5028,
	.clkr = {
		.enable_reg = 0x5028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mmssnoc_bto_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch mmss_mmssnoc_axi_clk = {
	.halt_reg = 0x506c,
	.clkr = {
		.enable_reg = 0x506c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mmssnoc_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_s0_axi_clk = {
	.halt_reg = 0x5064,
	.clkr = {
		.enable_reg = 0x5064,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_s0_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch oxili_gfx3d_clk = {
	.halt_reg = 0x4028,
	.clkr = {
		.enable_reg = 0x4028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "oxili_gfx3d_clk",
			.parent_names = (const char *[]){
				"gfx3d_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch oxilicx_ahb_clk = {
	.halt_reg = 0x403c,
	.clkr = {
		.enable_reg = 0x403c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "oxilicx_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch oxilicx_axi_clk = {
	.halt_reg = 0x4038,
	.clkr = {
		.enable_reg = 0x4038,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "oxilicx_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch venus0_ahb_clk = {
	.halt_reg = 0x1030,
	.clkr = {
		.enable_reg = 0x1030,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "venus0_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch venus0_axi_clk = {
	.halt_reg = 0x1034,
	.clkr = {
		.enable_reg = 0x1034,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "venus0_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch venus0_vcodec0_clk = {
	.halt_reg = 0x1028,
	.clkr = {
		.enable_reg = 0x1028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "venus0_vcodec0_clk",
			.parent_names = (const char *[]){
				"vcodec0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static const struct pll_config mmpll1_config = {
	.l = 60,
	.m = 25,
	.n = 32,
	.vco_val = 0x0,
	.vco_mask = 0x3 << 20,
	.pre_div_val = 0x0,
	.pre_div_mask = 0x7 << 12,
	.post_div_val = 0x0,
	.post_div_mask = 0x3 << 8,
	.mn_ena_mask = BIT(24),
	.main_output_mask = BIT(0),
};

static struct gdsc venus0_gdsc = {
	.gdscr = 0x1024,
	.cxcs = (unsigned int []){ 0x1028 },
	.cxc_count = 1,
	.resets = (unsigned int []){ VENUS0_RESET },
	.reset_count = 1,
	.pd = {
		.name = "venus0",
	},
	.pwrsts = PWRSTS_ON,
};

static struct gdsc oxili_gdsc = {
	.gdscr = 0x4024,
	.cxcs = (unsigned int []){ 0x4028 },
	.cxc_count = 1,
	.pd = {
		.name = "oxili",
	},
	.pwrsts = PWRSTS_OFF_ON,
};

static struct gdsc oxilicx_gdsc = {
	.gdscr = 0x4034,
	.pd = {
		.name = "oxilicx",
	},
	.parent = &oxili_gdsc.pd,
	.pwrsts = PWRSTS_OFF_ON,
};

static struct clk_regmap *mmcc_msm8226_clocks[] = {
	[MMSS_AHB_CLK_SRC] = &mmss_ahb_clk_src.clkr,
	[MMSS_AXI_CLK_SRC] = &mmss_axi_clk_src.clkr,
	[MMPLL0] = &mmpll0.clkr,
	[MMPLL0_VOTE] = &mmpll0_vote,
	[MMPLL1] = &mmpll1.clkr,
	[MMPLL1_VOTE] = &mmpll1_vote,
	[MMPLL2] = &mmpll2.clkr,
	[CSI0_CLK_SRC] = &csi0_clk_src.clkr,
	[CSI1_CLK_SRC] = &csi1_clk_src.clkr,
	[VFE0_CLK_SRC] = &vfe0_clk_src.clkr,
	[MDP_CLK_SRC] = &mdp_clk_src.clkr,
	[JPEG0_CLK_SRC] = &jpeg0_clk_src.clkr,
	[PCLK0_CLK_SRC] = &pclk0_clk_src.clkr,
	[VCODEC0_CLK_SRC] = &vcodec0_clk_src.clkr,
	[CCI_CLK_SRC] = &cci_clk_src.clkr,
	[MCLK0_CLK_SRC] = &mclk0_clk_src.clkr,
	[MCLK1_CLK_SRC] = &mclk1_clk_src.clkr,
	[CSI0PHYTIMER_CLK_SRC] = &csi0phytimer_clk_src.clkr,
	[CSI1PHYTIMER_CLK_SRC] = &csi1phytimer_clk_src.clkr,
	[CPP_CLK_SRC] = &cpp_clk_src.clkr,
	[BYTE0_CLK_SRC] = &byte0_clk_src.clkr,
	[ESC0_CLK_SRC] = &esc0_clk_src.clkr,
	[VSYNC_CLK_SRC] = &vsync_clk_src.clkr,
	[MMSS_MISC_AHB_CLK] = &mmss_misc_ahb_clk.clkr,
	[MMSS_MMSSNOC_AHB_CLK] = &mmss_mmssnoc_ahb_clk.clkr,
	[MMSS_MMSSNOC_BTO_AHB_CLK] = &mmss_mmssnoc_bto_ahb_clk.clkr,
	[MMSS_MMSSNOC_AXI_CLK] = &mmss_mmssnoc_axi_clk.clkr,
	[MMSS_S0_AXI_CLK] = &mmss_s0_axi_clk.clkr,
	[OXILI_GFX3D_CLK] = &oxili_gfx3d_clk.clkr,
	[OXILICX_AHB_CLK] = &oxilicx_ahb_clk.clkr,
	[OXILICX_AXI_CLK] = &oxilicx_axi_clk.clkr,
	[VENUS0_AHB_CLK] = &venus0_ahb_clk.clkr,
	[VENUS0_AXI_CLK] = &venus0_axi_clk.clkr,
	[VENUS0_VCODEC0_CLK] = &venus0_vcodec0_clk.clkr,
};

static const struct qcom_reset_map mmcc_msm8226_resets[] = {
	[SPDM_RESET] = { 0x0200 },
	[SPDM_RM_RESET] = { 0x0300 },
	[VENUS0_RESET] = { 0x1020 },
	[OXILI_RESET] = { 0x4020 },
	[OXILICX_RESET] = { 0x4030 },
	[MMSS_RBCRP_RESET] = { 0x4080 },
	[MMSSNOCAHB_RESET] = { 0x5020 },
	[MMSSNOCAXI_RESET] = { 0x5060 },
};

static struct gdsc *mmcc_msm8226_gdscs[] = {
	[VENUS0_GDSC] = &venus0_gdsc,
	[OXILI_GDSC] = &oxili_gdsc,
	[OXILICX_GDSC] = &oxilicx_gdsc,
};

static const struct regmap_config mmcc_msm8226_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x5104,
	.fast_io	= true,
};

static const struct qcom_cc_desc mmcc_msm8226_desc = {
	.config = &mmcc_msm8226_regmap_config,
	.clks = mmcc_msm8226_clocks,
	.num_clks = ARRAY_SIZE(mmcc_msm8226_clocks),
	.resets = mmcc_msm8226_resets,
	.num_resets = ARRAY_SIZE(mmcc_msm8226_resets),
	.gdscs = mmcc_msm8226_gdscs,
	.num_gdscs = ARRAY_SIZE(mmcc_msm8226_gdscs),
};

static const struct of_device_id mmcc_msm8226_match_table[] = {
	{ .compatible = "qcom,mmcc-msm8226" },
	{ }
};
MODULE_DEVICE_TABLE(of, mmcc_msm8226_match_table);

static int mmcc_msm8226_probe(struct platform_device *pdev)
{
	struct regmap *regmap;

	regmap = qcom_cc_map(pdev, &mmcc_msm8226_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	clk_pll_configure_sr_hpm_lp(&mmpll1, regmap, &mmpll1_config, true);

	return qcom_cc_really_probe(pdev, &mmcc_msm8226_desc, regmap);
}

static struct platform_driver mmcc_msm8226_driver = {
	.probe		= mmcc_msm8226_probe,
	.driver		= {
		.name	= "mmcc-msm8226",
		.of_match_table = mmcc_msm8226_match_table,
	},
};
module_platform_driver(mmcc_msm8226_driver);

MODULE_DESCRIPTION("QCOM MMCC MSM8226 Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mmcc-msm8226");

