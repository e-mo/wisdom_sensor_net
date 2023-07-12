// Negotiate with another node the lowest stable
// transmission power required
bool _tx_negotiate_power_level(
		Rfm69 *rfm,
		uint8_t address,
		uint8_t *power_level
)
{
	bool success = false;

	// Send packet with current power level
	// starting at minimum
	// wait for ack
	// if no ack, turn up power
	// repeat until ack confirming acceptable rssi
	// end transmission

	return success;
}

bool _rx_negotiate_power_level(
		Rfm69 *rfm,
		uint8_t *address,
		uint8_t *power_level
)
{
	bool success = false;

	// wait for power level packet
	// check rssi
	// if too low, send ack but no ok
	// if high enough, send ack and ok
	// end transmission

	return success;
}
