/* 
 * File:   testlibusb.c
 * Author: michael  <mich dot peeters at gmail dot com>
 *
 * license: LGPL
 *translated from the sbtool perl script of Stefan Arts, Holland <stuffie at steunopensource dot nl>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <usb.h>

#include "smadef.h"

int verbose = 0;
int detailed = 0;

struct sunnybeam {
    DWORD serialnum;
    DWORD deviceid;
} sunnybeamdata;


/*
 *copied from YASDI: smanet.c
 */

/* Default ACCM in SMA-Net */
DWORD Accm = 0x000E0000L;

/* for the FCS calculation */
const WORD PPPGOODFCS16 = 0xf0b8;
const WORD PPPINITFCS16 = 0xffff;


/* FCS lookup table as calculated by the table generator. */
WORD fcstab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


/* remove: device specific???
 * void hostToLe32(DWORD val, BYTE * dst) {
    dst[3] = (BYTE) ((val & 0x000000ffl));
    dst[2] = (BYTE) ((val & 0x0000ff00l) >> 8);
    dst[1] = (BYTE) ((val & 0x00ff0000l) >> 16);
    dst[0] = (BYTE) ((val & 0xff000000l) >> 24);
}
 */

/**************************************************************************
   Description   : Berechnet HDLC-Checksumme
 * Modified by Michael to support 0x7d escaping
   Parameter     : fcs = zwischenwert der FCS-Berechnung
                    pCh = Zeiger auf den Datenpuffer
                    wLen = Datenpuffergr�e
   Return-Value  : neue FCS
   Changes       : Author, Date, Version, Reason
 ********************************************************
                   PRUESSING, 21.05.2001, 1.0, Created
 **************************************************************************/
WORD TSMANet_CalcFCSRaw(BYTE* pCh, WORD wLen, int ignore_escaping) {
    WORD fcs = PPPINITFCS16;
    // int escape = 0;
    while (wLen--) {

        BYTE myByte = *pCh++;
        //printf("len %d, %02X, %04X\n", wLen, myByte, fcs);
        if (!ignore_escaping && myByte == 0x7d) {
            myByte = *pCh++;
            //    printf("len %d, %02X, %04X\n", wLen, myByte, fcs);
            myByte ^= 0x20;
            wLen--;
            //  printf("escaped: %d, %02X, %04X\n", wLen, myByte, fcs);
        }
        fcs = (WORD) ((fcs >> 8) ^ fcstab[(fcs ^ myByte) & 0xff]);
    }
    return (fcs ^ 0xFFFF);
}

/*
 *end of YASDI copy: smanet.c
 */


int send_raw_message(struct usb_dev_handle *udev, BYTE* msg, int msglen, int set_deviceid) {
    if (set_deviceid)
        memcpy((BYTE*) & msg [7], &sunnybeamdata.deviceid, 2);
    WORD crc = TSMANet_CalcFCSRaw((BYTE*) & msg[1], msglen - 4, 0);
    // TODO check byte order??? --> ntohl() ??
    memcpy((BYTE*) & msg[msglen - 3], &crc, sizeof (WORD));
    usleep(200 * 1000);
    if (verbose) {
        printf("Sent: ");
        int i;
        for (i = 0; i < msglen; i++) {
            printf("%02X", msg[i]);
        }
        printf("\n");
    }

    return usb_bulk_write(udev, 0x02, msg, msglen, 1000);
}

int read_raw_message(struct usb_dev_handle *udev, BYTE* buf, int buflen, int max_iterations) {
    int pos = 0;
    BYTE tmpbuf[1024];
    // reading can spawn multiple 'usb_bulk_read operations
    // always ignore the first two raw bytes and seek for "0x7e...0x7e sequence
    usleep(300 * 1000);
    int start_found = false;
    int i;
    int previous_char_is_escape = false;
    BYTE prev_byte = 0x00;
    for (i = 0; i < max_iterations; i++) {
        usleep(70 * 1000);
        int char_read = usb_bulk_read(udev, 0x81, tmpbuf, sizeof (tmpbuf), 1000);
        if (pos + char_read > buflen) {
            fprintf(stderr, "increase the usb read buffer");
            return pos;
        }
        if (verbose) {
            printf("raw_read: ");
            int j;
            for (j = 0; j < char_read; j++) {
                printf("%02X", tmpbuf[j]);
            }
            printf("\n");
        }
        if (char_read > 2) {
            int charpos;
            int end_found = false;

            for (charpos = 2; charpos < char_read; charpos++) {
                BYTE myByte = tmpbuf[charpos];
                if (myByte == 0x7e) {
                    if (!start_found)
                        start_found = true;
                    else
                        end_found = true;
                }
                // long communications get a 0x01 0x60 in between. not sure why...
                if ((start_found) && (myByte == 0x60) && (buf[pos - 1] == 0x01)) {
                    pos--;
                    continue;
                }
                if (myByte == 0x7d) {
                    previous_char_is_escape = true;
                    continue;
                }
                if (previous_char_is_escape) {
                    switch (myByte) {
                        case 0x5e: myByte = 0x7e; // not end!
                            break;
                        case 0x5d: myByte = 0x7d; // not and escape char!
                            break;
                        default:
                            myByte ^= 0x20;
                    }
                    previous_char_is_escape = false;
                }
                buf[pos++] = myByte;
                if (end_found)
                    break; //stop loop1
            }
            if (end_found)
                break; // stop loop2
        }
    }
    if (verbose) {
        printf("raw_read processed: ");
        int j;
        for (j = 0; j < pos; j++) {
            printf("%02X", buf[j]);
        }
        printf("\n");
    }
    int resu = 0;
    if (pos > 4) {
        WORD crc = TSMANet_CalcFCSRaw((BYTE*) & buf[1], pos - 4, 1);
        WORD returned_crc;
        // TODO check byte order???
        memcpy(&returned_crc, (BYTE*) & buf[pos - 3], sizeof (WORD));
        if (crc == returned_crc) {
            resu = pos;
        } else {
            printf("read bad crc %04X, should be %04X. Message *should* be rejected.\n", returned_crc, crc);
            // TODO block on this? But: can freeze the device
            resu = pos;
        }
    }

    return resu;
}

int search_device_id(struct usb_device *dev, struct usb_dev_handle *udev) {
    BYTE basicmsg[] = {0x7e, 0xFF, 0x03, 0x40, 0x41, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e};
    // TODO check byte order???
    memcpy((BYTE*) &(basicmsg [12]), &sunnybeamdata.serialnum, sizeof (DWORD));

    int resu = send_raw_message(udev, (BYTE*) & basicmsg, sizeof (basicmsg), false);
    if (resu > 0) {
        BYTE buf[1024];
        resu = read_raw_message(udev, (BYTE*) & buf, sizeof (buf), 20);
        if (resu <= 0)
            return 0;
        memcpy(&sunnybeamdata.deviceid, &(buf[5]), sizeof (DWORD));
        if (verbose)
            printf("device id= %04lX\n", sunnybeamdata.deviceid);
        resu = 1;
    }
    return resu;
}

int do_syn_online(struct usb_dev_handle * udev) {
    BYTE cmd_syn_online[] = {0x7e, 0xff, 0x03, 0x40, 0x41, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x2e, 0x7e};
    int resu = send_raw_message(udev, (BYTE*) & cmd_syn_online, sizeof (cmd_syn_online), false);
    if (resu == 0)
        return resu;
    // always read dummy data
    BYTE buf[1024];
    resu = read_raw_message(udev, (BYTE*) & buf, sizeof (buf), 5);
    return 1;
}

int do_device_query(struct usb_device *dev, struct usb_dev_handle * udev) {
    if (!do_syn_online(udev))
        return 0;
    BYTE cmd_get_data[] = {0x7e, 0xff, 0x03, 0x40, 0x41, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0b, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x7e};

    int resu = send_raw_message(udev, (BYTE*) & cmd_get_data, sizeof (cmd_get_data), true);
    if (resu > 0) {
        BYTE buf[1024];
        resu = read_raw_message(udev, (BYTE*) & buf, sizeof (buf), 50);
        if (resu <= 0)
            return 0;
        float pac;
        float etoday;
        float etotal;
        memcpy(&pac, (BYTE*) & buf[25], sizeof (float));
        memcpy(&etoday, (BYTE*) & buf[29], sizeof (float));
        memcpy(&etotal, (BYTE*) & buf[33], sizeof (float));

        printf("pac: %.2f W\n", pac);
        printf("e-today: %.2f kWh\n", etoday);
        printf("e-total: %.2f kWh\n", etotal);
        resu = 1;
    }

    return resu;
}

int do_combined_read_messages(struct usb_dev_handle * udev, BYTE* input_msg, int input_msg_len, BYTE* output_buf, int output_buf_len) {
    if (!do_syn_online(udev))
        return 0;
    // first message
    int resu = send_raw_message(udev, input_msg, input_msg_len, true);
    if (resu == 0)
        return 0;
    int pos = 0;
    BYTE tmpbuf[1024 * 2];
    BYTE* posbuf = output_buf;

    int min = 20;
    BYTE linecnt = 0xFF;
    while (linecnt != 0) {
        if (min-- == 0)
            break;
        if (linecnt != 0xFF) {
            // ask next messages
            BYTE cmd_get_data[] = {0x7e, 0xff, 0x03, 0x40, 0x41, 0x00, 0x00, 0x00, 0x00, 0x10, linecnt, 0x0b, 0x00, 0x00, 0x7e};
            int resu = send_raw_message(udev, (BYTE*) & cmd_get_data, sizeof (cmd_get_data), true);
            if (resu == 0)
                return 0;
        }
        int char_read = read_raw_message(udev, (BYTE*) & tmpbuf, sizeof (tmpbuf), 50);
        if (pos + char_read > output_buf_len) {
            fprintf(stderr, "increase the usb read buffer for combined reading");
            return pos;
        }

        if (char_read > 12) {
            // remove first 12 bytes and last 3 bytes (CRC + 7e
            memcpy(posbuf, &tmpbuf[12], char_read - 12 - 3);
            pos += char_read - 12 - 3;
            posbuf += char_read - 12 - 3;
        }
        linecnt = tmpbuf[10];
    }
    if (verbose) {
        printf("Read multiple msgs: ");
        int j;
        for (j = 0; j < pos; j++) {
            printf("%02X", output_buf[j]);
        }
        printf("\n");
    }
    return pos;
}

int parse_measurements(char* title, char* timeFormat, char* printfFormat, BYTE* buf, int bytes_read) {
    if (bytes_read <= 0)
        return 0;
    char output_lines[100][200];
    printf("\n%s:\n", title);
    int i;
    int line = 0;
    int pos_write = 0;
    int bytes_after_filter = bytes_read;

    for (i = 1 + sizeof (DWORD); i < bytes_after_filter; i += 3 * sizeof (DWORD)) {
        // splitting is not really needed anymore, but it offers a clean printf :-)
        BYTE part_buf[3 * sizeof (DWORD)];
        int k;
        for (k = 0; k < sizeof (part_buf); k++) {
            part_buf[k] = buf[i + k];
        }

        if (verbose) {
            printf("day: ");
            int j;
            for (j = 0; j < sizeof ( part_buf); j++) {
                printf("%02X", part_buf[ j]);
            }
            printf("\n");
        }
        float val;
        memcpy(&val, (BYTE*) & part_buf[2 * sizeof (DWORD)], sizeof (float));

        time_t timestamp;
        time_t timestamp_diff;
        struct tm *ts;
        char timebuf[80];

        /* Get the current time */
        memcpy(&timestamp, part_buf, sizeof (time_t));
        memcpy(&timestamp_diff, (BYTE*) & part_buf[ sizeof (DWORD)], sizeof (time_t));
        // remove one hour ??
        timestamp -= 60 * 60 * 2;
        //timestamp += 24 * 60 * 60;
        /* Format and print the time, "ddd yyyy-mm-dd hh:mm:ss zzz" */
        ts = localtime(&timestamp);
        strftime(timebuf, sizeof (timebuf), timeFormat, ts);
        snprintf(output_lines[line++], 200, printfFormat, timebuf, val);
    }
    // reverse print
    line--;
    while (line >= 0) {
        printf("%s\n", output_lines[line--]);
    }
}

int do_today_details_query(struct usb_device *dev, struct usb_dev_handle * udev) {
    // line 991: 7e ff 03 40 41 00 00 d4 f5 10 00 0b 04 19 01 d1 4c 20 4a ff ff ff 7f 71 41 7e
    BYTE cmd_get_data[] = {0x7e, 0xff, 0x03, 0x40, 0x41, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0b, 0x04, 0x19, 0x01, 0xd1, 0x4c, 0x20, 0x4a, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x7e};

    BYTE buf[5 * 1024];

    int resu = do_combined_read_messages(udev, (BYTE*) & cmd_get_data, sizeof (cmd_get_data), (BYTE*) buf, sizeof (buf));

    parse_measurements("today", "%Y-%m-%d %H:%M:%S", "%s: %6.0f W", buf, resu);

    return resu;

}

int do_last_month_query(struct usb_device *dev, struct usb_dev_handle * udev) {
    BYTE cmd_get_data[] = {0x7e, 0xff, 0x03, 0x40, 0x41, 0x00, 0x00, 0xd4, 0xf5, 0x10, 0x00, 0x0b, 0x04, 0x7d, 0x31, 0x02, 0x7f, 0x25, 0x1f, 0x4a, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x7e};

    BYTE buf[5 * 1024];

    int resu = do_combined_read_messages(udev, (BYTE*) & cmd_get_data, sizeof (cmd_get_data), (BYTE*) buf, sizeof (buf));

    parse_measurements("last month", "%Y-%m-%d", "%s: %8.3f kWh", buf, resu);

    return resu;
}

int init_device(struct usb_device *dev, struct usb_dev_handle * udev) {
    char string[256];

    if (!dev->descriptor.iSerialNumber)
        return 0;

    int ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof (string));
    if (ret > 0) {
        char* end;
        DWORD serialnum = strtol(string, &end, 10) + 140000000;
        //        hostToLe32(serialnum, (BYTE*) & sunnybeamdata.serialnum);
        sunnybeamdata.serialnum = serialnum;
        printf("Serial Number: %s", string);
        if (verbose)
            printf(" - USB format: %ld %08lx", serialnum, sunnybeamdata.serialnum);
        printf("\n");
        if ((ret = usb_claim_interface(udev, 0)) == 0) {
            // First do a SET_FEATURE config
            ret = usb_control_msg(udev, 0x40, 0x03, 0x4138, 0x0000, NULL, 0x0000, 1000);

            if (ret == 0)
                ret = 1;
        }

    }
    return ret;
}

int process_device(struct usb_device * dev) {
    usb_dev_handle *udev;
    char description[256];
    char string[256];
    int ret, i;

    udev = usb_open(dev);
    if (!udev)
        return 0;
    int failed = 1;

    ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof (string));
    if (ret > 0) {
        snprintf(description, sizeof (description), "%04X - %04X -%s - ", dev->descriptor.idVendor, dev->descriptor.idProduct, string);
        printf("Dev #%d: %s\n", dev->devnum, description);

        if (init_device(dev, udev)) {
            if (search_device_id(dev, udev)) {
                if (do_device_query(dev, udev)) {
                    if (!detailed) {
                        failed = 0;
                    } else {
                        if (do_today_details_query(dev, udev)) {
                            if (do_last_month_query(dev, udev)) {
                                failed = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    usb_close(udev);
    return !failed;
}

int main(int argc, char *argv[]) {
    struct usb_bus *bus;

    int i = argc;
    while (i > 0) {
        i--;
        if (strcmp(argv[i], "-v") == 0)
            verbose = 1;
        else if (strcmp(argv[i], "-l") == 0)
            detailed = 1;
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("%s [-h] [--help] [-l] [-v]\n  -h or --help\n  -l: dump detailed measurements\n  -v: verbose logging\n", argv[0]);
            return 1;
        }
    }


    usb_init();

    usb_find_busses();
    usb_find_devices();

    int found = 0;
    for (bus = usb_busses; bus; bus = bus->next) {
        struct usb_device *dev;
        for (dev = bus->devices; dev; dev = dev->next)
            if (dev->descriptor.idVendor == 0x1587 && dev->descriptor.idProduct == 0x002D) {
                found = 1;
                if (!process_device(dev)) {
                    fprintf(stderr, "Sunny Beam not accessible. Run this program as root.\n");
                    return -2;
                }
            }
    }

    if (found)
        return 0;
    // else
    fprintf(stderr, "Sunny Beam not found.\n");
    return -1;
}


