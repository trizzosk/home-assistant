# Home Assistant setup

This guideline **include** my setup of the `Home Assistant`, especially:
- Environment measurements (temp, CO2, etc.) inside rooms
- Daily backups with offloading of backup files to NAS or any other device using `Samba Backup` add-on
- Very simple, basic, Home alarm (sensors, management, etc.)

This guideline **does not cover/include** standard `Home Assistant` setup procedure (hardware and software), including booting from PCIe SSD, etc..
 
# Environment measurements using custom ESP32 board and IKEA VINDRIKTNING sensor

Detailed step-by-step procedure is in [this document](./laskakit-vindriktning.md).

# Daily backups with backup offloading, using `Samba Backup` add-on

Although my `Home Assistant` boots from SSD drive (attached using PCIe to IO board) with 512 GB of space, for doing proper backups you have to offload backups to another location, another device. Yes, I am not the enterprise, but I do not want to rely only on SSD drive. `Anything that can possibly go wrong, does.` - so doing proper backups is pretty important for me. Detailed procedure is described in [this document](./backup-offloading-samba.md).

# To-Do:

Additional chapters will be provided in upcoming days/weeks:
- Custom, pretty easy and basic, home alarm using PIR and door sensors, Node-Red flows and Matrix messenger component for notifications and commands
- e-INK display with enclosure for displaying environment measurements in fancy way.
