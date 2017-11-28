function octave_example_callback()
    more off;

    % FIXME: This example is incomplete

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your Thermal Imaging Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    ti = javaObject("com.tinkerforge.BrickletThermalImaging", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register high contrast image callback to function cb_high_contrast_image
    ti.addHighContrastImageCallback(@cb_high_contrast_image);

    % Enable high contrast image transfer for callback
    ti.setImageTransferConfig(ti.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

% Callback function for high contrast image callback
function cb_high_contrast_image(e)
    fprintf("Image: %d\n", e.image);
end
