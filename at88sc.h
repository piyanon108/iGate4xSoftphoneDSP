#ifndef AT88SC_H
#define AT88SC_H
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <string.h>
#include <stdio.h>
#define  AT88SC_CMD_MAX_NR 			20
#define  AT88SC_CMD_MAGIC 			'x'
#define  AT88SC_DEV                 "/dev/i2c-1"
#define  AT88SC_ADDRESS 			0x58
#define  COMMUNICATION_TEST  		0x01
#define  AUTHENTICATION				0x02
#define  VERIFY_WRITE_PASSWORD		0x03
#define  SET_USER_ZONE				0x04
#define  READ_USER_ZONE				0x05
#define  WRITE_USER_ZONE			0x06
#define  WRITE_CONFIG_ZONE			0x07
#define  READ_CONFIG_ZONE			0x08
#define  SEND_CHECKSUM				0x09
#define  READ_CHECKSUM				0x0A
#define  READ_FUSE_BYTE     		0x0B
#define  BURN_FUSE                  0x0C
#define DEFAULT_I2C_BUS      "/dev/i2c-1"
#define DEFAULT_EEPROM_ADDR  0x58         /* the 24C16 sits on i2c address 0x50 */
#define DEFAULT_NUM_PAGES    8            /* we default to a 24C16 eeprom which has 8 pages */
#define BYTES_PER_PAGE       256          /* one eeprom page is 256 byte */
#define MAX_BYTES            8            /* max number of bytes to write in one chunk */
class AT88SC
{
public:
    AT88SC();

private:
    unsigned char communication_test(int fd);
    int set_user_zone();
    int eeprom_write(unsigned int addr, unsigned int offset, unsigned char *buf, unsigned char len);
    int eeprom_read(unsigned int addr, unsigned int offset,	 unsigned char *buf, unsigned char len);
    int fd;
};

#endif // AT88SC_H
