
module vidor_sys (
	clk_clk,
	iceboardcontrol_0_conduit_end_rx,
	iceboardcontrol_0_conduit_end_tx,
	reset_reset_n,
	spi_bridge_mosi_to_the_spislave_inst_for_spichain,
	spi_bridge_nss_to_the_spislave_inst_for_spichain,
	spi_bridge_miso_to_and_from_the_spislave_inst_for_spichain,
	spi_bridge_sclk_to_the_spislave_inst_for_spichain);	

	input		clk_clk;
	input		iceboardcontrol_0_conduit_end_rx;
	output		iceboardcontrol_0_conduit_end_tx;
	input		reset_reset_n;
	input		spi_bridge_mosi_to_the_spislave_inst_for_spichain;
	input		spi_bridge_nss_to_the_spislave_inst_for_spichain;
	inout		spi_bridge_miso_to_and_from_the_spislave_inst_for_spichain;
	input		spi_bridge_sclk_to_the_spislave_inst_for_spichain;
endmodule
