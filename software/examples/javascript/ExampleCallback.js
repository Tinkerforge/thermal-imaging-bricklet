var Tinkerforge = require('tinkerforge');

// FIXME: This example is incomplete

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your Thermal Imaging Bricklet

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var ti = new Tinkerforge.BrickletThermalImaging(UID, ipcon); // Create device object

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        // Enable high contrast image transfer for callback
        ti.setImageTransferConfig(Tinkerforge.BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);
    }
);

// Register high contrast image callback
ti.on(Tinkerforge.BrickletThermalImaging.CALLBACK_HIGH_CONTRAST_IMAGE,
    // Callback function for high contrast image callback
    function (image) {
        console.log('Image: ' + image);
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
