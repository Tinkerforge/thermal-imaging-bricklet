using Tinkerforge;
using System;
using System.Threading;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

//
// Shows live thermal image video in Windows.Forms window
// This example needs /reference:System.Drawing.dll and /reference:System.Windows.Forms.dll
//

class Example : System.Windows.Forms.Form
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your Thermal Imaging Bricklet

	private static int WIDTH = 80;
	private static int HEIGHT = 60;
	private static int SCALE = 5;

	private byte[] imageData = new byte[80*60];

	// Creates standard thermal image color palette (blue=cold, red=hot)
	private byte[] paletteR = new byte[256];
	private byte[] paletteG = new byte[256];
	private byte[] paletteB = new byte[256];
	void CreateThermalImageColorPalette()
	{
		// The palette is gnuplot's PM3D palette.
		// See here for details: https://stackoverflow.com/questions/28495390/thermal-imaging-palette
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

	// Callback function for the high contrast image
	void HighContrastImageCB(BrickletThermalImaging sender, byte[] image)
	{
		// Save image and trigger paint event handler
		imageData = image;
		this.Refresh();
	}
	
	public Example()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletThermalImaging ti = new BrickletThermalImaging(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		CreateThermalImageColorPalette();

		// Register high contrast callback to function cb_high_contrast_image
    	ti.HighContrastImageCallback += HighContrastImageCB;

    	// Enable high contrast image transfer for callback
    	ti.SetImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

		// Use correct size for window and add paint event handler
		this.ClientSize = new System.Drawing.Size(WIDTH*SCALE, HEIGHT*SCALE);
		this.Paint += new System.Windows.Forms.PaintEventHandler(this.ThermalImagePaint);

	}

	private void ThermalImagePaint(object sender, System.Windows.Forms.PaintEventArgs e)
	{
		// Create PNG with Bitmap from System.Drawing
		Bitmap bitmap = new Bitmap(WIDTH, HEIGHT);
		for(int row = 0; row < WIDTH; row++)
		{
			for(int col = 0; col < HEIGHT; col++)
			{
				int color = imageData[row + col*WIDTH];
				bitmap.SetPixel(row, col, Color.FromArgb(paletteR[color], paletteG[color], paletteB[color]));
			}
		}

		// Scale to bigger size (can be changed with SCALE constant)!
		bitmap = new Bitmap(bitmap, new Size(WIDTH*SCALE, HEIGHT*SCALE));
		e.Graphics.DrawImage(bitmap, 0, 0);
	
		// Dispose
		e.Graphics.Dispose();		
	}

	static void Main() 
	{
		Application.Run(new Example());
	}
}
