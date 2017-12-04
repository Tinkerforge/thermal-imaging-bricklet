function matlab_example_live_video()
  import java.awt.*;
  import javax.swing.*;
  import java.awt.Image;
  import javax.swing.BoxLayout;
  import javax.swing.ImageIcon;
  import java.awt.event.WindowEvent;
  import com.tinkerforge.IPConnection;
  import java.awt.image.BufferedImage;
  import com.tinkerforge.BrickletThermalImaging;

  % Shows live thermal image video in in swing label

  HOST = 'localhost';
  PORT = 4223;
  UID = 'XYZ'; % Change XYZ to the UID of your Thermal Imaging Bricklet

  WIDTH = 80;
  HEIGHT = 60;
  SCALE = 5;

% Creates standard thermal image color palette (blue=cold, red=hot)
  paletteR = zeros(1, 255, 'int32');
  paletteG = zeros(1, 255, 'int32');
  paletteB = zeros(1, 255, 'int32');

  function createThermalImageColorPalette()
    for x = 1:1:255
      paletteR(x) = int32(fix(255 * sqrt(x / 255)));
      paletteG(x) = int32(fix(255 * (x / 255)^3));
      paletteB(x) = 0;

      paletteBSine = sin(2 * pi * (x / 255));

      if paletteBSine >= 0
        paletteB(x) = int32(fix(255 * sin(2 * pi * (x / 255))));
      end
    end
  end

  % Function to handle example exit
  function end_example()
      ipcon.disconnect();
      frameExample.setVisible(false);
  end

  ipcon = IPConnection(); % Create IP connection
  ti = handle(BrickletThermalImaging(UID, ipcon), 'CallbackProperties'); % Create device object

  ipcon.connect(HOST, PORT); % Connect to brickd
  % Do not use device before ipcon is connected

  % Enable high contrast image transfer for callback
  ti.setImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE);

  createThermalImageColorPalette();

  % If we change between transfer modes we have to wait until one more
  % image is taken after the mode is set and the first image is saved
  % we can call get_high_contrast_image any time.
  pause on;
  pause(0.5);

  % Prepare the JPanel and the JFrame
  panelExample = JPanel();
  frameExample = JFrame();

  set(frameExample, 'WindowClosingCallback', @(h, e) end_example());
  frameExample.setSize(WIDTH*SCALE, HEIGHT*SCALE);
  panelExample.setLayout(BoxLayout(panelExample, BoxLayout.Y_AXIS));

  % Prepare JLabel
  bufferedImage = BufferedImage(80, 60, BufferedImage.TYPE_INT_ARGB);

  newW = WIDTH*SCALE;
  newH = HEIGHT*SCALE;
  scaledSourceBufferedImage = bufferedImage.getScaledInstance(newW, ...
                                                              newH, ...
                                                              Image.SCALE_SMOOTH);
  resizedBufferedImage = BufferedImage(newW, newH, BufferedImage.TYPE_INT_ARGB);
  g2d = resizedBufferedImage.createGraphics();
  g2d.drawImage(scaledSourceBufferedImage, 0, 0, []);
  g2d.dispose();

  labelExample = JLabel(ImageIcon(resizedBufferedImage));
  labelExample.setAlignmentX(Component.CENTER_ALIGNMENT);

  % Prepare JButton
  buttonExample = JButton('End Example');
  set(buttonExample, ...
      'MouseClickedCallback', ...
      @(h, e) frameExample.dispatchEvent(WindowEvent(frameExample, ...
                                                     WindowEvent.WINDOW_CLOSING)));
  buttonExample.setAlignmentX(Component.CENTER_ALIGNMENT);

  % Populate the layout
  panelExample.add(labelExample);
  panelExample.add(buttonExample);

  frameExample.getContentPane().add(panelExample);
  frameExample.pack();
  frameExample.setVisible(true);

  % Register high contrast image callback to function cb_high_contrast_image
  set(ti, 'HighContrastImageCallback', @(h, e) cb_high_contrast_image(e, ...
                                                                      paletteR, ...
                                                                      paletteG, ...
                                                                      paletteB, ...
                                                                      WIDTH, ...
                                                                      HEIGHT, ...
                                                                      SCALE, ...
                                                                      labelExample));
end

% Callback function for high contrast image callback
function cb_high_contrast_image(e, ...
                                paletteR, ...
                                paletteG, ...
                                paletteB, ...
                                WIDTH, ...
                                HEIGHT, ...
                                SCALE, ...
                                labelExample)
    import java.awt.Image;
    import javax.swing.JLabel;
    import javax.swing.ImageIcon;
    import java.awt.image.BufferedImage;

    image = typecast(e.image, 'int32');

    % Use palette mapping to create thermal image coloring
    for i = 1:1:4800
      % Because in MATLAB/Octave indexing starts from 1
      if image(i) < 255
        image(i) = image(i) + 1;
      end

      alphaLSH = uint32(bitshift(255, 24));
      redLSH = uint32(bitshift(paletteR(image(i)), 16));
      greenLSH = uint32(bitshift(paletteG(image(i)), 8));
      blueLSH = uint32(bitshift(paletteB(image(i)), 0));

      image(i) = typecast(bitor(bitor(alphaLSH, redLSH), bitor(greenLSH, blueLSH)), ...
                          'int32');
    end

    % Create BufferedImage with data
    bufferedImage = BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_ARGB);
    bufferedImage.setRGB(0, 0, WIDTH, HEIGHT, image, 0, WIDTH);

    % Simple buffer resize
    newW = WIDTH*SCALE;
    newH = HEIGHT*SCALE;
    scaledSourceBufferedImage = bufferedImage.getScaledInstance(newW, ...
                                                                newH, ...
                                                                Image.SCALE_SMOOTH);
    resizedBufferedImage = BufferedImage(newW, newH, BufferedImage.TYPE_INT_ARGB);
    g2d = resizedBufferedImage.createGraphics();
    g2d.drawImage(scaledSourceBufferedImage, 0, 0, []);
    g2d.dispose();

    % Set resized buffered image as icon of label. Change SCALE to change the
    % size of the video
    labelExample.setIcon(ImageIcon(resizedBufferedImage));
end
