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

import java.awt.Container;
import java.awt.EventQueue;
import javax.swing.GroupLayout;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;

//
// Shows live thermal image video in in swing label
//

public class ExampleLiveVideo extends JFrame  {
	static final long serialVersionUID = 42L;

	private static final String HOST = "localhost";
	private static final int PORT = 4223;
	private static final String UID = "XYZ"; // Change XYZ to the UID of your Thermal Imaging Bricklet

	private static final int WIDTH = 80;
	private static final int HEIGHT = 60;
	private static final int SCALE = 5;

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

	public ExampleLiveVideo() {
		try {
			IPConnection ipcon = new IPConnection(); // Create IP connection
			BrickletThermalImaging ti = new BrickletThermalImaging(UID, ipcon); // Create device object

			ipcon.connect(HOST, PORT); // Connect to brickd
			// Don't use device before ipcon is connected

			// Enable high contrast image transfer for manual getter 
			ti.setImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE);

			createThermalImageColorPalette();

			// Create label and layout with empty Icon in it, so the window automatically resizes correctly
			setDefaultCloseOperation(EXIT_ON_CLOSE);
			JLabel label = new JLabel(new ImageIcon(resize(new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_ARGB), WIDTH*SCALE, HEIGHT*SCALE)));
			GroupLayout gl = new GroupLayout(getContentPane());
			getContentPane().setLayout(gl);
			gl.setAutoCreateContainerGaps(true);
			gl.setHorizontalGroup(gl.createSequentialGroup().addComponent(label));
			gl.setVerticalGroup(gl.createParallelGroup().addComponent(label));
			pack();

			// Enable high contrast image transfer for callback
			ti.setImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

			// Add and implement high contrast image listener
			ti.addHighContrastImageListener(new BrickletThermalImaging.HighContrastImageListener() {
				public void highContrastImage(int[] image) {
					// Use palette mapping to create thermal image coloring 
					for(int i = 0; i < WIDTH*HEIGHT; i++) {
						image[i] = (255 << 24) | (paletteR[image[i]] << 16) | (paletteG[image[i]] << 8) | (paletteB[image[i]] << 0);
					}

					// Create BufferedImage with data
					DataBuffer buffer = new DataBufferInt(image, image.length);
					BufferedImage bufferedImage = new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_ARGB);
					bufferedImage.setRGB(0, 0, WIDTH, HEIGHT, image, 0, WIDTH);

					// Set resized buffered image as icon of label
					// change SCALE to change the size of the video
					label.setIcon(new ImageIcon(resize(bufferedImage, WIDTH*SCALE, HEIGHT*SCALE)));
				}
			});
		} catch (Exception e) {
			System.out.println(e.getMessage());
		}
    }

	public static void main(String args[]) throws Exception {
		EventQueue.invokeLater(() -> {
			ExampleLiveVideo elv = new ExampleLiveVideo();
			elv.setVisible(true);
		});
	}
}
