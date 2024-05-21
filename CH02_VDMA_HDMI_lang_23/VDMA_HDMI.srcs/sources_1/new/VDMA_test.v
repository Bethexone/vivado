`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/05/06 15:59:52
// Design Name: 
// Module Name: VDMA_test
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module VDMA_test(
  inout [14:0]      DDR_addr            ,
  inout [2:0]       DDR_ba              ,
  inout             DDR_cas_n           ,
  inout             DDR_ck_n            ,
  inout             DDR_ck_p            ,
  inout             DDR_cke             ,
  inout             DDR_cs_n            ,
  inout [3:0]       DDR_dm              ,
  inout [31:0]      DDR_dq              ,
  inout [3:0]       DDR_dqs_n           ,
  inout [3:0]       DDR_dqs_p           ,
  inout             DDR_odt             ,
  inout             DDR_ras_n           ,
  inout             DDR_reset_n         ,
  inout             DDR_we_n            ,
  inout             FIXED_IO_ddr_vrn    ,
  inout             FIXED_IO_ddr_vrp    ,
  inout [53:0]      FIXED_IO_mio        ,
  inout             FIXED_IO_ps_clk     ,
  inout             FIXED_IO_ps_porb    ,
  inout             FIXED_IO_ps_srstb   ,
                                        
  inout             hdmi_scl            ,
  inout             hdmi_sda            ,
                                        
  input             sys_clk             ,
  input             rst_n               ,
  
  output            vio_clk_out         ,
  output            vid_out_active      ,
  output            vid_out_hsync       ,
  output            vid_out_vsync       ,
  output [23:0]     video_out_data      
                     
    );
wire   clk_100m;  
wire  vid_in_active  ;
//wire  vid_in_clk     ;
wire  [7:0]vid_in_data    ;
wire  vid_in_vsync   ;
VDMA_wrapper VDMA_u0
(   .DDR_addr            ( DDR_addr         ),
    .DDR_ba              ( DDR_ba           ),
    .DDR_cas_n           ( DDR_cas_n        ),
    .DDR_ck_n            ( DDR_ck_n         ),
    .DDR_ck_p            ( DDR_ck_p         ),
    .DDR_cke             ( DDR_cke          ),
    .DDR_cs_n            ( DDR_cs_n         ),
    .DDR_dm              ( DDR_dm           ),
    .DDR_dq              ( DDR_dq           ),
    .DDR_dqs_n           ( DDR_dqs_n        ),
    .DDR_dqs_p           ( DDR_dqs_p        ),
    .DDR_odt             ( DDR_odt          ),
    .DDR_ras_n           ( DDR_ras_n        ),
    .DDR_reset_n         ( DDR_reset_n      ),
    .DDR_we_n            ( DDR_we_n         ),
    .FIXED_IO_ddr_vrn    ( FIXED_IO_ddr_vrn ),
    .FIXED_IO_ddr_vrp    ( FIXED_IO_ddr_vrp ),
    .FIXED_IO_mio        ( FIXED_IO_mio     ),
    .FIXED_IO_ps_clk     ( FIXED_IO_ps_clk  ),
    .FIXED_IO_ps_porb    ( FIXED_IO_ps_porb ),
    .FIXED_IO_ps_srstb   ( FIXED_IO_ps_srstb),
    
    .IIC_0_scl_io          (hdmi_scl),
    .IIC_0_sda_io          (hdmi_sda),
    
    .clk_100m              (clk_100m),
    .clk_in1_0             (sys_clk),
    .reset_n               (rst_n),
    
    .vid_in_active         (vid_in_active),
    .vid_in_clk            (clk_100m),
    .vid_in_data           (vid_in_data  ),
    .vid_in_vsync          (vid_in_vsync ),
    
    .vid_out_active        (vid_out_active),
    .vid_out_hsync         (vid_out_hsync ),
    .vid_out_vsync         (vid_out_vsync ),
    .video_out_data        (video_out_data),
    .vio_clk_out           (vio_clk_out   )
    
);  
    


img_gen  
#(
	.ACTIVE_IW(1280),
	.ACTIVE_IH(720),
	.TOTAL_IW (1650),
	.TOTAL_IH (750),
	.H_START  (5), 
	.V_START  (5) 
)
img
( 
	.clk	(clk_100m),
	.rst_n	(rst_n),
	.vs		(vid_in_vsync),
	.de		(vid_in_active),
	.data	(vid_in_data)
);

endmodule
