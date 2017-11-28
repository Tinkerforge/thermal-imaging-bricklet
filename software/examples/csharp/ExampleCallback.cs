using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your Thermal Imaging Bricklet

	// Callback function for high contrast image callback
	static void HighContrastImageCB(BrickletThermalImaging sender, byte[] image)
	{
		// image is a array of size 80*60 with 8 bit grey value for each element
	}

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletThermalImaging ti = new BrickletThermalImaging(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Register high contrast image callback to function HighContrastImageCB
		ti.HighContrastImageCallback += HighContrastImageCB;

		// Enable high contrast image transfer for callback
		ti.SetImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
