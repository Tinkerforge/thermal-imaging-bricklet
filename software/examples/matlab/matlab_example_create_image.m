function matlab_example_create_image()
  import java.io.File;
  import javax.imageio.ImageIO;
  import com.tinkerforge.IPConnection;
  import java.awt.image.BufferedImage;
  import com.tinkerforge.BrickletThermalImaging;

  % Takes one thermal image and saves it as PNG

  HOST = 'localhost';
  PORT = 4223;
  UID = 'XYZ'; % Change XYZ to the UID of your Thermal Imaging Bricklet

  WIDTH = 80;
  HEIGHT = 60;
  SCALE = 10;

  % Creates standard thermal image color palette (blue=cold, red=hot)
  paletteR = zeros(1, 255, 'int32');
  paletteG = zeros(1, 255, 'int32');
  paletteB = zeros(1, 255, 'int32');

  function createThermalImageColorPalette()
    % The palette is gnuplot's PM3D palette.
    % See here for details: https://stackoverflow.com/questions/28495390/thermal-imaging-palette
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

  % Helper function for simple buffer resize
  function resizedBufferedImage = resize(sourceBufferedImage, newW, newH)
    import java.awt.Image;
    import java.awt.image.BufferedImage;

    scaledSourceBufferedImage = sourceBufferedImage.getScaledInstance(newW, ...
                                                                      newH, ...
                                                                      Image.SCALE_SMOOTH);
    resizedBufferedImage = BufferedImage(newW, newH, BufferedImage.TYPE_INT_ARGB);

    g2d = resizedBufferedImage.createGraphics();
    g2d.drawImage(scaledSourceBufferedImage, 0, 0, []);
    g2d.dispose();
  end

  ipcon = IPConnection(); % Create IP connection
  ti = handle(BrickletThermalImaging(UID, ipcon), 'CallbackProperties'); % Create device object

  ipcon.connect(HOST, PORT); % Connect to brickd
  % Do not use device before ipcon is connected

  % Enable high contrast image transfer for getter
  ti.setImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE);

  createThermalImageColorPalette();

  % If we change between transfer modes we have to wait until one more
  % image is taken after the mode is set and the first image is saved
  % we can call get_high_contrast_image any time.
  pause on;
  pause(0.5);

  image = typecast(ti.getHighContrastImage(), 'int32');

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

  % Scale to 800x600 and save thermal image!
  ImageIO.write(resize(bufferedImage, WIDTH*SCALE, HEIGHT*SCALE), 'png', File('thermal_image.png'));

  input('Press key to exit\n', 's');

  ipcon.disconnect();
end
