<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletThermalImaging.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletThermalImaging;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your Thermal Imaging Bricklet

// Callback function for high contrast image callback
function cb_highContrastImage($image)
{
    // $image is a array of size 80*60 with 8 bit grey value for each element
}

$ipcon = new IPConnection(); // Create IP connection
$ti = new BrickletThermalImaging(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Register high contrast image callback to function cb_highContrastImage
$ti->registerCallback(BrickletThermalImaging::CALLBACK_HIGH_CONTRAST_IMAGE,
                      'cb_highContrastImage');

// Enable high contrast image transfer for callback
$ti->setImageTransferConfig(BrickletThermalImaging::IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
