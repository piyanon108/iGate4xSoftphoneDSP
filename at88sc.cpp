#include "at88sc.h"

AT88SC::AT88SC()
{
    int ret,i;
    unsigned char read_buf[8] = {0};
    unsigned char test_data[8];
    unsigned char user_data[16] = {0x77, 0x77, 0x77, 0x2E, 0x67, 0x7A, 0x73, 0x65, 0x65, 0x69, 0x6E, 0x67, 0x2E, 0x63, 0x6F, 0x6d};//www.gzseeing.com
    unsigned char read_user_data[16];

    fd = open(AT88SC_DEV,O_RDWR);
    if (fd <= 0){
        printf("open erro ,the erro num is %d \n",fd);
    }

    set_user_zone();

}
unsigned char AT88SC::communication_test(int fd)
{
    unsigned char ret;
    ret = ioctl(fd, I2C_SLAVE, AT88SC_ADDRESS);
    return ret;
}

int AT88SC::eeprom_write(unsigned int addr, unsigned int offset, unsigned char *buf, unsigned char len)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg             i2cmsg;
    int i;
    unsigned char _buf[MAX_BYTES + 1];

    if(len>MAX_BYTES){
        fprintf(stderr,"I can only write MAX_BYTES bytes at a time!\n");
        return -1;
    }

    if(len+offset >256){
        fprintf(stderr,"Sorry, len(%d)+offset(%d) > 256 (page boundary)\n",
            len,offset);
        return -1;
    }

    _buf[0]=offset;    /* _buf[0] is the offset into the eeprom page! */
    for(i=0;i<len;i++) /* copy buf[0..n] -> _buf[1..n+1] */
        _buf[1+i]=buf[i];

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;

    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1+len;
    i2cmsg.buf   = _buf;

    if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
        perror("ioctl()");
        fprintf(stderr,"ioctl returned %d\n",i);
        return -1;
    }

    if(len>0)
        fprintf(stderr,"Wrote %d bytes to eeprom at 0x%02x, offset %08x\n",
            len,addr,offset);
    else
        fprintf(stderr,"Positioned pointer in eeprom at 0x%02x to offset %08x\n",
            addr,offset);

    return 0;
}
int AT88SC::eeprom_read(unsigned int addr, unsigned int offset,	 unsigned char *buf, unsigned char len)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg             i2cmsg;
    int i;

    if(len>MAX_BYTES){
        fprintf(stderr,"I can only write MAX_BYTES bytes at a time!\n");
        return -1;
    }

    if(eeprom_write(addr,offset,NULL,0)<0)
        return -1;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;

    i2cmsg.addr  = addr;
    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = len;
    i2cmsg.buf   = buf;

    if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
        perror("ioctl()");
        fprintf(stderr,"ioctl returned %d\n",i);
        return -1;
    }

    fprintf(stderr,"Read %d bytes from eeprom at 0x%02x, offset %08x\n",
        len,addr,offset);

    return 0;
}

int AT88SC::set_user_zone()
{
    unsigned char write_buf[3] = {0x03,0x00,0x00};
    return eeprom_write(AT88SC_ADDRESS,0xb4,write_buf,3);
}
