/**
 * @file
 *
 * @date 3/26/2018
 * @author Anthony Hilyard
 * @brief A simple program to fill the framebuffer with a given color.
 */

#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <cstdlib>

using namespace std;

#define DEFAULT_FB_DEV		"/dev/fb0"

/**
 * @brief Converts the given string to a number and ensures it is within range.
 * @param optionString The string to convert.
 * @param value The output variable this value is stored in.
 * @return False on error, otherwise true.
 */
bool calcComponent(const string &optionString, unsigned char &value)
{
	unsigned long converted = strtoul(optionString.c_str(), 0, 0);
	if (converted > 0xFF)
	{
		cerr << "Component arguments must be in range 0-255!" << endl;
		return false;
	}

	value = converted;
	return true;
}

/**
 * @brief Main function for fbfill program.
 * @param argc Number of arguments.
 * @param argv Arguments.
 * @return 0 on success, 1 on argument-related error, 2 on framebuffer open error, 3 framebuffer read error.
 */
int main(int argc, char *argv[])
{
	unsigned char red = 0,
				  blue = 0,
				  green = 0,
				  alpha = 0;
	string framebuffer = DEFAULT_FB_DEV;

	int opt;
	bool err = false;
	while ((opt = getopt(argc, argv, "r:g:b:a:f:")) != -1)
	{
		switch (opt)
		{
			case 'r':
				if (!calcComponent(optarg, red))
				{
					err = true;
				}
				break;
			case 'g':
				if (!calcComponent(optarg, green))
				{
					err = true;
				}
				break;
			case 'b':
				if (!calcComponent(optarg, blue))
				{
					err = true;
				}
				break;
			case 'a':
				if (!calcComponent(optarg, alpha))
				{
					err = true;
				}
				break;
			case 'f':
				framebuffer = optarg;
			default:
				err = true;
		}
		if (err)
		{
			cerr << "Usage: " << argv[0] << " [-r value] [-g value] [-b value] [-a value] [-f device]" << endl;
			return 1;
		}
	}

	// Okay, our options are all parsed, let's open up the framebuffer.
	int fbfd = 0;
	fb_var_screeninfo vinfo;
	long int screensize = 0;
	char *fbp = 0;

	fbfd = open(framebuffer.c_str(), O_RDWR);
	if (!fbfd)
	{
		cerr << "Error: cannot open framebuffer device." << endl;
		return 2;
	}

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
	{
		cerr << "Error reading variable screen information." << endl;
		return 3;
	}

	// Now we're going to use the variable screen info to figure out the bit format of the framebuffer.
	int alphaLen = vinfo.transp.length, alphaOffset = vinfo.transp.offset;

	if (alphaLen == 0 && alphaOffset == 0)
	{
		// Maybe alpha isn't supported, but check just in case.
		if (vinfo.red.length + vinfo.green.length + vinfo.blue.offset < vinfo.bits_per_pixel)
		{
			// Okay, there's still space in each pixel--let's cram alpha in there.
			alphaLen = vinfo.bits_per_pixel - (vinfo.red.length + vinfo.green.length + vinfo.blue.length);
			alphaOffset = vinfo.bits_per_pixel - alphaLen;
		}
	}

	// Store all the color info in one variable.
	uint32_t colorValue = 0;
	colorValue |= (red & ((1 << vinfo.red.length) - 1)) << vinfo.red.offset;
	colorValue |= (green & ((1 << vinfo.green.length) - 1)) << vinfo.green.offset;
	colorValue |= (blue & ((1 << vinfo.blue.length) - 1)) << vinfo.blue.offset;
	if (alphaLen > 0)
	{
		colorValue |= (alpha & ((1 << alphaLen) - 1)) << alphaOffset;
	}

	// Now mmap the framebuffer...
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

	// And finally, fill it with the color value we calculated.
	fill((uint32_t *)fbp, (uint32_t *)(fbp + screensize), colorValue);

	munmap(fbp, screensize);

	close(fbfd);
	return 0;
}
