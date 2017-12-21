import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletThermalImaging;

public class ExampleCallback {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your Thermal Imaging Bricklet
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletThermalImaging ti = new BrickletThermalImaging(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Add high contrast image listener
		ti.addHighContrastImageListener(new BrickletThermalImaging.HighContrastImageListener() {
			public void highContrastImage(int[] image) {
				// image is a array of size 80*60 with 8 bit grey value for each element
			}
		});

		// Enable high contrast image transfer for callback
		ti.setImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
