#!/usr/bin/perl

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletThermalImaging;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your Thermal Imaging Bricklet

# Callback subroutine for high contrast image callback
sub cb_high_contrast_image
{
    my ($image) = @_;

    # image is an array of size 80*60 with a 8 bit grey value for each element
}

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $ti = Tinkerforge::BrickletThermalImaging->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Register high contrast image callback to subroutine cb_high_contrast_image
$ti->register_callback($ti->CALLBACK_HIGH_CONTRAST_IMAGE, 'cb_high_contrast_image');

# Enable high contrast image transfer for callback
$ti->set_image_transfer_config($ti->IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
