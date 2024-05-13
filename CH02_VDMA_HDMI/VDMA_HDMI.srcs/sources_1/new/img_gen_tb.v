`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/05/09 20:32:58
// Design Name: 
// Module Name: img_gen_tb
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


module img_gen_tb();

reg clk;
reg rst_n;

initial begin
clk = 0;
rst_n = 0;
#20
rst_n = 1;
end
always #10 clk=~clk;

/* clk_wiz_0 clk_1
   (
    // Clock out ports
    .clk_out1(sys_clk),     // output clk_out1
    // Status and control signals
    .reset(rst_n), // input reset
    .locked(locked),       // output locked
   // Clock in ports
    .clk_in1(clk)      // input clk_in1
);*/
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
	.clk	(clk),
	.rst_n	(rst_n),
	.vs		(vid_in_vsync),
	.de		(vid_in_active),
	.data	(vid_in_data)
);
endmodule
