# ESPHome BWT sensor

I own a [decalcifier made by BWT](https://www.bwt.com/de-de/produkte/perlwasseranlagen/aqa-life/) that isn't smart, actually its pretty dumb.

As I had quite a lot of trouble in the recent years, I searched for a non-invasive way to determin the opertaion status of the device.

I came up with a ESP32 and a TCS34725 color sensor to detect the display color of the device.
The display is blue in normal mode, yellowish green when it regenerates and red when it has an error like salt empty.

The ESP reads the color values (R, G, B), the color temperature and the illuminance.
It then checks which color is dominat, blue means **Normal**, green means **Regeneration** and red means **Salt empty**.

Actually red can be any error but I have to manually fill up the salt anyway, so I decided to use **Salt empty** and not **Erorr**.
