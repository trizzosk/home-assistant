# Home Assistant setup

This guideline **include** my setup of the `Home Assistant`, especially:
- Environment measurements (temp, CO2, etc.) inside rooms
- Simple status display using eink Lilygo EPD47 unit
- Daily backups with offloading of backup files to NAS or any other device using `Samba Backup` add-on
- Very simple, basic, Home alarm (sensors, management, etc.)
- Custom nameday sensor with Slovakian names
- Showing upcoming birthdays of your chosen family members

This guideline **does not cover/include** standard `Home Assistant` setup procedure (hardware and software), including booting from PCIe SSD, etc..

# Disclaimer

All information are provided as is, with no guarantees and liabilities. I am not responsible for any damage to any equipment which you code based on code examples published here. Be careful. 

Provided code is dirty, sometimes can be not fully optimized. But still works properly. Use it as example, inspiration for your journey.

# Environment measurements using custom ESP32 board and IKEA VINDRIKTNING sensor

Detailed step-by-step procedure is in [this document](./laskakit-vindriktning.md).

# Simple status display using eink Lilygo EPD47 unit

Using quite cheap `Lilygo EPD47` eink diplay unit we have immediate overview about current status - details are [here](./eink-display/README.md).

# Daily backups with backup offloading, using `Samba Backup` add-on

Although my `Home Assistant` boots from SSD drive (attached using PCIe to IO board) with 512 GB of space, for doing proper backups you have to offload backups to another location, another device. Yes, I am not the enterprise, but I do not want to rely only on SSD drive. `Anything that can possibly go wrong, does.` - so doing proper backups is pretty important for me. Detailed procedure is described in [this document](./backup-offloading-samba.md).

# Nameday sensor

Let's be honest - I am not perfect. Quite often I forgot about names and nameday. Nothing really that much important, but still sometimes it's needed (especially about your family, right?).

I tried several solutions and addons for Home assistant. Some good, some awful, some bad. I find a pretty simple json file on github which contains structured data - I made a fork of it but the original kudos goes to [@zoltancsontos](https://github.com/zoltancsontos) on github (original repo: [here](https://github.com/zoltancsontos/slovak-name-days-json)).

More details and how it works - see [this document](./nameday-sensor/README.md).

# Birthdays section and sensor

I wanted to have a simple sensor showing next lucky person to have bitrhday party. I was struggling because there was no really smooth and easy integration so I started my small research and setup. For more details have a look in [this document](./birthdays-sensor.md).

# To-Do:

Additional chapters will be provided in upcoming days/weeks:
- Custom, pretty easy and basic, home alarm using PIR and door sensors, Node-Red flows and Matrix messenger component for notifications and commands

