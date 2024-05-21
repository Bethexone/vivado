`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/05/06 19:06:31
// Design Name: 
// Module Name: img_gen
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


module img_gen 
#(
	parameter ACTIVE_IW  =640,
	parameter ACTIVE_IH  =480,
	parameter TOTAL_IW   =800,
	parameter TOTAL_IH   =525,
	parameter H_START    =143, 
	parameter V_START    =34   
)
( 
	input 				clk		,
	input 				rst_n	, 
	output reg 			vs		, 
	output reg 			de		,
	output reg [7:0]	data	
);

reg  [7:0] raw_array [ACTIVE_IW*ACTIVE_IH-1:0];

integer i;
 
initial begin

	for (i=0; i<ACTIVE_IW*ACTIVE_IH; i=i+1) 
		raw_array[i] = 0;
		
end

/* initial begin
	$readmemh("F:/lesson/lesson1/data/pre.txt",raw_array);
end */

reg	[31:0]	cnt_delay	;

reg [15:0] 	hcnt		;
reg [15:0] 	vcnt		;

reg			h_de		;
reg			v_de		;

reg			index_de	;	
reg	[7:0]	index       ;

always@(posedge clk or negedge rst_n)
	if(!rst_n)
		cnt_delay <= 'd0;
	else if(cnt_delay==100000000*5-1)
		cnt_delay <= cnt_delay;
	else
		cnt_delay <= cnt_delay + 1'b1;

always@(posedge clk or negedge rst_n)
	if(!rst_n)
		hcnt <= 'd0;
	else if(hcnt==TOTAL_IW-1)
		hcnt <= 'd0;
	else if(cnt_delay==100000000*5-1)
		hcnt <= hcnt + 1'b1;
	else
		hcnt <= hcnt;
		
always@(posedge clk or negedge rst_n)
	if(!rst_n)
		vcnt <= 'd0;
	else if(hcnt==TOTAL_IW-1&&vcnt==TOTAL_IH-1)
		vcnt <= 'd0;
	else if(hcnt==TOTAL_IW-1)
		vcnt <= vcnt + 1'b1;
	else
		vcnt <= vcnt;
					
always@(posedge clk or negedge rst_n)
	if(!rst_n)
		vs <= 1'b0;
	else if(vcnt>=2)
		vs <= 1'b1;
	else
		vs <= 1'b0;

always@(posedge clk or negedge rst_n)
	if(!rst_n)
		h_de <= 1'b0;
	else if(hcnt>=H_START&&hcnt<H_START+ACTIVE_IW)
		h_de <= 1'b1;
	else
		h_de <= 1'b0;
	
always@(posedge clk or negedge rst_n)
	if(!rst_n)
		v_de <= 1'b0;
	else if(vcnt>=V_START&&vcnt<V_START+ACTIVE_IH)
		v_de <= 1'b1;
	else
		v_de <= 1'b0;

always@(posedge clk or negedge rst_n)
	if(!rst_n)
		index_de <= 1'b0;
	else if(h_de==1'b1&&v_de==1'b1)
		index_de <= 1'b1;
	else
		index_de <= 1'b0;
		
always@(posedge clk or negedge rst_n)
	if(!rst_n)
		index <= 'd0;
	else if(index_de==1'b1)
		index <= index + 1'b1;
	else
		index <= 'd0;
		
always@(posedge clk or negedge rst_n)
	if(!rst_n)
		de <= 1'b0;
	else
		de <= index_de;
		
always@(posedge clk)
	if(index_de==1'b1)
		data <= index;
	else
		data <= 0;


 
endmodule 
