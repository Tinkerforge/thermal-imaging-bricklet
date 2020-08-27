#include "bindings/hal_common.h"
#include "bindings/bricklet_thermal_imaging.h"

#define UID "XYZ" // Change XYZ to the UID of your Thermal Imaging Bricklet

void example_setup(TF_HalContext *hal);
void example_loop(TF_HalContext *hal);

void check(int rc, const char* msg);

static uint16_t image_size = 80 * 60;

static int lowest_index = -1;
static uint8_t lowest_value = 255;

// Callback function for high contrast image callback
static void high_contrast_image_low_level_handler(struct TF_ThermalImaging *device, uint16_t image_chunk_offset, uint8_t image_chunk_data[62], void *user_data) {
	(void)device;
	(void)user_data; // avoid unused parameter warning

	uint16_t chunk_size = 62;
	bool last_chunk = image_chunk_offset + chunk_size > image_size;
	uint16_t elements_to_read;

	if (last_chunk) {
		// Only read the remaining pixels
		elements_to_read = image_size - image_chunk_offset;
	} else {
		// Read the complete chunk
		elements_to_read = chunk_size;
	}

	if (image_chunk_offset == 0) {
		lowest_value = 255;
		lowest_index = -1;
	}

	for(int i = 0; i < elements_to_read; ++i) {
		if(image_chunk_data[i] < lowest_value) {
			lowest_value = image_chunk_data[i];
			lowest_index = image_chunk_offset + i;
		}
	}

	if(last_chunk) {
		tf_hal_printf("Image streamed successfully. Lowest value is %d at coordinates (%d, %d)\n", lowest_value, lowest_index % 80, lowest_index / 80);
	}
}

static TF_ThermalImaging ti;

void example_setup(TF_HalContext *hal) {
	// Create device object
	check(tf_thermal_imaging_create(&ti, UID, hal), "create device object");

	// Register high contrast image callback to function high_contrast_image_low_level_handler
	tf_thermal_imaging_register_high_contrast_image_low_level_callback(&ti,
	                                                                   high_contrast_image_low_level_handler,
	                                                                   NULL);

	// Enable high contrast image transfer for callback
	check(tf_thermal_imaging_set_image_transfer_config(&ti,
	                                                   TF_THERMAL_IMAGING_IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE), "call set_image_transfer_config");
}

void example_loop(TF_HalContext *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);
}
