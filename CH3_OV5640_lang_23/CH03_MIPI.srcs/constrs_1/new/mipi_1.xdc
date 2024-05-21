#reset
#set_property PACKAGE_PIN C18 [get_ports {cam_gpio[0]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {cam_gpio[0]}]
#set_property PULLUP true [get_ports {cam_gpio[0]}]
#pwdn
`
#strobe
#set_property PACKAGE_PIN G19 [get_ports {cam_gpio[2]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {cam_gpio[2]}]

##cam_clk
#set_property PACKAGE_PIN H19 [get_ports cam_clk]
#set_property IOSTANDARD LVCMOS33 [get_ports cam_clk]

#set_property PACKAGE_PIN E18  [get_ports cam_i2c_scl_io]
#set_property PACKAGE_PIN F19 [get_ports cam_i2c_sda_io]

#set_property IOSTANDARD LVCMOS33 [get_ports cam_i2c_scl_io]
#set_property IOSTANDARD LVCMOS33 [get_ports cam_i2c_sda_io]

#set_property PULLUP true [get_ports cam_i2c_scl_io]
#set_property PULLUP true [get_ports cam_i2c_sda_io]

#set_property PACKAGE_PIN B19  [get_ports mipi_phy_if_0_clk_hs_p]
#set_property PACKAGE_PIN A21 [get_ports {mipi_phy_if_0_data_hs_p[0]}]
#set_property PACKAGE_PIN D22 [get_ports {mipi_phy_if_0_data_hs_p[1]}]

#set_property IOSTANDARD LVCMOS33 [get_ports mipi_phy_if_0_clk_hs_p]
#set_property IOSTANDARD LVCMOS33 [get_ports {mipi_phy_if_0_data_hs_p[0]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {mipi_phy_if_0_data_hs_p[1]}]

#set_property PACKAGE_PIN D20  [get_ports mipi_phy_if_0_clk_lp_p]
#set_property PACKAGE_PIN E19 [get_ports {mipi_phy_if_0_data_lp_p[0]}]
#set_property PACKAGE_PIN G20 [get_ports {mipi_phy_if_0_data_lp_p[1]}]

#set_property IOSTANDARD LVCMOS33 [get_ports mipi_phy_if_0_clk_lp_p]
#set_property IOSTANDARD LVCMOS33 [get_ports {mipi_phy_if_0_data_lp_p[0]}]
#set_property IOSTANDARD LVCMOS33 [get_ports {mipi_phy_if_0_data_lp_p[1]}]

set_property INTERNAL_VREF 0.6 [get_iobanks 35]

set_property -dict {PACKAGE_PIN B19 IOSTANDARD LVDS_25} [get_ports mipi_phy_if_0_clk_hs_p]
set_property -dict {PACKAGE_PIN B20 IOSTANDARD LVDS_25} [get_ports mipi_phy_if_0_clk_hs_n]

set_property -dict {PACKAGE_PIN A21 IOSTANDARD LVDS_25} [get_ports {mipi_phy_if_0_data_hs_p[0]}]
set_property -dict {PACKAGE_PIN A22 IOSTANDARD LVDS_25} [get_ports {mipi_phy_if_0_data_hs_n[0]}]
set_property -dict {PACKAGE_PIN D22 IOSTANDARD LVDS_25} [get_ports {mipi_phy_if_0_data_hs_p[1]}]
set_property -dict {PACKAGE_PIN C22 IOSTANDARD LVDS_25} [get_ports {mipi_phy_if_0_data_hs_n[1]}]

set_property -dict {PACKAGE_PIN D20 IOSTANDARD HSUL_12} [get_ports mipi_phy_if_0_clk_lp_p]
set_property -dict {PACKAGE_PIN C20 IOSTANDARD HSUL_12} [get_ports mipi_phy_if_0_clk_lp_n]

set_property -dict {PACKAGE_PIN E19 IOSTANDARD HSUL_12} [get_ports {mipi_phy_if_0_data_lp_p[0]}]
set_property -dict {PACKAGE_PIN E20 IOSTANDARD HSUL_12} [get_ports {mipi_phy_if_0_data_lp_n[0]}]
set_property -dict {PACKAGE_PIN G20 IOSTANDARD HSUL_12} [get_ports {mipi_phy_if_0_data_lp_p[1]}]
set_property -dict {PACKAGE_PIN G21 IOSTANDARD HSUL_12} [get_ports {mipi_phy_if_0_data_lp_n[1]}]

