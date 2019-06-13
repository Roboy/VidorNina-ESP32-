	vidor_sys u0 (
		.clk_clk                                                    (<connected-to-clk_clk>),                                                    //                           clk.clk
		.id_switch_sw                                               (<connected-to-id_switch_sw>),                                               //                     id_switch.sw
		.id_switch_debug_out1                                       (<connected-to-id_switch_debug_out1>),                                       //                              .debug_out1
		.reset_reset_n                                              (<connected-to-reset_reset_n>),                                              //                         reset.reset_n
		.spi_bridge_mosi_to_the_spislave_inst_for_spichain          (<connected-to-spi_bridge_mosi_to_the_spislave_inst_for_spichain>),          //                    spi_bridge.mosi_to_the_spislave_inst_for_spichain
		.spi_bridge_nss_to_the_spislave_inst_for_spichain           (<connected-to-spi_bridge_nss_to_the_spislave_inst_for_spichain>),           //                              .nss_to_the_spislave_inst_for_spichain
		.spi_bridge_miso_to_and_from_the_spislave_inst_for_spichain (<connected-to-spi_bridge_miso_to_and_from_the_spislave_inst_for_spichain>), //                              .miso_to_and_from_the_spislave_inst_for_spichain
		.spi_bridge_sclk_to_the_spislave_inst_for_spichain          (<connected-to-spi_bridge_sclk_to_the_spislave_inst_for_spichain>),          //                              .sclk_to_the_spislave_inst_for_spichain
		.ptp_piezo_interface0_piezo_interface_in                    (<connected-to-ptp_piezo_interface0_piezo_interface_in>),                    //          ptp_piezo_interface0.piezo_interface_in
		.ptp_piezo_interface0_piezo_interface_out                   (<connected-to-ptp_piezo_interface0_piezo_interface_out>),                   //                              .piezo_interface_out
		.ptp_piezo_interface0_time_data_master                      (<connected-to-ptp_piezo_interface0_time_data_master>),                      //                              .time_data_master
		.ptp_piezo_interface0_time_data_slave                       (<connected-to-ptp_piezo_interface0_time_data_slave>),                       //                              .time_data_slave
		.rtc_0_conduit_end_event_trigger                            (<connected-to-rtc_0_conduit_end_event_trigger>),                            //             rtc_0_conduit_end.event_trigger
		.rtc_0_conduit_end_piezo_enable                             (<connected-to-rtc_0_conduit_end_piezo_enable>),                             //                              .piezo_enable
		.rtc_0_conduit_end_event_trigger2                           (<connected-to-rtc_0_conduit_end_event_trigger2>),                           //                              .event_trigger2
		.piezo_controller_piezo_out_export                          (<connected-to-piezo_controller_piezo_out_export>),                          //    piezo_controller_piezo_out.export
		.piezo_controller_piezo_enable_export                       (<connected-to-piezo_controller_piezo_enable_export>),                       // piezo_controller_piezo_enable.export
		.piezo_controller_piezo_enable_piezo_enable_in              (<connected-to-piezo_controller_piezo_enable_piezo_enable_in>),              //                              .piezo_enable_in
		.piezo_controller_piezo_status_export                       (<connected-to-piezo_controller_piezo_status_export>)                        // piezo_controller_piezo_status.export
	);

