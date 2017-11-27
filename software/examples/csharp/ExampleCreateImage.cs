using Tinkerforge;
using System;
using System.Threading;
using System.Drawing;
using System.Drawing.Imaging;

//
// Takes one thermal image and saves it as PNG
// This example needs /reference:System.Drawing.dll
//

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change to your UID

	// Creates standard thermal image color palette (blue=cold, red=hot)
	private static byte[] paletteR = new byte[256];
	private static byte[] paletteG = new byte[256];
	private static byte[] paletteB = new byte[256];
	static void CreateThermalImageColorPalette()
	{
		for(int x = 0; x < 256; x++)
		{
			paletteR[x] = System.Convert.ToByte(255*Math.Sqrt(x/255.0));
			paletteG[x] = System.Convert.ToByte(255*Math.Pow(x/255.0, 3));
			if(Math.Sin(2*Math.PI * (x/255.0)) >= 0) 
			{
				paletteB[x] = System.Convert.ToByte(255*Math.Sin(2*Math.PI * (x/255.0)));
			} else 
			{
				paletteB[x] = 0;
			}
		}
	}

	static void Main() 
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletThermalImaging ti = new BrickletThermalImaging(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		CreateThermalImageColorPalette();

    	// Enable high contrast image transfer for callback
    	ti.SetImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE);

		// If we change between transfer modes we have to wait until one more 
		// image is taken after the mode is set and the first image is saved 
		// we can call GetHighContrastImage any time.
		Thread.Sleep(250);

		byte[] imageData = ti.GetHighContrastImage();

		// Create PNG with Bitmap from System.Drawing
		Bitmap bitmap = new Bitmap(80, 60);
		for(int row = 0; row < 80; row++)
		{
			for(int col = 0; col < 60; col++)
			{
				int color = imageData[row + col*80];
				bitmap.SetPixel(row, col, Color.FromArgb(paletteR[color], paletteG[color], paletteB[color]));
			}
		}

		// Scale to 800x600 and save thermal image!
		bitmap = new Bitmap(bitmap, new Size(80*10, 60*10));
		bitmap.Save("thermal_image.png", ImageFormat.Png);

		System.Console.WriteLine("Press enter to exit");
		System.Console.ReadLine();
		ipcon.Disconnect();
	}
}
