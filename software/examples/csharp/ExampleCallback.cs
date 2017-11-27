using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change to your UID

	// Callback function for the high contrast image
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

		// Register high contrast callback to function HighContrastImageCB
    	ti.HighContrastImageCallback += HighContrastImageCB;

    	// Enable high contrast image transfer for callback
    	ti.SetImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

		System.Console.WriteLine("Press enter to exit");
		System.Console.ReadLine();
		ipcon.Disconnect();
	}
}
