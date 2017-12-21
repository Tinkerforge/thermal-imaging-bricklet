function matlab_example_callback()
    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletThermalImaging;

    % FIXME: This example is incomplete

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your Thermal Imaging Bricklet

    ipcon = IPConnection(); % Create IP connection
    ti = handle(BrickletThermalImaging(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register high contrast image callback to function cb_high_contrast_image
    set(ti, 'HighContrastImageCallback', @(h, e) cb_high_contrast_image(e));

    % Enable high contrast image transfer for callback
    ti.setImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

    input('Press key to exit\n', 's');
    ipcon.disconnect();
end

% Callback function for high contrast image callback
function cb_high_contrast_image(e)

end
