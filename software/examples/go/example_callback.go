package main

import (
	"fmt"
	"github.com/tinkerforge/go-api-bindings/ipconnection"
    "github.com/tinkerforge/go-api-bindings/thermal_imaging_bricklet"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your Thermal Imaging Bricklet.

func main() {
	ipcon := ipconnection.New()
    defer ipcon.Close()
    
	ti, _ := thermal_imaging_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
    defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.
    

	ti.RegisterHighContrastImageCallback(func(image []uint8) {
        // Image is a slice of size 80*60 with an 8 bit grey value for each element.
	})

	// Enable high contrast image transfer for callback
	ti.SetImageTransferConfig(thermal_imaging_bricklet.ImageTransferCallbackHighContrastImage)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()

	ipcon.Disconnect()
}
