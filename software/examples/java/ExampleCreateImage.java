import com.tinkerforge.BrickletThermalImaging;
import com.tinkerforge.IPConnection;

import java.awt.image.BufferedImage;
import javax.imageio.ImageIO;
import java.io.File;
import java.awt.Image;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferInt;
import java.awt.Graphics2D;
import java.lang.Math;

//
// Takes one thermal image and saves it as PNG
//

public class ExampleCreateImage {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;
	private static final String UID = "XYZ"; // Change XYZ to the UID of your Thermal Imaging Bricklet

	// Creates standard thermal image color palette (blue=cold, red=hot)
	private static int[] paletteR = new int[256];
	private static int[] paletteG = new int[256];
	private static int[] paletteB = new int[256];
	public static void createThermalImageColorPalette()
	{
		// The palette is gnuplot's PM3D palette.
		// See here for details: https://stackoverflow.com/questions/28495390/thermal-imaging-palette
		for(int x = 0; x < 256; x++)
		{
			paletteR[x] = (int)(255*Math.sqrt(x/255.0));
			paletteG[x] = (int)(255*Math.pow(x/255.0, 3));
			if(Math.sin(2*Math.PI * (x/255.0)) >= 0.0) 
			{
				paletteB[x] = (int)(255*Math.sin(2*Math.PI * (x/255.0)));
			} else 
			{
				paletteB[x] = 0;
			}
		}
	}

	// Helper function for simple buffer resize
	public static BufferedImage resize(BufferedImage img, int newW, int newH) { 
		Image tmp = img.getScaledInstance(newW, newH, Image.SCALE_SMOOTH);
		BufferedImage dimg = new BufferedImage(newW, newH, BufferedImage.TYPE_INT_ARGB);

		Graphics2D g2d = dimg.createGraphics();
		g2d.drawImage(tmp, 0, 0, null);
		g2d.dispose();

		return dimg;
	}  

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions you
	//       might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletThermalImaging ti = new BrickletThermalImaging(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Enable high contrast image transfer for manual getter 
		ti.setImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE);

		createThermalImageColorPalette();

		// If we change between transfer modes we have to wait until one more 
		// image is taken after the mode is set and the first image is saved 
		// we can call GetHighContrastImage any time.
		Thread.sleep(250);

		int[] image = ti.getHighContrastImage();

		// Use palette mapping to create thermal image coloring 
		for(int i = 0; i < 80*60; i++) {
			image[i] = (255 << 24) | (paletteR[image[i]] << 16) | (paletteG[image[i]] << 8) | (paletteB[image[i]] << 0);
		}

		// Create BufferedImage with data
		DataBuffer buffer = new DataBufferInt(image, image.length);
		BufferedImage bufferedImage = new BufferedImage(80, 60, BufferedImage.TYPE_INT_ARGB);
		bufferedImage.setRGB(0, 0, 80, 60, image, 0, 80);

		// Scale to 800x600 and save thermal image!
		ImageIO.write(resize(bufferedImage, 80*10, 60*10), "png", new File("thermal_image.png"));

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
