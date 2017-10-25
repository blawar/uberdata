/* $Id: ms3_init.c,v 1.377.2.9 2013/05/22 13:48:01 jsmcortina Exp $
 * Copyright 2007, 2008, 2009, 2010, 2011, 2012 James Murray and Kenneth Culver
 *
 * This file is a part of Megasquirt-3.
 *
 * cp_flash_ram()
    Origin: James Murray
    Majority: James Murray
 * set_prime_ASE
    Origin: Al Grippo
    Major: James Murray / Kenneth Culver
    Majority: Al Grippo / James Murray / Kenneth Culver
 * generic_adc_setup
     Origin: Kenneth Culver (as generic_IO_setup)
     Moderate: Split out and add portusage. James Murray
     Majority: Kenneth Culver, James Murray
 * generic_hwpwm_setup
     Origin: Kenneth Culver (as generic_IO_setup)
     Moderate: Split out and add portusage. James Murray
     Majority: Kenneth Culver, James Murray
 * generic_digout_setup
     Origin: Kenneth Culver (as generic_IO_setup)
     Moderate: Split out and add portusage. James Murray
     Majority: Kenneth Culver, James Murray
 * generic_digin_setup
     Origin: Kenneth Culver (as generic_IO_setup)
     Moderate: Split out and add portusage. James Murray
     Majority: Kenneth Culver, James Murray
 * generic_timer_setup
    Origin: James Murray
    Majority: James Murray
 * main_init
    Origin: Al Grippo
    Major: Rewrite for XEP, new features, generic_IO. James Murray / Kenneth Culver
    Majority: James Murray / Kenneth Culver
 * vss_init
    Origin: James Murray
    Majority: James Murray
 * water_inj_init
    Origin: James Murray
    Majority: James Murray
 * spr_port_init
    Origin: James Murray
    Majority: James Murray
 * var_init
    Origin: James Murray / Kenneth Culver
    Majority: James Murray / Kenneth Culver
 * egt_init
    Origin: James Murray
    Majority: James Murray
 * sensors_init
    Origin: James Murray
    Majority: James Murray
 * pinport_init
    Origin: James Murray
    Majority: James Murray
 *
 * You should have received a copy of the code LICENSE along with this source, please
 * ask on the www.msextra.com forum if you did not.
 *
*/

#include "ms3.h"

void ck_ee_erased(void)
{
    unsigned int *addr = (unsigned int *) 0x800;
    unsigned int x, result = 0;

    for (x = 0; x < 512; x++) {
        if (*addr == 0xffff) {
            result++;
        }
        addr++;                 // next word
    }
    if (result == 512) {
        conf_err = 49;
    }
}

void cp_flash_ram(void)
{
    unsigned char EPAGE_save;
    int x;
// copy the flash tables into RAM at startup

    EPAGE_save = EPAGE;

    for (x = 0 ; x < NO_TBLES ; x++) {
        if (cpf2r[x]) {
            RPAGE = tables[x].rpg;
            EPAGE = tables[x].addrFlash / 0x400;
            ck_ee_erased();
            memcpy((unsigned char *) (tables[x].adhi << 8), (unsigned char *) 0x0800, tables[x].n_bytes);
        }
    }

    /* Long term trim copy*/
    {
        unsigned int r[16];
        int l[16], x;

        /* copy axes from VE1 */
        RPAGE = tables[12].rpg;
        for (x = 0 ; x < 16; x++) {
            r[x] = ram_window.pg19.frpm_tablev1[x];
            l[x] = ram_window.pg19.fmap_tablev1[x];
        }
        RPAGE = tables[25].rpg;
        for (x = 0 ; x < 16; x++) {
            ram_window.trimpage.ltt_rpms[x] = r[x];
            ram_window.trimpage.ltt_loads[x] = l[x];
        }
    }
    {
        int x, y;

        /* pick copy A or B */
        EPAGE = 0x19;
        if ((*(unsigned char*)0x8ff == 0xff) && (*(unsigned char*)0x9ff == 0xff)) {
            /* both sectors in erased state, default to zero */
            for (x = 0; x < 16; x++) {
                for (y = 0; y < 16; y++) {
                    ram_window.trimpage.ltt_table1[x][y] = 0;
                }
            }
        } else {
            if (*(unsigned char*)0x8ff == 0xff) {
                ltt_fl_ad = 0x900;
            } else if (*(unsigned char*)0x9ff == 0xff) {
                ltt_fl_ad = 0x800;
            } else {
                /* both readable (normal case)
                find newest */
                if ((*(unsigned char*)0x8ff == 0) && (*(unsigned char*)0x9ff == 0x7f)) {
                    ltt_fl_ad = 0x800;
                } else if ((*(unsigned char*)0x9ff == 0) && (*(unsigned char*)0x8ff == 0x7f)) {
                    ltt_fl_ad = 0x900;
                } else {
                    if (*(unsigned char*)0x8ff > *(unsigned char*)0x9ff) {
                        ltt_fl_ad = 0x800;
                    } else {
                        ltt_fl_ad = 0x900;
                    }
                }
            }
            memcpy((unsigned char *) 0x1a00, (unsigned char *) ltt_fl_ad, 256);   // trim table
            /* top RH corner (last byte in sector) is used for sector no. so fake the data there */
            ram_window.trimpage.ltt_table1[15][15] = ram_window.trimpage.ltt_table1[15][14]; 
        }
    }

    EPAGE = EPAGE_save;
}

void set_prime_ASE(void)
{
    if (pin_dualfuel && (ram5.dualfuel_sw2 & 0x01) && (!(*port_dualfuel & pin_dualfuel))) {
        PrimeP = (unsigned short) CW_table(outpc.clt, (int *) ram_window.pg21.CWPrime2, (int *) ram_window.pg21.temp_table_p21, 21);     // msx10
    } else {
        PrimeP = (unsigned short) CW_table(outpc.clt, (int *) ram_window.pg8.CWPrime, (int *) ram_window.pg8.temp_table_p5, 8);     // msx10
    }

    if (pin_dualfuel && (ram5.dualfuel_sw & 0x80) && (!(*port_dualfuel & pin_dualfuel))) {
        AWEV = (unsigned short) CW_table(outpc.clt, (int *) ram_window.pg21.CWAWEV2, (int *) ram_window.pg21.temp_table_p21, 21);        // %
        AWC = (unsigned short) CW_table(outpc.clt, (int *) ram_window.pg21.CWAWC2, (int *) ram_window.pg21.temp_table_p21, 21);  // cycles
    } else {
        AWEV = (unsigned short) CW_table(outpc.clt, (int *) ram_window.pg8.CWAWEV, (int *) ram_window.pg8.temp_table_p5, 8);        // %
        AWC = (unsigned short) CW_table(outpc.clt, (int *) ram_window.pg8.CWAWC, (int *) ram_window.pg8.temp_table_p5, 8);  // cycles
    }
}

void generic_adc_setup(volatile unsigned short **port, unsigned char feature_conferr,
                      unsigned char pinsetting, unsigned char feature_num)
{
//XXX      = bits,   U08,  YY, [0:4], "Mainboard", "JS5 (ADC6)", "JS4 (ADC7)", "EXT_MAP (ADC11)", "EGO2 (ADC12)", "Spare ADC (ADC13)", "MAP port", "INVALID", "CAN ADC01", "CAN ADC02", "CAN ADC03", "CAN ADC04", "CAN ADC05", "CAN ADC06", "CAN ADC07", "CAN ADC08", "CAN ADC09", "CAN ADC10", "CAN ADC11", "CAN ADC12", "CAN ADC13", "CAN ADC14", "CAN ADC15", "CAN ADC16", "CAN ADC17", "CAN ADC18", "CAN ADC19", "CAN ADC20", "CAN ADC21", "CAN ADC22", "CAN ADC23", "CAN ADC24"

    portpins *portusage;
    portusage = init_portusage_addr; // place on stack - only called from init

    if (pinsetting == 0) {    // Share MAP (for stream logging)
        *port = &ATD0DR0;
    } else if (pinsetting == 1) {    // AN6
        if ((portusage->ad0l[6]) && feature_conferr) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0l[6];
        } else {
            portusage->ad0l[6] = feature_num;
            *port = &ATD0DR6;
        }
    } else if (pinsetting == 2) {     // AN7
        if ((portusage->ad0l[7]) && feature_conferr) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0l[7];
        } else {
            portusage->ad0l[7] = feature_num;
            *port = &ATD0DR7;
        }
    } else if (pinsetting == 3) {     // AN11
        if ((portusage->ad0h[3]) && feature_conferr) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0h[3];
        } else { 
            portusage->ad0h[3] = feature_num;
            *port = &ATD0DR11;
        }
    } else if (pinsetting == 4) {     // AN12
        if ((portusage->ad0h[4]) && feature_conferr) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0h[4];
        } else {
            portusage->ad0h[4] = feature_num;
            *port = &ATD0DR12;
        }
    } else if (pinsetting == 5) {     // AN13
        if ((portusage->ad0h[5]) && feature_conferr) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0h[5];
        } else {
            portusage->ad0h[5] = feature_num;
            *port = &ATD0DR13;
        }
    } else if (pinsetting == 6) {     // AN0
        if ((portusage->ad0l[0]) && feature_conferr) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0l[0];
        } else {
            portusage->ad0l[0] = feature_num;
            *port = &ATD0DR0;
        }
    // 7 invalid
    } else if ((pinsetting >= 8) && (pinsetting <= 31)) {      // CAN ADCs
        if ((portusage->canadc[pinsetting - 8]) && feature_conferr) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canadc[pinsetting - 8];
        } else {
            portusage->canadc[pinsetting - 8] = feature_num;
            *port = (volatile unsigned short *)&datax1.adc[pinsetting - 8];
        }
    }
}

void generic_hwpwm_setup(volatile unsigned char * volatile *port, unsigned char feature_conferr,
                      unsigned char pinsetting, unsigned char hwpwm,
                      unsigned char polarity, unsigned char feature_num)
{

    portpins *portusage;
    portusage = init_portusage_addr; // place on stack - only called from init

    hwpwm &= 0x0f;
    if (pinsetting == 0) {  // Boost
        if (portusage->p[3]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->p[3];
        } else {
            portusage->p[3] = feature_num;
            *port = (unsigned char*)&PWMDTY3;
            PWMCLK |= 0x08; // PP3 now uses SB
            if ((portusage->pwmsclb[0] == 0xff) || (portusage->pwmsclb[0] == hwpwm)) { // 0xff means unused
                portusage->pwmsclb[0] = hwpwm;
                PWMSCLB = pwmopts[hwpwm];    // pwm clk = B clk/(2*SCLB)
                portusage->pwmsclb[1] = feature_num;
            } else {
                conf_err = 148;
                conf_err_feat = portusage->pwmsclb[1];
            }
            PWME |= 0x08;
            PWMPER3 = 255;
            PWMDTY3 = 0;
            if (polarity) {
                PWMPOL &= ~0x08;
            } else {
                PWMPOL |= 0x08;
            }
        }
    } else if (pinsetting == 1) {  // Idle
        if (portusage->p[2]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->p[2];
        } else {
            portusage->p[2] = feature_num;
            *port = (unsigned char*)&PWMDTY2;
            PWMCLK |= 0x04; // PP2 now uses SB
            if ((portusage->pwmsclb[0] == 0xff) || (portusage->pwmsclb[0] == hwpwm)) { // 0xff means unused
                portusage->pwmsclb[0] = hwpwm;
                PWMSCLB = pwmopts[hwpwm];    // pwm clk = B clk/(2*SCLB)
                portusage->pwmsclb[1] = feature_num;
            } else {
                conf_err = 148;
                conf_err_feat = portusage->pwmsclb[1];
            }
            PWME |= 0x04;
            PWMPER2 = 255;
            PWMDTY2 = 0;
            if (polarity) {
                PWMPOL &= ~0x04;
            } else {
                PWMPOL |= 0x04;
            }
        }
    } else if (pinsetting == 2) {  // VVT
        if (portusage->p[6]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->p[6];
        } else {
            portusage->p[6] = feature_num;
            *port = (unsigned char*)&PWMDTY6;
            PWMCLK |= 0x40; // PP6 now uses SB
            if ((portusage->pwmsclb[0] == 0xff) || (portusage->pwmsclb[0] == hwpwm)) { // 0xff means unused
                portusage->pwmsclb[0] = hwpwm;
                PWMSCLB = pwmopts[hwpwm];    // pwm clk = B clk/(2*SCLB)
                portusage->pwmsclb[1] = feature_num;
            } else {
                conf_err = 148;
                conf_err_feat = portusage->pwmsclb[1];
            }
            PWME |= 0x40;
            PWMPER6 = 255;
            PWMDTY6 = 0;
            if (polarity) {
                PWMPOL &= ~0x40;
            } else {
                PWMPOL |= 0x40;
            }
        }
    } else if (pinsetting == 3) {  // Nitrous1
        if (portusage->p[4]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->p[4];
        } else {
            portusage->p[4] = feature_num;
            *port = (unsigned char*)&PWMDTY4;
            PWMCLK |= 0x10; // PP4 now uses SA
            if ((portusage->pwmscla[0] == 0xff) || (portusage->pwmscla[0] == hwpwm)) { // 0xff means unused
                portusage->pwmscla[0] = hwpwm;
                PWMSCLA = pwmopts[hwpwm];    // pwm clk = A clk/(2*SCLA)
                portusage->pwmscla[1] = feature_num;
            } else {
                conf_err = 147;
                conf_err_feat = portusage->pwmscla[1];
            }
            PWME |= 0x10;
            PWMPER4 = 255;
            PWMDTY4 = 0;
            if (polarity) {
                PWMPOL &= ~0x10;
            } else {
                PWMPOL |= 0x10;
            }
        }
    } else if (pinsetting == 4) {  // Nitrous2
        if (portusage->p[5]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->p[5];
        } else {
            portusage->p[5] = feature_num;
            *port = (unsigned char*)&PWMDTY5;
            PWMCLK |= 0x20; // PP5 now uses SA
            if ((portusage->pwmscla[0] == 0xff) || (portusage->pwmscla[0] == hwpwm)) { // 0xff means unused
                portusage->pwmscla[0] = hwpwm;
                PWMSCLA = pwmopts[hwpwm];    // pwm clk = A clk/(2*SCLA)
                portusage->pwmscla[1] = feature_num;
            } else {
                conf_err = 147;
                conf_err_feat = portusage->pwmscla[1];
            }
            PWME |= 0x20;
            PWMPER5 = 255;
            PWMDTY5 = 0;
            if (polarity) {
                PWMPOL &= ~0x20;
            } else {
                PWMPOL |= 0x20;
            }
        }
    } else if (pinsetting == 5) {  // FIDLE
        if (portusage->p[7]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->p[7];
        } else {
            portusage->p[7] = feature_num;
            *port = (unsigned char*)&PWMDTY7;
            PWMCLK |= 0x80; // PP7 now uses SB
            if ((portusage->pwmsclb[0] == 0xff) || (portusage->pwmsclb[0] == hwpwm)) { // 0xff means unused
                portusage->pwmsclb[0] = hwpwm;
                PWMSCLB = pwmopts[hwpwm];    // pwm clk = B clk/(2*SCLB)
                portusage->pwmsclb[1] = feature_num;
            } else {
                conf_err = 148;
                conf_err_feat = portusage->pwmsclb[1];
            }
            PWME |= 0x80;
            PWMPER7 = 255;
            PWMDTY7 = 0;
            if (polarity) {
                PWMPOL &= ~0x80;
            } else {
                PWMPOL |= 0x80;
            }
        }
    } else if (pinsetting == 6) {  // Inj 1
        if (portusage->p[0]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->p[0];
        } else if (portusage->t[1]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->t[1];
        } else {
            portusage->p[0] = feature_num;
            portusage->t[1] = feature_num;
            *port = (unsigned char*)&PWMDTY0;
            PWMCLK |= 0x01; // PP0 now uses SA
            if ((portusage->pwmscla[0] == 0xff) || (portusage->pwmscla[0] == hwpwm)) { // 0xff means unused
                portusage->pwmscla[0] = hwpwm;
                PWMSCLA = pwmopts[hwpwm];    // pwm clk = A clk/(2*SCLA)
                portusage->pwmscla[1] = feature_num;
            } else {
                conf_err = 147;
                conf_err_feat = portusage->pwmscla[1];
            }
            PWME |= 0x01;
            PWMPER0 = 255;
            PWMDTY0 = 0;
            if (polarity) {
                PWMPOL &= ~0x01;
            } else {
                PWMPOL |= 0x01;
            }
            PTT |= 0x02;
        }
    } else if (pinsetting == 7) {  // Inj 2
        if (portusage->p[1]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->p[1];
        } else if (portusage->t[3]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->t[3];
        } else {
            portusage->p[1] = feature_num;
            portusage->t[3] = feature_num;
            *port = (unsigned char*)&PWMDTY1;
            PWMCLK |= 0x02; // PP1 now uses SA
            if ((portusage->pwmscla[0] == 0xff) || (portusage->pwmscla[0] == hwpwm)) { // 0xff means unused
                portusage->pwmscla[0] = hwpwm;
                PWMSCLA = pwmopts[hwpwm];    // pwm clk = A clk/(2*SCLA)
                portusage->pwmscla[1] = feature_num;
            } else {
                conf_err = 147;
                conf_err_feat = portusage->pwmscla[1];
            }
            PWME |= 0x02;
            PWMPER1 = 255;
            PWMDTY1 = 0;
            if (polarity) {
                PWMPOL &= ~0x02;
            } else {
                PWMPOL |= 0x02;
            }
            PTT |= 0x08;
        }
    } else {
        conf_err = feature_conferr;
	    conf_err_feat = 0;
    }
}

void generic_digout_setup(volatile unsigned char * volatile *port, unsigned char *pin,
                       unsigned char feature_conferr,
                      unsigned char pinsetting, unsigned char feature_num)
{
/* "Off", "IAC1", "IAC2", "FIDLE", "D15", "Nitrous 1", "Nitrous 2", "Tacho", "Idle", "Boost", "VVT", "Inj Bank 1", "Inj Bank 2", "D14", "D16", "JS11", "Inj A", "Inj B", "Inj C", "Inj D", "Inj E", "Inj F", "Inj G", "Inj H", "PT4", "IGN (JS10)", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "CANOUT1", "CANOUT2", "CANOUT3", "CANOUT4", "CANOUT5", "CANOUT6", "CANOUT7", "CANOUT8", "CANOUT9", "CANOUT10", "CANOUT11", "CANOUT12", "CANOUT13", "CANOUT14", "CANOUT15", "CANOUT16" */
    portpins *portusage;
    portusage = init_portusage_addr; // place on stack - only called from init

    if (pinsetting == 0) {   // JS10
        if (portusage->t[5]) {
            conf_err = feature_conferr;
	        conf_err_feat = portusage->t[5]; 
        } else {
            portusage->t[5] = feature_num;
            DDRT |= 0x20; // ensure port set as bit-bash output
            OCPD |= 0x20;
            *port = pPTT;
            *pin = 0x20;
        }
    } else if (pinsetting == 1) {     // on IAC1
        if (portusage->j[1]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->j[1];
        } else {
            portusage->j[1] = feature_num;
            DDRJ |= 0x02;
            *port = pPTJ;
            *pin = 0x02;
        }
    } else if (pinsetting == 2) {      // on IAC2
        if (portusage->j[0]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->j[0];
        } else {
            portusage->j[0] = feature_num;
            DDRJ |= 0x01;
            *port = pPTJ;
            *pin = 0x01;
        }
    } else if (pinsetting == 3) {      // on FIDLE
        if (portusage->p[7]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->p[7];
        } else {
            portusage->p[7] = feature_num;
            DDRP |= 0x80;
            PWME &= ~0x80;
            *port = pPTP;
            *pin = 0x80;
        }
    } else if (pinsetting == 4) {      // D15 Middle LED
        if (portusage->m[5]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->m[5];
        } else {
            portusage->m[5] = feature_num;
            DDRM |= 0x20;
            *port = pPTM;
            *pin = 0x20;
        }
    } else if (pinsetting == 5) {      // Nitrous1 on MS3X
        if (portusage->p[4]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->p[4];
        } else {
            portusage->p[4] = feature_num;
            DDRP |= 0x10;
            PWME &= ~0x10;
            *port = pPTP;
            *pin = 0x10;
        }
    } else if (pinsetting == 6) {      // Nitrous2 on MS3X
        if (portusage->p[5]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->p[5];
        } else {
            portusage->p[5] = feature_num;
            DDRP |= 0x20;
            PWME &= ~0x20;
            *port = pPTP;
            *pin = 0x20;
        }
    } else if (pinsetting == 7) {      // Tacho on MS3X
        if (portusage->k[0]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->k[0];
        } else {
            portusage->k[0] = feature_num;
            DDRK |= 0x01;
            *port = pPTK;
            *pin = 0x01;
        }
    } else if (pinsetting == 8) {      // Idle on MS3X
        if (portusage->p[2]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->p[2];
        } else {
            portusage->p[2] = feature_num;
            DDRP |= 0x04;
            PWME &= ~0x04;
            *port = pPTP;
            *pin = 0x04;
        }
    } else if (pinsetting == 9) {      // Boost on MS3X
        if (portusage->p[3]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->p[3];
        } else {
            portusage->p[3] = feature_num;
            DDRP |= 0x08;
            PWME &= ~0x08;
            *port = pPTP;
            *pin = 0x08;
        }
    } else if (pinsetting == 10) {     // VVT on MS3X
        if (portusage->p[6]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->p[6];
        } else {
            portusage->p[6] = feature_num;
            DDRP |= 0x40;
            PWME &= ~0x40;
            *port = pPTP;
            *pin = 0x40;
        }
    } else if (pinsetting == 11) {     // Inj bank 1. T1 + P0
        if (portusage->t[1]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[1];
        } else {
            portusage->t[1] = feature_num;
            DDRT |= 0x02;
            PWME &= ~0x01;
            OCPD |= 0x02;
            PTP |= 0x01;
            *port = pPTT;
            *pin = 0x02;
        }
    } else if (pinsetting == 12) {     // Inj bank 2. T3 + P1
        if (portusage->t[3]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[3];
        } else {
            portusage->t[3] = feature_num;
            DDRT |= 0x08;
            PWME &= ~0x02;
            OCPD |= 0x08;
            PTP |= 0x02;
            *port = pPTT;
            *pin = 0x08;
        }
    } else if (pinsetting == 13) {     // D14
        if (portusage->m[3]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->m[3];
        } else {
            portusage->m[3] = feature_num;
            DDRM |= 0x08;
            *port = pPTM;
            *pin = 0x08;
        }
    } else if (pinsetting == 14) {     // D16
        if (portusage->m[4]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->m[4];
        } else {
            portusage->m[4] = feature_num;
            DDRM |= 0x10;
            *port = pPTM;
            *pin = 0x10;
        }
    } else if (pinsetting == 15) {     // JS11
        if (portusage->j[7]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->j[7];
        } else {
            portusage->j[7] = feature_num;
            DDRJ |= 0x80;
            *port = pPTJ;
            *pin = 0x80;
        }
    } else if (pinsetting == 16) {     // InjA on MS3X
        if (portusage->a[0]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[0];
        } else {
            portusage->a[0] = feature_num;
            DDRA |= 0x01;
            *port = pPTA;
            *pin = 0x01;
        }
    } else if (pinsetting == 17) {     // InjB on MS3X
        if (portusage->a[1]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[1];
        } else {
            portusage->a[1] = feature_num;
            DDRA |= 0x02;
            *port = pPTA;
            *pin = 0x02;
        }
    } else if (pinsetting == 18) {     // InjC on MS3X
        if (portusage->a[2]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[2];
        } else {
            portusage->a[2] = feature_num;
            DDRA |= 0x04;
            *port = pPTA;
            *pin = 0x04;
        }
    } else if (pinsetting == 19) {     // InjD on MS3X
        if (portusage->a[3]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[3];
        } else {
            portusage->a[3] = feature_num;
            DDRA |= 0x08;
            *port = pPTA;
            *pin = 0x08;
        }
    } else if (pinsetting == 20) {     // InjE on MS3X
        if (portusage->a[4]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[4];
        } else {
            portusage->a[4] = feature_num;
            DDRA |= 0x10;
            *port = pPTA;
            *pin = 0x10;
        }
    } else if (pinsetting == 21) {     // InjF on MS3X
        if (portusage->a[5]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[5];
        } else {
            portusage->a[5] = feature_num;
            DDRA |= 0x20;
            *port = pPTA;
            *pin = 0x20;
        }
    } else if (pinsetting == 22) {     // InjG on MS3X
        if (portusage->a[6]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[6];
        } else {
            portusage->a[6] = feature_num;
            DDRA |= 0x40;
            *port = pPTA;
            *pin = 0x40;
        }
    } else if (pinsetting == 23) {     // InjH on MS3X
        if (portusage->a[7]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->a[7];
        } else {
            portusage->a[7] = feature_num;
            DDRA |= 0x80;
            *port = pPTA;
            *pin = 0x80;
        }
    } else if (pinsetting == 24) {     // T4
        if (portusage->t[4]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[4];
        } else {
            portusage->t[4] = feature_num;
            DDRT |= 0x10;
            OCPD |= 0x10;
            *port = pPTT;
            *pin = 0x10;
        }
    } else if (pinsetting == 25) {   // JS10
        if (portusage->t[5]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[5]; 
        } else {
            portusage->t[5] = feature_num;
            DDRT |= 0x20; // ensure port set as bit-bash output
            OCPD |= 0x20;
            *port = pPTT;
            *pin = 0x20;
        }
    } else if ((pinsetting >= 32) && (pinsetting < 40)) {     // CANPWMout1-8
        if (portusage->canpwm[pinsetting & 0x07]) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canpwm[pinsetting & 0x07];
        } else {
            portusage->canpwm[pinsetting & 0x07] = feature_num;
            *port = (volatile unsigned char *)&datax1.canpwmout[pinsetting & 0x07];
            *pin = 255; // magic number
        }
    } else if (pinsetting == 48) {   // CANout1
        if ((portusage->canout1[0]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[0];
        } else {
            portusage->canout1[0] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x01;
        }
    } else if (pinsetting == 49) {   // CANout2
        if ((portusage->canout1[1]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[1];
        } else {
            portusage->canout1[1] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x02;
        }
    } else if (pinsetting == 50) {   // CANout3
        if ((portusage->canout1[2]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[2];
        } else {
            portusage->canout1[2] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x04;
        }
    } else if (pinsetting == 51) {   // CANout4
        if ((portusage->canout1[3]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[3];
        } else {
            portusage->canout1[3] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x08;
        }
    } else if (pinsetting == 52) {   // CANout5
        if ((portusage->canout1[4]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[4];
        } else {
            portusage->canout1[4] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x10;
        }
    } else if (pinsetting == 53) {   // CANout6
        if ((portusage->canout1[5]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[5];
        } else {
            portusage->canout1[5] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x20;
        }
    } else if (pinsetting == 54) {   // CANout7
        if ((portusage->canout1[6]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[6];
        } else {
            portusage->canout1[6] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x40;
        }
    } else if (pinsetting == 55) {   // CANout8
        if ((portusage->canout1[7]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout1[7];
        } else {
            portusage->canout1[7] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout1_8;
            *pin = 0x80;
        }
    } else if (pinsetting == 56) {   // CANout9
        if ((portusage->canout2[0]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[0];
        } else {
            portusage->canout2[0] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x01;
        }
    } else if (pinsetting == 57) {   // CANout10
        if ((portusage->canout2[1]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[1];
        } else {
            portusage->canout2[1] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x02;
        }
    } else if (pinsetting == 58) {   // CANout11
        if ((portusage->canout2[2]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[2];
        } else {
            portusage->canout2[2] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x04;
        }
    } else if (pinsetting == 59) {   // CANout12
        if ((portusage->canout2[3]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[3];
        } else {
            portusage->canout2[3] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x08;
        }
    } else if (pinsetting == 60) {   // CANout13
        if ((portusage->canout2[4]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[4];
        } else {
            portusage->canout2[4] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x10;
        }
    } else if (pinsetting == 61) {   // CANout14
        if ((portusage->canout2[5]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[5];
        } else {
            portusage->canout2[5] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x20;
        }
    } else if (pinsetting == 62) {   // CANout15
        if ((portusage->canout2[6]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[6];
        } else {
            portusage->canout2[6] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x40;
        }
    } else if (pinsetting == 63) {   // CANout16
        if ((portusage->canout2[7]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canout2[7];
        } else {
            portusage->canout2[7] = feature_num;
            *port = (volatile unsigned char *)&outpc.canout9_16;
            *pin = 0x80;
        }
    }  
}

void generic_digin_setup(volatile unsigned char * volatile *port, unsigned char *pin,
                      unsigned char *pin_match, unsigned char feature_conferr,
                      unsigned char pinsetting, unsigned char feature_num)
{

    portpins *portusage;
    portusage = init_portusage_addr; // place on stack - only called from init

    if(pin_match) {
        *pin_match = 0; // default is active low
    }
// "Off", "Tableswitch", "PE0/JS7", "PE1", "JS10", "JS11", "JS5 (ADC6)", "JS4 (ADC7)", "Nitrous In", "Launch in", "Datalog In", "PT4 Spare in", "PT2 Cam in", "PE2 Flex", "INVALID", "INVALID"
    if (pinsetting == 1) {  // TableSwitch
        if ((portusage->h[6]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->h[6];
        } else {
            portusage->h[6] = feature_num;
            *port = pPTH;
            *pin = 0x40;
        }
    } else if (pinsetting == 2) {  // PE0/JS7
        if ((portusage->e[0]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->e[0];
        } else {
            portusage->e[0] = feature_num;
            *port = pPTE;
            *pin = 0x01;
        }
    } else if (pinsetting == 3) { // PE1
        if ((portusage->e[1]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->e[1];
        } else {
            portusage->e[1] = feature_num;
            *port = pPTE;
            *pin = 0x02;
        }
    } else if (pinsetting == 4) { // JS10
        if ((portusage->t[5]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[5];
        } else {
            portusage->t[5] = feature_num;
            DDRT &= ~0x20;          // set PTT5 as input
            OCPD |= 0x20;           // disconnect from logic
            PERT |= 0x20;
            PPST &= 0x20;           // pull up
            *port = pPTT;
            *pin = 0x20;
        }
    } else if (pinsetting == 5) { // JS11
        //JS11 was PA0 on MS2/Extra. Now connected to PJ7
        if ((portusage->j[7]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->j[7];
        } else {
            portusage->j[7] = feature_num;
            DDRJ &= ~0x80;  // configure as input
            PERJ |= 0x80;
            PPSJ &= ~0x80;
            *port = pPTJ;
            *pin = 0x80;
        }
    } else if (pinsetting == 6) { // JS5
        if ((portusage->ad0l[6]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0l[6];
        } else {
            portusage->ad0l[6] = feature_num;
            DDRAD0L &= ~0x40;       // ensure pin AD06 is input
            ATD0DIENL |= 0x40;
            PERAD0L |= 0x40;
            *port = pPTAD0L;
            *pin = 0x40;
        }
    } else if (pinsetting == 7) { // JS4
        if ((portusage->ad0l[7]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->ad0l[7];
        } else {
            portusage->ad0l[7] = feature_num;
            DDRAD0L &= ~0x80;       // ensure pin AD07 is input
            ATD0DIENL |= 0x80;
            PERAD0L |= 0x80;
            *port = pPTAD0L;
            *pin = 0x80;
        }
    } else if (pinsetting == 8) { // Nitrous In 
        if ((portusage->h[7]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->h[7];
        } else {
            portusage->h[7] = feature_num;
            DDRH &= ~0x80;  // configure as input
            if ((ram4.feature3 & 0x20) == 0) {
                PERH |= 0x80;
                PPSH |= 0x80;   // pull DOWN
            }
            *port = pPTH;
            *pin = 0x80;
            if(pin_match) {
                *pin_match = *pin; // nitrous is active high
            }
        }
    } else if (pinsetting == 9) {      //  PK2 Launch in
        if ((portusage->k[2]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->k[2];
        } else {
            portusage->k[2] = feature_num;
            DDRK &= ~0x04;
            *port = pPTK;
            *pin = 0x04;
        }
    } else if (pinsetting == 10) {  // Datalog In
        if ((portusage->t[6]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[6];
        } else {
            portusage->t[6] = feature_num;
            OCPD |= 0x40;
            DDRT &= ~0x40;
            *port = pPTT;
            *pin = 0x40;
#ifdef PULLUP_WITHOUT_MS3X
            PERT |= 0x40;
            PPST &= ~0x40;
#endif
        }
    } else if (pinsetting == 11) {  // PT4 In
        if ((portusage->t[4]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[4];
        } else {
            portusage->t[4] = feature_num;
            OCPD |= 0x10;
            DDRT &= ~0x10;
            *port = pPTT;
            *pin = 0x10;
            PERT |= 0x10;
            PPST &= ~0x10;
        }
    } else if (pinsetting == 12) {  // PT2 In
        if ((portusage->t[2]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->t[2];
        } else {
            portusage->t[2] = feature_num;
            OCPD |= 0x04;
            DDRT &= ~0x04;
            *port = pPTT;
            *pin = 0x04;
            PERT |= 0x04;
            PPST &= ~0x04;
        }
    } else if (pinsetting == 13) {  // PE2
        if ((portusage->e[2]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->e[2];
        } else {
            portusage->e[2] = feature_num;
            *port = &PORTE;
            *pin = 0x04;
        }
    /*
     *
     * options 14,15 not used
     *
     */
    } else if (pinsetting == 16) {  // CANIN1
        if ((portusage->canin[0]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[0];
        } else {
            portusage->canin[0] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x01;
        }
    } else if (pinsetting == 17) {  // CANIN2
        if ((portusage->canin[1]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[1];
        } else {
            portusage->canin[1] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x02;
        }
    } else if (pinsetting == 18) {  // CANIN3
        if ((portusage->canin[2]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[2];
        } else {
            portusage->canin[2] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x04;
        }
    } else if (pinsetting == 19) {  // CANIN4
        if ((portusage->canin[3]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[3];
        } else {
            portusage->canin[3] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x08;
        }
    } else if (pinsetting == 20) {  // CANIN5
        if ((portusage->canin[4]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[4];
        } else {
            portusage->canin[4] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x10;
        }
    } else if (pinsetting == 21) {  // CANIN6
        if ((portusage->canin[5]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[5];
        } else {
            portusage->canin[5] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x20;
        }
    } else if (pinsetting == 22) {  // CANIN7
        if ((portusage->canin[6]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[6];
        } else {
            portusage->canin[6] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x40;
        }
    } else if (pinsetting == 23) {  // CANIN8
        if ((portusage->canin[7]) && (feature_conferr)) {
            conf_err = feature_conferr;
			conf_err_feat = portusage->canin[7];
        } else {
            portusage->canin[7] = feature_num;
            *port = (volatile unsigned char *)&outpc.canin1_8;
            *pin = 0x80;
        }
    }
}

void generic_timer_setup(unsigned char feature_conferr, unsigned char pinsetting, unsigned char pol, unsigned char feature_num)
{
    portpins *portusage;
    portusage = init_portusage_addr; // place on stack - only called from init

    if (pinsetting == 0) {  // MS3X PT4 spare input
        if (portusage->t[4]) {
            conf_err = feature_conferr;
            conf_err_feat = portusage->t[4];
        } else {
            portusage->t[4] = feature_num;
            OCPD |= 0x10;
            TCTL1 &= ~0x03;     // not OC
            DDRT &= ~0x10;      // ensure PT4 = input
            TIOS &= ~0x10;      // set as input capture
            TCTL3 = TCTL3 & 0xfc;
            if (pol & 1) {
                TCTL3 |= 0x01;
            }
            if (pol & 2) {
                TCTL3 |= 0x02;
            }
            TFLG1 = 0x10;
            TIE |= 0x10;
        }
    } else if (pinsetting == 1) {  // MS3X PT2 cam input
        if (portusage->t[2]) {
            conf_err = feature_conferr;
            conf_err_feat = portusage->t[2];
        } else {
            portusage->t[2] = feature_num;
            OCPD |= 0x04;
            TCTL2 &= ~0x30;     // not OC
            DDRT &= ~0x04;      // ensure PT2 = input
            TIOS &= ~0x04;      // set as input capture
            TCTL4 = TCTL4 & 0xcf;
            if (pol & 1) {
                TCTL4 |= 0x10;
            }
            if (pol & 2) {
                TCTL4 |= 0x20;
            }
            TFLG1 = 0x04;
            TIE |= 0x04;
        }
    } else if (pinsetting == 2) {  // JS10 PT5 cam input
        if (portusage->t[5]) {
            conf_err = feature_conferr;
            conf_err_feat = portusage->t[5];
        } else {
            portusage->t[5] = feature_num;
            OCPD |= 0x20;
            TCTL1 &= ~0x0c;     // not OC
            DDRT &= ~0x20;      // ensure PT5 = input
            TIOS &= ~0x20;      // set as input capture
            TCTL3 = TCTL3 & 0xf3;
            if (pol & 1) {
                TCTL3 |= 0x04;
            }
            if (pol & 2) {
                TCTL3 |= 0x08;
            }
            TFLG1 = 0x20;
            TIE |= 0x20;
        }
    } else if (pinsetting == 3) {  // Datalog PT6 cam input
        if (portusage->t[6]) {
            conf_err = feature_conferr;
            conf_err_feat = portusage->t[6];
        } else {
            if (ram4.EngStroke & 0x02) {
                conf_err = feature_conferr;
                conf_err_feat = 109;
            } else {
                portusage->t[6] = feature_num;
                /* set up input capture with same polarity as crank */
                OCPD |= 0x40;
                TCTL1 &= ~0x30;     // not OC
                DDRT &= ~0x40;      // ensure PT6 = input
                TIOS &= ~0x40;      // set as input capture
                TCTL3 = TCTL3 & 0xcf;
                if (pol & 1) {
                    TCTL3 |= 0x10;
                }
                if (pol & 2) {
                    TCTL3 |= 0x20;
                }
                TFLG1 = 0x40;
                TIE |= 0x40;
            }
        }
    }
}

void check_tsw(unsigned char opt, unsigned char err)
{
    portpins *portusage;
    portusage = init_portusage_addr; // place on stack - only called from init

    if ((opt == 0x01) && portusage->h[6]) {
        conf_err_feat = portusage->h[6];
        conf_err = err;
    } else if ((opt == 0x02) && portusage->e[0]) {
        conf_err_feat = portusage->e[0];
        conf_err = err;
    } else if ((opt == 0x03) && portusage->e[1]) {
        conf_err_feat = portusage->e[1];
        conf_err = err;
    } else if ((opt == 0x04) && portusage->t[4]) {
        conf_err_feat = portusage->t[4];
        conf_err = err;
    } else if ((opt == 0x05) && portusage->j[7]) {
        conf_err_feat = portusage->j[7];
        conf_err = err;
    } else if ((opt == 0x06) && portusage->ad0l[6]) {
        conf_err_feat = portusage->ad0l[6];
        conf_err = err;
    } else if ((opt == 0x07) && portusage->ad0l[7]) {
        conf_err_feat = portusage->ad0l[7];
        conf_err = err;
    } else if ((opt == 0x08) && portusage->h[7]) {
        conf_err_feat = portusage->h[7];
        conf_err = err;
    } else if ((opt == 0x09) && portusage->k[2]) {
        conf_err_feat = portusage->k[2];
        conf_err = err;
    } else if ((opt == 0x0a) && portusage->t[6]) {
        conf_err_feat = portusage->t[6];
        conf_err = err;
    } else if ((opt == 0x0b) && portusage->t[4]) {
        conf_err_feat = portusage->t[4];
        conf_err = err;
    } else if ((opt == 0x0c) && portusage->t[2]) {
        conf_err_feat = portusage->t[2];
        conf_err = err;
    } else if ((opt == 0x0d) && portusage->e[2]) {
        conf_err_feat = portusage->e[2];
        conf_err = err;
    }
}


//#define PULLUP_WITHOUT_MS3X
void main_init(void)
{
    int ix;

    unsigned char port_inits[50], uc1;
    unsigned char idlectl_tmp;

    portpins portusage;
    init_portusage_addr = &portusage; // store pointer to local array, allows disposal

    tooth_init = 0;

// the PORTE settings here should already have been done in the monitor - do it here to get the FP off
    PUCR |= 0xFC;               // enable pullups for ports K,E,D,C. Not A&B
    ECLKCTL = 0xc0;             // disable ECLK and ECLKX2 which are enabled by default
    IRQCR &= ~0x40;             // remove External IRQ pins from interrupt logic

    port_fp = &PORTE;
    pin_fp = 0x10;
    DDRE = 0x10;                // PE4 output, others inputs

    *port_fp &= ~pin_fp;    // turn off FP until code decides to turn it on

    // initalize PLL - reset default is Oscillator clock
    // 4 MHz oscillator, PLL freq = 100 MHz, 50 MHz bus,
    //  divide by 16 for timer of 2/3 usec tic
    PLLCTL &= 0xBF;             // Turn off PLL so can change freq
    SYNR = 0xD8;                // set PLL/ Bus freq to 100/ 50 MHz
    REFDV = 0x03;               // use 0x41 for 4MHz crystal
    POSTDIV = 0x00;
    PLLCTL |= 0x40;             // Turn on PLL
    // wait for PLL lock
    while (!(CRGFLG & 0x08));
    CLKSEL = 0x80;              // select PLL as clock

    // wait for clock transition to finish
    for (ix = 0; ix < 120; ix++);

    debug_init();

    // set initial pin states
    EPAGE = 0x1f;
// 0x800 not used due to typo in monitor code.
// other ports set by the monitor on boot before MS3 app runs
    if (MONVER < 0x305) {
        DDRAD0H = *(unsigned char *) 0x80f;
        DDRAD0L = *(unsigned char *) 0x810;
    }
/* port allocation is as follows:
0x800 DDRA (typo error, preserved for backwards compatability. Do not use.)
0x801 DDRA
0x802 DDRB
0x803 DDRE
0x804 DDRH
0x805 DDRJ
0x806 DDRK
0x807 ATD0DIENH
0x808 ATD0DIENL
0x809 DDRM
0x80a DDRP
0x80b DDRS
0x80c DDRT
0x80d OCPD
0x80e PWME
0x80f DDRAD0H (ver 0305+)
0x810 DDRAD0L (ver 0305+)
0x811 PORTA
0x812 PORTB
0x813 PORTE
0x814 PTH
0x815 PTJ
0x816 PORTK
0x817 PTAD0H
0x818 PTAD0L
0x819 PTM
0x81a PTP
0x81b PTS
0x81c PTT
0x81d unassigned
0x81e unassigned
0x81f unassigned
0x820 zero
0x821 zero
0x822-0x827 spare
*/
    // set up i/o ports - most of these listed just here are wrong FIXME
    //    - port M2 is fast idle solenoid
    //    - port M3 is inj led
    //    - port M4 is accel led
    //    - port M5 is warmup led
    //    - port E0 is flex fuel sensor input
    //    - port E4 is fuel pump
    //    - port P5 is bootload pin (input)
    //    - port J0 is IAC Coil A
    //    - port J1 is IAC Coil B
    //    - port J6 is IAC Enable
    //    - port J7 is JS11
    //    - port AD6 can be spark 5 (JS5)
    //    - port AD7 can be spark 6 (JS4)

//The commented DDRs are set above. Default data is in ms3_main_decls.h
//    DDRA = 0xff;                // all outputs - port A - inj outputs
//    DDRB = 0xff;                // port B - spk outputs

    DDRC = 0;                   // non bonded input
    DDRD = 0;                   // non bonded input

    RDRIV = 0;                  // full strength
//    DDRH = 0x08;                // SD card. inputs PH0-5, except PH3/SS as output.  Spares PH6,7 set as input
    PERH = 0x30;                // pulls on PH4,5
    PPSH = 0x00;                // pullUP devices
//    DDRJ = 0x43;                // bits 0,1,6 are outputs
    PERJ = 0x80;                // enable pullup resistance for port J7 knock input
//    DDRK = 0x01;                // bits 0 output only.
//    DDRM = 0x3c;                // port M - LEDs outputs, full drive by default and comms port
//    DDRP = 0xff;                // all outputs
    PERP = 0x00;                // no pulls
//    DDRS = 0x00;                // inputs or not connected
    PERS = 0xff;                // enable all pullups
//    DDRT = 0x7a;                // port T7,2,0 = inputs; T6,5,4,3,1 = outputs   - ECT pins
//    OCPD = 0x80;                // default connecting timer to pins, except PT7 which is shared with AN7
//    ATD0DIEN = 0;               // Initially all inputs are analogue (PAD15:0)

    conf_err = 0;               // no config errors yet

    // open flash programming capability
    Flash_Init();

    TCTL1 = 0;
    TCTL2 = 0;
    TCTL3 = 0;
    TCTL4 = 0;
    TIOS = 0;
    OCPD = 0xff; // no output compares unless specifically enabled

//disable EEPROM
    FCCOBIX = 0;
    FCCOBHI = 0x14;
    FCCOBLO = 0;
    FSTAT = 0x80;              // launch flash command to disable EEPROM

//copy flash pages into RAM
    cp_flash_ram();

    flagbyte0 = 0;              // must do these before ign_reset as it sets various bits
    flagbyte1 = 0;
    flagbyte2 = 0;
    flagbyte3 = 0;
    flagbyte4 = 0;
    flagbyte5 = 0;
    flagbyte6 = 0;
    flagbyte7 = 0;
    flagbyte8 = 0;
    flagbyte9 = 0;
    flagbyte10 = 0;
    flagbyte11 = 0;
    flagbyte12 = 0;
    flagbyte13 = 0;
    flagbyte14 = 0;
    flagbyte15 = 0;
    flagbyte16 = 0;
    flagbyte17 = 0;

    // sets baud to user settable baud rate
    // restrict to sensible rates to prevent data corruption causing a problem
    if ((ram4.baud == 9600) || (ram4.baud == 19200) || (ram4.baud == 38400) || (ram4.baud == 57600)) {
        // SCI baud rate = SCI bus clock / (16 x SBR[12:0])
        // SBR[12:0] = SCI bus clock / (16 x SCI baud rate )
        *(volatile unsigned int*)&SCI0BDH = (unsigned int) (50000000 / (16L * ram4.baud));
    } else {
        // sets baud to 115200. Default / fallback
        SCI0BDH = 0;
        SCI0BDL = 27;
    }
    SCI0CR1 = 0x00;
    SCI0CR2 = 0x24;             // TIE=0,RIE = 1; TE=0,RE =1

    if ((MONVER >= 0x380) && (MONVER <= 0x38f)) { // two serial ports
        /* now set SCI1 the same */
        *(volatile unsigned int*)&SCI1BDH = *(volatile unsigned int*)&SCI0BDH;
        SCI1CR1 = 0x00;
        SCI1CR2 = 0x24;             // TIE=0,RIE = 1; TE=0,RE =1
        port_sci = NULL;
    } else {
        port_sci = &SCI0BDH;
        flagbyte2 |= FLAGBYTE2_SCI_CONFIRMED;
    }
    txcnt = 0;
    rxcnt = 0;
    rxmode = 0;
    txmode = 0;
    txgoal = 0;
    rcv_timeout = 0xFFFFFFFF;
    sci_lock_timer = 0;

    if (conf_err) {
        goto SKIP_INIT;         // if flash problem do not try anything else
    }

    burnstat = 0;

    outpc.status1 = 0;
    outpc.status2 = 0;
    outpc.status3 = 0;
    outpc.status4 = 0;
    outpc.istatus5 = 0;
    outpc.status6 = 0;
    outpc.status7 = 0;
    tim_mask = 0;
    tim_mask_run = 0x01; // main tach

    smallest_tooth_crk = 0;
    smallest_tooth_cam = 0;
    false_mask_crk = 0;
    false_mask_cam = 0;
    false_period_crk_tix = 0;
    false_period_cam_tix = 0;
    mapadc_thresh = 0;

    memcpy(port_inits, (unsigned char *) default_port_inits, 40);
#define port_inits_DDRA port_inits[0x01]
#define port_inits_DDRB port_inits[0x02]
#define port_inits_DDRE port_inits[0x03]
#define port_inits_DDRH port_inits[0x04]
#define port_inits_DDRJ port_inits[0x05]
#define port_inits_DDRK port_inits[0x06]
#define port_inits_ATD0DIENH port_inits[0x07]
#define port_inits_ATD0DIENL port_inits[0x08]
#define port_inits_DDRM port_inits[0x09]
#define port_inits_DDRP port_inits[0x0a]
#define port_inits_DDRS port_inits[0x0b]
#define port_inits_DDRT port_inits[0x0c]
#define port_inits_OCPD port_inits[0x0d]
#define port_inits_PWME port_inits[0x0e]
#define port_inits_DDRAD0H port_inits[0x0f]
#define port_inits_DDRAD0L port_inits[0x10]
#define port_inits_PORTA port_inits[0x11]
#define port_inits_PORTB port_inits[0x12]
#define port_inits_PORTE port_inits[0x13]
#define port_inits_PTH port_inits[0x14]
#define port_inits_PTJ port_inits[0x15]
#define port_inits_PORTK port_inits[0x16]
#define port_inits_PTAD0H port_inits[0x17]
#define port_inits_PTAD0L port_inits[0x18]
#define port_inits_PTM port_inits[0x19]
#define port_inits_PTP port_inits[0x1a]
#define port_inits_PTS port_inits[0x1b]
#define port_inits_PTT port_inits[0x1c]

    // start with port values as zero
    // want to get ignition outputs right from the earliest point
    port_inits_DDRA = 0xff;
    port_inits_PORTA = 0x00;
    port_inits_DDRB = 0xff;
    if ((MONVER >= 0x380) && (MONVER <= 0x38f)) {
        port_inits_PORTB = 0xff; // flipped
    } else {
        port_inits_PORTB = 0x00;
    }
//    port_inits_DDRT = 0xff;
    port_inits_DDRM = 0x3c;
    port_inits_DDRT = 0x0a;
    port_inits_PTM = 0x00;
    port_inits_PTT = 0x00;
    port_inits_PTAD0L = 0x00;

    spkmode = ram4.spk_mode & 0x3f; // by setting here prevents spurious changes while running and allows mask here

    // need to do this here to set the various flagbyte5 bits
    ign_wheel_init();
    calc_absangs();
    flash_adv_offset = ram4.adv_offset; // save these power on values for "trigger wizard"
    flash_Miss_ang = ram4.Miss_ang;

    if (((ram4.EngStroke & 0x03) == 0x03) && (cycle_deg == 7200)) {
        conf_err = 44;      // rotary doesn't need 720deg i.e. cam speed reset
    }

    if (((ram4.EngStroke & 0x03) == 0x03) && (num_cyl > 2) &&
        ((ram4.hardware & 0x7) != 0x2)) {
        conf_err = 122;
    }
       

    if (no_triggers > NUM_TRIGS) {
        conf_err = 75;
	}

// SD+SPI card setup
    MODRR |= 0x20;              // re-route SPI1 to PTH0-3. SPI setup when we need it.

    // FIXME re-write using memset and sizeof(struct)
    //  Set pointers to real port addresses - used only for clash checking
    for (ix = 0; ix < 8; ix++) {
        portusage.a[ix] = 0;
        portusage.b[ix] = 0;
        portusage.e[ix] = 0;
        portusage.h[ix] = 0;
        portusage.j[ix] = 0;
        portusage.k[ix] = 0;
        portusage.m[ix] = 0;
        portusage.p[ix] = 0;
        portusage.t[ix] = 0;
        portusage.ad0h[ix] = 0;
        portusage.ad0l[ix] = 0;
        portusage.canin[ix] = 0;
        portusage.canout1[ix] = 0;
        portusage.canout2[ix] = 0;
        portusage.canpwm[ix] = 0;
    }
    for (ix = 0; ix < 24; ix++) {
        portusage.canadc[ix] = 0; // only for config error checking
    }
    portusage.pwmscla[0] = 0xff;
    portusage.pwmsclb[0] = 0xff;

    // assign these pointers as dummies unless specifically used later
    pPTJpin0 = &dummyReg;
    pPTJpin1 = &dummyReg;
    pPTJpin6 = &dummyReg;
    pPTMpin3 = &dummyReg;
    pPTMpin4 = &dummyReg;
    pPTMpin5 = &dummyReg;

    pinport_init();

    //  Initialize timers:
    // Megasquirt timer uses
    //   Note: Inj outputs direct drive so inverse of Megasquirt
    //    -when turn on inj1 i/o, also enable pwm1 @ 100 % duty &
    //      set pwm1 timer = 0 (.1 ms units).
    //    -When pwm1 timer = InjPWMTim, set duty cycle to InjPWMDty.
    //    -when done injecting, turn off inj1 i/o and pwm1.
    //   TC0: Input capture (tach) - no pullup (default) for MSII
    //   TC1: Timer for bit bash spark output
    //   TC2: Cam input if MS3X or additional cam input.
    //   TC3: Rotary Spark
    //   TC4: Free or additional cam input.
    //   TC5: Cam input if JS10 or additional cam input.
    //   TC6: Rotary dwell timer or additional cam input.
    //   TC7: Dwell timer

    TSCR1 = 0x00;               // Timer off
    DLYCT = ram4.dlyct;         // delay on inputs for noise filtering.
    TSCR2 = 0x84;               // enable overflow int
    TIOS |= 0xFE;               // Timer ch 0 = IC, ch 1-5 = OC & PWM, will get changed if needed
    // ch 6,7 = I/O output
    // Set prescaler to 32.
    // low iaw OL1,3,5 (not toggle or disable)
//    TCTL2 |= 0x80;              // bit OM1,3,5 = 1. OC output line high or //FIXME what's this for?
    // Only set up for OC directly controlling output pin when not in wasted spk
    // mode.

    PTPSR = 49;
    if (ram4.feature3 & 0x10) {
        TIMPTPSR = 199; // 4X pulsewidth
        if (MONVER < 0x307) {
            conf_err = 110; // this monitor doesn't support changing the clock
        }
    } else {
        TIMPTPSR = 49; // normal
    }


    OC7M = 0;                   // don't want to use OC7 feature
//    MODRR = 0x14;     // Make Port T pins 2,4 be PWM
    PWME = 0;                   // disable pwms initially
    PWMPOL = 0x03;              // polarity = 1, => go hi when start
    PWMCLK = 0x00;              // Intially no scaled clocks
    PWMPRCLK = 0x55;            // B clock = bus/32 = 1.56MHz : A clock = bus/32 = 1.56 MHz 
    PWMCTL = 0x00;              // don't concatenate PWM channels
    PWMCAE = 0x00;              // standard left align pulse: -----
    //                           |     |______
    //                             duty
    //                           <---period--->
    PWMPER0 = ram4.InjPWMPd;    // set PWM period (us)
    PWMDTY0 = (int) (ram4.InjPWMDty * ram4.InjPWMPd) / 100;     // set PWM duty
    PWMCNT0 = 0x00;

    PWMPER1 = ram4.InjPWMPd2;   // set PWM period (us)
    PWMDTY1 = (int) (ram4.InjPWMDty * ram4.InjPWMPd2) / 100;    // set PWM duty
    PWMCNT1 = 0x00;
    // only enable PWMs when ready

    //calculate number of spark outputs once at startup and store in ram
    if ((spkmode & 0xfe) == 2) {  // basic trigger, trigger return
//        if (ram4.ICIgnOption & 0x8) {
//            num_spk = 2;        // oddfire - FIXME check!
//        } else {
            num_spk = 1;
//        }
    } else if (spkmode < 2) { // EDIS
        num_spk = 1;
    } else if (spkmode == 31) { // Fuel only
        num_spk = 0;
    } else if (spkmode == 14) { // Twin trigger
        num_spk = 2;
    } else {
        //How many spark outputs? Check for special cases first.
        if ((ram4.no_cyl & 0x1f) == 1) {
            num_spk = 1;

        } else if ((ram4.no_cyl & 0x1f) == 2) {
            if (ram4.EngStroke & 1) { // 2 stroke
                if (((ram4.spk_mode3 & 0xc0) != 0) || (ram4.ICIgnOption & 0x08)) {
                    num_spk = 2; // fires every 180deg or odd
                } else {
                    num_spk = 1; // fires every 360deg
                }
            } else { // 4 stroke
                if ((ram4.ICIgnOption & 8) == 0) { // even
                    if ((ram4.spk_mode3 & 0xc0) == 0x80) { // COP
                        num_spk = 2;
                    } else {
                        num_spk = 1; // will be wasted
                    }
                } else { // odd
                    num_spk = 2; // always two outputs whatever the user said
                }
            }

        } else if ((ram4.no_cyl & 0x1f) == 3) {
/*  why ?
            if (ram4.EngStroke & 1) { // 2 stroke
                num_spk = 1;
            } else*/ {
                num_spk = 3;
            }

        } else { // regular engines
            switch (ram4.spk_mode3 & 0xc0) {
            case 0x40:
                if (ram4.spk_mode3 & 0x20) {
                    num_spk = num_cyl; // wasted COP
                } else {
                    num_spk = num_cyl >> 1;
                }
                break;
            case 0x80:
                num_spk = num_cyl;
                break;
            case 0xc0:
                num_spk = 2;
                break;
            case 0:
            default:
                num_spk = 1;
                break;
            }
        }
    }

    // check for illegal spark output modes
    if ((spkmode != 31) && ((ram4.no_cyl & 0x1f) > 2) && (ram4.no_cyl & 0x1)   // not fuel-only, > 2 and odd no. cylinders
       && ((ram4.spk_mode3 & 0xe0) != 0) && ((ram4.spk_mode3 & 0xe0) != 0x80) ) {
        conf_err = 107;
    }

    if ((spkmode < 4) && (ram4.spk_mode3 & 0xe0) && (num_cyl > 3) ) {
        conf_err = 108;
    }

    if (ram4.ICIgnOption & 0x10) {      // "inverted" spark = 0
        uc1 = 0xff;
        if ((MONVER >= 0x380) && (MONVER <= 0x38f)) {
            flagbyte10 &= ~0x10;    // flipped for these monitors
        } else {
            flagbyte10 |= 0x10;     // matching normally
        }
    } else {
        uc1 = 0x00;
        if ((MONVER >= 0x380) && (MONVER <= 0x38f)) {
            flagbyte10 |= 0x10;    // flipped for these monitors
        } else {
            flagbyte10 &= ~0x10;     // matching normally
        }
    }

    if ((ram4.dwellmode & 3) == 2) {  // time after spark is opposite of expected
        uc1 = uc1 ^ 0xff;
    }

    if (((ram4.EngStroke & 0x03) == 0x03)) {
        num_spk <<= 1;
        if (num_cyl == 2) {
            if (ram4.spk_mode3 & 0x40) {  // Rotary wasted spark means FC/FD leading
                num_spk = 3;
            } 
        }
    } else if (ram4.spk_conf2 & 0x80) { // toyota DLI
        if (((ram4.spk_mode3 & 0xd0) != 0x40) || ((ram4.ICIgnOption & 0x8) == 0x8)) {
            /* not wasted spark or is oddfire */
            conf_err = 128;
        }
        if (num_cyl == 4) {
            flagbyte11 |= FLAGBYTE11_DLI4;
        } else if (num_cyl == 6) {
            flagbyte11 |= FLAGBYTE11_DLI6;
        } else {
            conf_err = 128;
        }
    }

    port_spki = &PTM;
    port_spkj = &PTM;
    pin_spki = 0x08;
    pin_spkj = 0x10;

    if (ram4.hardware & HARDWARE_MS3XSPK) {
        // using MS3X for spark
        if (num_spk) {
        portusage.b[0] = 10; //PB0
        port_inits_PORTB |= (uc1 ^ 0x01) & 0x01;
        DDRB |= 0x01;
        }
        if ((num_spk >= 2) || ((spkmode == 2) && (ram4.spk_conf2 & 1))) {
            // >=2 coils or HEI bypass
            portusage.b[1] = 10;     //PB1
            port_inits_PORTB |= (uc1 ^ 0x02) & 0x02;
            DDRB |= 0x02;
        }
        if (num_spk >= 3) {
            portusage.b[2] = 10;     //PB2
            port_inits_PORTB |= (uc1 ^ 0x04) & 0x04;
            DDRB |= 0x04;
        }
        if (num_spk >= 4) {
            portusage.b[3] = 10;     //PB3
            port_inits_PORTB |= (uc1 ^ 0x08) & 0x08;
            DDRB |= 0x08;
        }
        if (num_spk >= 5) {
            portusage.b[4] = 10;     //PB4
            port_inits_PORTB |= (uc1 ^ 0x10) & 0x10;
            DDRB |= 0x10;
        }
        if (num_spk >= 6) {
            portusage.b[5] = 10;     //PB5
            port_inits_PORTB |= (uc1 ^ 0x20) & 0x20;
            DDRB |= 0x20;
        }
        if (num_spk >= 7) {
            portusage.b[6] = 10;     //PB6
            port_inits_PORTB |= (uc1 ^ 0x40) & 0x40;
            DDRB |= 0x40;
        }
        if (num_spk >= 8) {
            portusage.b[7] = 10;     //PB7
            port_inits_PORTB |= (uc1 ^ 0x80) & 0x80;
            DDRB |= 0x80;
        }
        if (num_spk >= 9) {
            if ((MONVER >= 0x390) && (MONVER <= 0x39f)) {
                portusage.k[4] = 10;    //PK4
                port_inits_PORTK |= (uc1 ^ 0x10) & 0x10;
                DDRK |= 0x10;
                port_spki = &PORTK;
                pin_spki = 0x10;
            } else {
                portusage.m[3] = 10;    //D14
                port_inits_PTM |= (uc1 ^ 0x08) & 0x08;
                DDRM |= 0x08;
                port_spki = &PTM;
                pin_spki = 0x08;
            }
        }
        if (num_spk >= 10) {
            if ((MONVER >= 0x390) && (MONVER <= 0x39f)) {
                portusage.k[5] = 10;    //PK5
                port_inits_PORTK |= (uc1 ^ 0x20) & 0x20;
                DDRK |= 0x20;
                port_spkj = &PORTK;
                pin_spkj = 0x20;
            } else {
                portusage.m[4] = 10;    //D16
                port_inits_PTM |= (uc1 ^ 0x10) & 0x10;
                DDRM |= 0x10;
                port_spkj = &PTM;
                pin_spkj = 0x10;
            }
        }
        if (num_spk >= 11) {
            portusage.m[5] = 10;    //D15
            port_inits_PTM |= (uc1 ^ 0x20) & 0x20;
            DDRM |= 0x20;
        }
        if (num_spk >= 12) {
            portusage.j[7] = 10;     //PJ7
            DDRJ |= 0x80;       // output
            port_inits_PTJ |= (uc1 ^ 0x80) & 0x80;
        }
    } else {
        // upgrade mode
        if (num_spk) {
            if ((ram4.hardware & 0x05) == 0x01) {
                portusage.m[3] = 10;    //D14
                port_inits_PTM |= (uc1 ^ 0x08) & 0x08;
                port_spki = &PTM;
                pin_spki = 0x08;
            } else if ((ram4.hardware & 0x05) == 0x00) {
                portusage.t[5] = 10;     //JS10
                port_inits_PTT |= (uc1 ^ 0x20) & 0x20;
                port_inits_DDRT |= 0x20;
                DDRT |= 0x20; // ensure port set as bit-bash output
                OCPD |= 0x20;
                port_spki = &PTT;
                pin_spki = 0x20;
            } else { // PK0 Tacho (for MSD white wire)
                portusage.k[0] = 10;    //PK0
                port_inits_PORTK |= (uc1 ^ 0x01) & 0x01;
                port_inits_DDRK |= 0x01;
                DDRK |= 0x01; // ensure port set as output
                port_spki = &PORTK;
                pin_spki = 0x01;
            }
        }

        if (num_spk >= 2) {
            portusage.m[4] = 10;    //D16
            port_inits_PTM |= (uc1 ^ 0x10) & 0x10;
            port_spkj = &PTM;
            pin_spkj = 0x10;
        }

        if (num_spk >= 3) {
            portusage.m[5] = 10;    //D15
            port_inits_PTM |= (uc1 ^ 0x20) & 0x20;
        }

        if (num_spk >= 4) {
            portusage.j[7] = 10;     //PJ7
            DDRJ |= 0x80;       // output
            port_inits_PTJ |= (uc1 ^ 0x80) & 0x80;
        }

        if ((spkmode == 2) && (ram4.spk_conf2 & 1)) {
            // Grab end LED D16 as HEI bypass
            portusage.m[4] = 11;
        }

        if (num_spk >= 5) {
            DDRAD0L |= 0x40;    // enable pin AD06 as digital output
            portusage.ad0l[6] = 10;
            port_inits_PTAD0L |= (uc1 & 0x40) & 0x40;
        }

        if (num_spk >= 6) {
            DDRAD0L |= 0x80;    // enable pin AD07 as digital output
            portusage.ad0l[7] = 10;
            port_inits_PTAD0L |= (uc1 & 0x80) & 0x80;
        }

        if (num_spk > 6) {
            conf_err = 37;      // too many spark outputs
        }
    }

    do_dualouts = 0;

    if ((ram4.staged & 0x7) || ((ram5.dualfuel_sw & 0x01) && (ram5.dualfuel_opt & 0x8)) ||
       ((ram5.dualfuel_sw & 0x1) && (ram5.dualfuel_opt & 0x4))) {
        do_dualouts = 1;
        if ((ram4.no_cyl > 4) && !(ram4.staged_extended_opts & STAGED_EXTENDED_USE_V3)) {
            if ((num_cyl != 6) && (num_cyl != 8) && (!(ram4.sequential & SEQ_SEMI) && (!(ram4.hardware & HARDWARE_MS3XFUEL)))) {
                // do permit 6, 8 cyl if SEMI on MS3X using 2 inj per output, otherwise config error
                conf_err = 109;
            }
        }
    }

    calc_reqfuel(ram4.ReqFuel); // call now as sets up injector variables

    num_inj_pri = 0; // no. primary injectors
    // reserve the port/pins we need to use
    if (!(ram4.hardware & HARDWARE_MS3XFUEL)) {
        // reserve V3 injectors
        portusage.t[1] = 7;
        num_inj_pri = 1;
        if ((ram4.no_cyl & 0x1f) > 1) {
            portusage.t[3] = 7;
            num_inj_pri = 2;
        }
    } else { // MS3X injectors
        int i;
        if (ram4.sequential & 0x3) {
            num_inj_pri = num_inj;
        } else {
            num_inj_pri = num_cyl;
        }
        for (i = 0 ; i < num_inj_pri ; i++) { // FIXME this looks wrong for stage cases
            if (i < 8) {
                portusage.a[i] = 7;
            } else if (i == 8) {
                if ((ram4.hardware & 0x20) 
                    && (!((MONVER >= 0x380) && (MONVER <= 0x3ff)))) { // only on MS3
                    portusage.k[3] = 7;
                    DDRK |= 0x08;
                } else {
                    portusage.t[1] = 7;
                    DDRT |= 0x02;
                    PWME &= ~0x01;
                    OCPD |= 0x02;
                    PTP |= 0x01;
                }
            } else if (i == 9) {
                if ((ram4.hardware & 0x20) 
                    && (!((MONVER >= 0x380) && (MONVER <= 0x3ff)))) { // only on MS3
                    portusage.k[1] = 7;
                    DDRK |= 0x02;
                } else {
                    portusage.t[3] = 7;
                    DDRT |= 0x08;
                    PWME &= ~0x02;
                    OCPD |= 0x08;
                    PTP |= 0x02;
                }
            } else if (i == 10) {
                if ((MONVER >= 0x380) && (MONVER <= 0x3ff)) { // non MS3
                    portusage.p[4] = 7;
                    DDRP |= 0x10;
                } else {
                    portusage.k[7] = 7;
                    DDRK |= 0x80;
                }
            } else if (i == 11) {
                if ((MONVER >= 0x380) && (MONVER <= 0x3ff)) { // non MS3
                    portusage.p[5] = 7;
                    DDRP |= 0x20;
                } else {
                    portusage.m[2] = 7;
                    DDRM |= 0x04;
                }
            }
        }
        if (do_dualouts) {
            if (num_inj > 8) {
                conf_err = 109;
            } else if (ram4.staged_extended_opts & STAGED_EXTENDED_USE_V3) {
                /* only claim these pins are used if V3 is enabled for staging */
                portusage.t[1] = 8;
                portusage.t[3] = 8;
            }
        }
    }

    /* Check for staging on and DF on with dual table or "different" output */
    /* not presently possible */
    if ((ram4.staged & 0x7)
    && (ram5.dualfuel_sw & 0x01) && ((ram5.dualfuel_opt & 0x8) || (ram5.dualfuel_opt & 0x4))) {
        conf_err = 140;
    }
    

    // Allocate IAC or PWM idle pins
    idlectl_tmp = ram4.IdleCtl & 0xF;
    if (idlectl_tmp == 1) {    // on/off idle
        port_idleonoff = pPTP;
        if (ram4.pwmidle_freq & 0x20) { //using alternate port PP2
            DDRP |= 0x04;
            PWME &= 0x04;
            pin_idleonoff = 0x04;
            portusage.p[2] = 5;
        } else {
            DDRP |= 0x80;
            PWME &= 0x80;
            pin_idleonoff = 0x80;
            portusage.p[7] = 5;
        }
    } else if ((idlectl_tmp == 4) || (idlectl_tmp == 6)) {    // PWM idle
        if (ram4.pwmidle_freq & 0x20) { //using alternate port PP2
            portusage.p[2] = 5;
            port_idlepwm = (unsigned char *) &PWMDTY2;
            pin_idlepwm = 0x04;
        } else {
            portusage.p[7] = 5;
            port_idlepwm = (unsigned char *) &PWMDTY7;
            pin_idlepwm = 0x80;
        }
        if ((ram4.pwmidle_freq & 0xc0) == 0x40) {       // FIDLE
            portusage.p[7] = 5;
            port_idlepwm3 = (unsigned char *) &PWMDTY7;
            pin_idlepwm3 = 0x80;
        } else if ((ram4.pwmidle_freq & 0xc0) == 0x80) {        // boost
            portusage.p[3] = 5;
            port_idlepwm3 = (unsigned char *) &PWMDTY3;
            pin_idlepwm3 = 0x08;
        } else if ((ram4.pwmidle_freq & 0xc0) == 0xc0) {        // VVT
            portusage.p[6] = 5;
            port_idlepwm3 = (unsigned char *) &PWMDTY6;
            pin_idlepwm3 = 0x40;
        } else {
            port_idlepwm3 = &dummyReg;
            pin_idlepwm3 = 0;
        }
    } else if ((idlectl_tmp == 2) || (idlectl_tmp == 3) || (idlectl_tmp == 5) || (idlectl_tmp == 7) || (idlectl_tmp == 8)) {       // stepper IAC
        /* assuming that we assigning these before anything else can use them */
        pPTJpin0 = pPTJ;
        pPTJpin1 = pPTJ;
        pPTJpin6 = pPTJ;
        portusage.j[0] = 6;
        portusage.j[1] = 6;
        portusage.j[6] = 6;
    }
    
    // Flex fuel input
    pin_flex = 0;
    if (ram4.FlexFuel & 0x1) {
        /* 0=PE0, 2 = PE1, 4 = PE2, 5 = n/a */
        if ((ram4.FlexFuel & 0x6) == 0) {
            if (portusage.e[0]) {
                conf_err = 5;
            } else {
                portusage.e[0] = 58;
                pin_flex = 1;
            }
        } else if ((ram4.FlexFuel & 0x6) == 2) {
            if (portusage.e[1]) {
                conf_err = 5;
            } else {
                portusage.e[1] = 58;
                pin_flex = 2;
            }
        } else {                // use MS3X input
            if (portusage.e[2]) {
                conf_err = 5;
            } else {
                portusage.e[2] = 58;
                pin_flex = 4;
            }
        }
    }

    mafload_no_air = 1000;

    /* set up MAF input for use or logging */
    if ((ram4.MAFOption & 0x20) || (ram4.MAFOption & 0x1f)) {
        flagbyte8 |= FLAGBYTE8_USE_MAF;

        if (ram4.MAFOption & 0x20) {
            unsigned char tmp_opt;
            /* freq based MAF */
            mafport = (unsigned short*)&mafperiod_accum; /* makes window code read period sample */
            tmp_opt = (ram4.MAFOption & 0xc0) >> 6;
            // options are PT4, PT2, PT5, PT6
            generic_timer_setup(131, tmp_opt, 1, 13);

            if (tmp_opt == 0) {
                flagbyte13 |= FLAGBYTE13_MAF_PT4;
                tim_mask |= 0x10;
            } else if (tmp_opt == 1) {
                flagbyte13 |= FLAGBYTE13_MAF_PT2;
                tim_mask |= 0x04;
            } else if (tmp_opt == 2) {
                flagbyte13 |= FLAGBYTE13_MAF_PT5;
                tim_mask |= 0x20;
            } else if (tmp_opt == 3) {
                flagbyte13 |= FLAGBYTE13_MAF_PT6;
                tim_mask |= 0x40;
            }

            if (ram5.maf_freq1 < 250) {
                /* < 250Hz uses .128ms steps, above uses 1us steps */
                flagbyte12 |= FLAGBYTE12_MAF_FSLOW;
            }
        } else {
            mafport = &ATD0DR11;
            generic_adc_setup(&mafport, 69, ram4.MAFOption & 0x1f, 13);
        }

        /* was 61840750 for CID */
        /* this factor is calculated as follows - only used to generate mafload
        by definition mafload = 101.35 kPa at 100% VE = 1013.5 units.
        Density of air at 20C and 101.35kPa = 1.2041 x 10^-3 g/cc
        airden factor = 100.0% at 20C = 1000 units
        mafload = ((MAFCoef / aircor) * maf) / rpm)
        at 100% VE, flow = size x rpm/120 x 1.2041 x 10^-3 x 100
        maf = 100 x flow  units
        The 120 is converting from rpm to cycles/sec on a 4 stroke
        MAFCoef = k / size

        So, 1013.5 = ((k/size / 1000) x 100 x flow) / rpm
            or k = 1.0135 x 10^6 x size x rpm x 120
                   ---------------------------------
                    100 x size x rpm x 1.2041 x 10^-3
          rpm, size drop out
            k = 1.0135 x 10^6 x 120 / (100 x 1.2041 x 10^-3) 
                = 1010048999
  (not quite the same as Al's figure 1013569893L which appears to use air density as 1.200x10^-3
        */
        MAFCoef = 1010048999L / ram4.enginesize;
        if (ram4.EngStroke & 0x01) {
            /* for 360 deg cycle engines, halve it as MAF will be double */
            MAFCoef >>= 1;
        }
    }

    if ((ram4.FuelAlgorithm & 0xF) == 5) {

        if (!(flagbyte8 & FLAGBYTE8_USE_MAF)) {
            /* MAF enabled but no input defined */
            conf_err = 146;
        }

        /* For reference
            1 = Speed Density - requires MAP
            2 = %baro - requires MAP
            3 = alpha-n - optionally uses MAP
            4 = MAFload - no MAP
            5 = MAF - no MAP
            6 = ITB - requires MAP
        */
        /* see if none of our algos use MAP */
        if (((ram4.FuelAlgorithm & 0xF) != 1) && ((ram4.FuelAlgorithm & 0xF0) != 0x10) && // not SD
            ((ram4.FuelAlgorithm & 0xF) != 2) && ((ram4.FuelAlgorithm & 0xF0) != 0x20) && // not %baro
            (((ram4.FuelAlgorithm & 0xF) != 3) || (((ram4.FuelAlgorithm & 0xF) == 3) && (!(ram4.loadopts & 4)))) &&
            // not alpha-n or is alpha-n but multiply MAP off
            (((ram4.FuelAlgorithm & 0xF0) != 0x30) || (((ram4.FuelAlgorithm & 0xF0) == 0x30) && (!(ram4.loadopts & 4)))) &&
            ((ram4.FuelAlgorithm & 0xF) != 6) && ((ram4.FuelAlgorithm & 0xF0) != 0x60) && // not ITB
            ((ram4.IgnAlgorithm & 0xF) != 1) && ((ram4.IgnAlgorithm & 0xF0) != 0x10) && // repeat for spark load etc.
            ((ram4.IgnAlgorithm & 0xF) != 2) && ((ram4.IgnAlgorithm & 0xF0) != 0x20) &&
            (((ram4.IgnAlgorithm & 0xF) != 3)  || (((ram4.IgnAlgorithm & 0xF) == 3) && (!(ram4.loadopts & 4)))) &&
            (((ram4.IgnAlgorithm & 0xF0) != 0x30)  || (((ram4.IgnAlgorithm & 0xF0) == 0x30) && (!(ram4.loadopts & 4)))) &&
            ((ram4.IgnAlgorithm & 0xF) != 6) && ((ram4.IgnAlgorithm & 0xF0) != 0x60) &&
            ((ram4.extra_load_opts & 0xF) != 1) && ((ram4.extra_load_opts & 0xF0) != 0x10) && 
            ((ram4.extra_load_opts & 0xF) != 2) && ((ram4.extra_load_opts & 0xF0) != 0x20) &&
            (((ram4.extra_load_opts & 0xF) != 3) || (((ram4.extra_load_opts & 0xF) == 3) && (!(ram4.loadopts & 4)))) &&
            (((ram4.extra_load_opts & 0xF0) != 0x30) || (((ram4.extra_load_opts & 0xF0) == 0x30) && (!(ram4.loadopts & 4)))) && 
            ((ram4.extra_load_opts & 0xF) != 6) && ((ram4.extra_load_opts & 0xF0) != 0x60))  {

            flagbyte8 |= FLAGBYTE8_USE_MAF_ONLY; /* no MAP available, so use MAFload for MAPdot */
        }

    } else {
        /* MAFload as secondary without MAF as primary */
        if (((ram4.FuelAlgorithm & 0xF0) == 0x40) ||
            ((ram4.IgnAlgorithm & 0xF) == 4) || ((ram4.IgnAlgorithm & 0xF0) == 0x40) ||
            ((ram4.extra_load_opts & 0xF) == 4) || ((ram4.extra_load_opts & 0xF0) == 0x40)) {
            if (!(flagbyte8 & FLAGBYTE8_USE_MAF)) {
                /* MAFload enabled but no input defined */
                conf_err = 146;
            }
        }
    }

    port_map2 = (unsigned short*)&dummyReg;
    if (ram4.mapport & 0x20) {
        unsigned char tmp_opt;
        /* freq based MAP */
        mapport = (unsigned short*)&mapperiod_accum; /* makes window code read period sample */
        tmp_opt = (ram4.mapport & 0xc0) >> 6;
        // options are PT4, PT2, PT5, PT6
        generic_timer_setup(130, tmp_opt, 1, 14);

        if (tmp_opt == 0) {
            flagbyte13 |= FLAGBYTE13_MAP_PT4;
            tim_mask |= 0x10;
        } else if (tmp_opt == 1) {
            flagbyte13 |= FLAGBYTE13_MAP_PT2;
            tim_mask |= 0x04;
        } else if (tmp_opt == 2) {
            flagbyte13 |= FLAGBYTE13_MAP_PT5;
            tim_mask |= 0x20;
        } else if (tmp_opt == 3) {
            flagbyte13 |= FLAGBYTE13_MAP_PT6;
            tim_mask |= 0x40;
        }

        if (ram5.map_freq1 < 250) {
            /* < 250Hz uses 4us steps, above uses 1us steps */
            flagbyte12 |= FLAGBYTE12_MAP_FSLOW;
        }
    } else {
        mapport = &ATD0DR0;
        generic_adc_setup(&mapport, 130, ram4.mapport, 14);

        /* 2nd MAP port */
        if (ram4.map2port & 0x1f) {
            generic_adc_setup(&port_map2, 138, ram4.map2port, 14);
        }
    }

    /* Check MAP sampling angles are sensible */
    if ((ram4.mapsample_opt & 0x04) == 0) { // not cycle average
        int an;
        if (ram4.EngStroke & 0x01) {
            an = 3600;
        } else {
            an = 7200;
        }
        an = an / (ram4.no_cyl & 0x1f);
        RPAGE = tables[10].rpg;
        for (ix = 0; ix < 8 ; ix++) {
            if ((ram_window.pg10.map_sample_timing[ix] < 0) || (ram_window.pg10.map_sample_timing[ix] >= an)) {
                conf_err = 129;
            }
        }
        if (ram4.map_sample_duration > an) {
            conf_err = 129;
        }
    }

    // Realtime baro
    if (ram4.BaroOption == 2) {
        baroport = &ATD0DR0;

        generic_adc_setup(&baroport, 2, ram4.rtbaroport, 15);
    }

    // EGO sensors
    if (ram4.EgoOption & 3) {
        for (ix = 0; ix < ram4.egonum ; ix++) { 
            egoport[ix] = (unsigned short*)&dummyReg;
            if ((ram5.egoport[ix] & 0x1f) == 0) { // normal
                egoport[ix] = (unsigned short*)&ATD0DR5; // default
            } else if ((ram5.egoport[ix] & 0x1f) == 7) { // can
                egoport[ix] = (unsigned short *)&datax1.ego[ix];
            } else {
                generic_adc_setup(&egoport[ix], 5, (ram5.egoport[ix] & 0x1f), 16);
            }
        }
    }
    //Tacho output
    if (ram4.tacho_opt & 0x80) {
        unsigned char tmp_opt;
        tmp_opt = ram4.tacho_opt & 0x01f;

        generic_digout_setup(&port_tacho, &pin_tacho, 6, tmp_opt, 17);
    }

    if (ram4.ac_idleup_settings & 0x80) {
        if (ram4.ac_idleup_io & 0x3f) {
            generic_digout_setup(&port_ac_out, &pin_ac_out, 73, ram4.ac_idleup_io & 0x3f, 18);
        } else {
            port_ac_out = (volatile unsigned char *) &dummyReg;
            pin_ac_out = 0;
        }
        generic_digin_setup(&port_ac_in, &pin_ac_in, &pin_match_ac_in, 73, ram4.ac_idleup_settings & 0x1f, 19);

        /* Make sure the output pin is off, wouldn't want AC on while cranking */
        *port_ac_out &= ~pin_ac_out;
    }

    if (ram4.fanctl_settings & 0x80) {
        generic_digout_setup(&port_fanctl_out, &pin_fanctl_out, 74, ram4.fanctl_settings & 0x3f, 20);

        /* Make sure the output pin is off, wouldn't want fan on while cranking */
        *port_fanctl_out &= ~pin_fanctl_out;
        if (ram4.fanctl_offtemp >= ram4.fanctl_ontemp) {
            conf_err = 111;
        }
    }

    if (ram4.boost_ctl_settings & BOOST_CTL_ON) {
        if ((ram4.boost_ctl_pwm & 0x30) == 0x20) { // sw pwm
            generic_digout_setup(&boostport, &boostpin, 27, ram4.boost_ctl_pins & 0x1f, 21);
        } else { // hwpwm
            generic_hwpwm_setup(&boostport, 27, ram4.boost_ctl_pins >> 5, ram4.boost_ctl_pwm, ram4.boost_ctl_settings & 0x20, 21);
            boostpin = 1; // any non-zero value
        }
        // second channel - uses same frequency and inversion as first channel
        if (ram5.boost_ctl_settings2 & BOOST_CTL_ON) {
            if ((ram4.boost_ctl_pwm & 0x30) == 0x20) { // sw pwm
                generic_digout_setup(&port_boost2, &pin_boost2, 116, ram5.boost_ctl_pins2 & 0x1f, 68);
            } else { // hwpwm
                generic_hwpwm_setup(&port_boost2, 116, ram5.boost_ctl_pins2 >> 5, ram4.boost_ctl_pwm, ram4.boost_ctl_settings & 0x20, 68);
                pin_boost2 = 1; // any non-zero value
            }
        }
    }

    if (ram4.N2Oopt & 0x04) {   // stage 1
        unsigned char tmp_opt;
        //Nitrous outputs from long list
        // split into nitrous stage 1 nitrous/fuel and nitrous stage 2 nitrous/fuel outputs. i.e. possible four pins
        //     0 off           "off"
/* "INVALID", "IAC1", "IAC2", "FIDLE", "D15", "Nitrous 1", "Nitrous2", "Tacho", "Idle", "Boost", "VVT", "Inj Bank 1", "Inj Bank 2", "Inj A", "Inj B", "Inj C", "Inj D", "Inj E", "Inj F", "Inj G", "Inj H", "D14", "D16", "JS11", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID"
*/
        port_n2o1n = (volatile unsigned char *) &dummyReg;
        pin_n2o1n = 0;

        generic_digout_setup(&port_n2o1n, &pin_n2o1n, 39, ram4.n2o1n_pins & 0x1f, 22);
            
        *port_n2o1n &= ~pin_n2o1n;      // turn it off

        // Now the stage 1 fuel pin
        tmp_opt = ram4.n2o1f_pins & 0x1f;
        //     0 off           "off"
/* "Same", "IAC1", "IAC2", "FIDLE", "D15", "Nitrous 1", "Nitrous2", "Tacho", "Idle", "Boost", "VVT", "Inj Bank 1", "Inj Bank 2", "Inj A", "Inj B", "Inj C", "Inj D", "Inj E", "Inj F", "Inj G", "Inj H", "D14", "D16", "JS11", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID"
*/
        if ((tmp_opt == 0) || (tmp_opt == (ram4.n2o1n_pins & 0x1f))) {
            port_n2o1f = port_n2o1n;
            pin_n2o1f = pin_n2o1n;
        } else {
            port_n2o1f = (volatile unsigned char *) &dummyReg;
            pin_n2o1f = 0;

            generic_digout_setup(&port_n2o1f, &pin_n2o1f, 39, tmp_opt, 23);
        }
        *port_n2o1f &= ~pin_n2o1f;      // turn it off

        // Stage 2 nitrous pin
        if (ram4.N2Oopt & 0x08) {
            tmp_opt = ram4.n2o2n_pins & 0x1f;
            port_n2o2n = (volatile unsigned char *) &dummyReg;
            pin_n2o2n = 0;

            generic_digout_setup(&port_n2o2n, &pin_n2o2n, 40, tmp_opt, 24);

            *port_n2o2n &= ~pin_n2o2n;  // turn it off

            // Now the stage 2 fuel pin
            tmp_opt = ram4.n2o2f_pins & 0x1f;
            //         0 off           "off"
            /* "Same", "IAC1", "IAC2", "FIDLE", "D15", "Nitrous 1", "Nitrous2", "Tacho", "Idle", "Boost", "VVT", "Inj Bank 1", "Inj Bank 2", "Inj A", "Inj B", "Inj C", "Inj D", "Inj E", "Inj F", "Inj G", "Inj H", "D14", "D16", "JS11", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID"
             */
            if ((tmp_opt == 0) || (tmp_opt == (ram4.n2o2n_pins & 0x1f))) {
                port_n2o2f = port_n2o2n;
                pin_n2o2f = pin_n2o2n;
            } else {
                port_n2o2f = (volatile unsigned char *) &dummyReg;
                pin_n2o2f = 0;

                generic_digout_setup(&port_n2o2f, &pin_n2o2f, 40, tmp_opt, 25);
            }
            *port_n2o2f &= ~pin_n2o2f;  // turn it off
        } else {
            port_n2o2n = (volatile unsigned char *) &dummyReg;
            pin_n2o2n = 0;
            port_n2o2f = (volatile unsigned char *) &dummyReg;
            pin_n2o2f = 0;
        }
//     Nitrous inputs

        pin_match_n2oin = 0;    // all bar one input is active low

        generic_digin_setup(&port_n2oin, &pin_n2oin, &pin_match_n2oin, 36, ram4.N2Oopt3 & 0x1f, 26);
    } else {
        port_n2o1n = (volatile unsigned char *) &dummyReg;
        pin_n2o1n = 0;
        port_n2o1f = (volatile unsigned char *) &dummyReg;
        pin_n2o1f = 0;
        port_n2o2n = (volatile unsigned char *) &dummyReg;
        pin_n2o2n = 0;
        port_n2o2f = (volatile unsigned char *) &dummyReg;
        pin_n2o2f = 0;
        port_n2oin = (volatile unsigned char *) &dummyReg;
        pin_n2oin = 0;
    }

    //Launch control
    if (ram4.launch_opt & 0x40) {
        unsigned char tmp_opt;
        tmp_opt = ram4.launch_opt2 & 0x1f;
        // standard digi inputs list
        generic_digin_setup(&port_launch, &pin_launch, &pin_match_launch, 7, tmp_opt, 27);

        /* delaybox */
        tmp_opt = ram5.timedout1_in & 0x1f;
        if (tmp_opt) {
            generic_digin_setup(&port_timed1_in, &pin_timed1_in, NULL, 144, tmp_opt, 114); // config errors

            tmp_opt = ram5.timedout1_out & 0x3f;
            if (tmp_opt) {
                generic_digout_setup(&port_timed1_out, &pin_timed1_out, 144, tmp_opt, 115); // config errors
            }
        }
        /* throttle stop */
        tmp_opt = ram5.tstop_out & 0x3f;
        if (tmp_opt) {
            generic_digout_setup(&port_tstop, &pin_tstop, 155, tmp_opt, 116); // config errors
        }
    }
    // Variable launch
    if ((ram4.launch_opt & 0x40) && (ram4.launch_var_on & 0x1f)) {
        generic_adc_setup(&port_launch_var, 55, ram4.launch_var_on & 0x1f, 28);
    }

    //3step control
    if ((ram4.launch_opt & 0x40) && (ram4.launch_3step & 0x1f)) {
        unsigned char tmp_opt;
        tmp_opt = ram4.launch_3step & 0x1f;
        generic_digin_setup(&port_3step, &pin_3step, NULL, 56, tmp_opt, 29); 
    }
    // Datalogging
    if (ram4.log_style & 0xc0) {
        unsigned char tmp_opt;
        if (ram4.log_style & 0x80) {    // Button mode or Button+WTF mode
            tmp_opt = ram4.log_style2_but & 0x1f;

            generic_digin_setup(&port_sdbut, &pin_sdbut, &pin_match_sdbut, 47, tmp_opt, 30);
        }
        // now check for LED output
        tmp_opt = ram4.log_style_led & 0x3f;
        port_sdled = (volatile unsigned char *) &dummyReg;
        if (tmp_opt != 0) {
            generic_digout_setup(&port_sdled, &pin_sdled, 46, tmp_opt, 31);
        }
    }

    // Shifter
    if (ram5.shift_cut_on & 0x01) {
            unsigned char tmp_opt;
            generic_digin_setup(&port_shift_cut_in, &pin_shift_cut_in, &pin_shift_cut_match, 81, ram5.shift_cut_in & 0x1f, 32);
            pin_shift_cut_match = 0;

        // now check for solenoid output
        tmp_opt = (ram5.shift_cut_out & 0x1f);
        port_shift_cut_out = (volatile unsigned char *) &dummyReg;
        if (tmp_opt != 0) {
            generic_digout_setup(&port_shift_cut_out, &pin_shift_cut_out, 82, tmp_opt, 33);
        }
    }

    // Generic PWM outputs
    for (ix = 0; ix < 6 ; ix++) {
        if (ram5.pwm_opt[ix] & 0x01) {
            unsigned char tmp_opt;
            // now check for solenoid output
            tmp_opt = (ram5.pwm_opt2[ix] & 0x3f);
            port_pwm[ix] = (volatile unsigned char *) &dummyReg;
            gp_clk[ix] = 1;
            gp_max_on[ix] = 1;
            gp_max_off[ix] = 7812;
            gp_stat[ix] = 0;
            if (tmp_opt != 0) {
                generic_digout_setup(&port_pwm[ix], &pin_pwm[ix], 83, tmp_opt, 34 + ix);
            }
        }
    }

    // Datalog stream
    if ((ram4.log_style & 0xc0) && ((ram4.log_style & 0x03) == 1)) {

        generic_adc_setup(&port_stream, 51, ram4.log_style3 & 0x7, 40);
    }

/* WATER INJECTION */
    if (ram4.water_freq & 0x10) {
        // Water injection - pump
        port_wipump = (volatile unsigned char *) &dummyReg;
        pin_wipump = 0;
        if (ram4.water_pins_pump & 0x1f) {
            generic_digout_setup(&port_wipump, &pin_wipump, 65, ram4.water_pins_pump & 0x1f, 54);
            *port_wipump &= ~pin_wipump;        // turn it off
        }
        // Water injection - valve
        port_wivalve = (volatile unsigned char *) &dummyReg;
        pin_wivalve = 0;
        if ((ram4.water_freq & 0x60) && (ram4.water_pins_valve & 0x1f)) {
            generic_digout_setup(&port_wivalve, &pin_wivalve, 66, ram4.water_pins_valve & 0x1f, 55);
            *port_wivalve &= ~pin_wivalve;      // turn it off
        }

        // safety input - active low except "nitrous" pin  //FIXME needs new system adding
        if (ram4.water_pins_in_shut & 0x1f) {
            // standard digi inputs list
            generic_digin_setup(&port_wiin, &pin_wiin, &pin_match_wiin, 106, (ram4.water_pins_in_shut & 0x1f), 56);
        }
    } else {
        port_wipump = (volatile unsigned char *) &dummyReg;
        pin_wipump = 0;
        port_wivalve = (volatile unsigned char *) &dummyReg;
        pin_wivalve = 0;
        port_wiin = (volatile unsigned char *) &dummyReg;
        pin_wiin = 0;
    }


    // Table switching and dual fuel. First set pin vars to zero.
    pin_tsw_rf = 0;
    pin_tsw_afr = 0;
    pin_tsw_stoich = 0;
    pin_boost_tsw = 0;
    pin_tsw_ob = 0;
    pin_tsf = 0;
    pin_tss = 0;

    // Dual fuel switching
    if (ram5.dualfuel_sw & 0x01) {
        generic_digin_setup(&port_dualfuel, &pin_dualfuel, NULL, 84, ram5.dualfuel_pin & 0x1f, 41);
        if (ram5.dualfuel_sw & 0x02) {
            port_tsf = port_dualfuel;
            pin_tsf = pin_dualfuel;
        }
        if (ram5.dualfuel_sw & 0x04) {
            port_tss = port_dualfuel;
            pin_tss = pin_dualfuel;
        }
        if (ram5.dualfuel_sw & 0x08) {
            port_tsw_afr = port_dualfuel;
            pin_tsw_afr = pin_dualfuel;
        }
        if (ram5.dualfuel_sw & 0x10) {
            port_tsw_rf = port_dualfuel;
            pin_tsw_rf = pin_dualfuel;
        }
        if (ram5.dualfuel_sw & 0x20) {
            port_tsw_stoich = port_dualfuel;
            pin_tsw_stoich = pin_dualfuel;
        }
        if (ram5.dualfuel_sw2 & 0x02) {
            port_tsf = port_dualfuel;
            pin_tsf = pin_dualfuel;
        }
        if (ram5.dualfuel_sw2 & 0x10) {
            port_tsw_ob = port_dualfuel;
            pin_tsw_ob = pin_dualfuel;
        }
        if (ram5.dualfuel_sw2 & 0x20) {
            port_boost_tsw = port_dualfuel;
            pin_boost_tsw = pin_dualfuel;
        }
        // Others, viz WUE, ASE, prime, crank, inj params, inj small pw are Dual Fuel only
    }

    // table switching - these functions can all use the same pin so two passes, clash check then setup

// Pins are "INVALID", "Tableswitch", "PE0/JS7", "PE1", "JS10", "JS11", "JS5 (ADC6)", "JS4 (ADC7)", "Nitrous In", "Launch in", "Datalog In", "PT4 Spare in", "PT2 Cam in", "PE2 Flex", "INVALID", "INVALID", "CANIN1", "CANIN2", "CANIN3", "CANIN4", "CANIN5", "CANIN6", "CANIN7", "CANIN8", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID"

    if ((ram4.feature5_0 & 2) && ((ram4.feature5_0 & 0x0c) == 0) && (!pin_tsf) ) { // fuel table sw
        check_tsw(ram4.tsw_pin_f & 0x1f, 52);
    }

    if ((ram4.feature5_0 & 0x10) && ((ram4.feature5_0 & 0x60) == 0) && (!pin_tss) ) {  // spark table sw
        check_tsw(ram4.tsw_pin_s & 0x1f, 53);
    }

    if ((ram4.boost_ctl_settings & 0x08) && (ram4.boost_feats & 0x1f) && (!pin_boost_tsw) )  {// boost table sw
        check_tsw(ram4.boost_feats & 0x1f, 54);
    }

    if ((ram4.OverBoostOption & 0x03) && (ram4.tsw_pin_ob & 0x0f) && (!pin_tsw_ob) ) { // over boost sw
        check_tsw(ram4.tsw_pin_ob & 0x1f, 100);
    }

    if ((ram4.tsw_pin_rf & 0x1f) && (!pin_tsw_rf))  {       // reqFuel sw
        check_tsw(ram4.tsw_pin_rf & 0x1f, 62);
    }

    if ((ram4.tsw_pin_afr & 0x1f) && (!pin_tsw_afr))  {      // AFR table sw
        check_tsw(ram4.tsw_pin_afr & 0x1f, 63);
    }

    if ((ram4.tsw_pin_stoich & 0x1f) && (!pin_tsw_stoich))  {   // Stoich sw
        check_tsw(ram4.tsw_pin_stoich & 0x1f, 64);
    }

// now we've x-refed for conflicts actually do the setting
    if ((ram4.feature5_0 & 2) && ((ram4.feature5_0 & 0x0c) == 0)  && (!pin_tsf)) {     // fuel table sw
        generic_digin_setup(&port_tsf, &pin_tsf, NULL, 0, (ram4.tsw_pin_f & 0x1f), 42);
    }

    if ((ram4.feature5_0 & 0x10) && ((ram4.feature5_0 & 0x60) == 0)  && (!pin_tss)) {     // spark table sw
        generic_digin_setup(&port_tss, &pin_tss, NULL, 0, (ram4.tsw_pin_s & 0x1f), 43);
    }

    if ((ram4.boost_ctl_settings & 0x08) && (ram4.boost_feats & 0x1f) && (!pin_boost_tsw) ) { // boost table sw 
        generic_digin_setup(&port_boost_tsw, &pin_boost_tsw, NULL, 0, ram4.boost_feats & 0x1f, 44);
    }

    if ((ram4.OverBoostOption & 0x03) && (ram4.tsw_pin_ob & 0x1f) && (!pin_tsw_ob) ) { // over boost sw
        generic_digin_setup(&port_tsw_ob, &pin_tsw_ob, NULL, 0, ram4.tsw_pin_ob & 0x1f, 45);
    }

    if ((ram4.tsw_pin_rf & 0x1f) && (!pin_tsw_rf)) {       // reqFuel sw
        generic_digin_setup(&port_tsw_rf, &pin_tsw_rf, NULL, 0, ram4.tsw_pin_rf & 0x1f, 46);
    }

    if ((ram4.tsw_pin_afr & 0x1f) && (!pin_tsw_afr)) {      // AFR table sw
        generic_digin_setup(&port_tsw_afr, &pin_tsw_afr, NULL, 0, ram4.tsw_pin_afr & 0x1f, 47);
    }

    if ((ram4.tsw_pin_stoich & 0x1f) && (!pin_tsw_stoich)) {   // Stoich sw
        generic_digin_setup(&port_tsw_stoich, &pin_tsw_stoich, NULL, 0, ram4.tsw_pin_stoich & 0x1f, 48);
    }
    // MaxAFR light out
    if ((ram4.maxafr_opt1 & 1) || (ram4.egt_conf & 0x01)) {
        unsigned char tmp_opt;
        // now check for LED output
        tmp_opt = ram4.maxafr_opt2 & 0x3f;
        port_maxafr = (volatile unsigned char *) &dummyReg;
        pin_maxafr = 0;

        if (tmp_opt != 0) {
            generic_digout_setup(&port_maxafr, &pin_maxafr, 57, tmp_opt, 49);
        }
        *port_maxafr &= ~pin_maxafr;    // turn it off
    }
    // check for valid cylinder counts
    {
        unsigned char tmp_no_cyl = ram4.no_cyl & 0x1f;
        if ((tmp_no_cyl == 11) || (tmp_no_cyl ==  13) || (tmp_no_cyl ==  15)) {
            conf_err = 8;           // might work in fuel-only though
        }
    }

    if ((ram4.ICIgnOption & 0xa) == 0xa) {
        conf_err = 9;           // trigret and oddfire not allowed
    }

    if (ram4.staged & 0x7) {
        if (!ram4.staged_pri_size || !ram4.staged_sec_size) {
            /* Need injector sizes */
            conf_err = 10;
        }

        if ((ram4.staged & 0x7) != 5) {
            if ((ram4.staged & 0x7) == ((ram4.staged & 0x38) >> 3)) {
                /* staged triggers the same */
                conf_err = 11;
            }

            if (ram4.staged_param_1 == 0) {
                /* no first param */
                conf_err = 12;
            }

            if ((ram4.staged & 0x38) && (!ram4.staged_param_2)) {
                /* second param configured with no second param */
                conf_err = 13;
            }

            if ((ram4.staged & 0x40) && (!ram4.staged_transition_events)) {
                /* Staging transition with no number of events */
                conf_err = 14;
            }
        }

        if (ram4.Alternate & 0x1) {
            if (!(ram4.hardware & HARDWARE_MS3XFUEL)) {
                conf_err = 15;
            }
        }
    }

    if ((spkmode == 2) || (spkmode == 3)) {
        if (ram4.adv_offset < 0) {
            conf_err = 31;
        }
    } else if (spkmode > 4) {
        if ((ram4.adv_offset > 200) || (ram4.adv_offset < -200)) {
             conf_err = 143;
        }
    }

    // see if we can enable ignition trigger led without conflict
    if ((!portusage.m[5]) && (ram4.feature4_0 & 0x2)
        && (spkmode == 3)) {
        portusage.m[5] = 57;
        flagbyte1 |= flagbyte1_igntrig;
    }

    *pPTJpin0 &= ~0x03;       // turn off IAC coils

    if (idlectl_tmp == 2) {
        *pPTJpin6 |= 0x40;      // disable current to motor (set bit= 1) PJ6
    } else {                    //  IdleCtl = 3, 5 or no stepper motor (set bit = 0)
        *pPTJpin6 &= ~0x40;     // enable current to motor always(set bit= 0)
        PTJ &= ~0x40;           // for non-IAC modes
    }

    config_xgate();
    xgate_deadman = 1;
    coilsel = 0;
    FIRE_COIL; // this causes a XGSWT which should clear xgate_deadman

    // set up CRG RTI Interrupt for .128 ms clock. CRG from 8MHz oscillator.  Demo uses 4MHz
    mms = 0;                    // .128 ms tics
    millisec = 0;               // 1.024 ms clock (8 tics) for adcs
    lmms = 0;
    cansendclk = 7812;
    can_bcast_last = 0;
    ltch_lmms = 0;
    outpc.seconds = 0;          // (1.0035) secs
    RTICTL = 0x10;              // load timeout register for .128 ms (smallest possible)
    CRGINT |= 0x80;             // enable interrupt
    CRGFLG = 0x80;              // clear interrupt flag (0 writes have no effect)
    //  COPCTL = 0x44; // Enable long C.O.P. timeout 2^20 ~0.125s   XXXX disabled

/* more timer setup */
    flagbyte5 &= ~(FLAGBYTE5_CAM_NOISE | FLAGBYTE5_CAM_POLARITY | FLAGBYTE5_CAM_BOTH | FLAGBYTE5_CRK_BOTH);     // ensure all off to start

/* VVT V10 BMW decoder forces both edges on CAM */
    RPAGE = 0xfb; // HARDCODED page 24
    if ((ram_window.pg24.vvt_opt1 & 0x03) && ((ram_window.pg24.vvt_opt4 & 0x03) == 1)) {
        flagbyte5 |= FLAGBYTE5_CAM_DOUBLE;
    }

    if (flagbyte5 & FLAGBYTE5_CAM) {
        if ((flagbyte5 & FLAGBYTE5_CAM_DOUBLE) == 0) {  // disabled if double (presently only rising and falling)
            if (ram4.secondtrigopts & 0x01) {   // noise filter
                flagbyte5 |= FLAGBYTE5_CAM_BOTH;
                flagbyte5 |= FLAGBYTE5_CAM_NOISE;
            } else  {    //cam polarity check now always on if possible
                flagbyte5 |= FLAGBYTE5_CAM_POLARITY;
            }
        } else {
            flagbyte5 |= FLAGBYTE5_CAM_BOTH;
        }

        // choose PT5 or PT2 as cam input
        if (ram4.hardware & HARDWARE_CAM) {     // MS3X PT2 cam input
            TCTL2 &= ~0x30;     // not OC
            DDRT &= ~0x04;      // ensure PTT2 = input
            TIOS &= ~0x04;      // set as input capture
            TIE |= 0x04;        //enable IC interrupts
            OCPD |= 0x20;

            if (flagbyte9 & FLAGBYTE9_CAS360_ON) {
                TFLG_trig2 = 0x10; // actually use PT4
                if (pin_xgcam) {
                    OCPD |= pin_xgcam; // set up polled cam input - used by flywheel modes
                    TIOS &= ~pin_xgcam;
                    DDRT &= ~pin_xgcam;
                }
            } else {
                TFLG_trig2 = 0x04; // PT2
            }

            if ((spkmode == 4) && (ram4.spk_config & 0x08)) {    // trigger wheel and 2nd trig, set active IC edges
                if (portusage.t[2]) {
                    conf_err = 45;
                } else {
                    unsigned char ctmp;
                    portusage.t[2] = 59;
                    ctmp = ram4.spk_config & 0x30;
                    if (ctmp == 0x30) { // poll level
                        RPAGE = 0xfb; // HARDCODED page 24
                        if (ram_window.pg24.vvt_opt1 & 0x03) { /* VVT user can set polarity */
                            if (ram_window.pg24.vvt_opt2 & 0x80) {
                                ctmp = 0x10;
                                flagbyte10 |= FLAGBYTE10_CAMPOL;
                            } else {
                                ctmp = 0x20;
                                flagbyte10 &= ~FLAGBYTE10_CAMPOL;
                            }
                        } else { /* non VVT */
                            // cam edge trigger on active level
                            if (ram4.spk_config & 0x01) {
                                ctmp = 0x10;
                                flagbyte10 |= FLAGBYTE10_CAMPOL;
                            } else {
                                ctmp = 0x20;
                                flagbyte10 &= ~FLAGBYTE10_CAMPOL;
                            }
                        }
                    } else {
                        if (ram4.spk_config & 0x10) {
                            flagbyte10 |= FLAGBYTE10_CAMPOL;
                        } else {
                            flagbyte10 &= ~FLAGBYTE10_CAMPOL;
                        }
                    }

                    if (flagbyte5 & FLAGBYTE5_CAM_BOTH) {
                        ctmp = 0x30;
                    }
                    TCTL4 = (TCTL4 & 0xcf) | ctmp;
                }
            } else {     // other modes that use the cam
                if (flagbyte9 & FLAGBYTE9_CAS360_ON) {
                    unsigned char pol;
                    if (flagbyte5 & FLAGBYTE5_CAM_BOTH) {
                        pol = 3;
                    } else {
                        pol = ram4.ICIgnOption & 0x01;
                        if (ram4.spk_mode3 & 0x04) {
                            pol ^= 0x01;
                        }
                        pol = 2 - pol; // (1 = rising, 2 = falling)
                    }
                    generic_timer_setup(45, 0, pol, 59); // setup PT4 instead of PT2 due to hardware
                } else {
                    if (portusage.t[2]) {
                        conf_err = 45;
                    } else {
                        portusage.t[2] = 59;
                        if (spkmode == 14) {
                            flagbyte2 |= flagbyte2_twintrig;        // set this quick flag
                        }
                        TCTL4 = TCTL4 & 0xcf;
                        if (ram4.ICIgnOption & 0x01) {  // 2nd is same polarity as primary trigger input
                            TCTL4 |= 0x10;
                            flagbyte10 |= FLAGBYTE10_CAMPOL;
                        } else {
                            TCTL4 |= 0x20;
                            flagbyte10 &= ~FLAGBYTE10_CAMPOL;
                        }
                        if (flagbyte5 & FLAGBYTE5_CAM_BOTH) {
                            TCTL4 |= 0x30;
                        }
                    }
                }
            }

        } else {                // original PT5/JS10
            TCTL1 &= ~0x0c;     // not OC
            DDRT &= ~0x20;      // ensure PTT5 = input (already done above)
            TIOS &= ~0x20;      // set as input capture
            TIE |= 0x20;        //enable IC interrupts
            OCPD |= 0x04;
            TFLG_trig2 = 0x20;

            if ((spkmode == 4) && (ram4.spk_config & 0x08)) {    // trigger wheel and 2nd trig, set active IC edges
                if (portusage.t[5]) {
                    conf_err = 45;
                } else {
                    unsigned char ctmp;
                    portusage.t[5] = 59;
                    ctmp = ram4.spk_config & 0x30;
                    if (ctmp == 0x30) { // poll level
                        RPAGE = 0xfb; // HARDCODED page 24
                        if (ram_window.pg24.vvt_opt1 & 0x03) { /* VVT user can set polarity */
                            if (ram_window.pg24.vvt_opt2 & 0x80) {
                                ctmp = 0x10;
                                flagbyte10 |= FLAGBYTE10_CAMPOL;
                            } else {
                                ctmp = 0x20;
                            }
                        } else { /* non VVT */
                            // cam edge trigger on active level
                            if (ram4.spk_config & 0x01) {
                                ctmp = 0x10;
                                flagbyte10 |= FLAGBYTE10_CAMPOL;
                            } else {
                                ctmp = 0x20;
                                flagbyte10 &= ~FLAGBYTE10_CAMPOL;
                            }
                        }
                    } else {
                        if (ram4.spk_config & 0x10) {
                            flagbyte10 |= FLAGBYTE10_CAMPOL;
                        } else {
                            flagbyte10 &= ~FLAGBYTE10_CAMPOL;
                        }
                    }
                    ctmp = ctmp >> 2;
                    if (flagbyte5 & FLAGBYTE5_CAM_BOTH) {
                        ctmp = 0xc;
                    }
                    TCTL3 = (TCTL3 & 0xf3) | ctmp;
                }
            } else {     // other modes that use the cam
                if (portusage.t[5]) {
                    conf_err = 45;
                } else {
                    unsigned char pol = ram4.ICIgnOption & 0x01;
                    if ((flagbyte9 & FLAGBYTE9_CAS360_ON) && (ram4.spk_mode3 & 0x04)) {
                        pol ^= 0x01;
                    }
                    portusage.t[5] = 59;
                    if (spkmode == 14) {
                        flagbyte2 |= flagbyte2_twintrig;        // set this quick flag
                    }
                    TCTL3 = TCTL3 & 0xf3;
                    if (pol) {  // 2nd is same polarity as primary trigger input
                        TCTL3 |= 0x04;
                        flagbyte10 |= FLAGBYTE10_CAMPOL;
                    } else {
                        TCTL3 |= 0x08;
                        flagbyte10 &= ~FLAGBYTE10_CAMPOL;
                    }
                    if (flagbyte5 & FLAGBYTE5_CAM_BOTH) {
                        TCTL3 |= 0x0c;
                    }
                }
            }
        }
    }

    tim_mask_run |= TFLG_trig2;

    // the single/double edge trigger flags are set in ign_wheel based on the mode

    // Presently, if the wheel already uses both edges, then we cannot enable the noise filter

    if (flagbyte5 & FLAGBYTE5_CRK_DOUBLE) {     // crank is double edge triggered
        flagbyte1 &= ~flagbyte1_noisefilter;    // disable noise filter
        flagbyte5 |= FLAGBYTE5_CRK_BOTH;
        TCTL4 |= 0x03;
    } else if (ram4.NoiseFilterOpts & 1) {
        flagbyte1 |= flagbyte1_noisefilter;     // permit noise filter
        flagbyte5 |= FLAGBYTE5_CRK_BOTH;
        TCTL4 |= 0x03;
    } else {
        flagbyte1 &= ~flagbyte1_noisefilter;    // disable noise filter
        TCTL4 &= ~0x03;
        if (ram4.ICIgnOption & 0x01) {
            TCTL4 |= 0x01;
        } else {
            TCTL4 |= 0x02;
        }
        flagbyte1 |= flagbyte1_polarity; // always do polarity check if possible
    }

    /* speedup for ign_in */
    if ((spkmode == 4) && (ram4.spk_config & 0x08) && ((ram4.spk_config & 0x30) == 0x30)) {
        flagbyte9 |= FLAGBYTE9_SPK4RISEFALL;
    }

    if ((flagbyte5 & FLAGBYTE5_CRK_BOTH) && ((spkmode & 0x20) == 0)) {
        // double edged crank but NOT one of the Mitsubishi/Nissan CAN modes
        flagbyte9 |= FLAGBYTE9_CRKPOLCHK;
    }

    /* Set up PIT */
    PITCFLMT = 0x80;
    PITCE = 0x07;               // enable channels 2&1&0
    PITMUX = 0x00;              // all channels user microtimer 0
    PITTF = 0xff;               // ensure all flags cleared
    PITINTE = 0x07;             // enable PIT2 & PIT1 & PIT0
    PITMTLD0 = 0x01;            // microtimer 0
    PITMTLD1 = 0x01;            // microtimer 1
    PITLD0 = 1250;              // Timer 0 - 50us interrupt interval
    PITFLT = 0x01;              // load timer 0
    PITLD1 = 3125;              // Timer 1 - 125us interrupt interval
    PITFLT = 0x02;              // load timer 1 (fractionally later)
    PITLD2 = 1250;              // Timer 2 - 50us interrupt interval (on XGATE)
    PITFLT = 0x04;              // load timer 2 (fractionally later)
    PITCFLMT |= 0x03;           // reload microtimers

    /* Idle control pins part initialised earlier */
    if ((idlectl_tmp == 4) || (idlectl_tmp == 6)) {
/* PWM idle - default is to use 390kHz clock -> 1.5kHz at 256 counts */
        if (ram4.pwmidle_freq & 0x20) { //using alternate port PP2
            PWMPER2 = 255;
        } else {                // standard port PP7
            PWMPER7 = 255;
        }
        *port_idlepwm = 0;
        if (ram4.pwmidleset & 0x08) {
            PWMPOL &= ~pin_idlepwm;
        } else {
            PWMPOL |= pin_idlepwm;
        }

        if ((portusage.pwmsclb[0] == 0xff) || (portusage.pwmsclb[0] == (ram4.pwmidle_freq & 0x0f))) { // 0xff means unused
            portusage.pwmsclb[0] = ram4.pwmidle_freq & 0x0f;
            PWMSCLB = pwmopts[ram4.pwmidle_freq & 0x0f]; // pwm clk = B clk/(2*SCLB)
            portusage.pwmsclb[1] = 5;
        } else {
            conf_err = 148;
		    conf_err_feat = portusage.pwmsclb[1];
        }

        PWMCLK |= pin_idlepwm | pin_idlepwm3;       // use SB

        if ((ram4.pwmidle_freq & 0xc0) == 0x40) {       // FIDLE
            PWMPER7 = 255;
        } else if ((ram4.pwmidle_freq & 0xc0) == 0x80) {        // boost
            PWMPER3 = 255;
        } else if ((ram4.pwmidle_freq & 0xc0) == 0xc0) {        // VVT
            PWMPER6 = 255;
        }
        *port_idlepwm3 = 0;
        if (ram4.pwmidle_freq & 0xc0) { // 3 wire
            if (ram4.pwmidleset & 0x08) {
                PWMPOL |= pin_idlepwm3;
            } else {
                PWMPOL &= ~pin_idlepwm3;
            }
        }
        PWME |= pin_idlepwm | pin_idlepwm3;
    }

    vss_init();

    ign_reset();

    flagbyte4 |= flagbyte4_first_edis;

    // IACStart = enough steps to set IAC wide open.
    //  Set current IAC position to IACStart (all way closed), then move
    //  to the 0 position (wide open - fast idle) and zero out. After
    //  this all subsequent IAC commands will gradually close air passage
    //  as clt temp rises.
    //  Note: Ignore all move commands until finished current move.
    IdleCtl = idlectl_tmp;
    flagbyte3 |= flagbyte3_firstIAC;
    IAC_moving = 0;
    motor_step = -1;

    pwmidle_reset = 4;
    IACmotor_reset = 0;

    outpc.iacstep = ram4.IACStart;      // set current motor step position

    if ((IdleCtl == 2) || (IdleCtl == 3) ||
        (IdleCtl == 5) || (IdleCtl == 7) || (IdleCtl == 8)) {
        IACmotor_pos = 0;
        (void) move_IACmotor();
    } else if ((IdleCtl == 4) || (IdleCtl == 6)) {
        idle_ctl_init();
    }
    pwmidle_timer = ram4.pwmidle_ms; // for stepper and PWM

    last_iacclt = -3200;
    iacpwmctr = 0;

    // set up ADC processing
    // ATD0
    //    - AN0 is MAP
    //    - AN1 is MAT
    //    - AN2 is CLT
    //    - AN3 is TPS
    //    - AN4 is BAT
    //    - AN5 is EGO1 (NB or WB)
    //    - AN6 is Spare or BARO or KNOCK or spark(BaroOption =2) or EGO2 (EgoOption = 2 or 4)
    //    - AN7 is Spare or KNOCK or spark
    // Set up ADCs so they continuously convert, then read result registers
    //  every millisecond
    next_adc = 0;               // specifies next adc channel to be read
    ATD0CTL0 = 0x0f;            // wrap after 15
    ATD0CTL1 = 0x3f;            // default An15 is external trigger, SMP_DIS
    ATD0CTL2 = 0x40;            // leave interrupt disabled, set fast flag clear
    ATD0CTL3 = 0x80;            // do 16 conversions/ sequence, right justified
    ATD0CTL4 = 0x43;            // 10-bit resoln, 8 tic conversion, (was 12 tic)
    // prescaler divide by 8 => 6.25MHz
    ATD0CTL5 = 0x30;            // continuous cnvsn,
    // sample 16 channels starting with AN0

    // wait for ADC engine charge-up or P/S ramp-up
    for (ix = 0; ix < 160; ix++) {
        while ((ATD0STAT0 & 0x80) == 0);        // wait until conversion sequence complete 160 times?
        ATD0STAT0 = 0x80;
    }
    // get all adc values
//    if (ram4.tpsmax < ram4.tps0) {
//        conf_err = 136;
//    }
    tps0_auto = tps0_orig = ram4.tps0;  // auto-zeroing happens at the end of init
    first_adc = 1;
    get_adc(0, 7);
    first_adc = 0;
    if (ram4.BaroOption < 2) {
        if ((ram4.BaroOption == 1) && (outpc.map < ram4.baro_upper)
            && (outpc.map > ram4.baro_lower)) {
            outpc.baro = outpc.map;     // If within range use the initial sensor value
        } else {
            if ((ram4.baro_default > 600) && (ram4.baro_default < 1050)) {      // hardcoded limits
                outpc.baro = ram4.baro_default; // Use entered value if it seems sensible kPa x 10
            } else {
                outpc.baro = 1000;      // failsafe 100 kPa
            }
        }
    }

    var_init();
    check_sensors_init();
    stack_watch_init();

    /* Initialize CAN comms */
    // can_reset flag set above when flagbyte3 cleared
    CanInit();

    spr_port_init();

    if (ram4.boost_ctl_settings & BOOST_CTL_ON) {
        boost_ctl_init();
    }

    //ego_init();
    //egt_init();
    accXport = (unsigned short *)&dummyReg;
    if (ram4.accXport) {
        generic_adc_setup(&accXport, 76, ram4.accXport, 50);
    }
    accYport = (unsigned short *)&dummyReg;
    if (ram4.accYport) {
        generic_adc_setup(&accYport, 76, ram4.accYport, 51);
    }
    accZport = (unsigned short *)&dummyReg;
    if (ram4.accZport) {
        generic_adc_setup(&accZport, 76, ram4.accZport, 52);
    }

/* Anti lag */
    RPAGE = 0xfb; // HARDCODED page 24
    if (ram_window.pg24.als_in_pin & 0x1f) {
        unsigned char tmp_opt;

        tmp_opt = ram_window.pg24.als_in_pin & 0x1f;
        generic_digin_setup(&port_alsin, &pin_alsin, &pin_match_alsin, 113, tmp_opt, 65); 

        tmp_opt = ram_window.pg24.als_out_pin & 0x3f;
        if (tmp_opt) { // on/off out
            generic_digout_setup(&port_alsout, &pin_alsout, 114, tmp_opt, 66);
        }

        tmp_opt = ram5.pwm_opt2[5] & 0x3f;
        if (ram_window.pg24.als_opt & 0x10) { // pwm out (uses PWM F)
            generic_digout_setup(&port_pwm[5], &pin_pwm[5], 115, tmp_opt, 67);
        }

        if ((ram_window.pg24.als_opt & 0x21) &&  // fuel cut or roving-idle
            (! ((ram4.hardware & HARDWARE_MS3XFUEL) && (ram4.sequential & (SEQ_SEMI | SEQ_FULL))) )) {
            /* Cannot use fuel cut if not in MS3X sequential */
            conf_err = 142;
        }
    }
/* end Anti lag */

/* VVT */
    vvt_intx = 0; // ensure zero if vvt off
    port_vvt[0] = &dummyReg;
    port_vvt[1] = &dummyReg;
    port_vvt[2] = &dummyReg;
    port_vvt[3] = &dummyReg;
    RPAGE = 0xfb; // HARDCODED page 24
    if ((ram_window.pg24.vvt_opt1 & 0x03) && (spkmode != 26) && (spkmode != 27)) {
        unsigned char tmp_opt;
        int i;

        vvt_intx = 0x39;

        // check a supported spark mode and set up, else report error
        /* to enable VVT for a mode, needs to be added to ms3_init.c, isr_ign.s and ms3_ign.c */
        if ( (spkmode == 4 && ((ram4.spk_config & 0xc) == 0xc))
            || (((spkmode == 4) && ((ram4.spk_config & 0xc) == 0x8)) && !(ram_window.pg24.vvt_opt2 & 0x1))
            || (spkmode == 9)
            || (spkmode == 25)
            || (spkmode == 28)
            || (spkmode == 46)
            || (spkmode == 48)
            || (spkmode == 49)
            || (spkmode == 50)
            ) {
            unsigned int numvvt = ram_window.pg24.vvt_opt1 & 0x03;
            if (numvvt > 2) {
                numvvt = 4;
            }

            for (i = 0; i < numvvt; i++) {
                tmp_opt = ram_window.pg24.vvt_out[i] & 0x0f;
                if (tmp_opt) {
                    generic_hwpwm_setup(&port_vvt[i], 119, tmp_opt - 1,
                            ram_window.pg24.vvt_opt6 & 0x0f,
                            ram_window.pg24.vvt_out[i] & 0x80, i + 101);
                }
            }

            outpc.vvt_duty[3] = outpc.vvt_duty[2] = outpc.vvt_duty[1] = outpc.vvt_duty[0] = 0;

            if ((ram_window.pg24.vvt_opt1 & 0x03) > 1) {  // 2+ cams   
                int x, end, opt;
                if ((ram_window.pg24.vvt_opt1 & 0x03) == 2) {
                    end = 1;
                } else {
                    end = 3;
                }
                for (x = 0 ; x < end ; x++) {
                    unsigned char pol;
                    /* input capture polarity per cam input */
                    if (ram_window.pg24.vvt_opt2 & twopow[x + 4]) {
                        pol = 1;
                    } else {
                        pol = 2;
                    }
                    if ((x == 1) && ((ram_window.pg24.vvt_opt4 & 0x03) == 1)) { // cam3 in BMW V10 mode
                        pol = 3;
                    }
                    opt = ram_window.pg24.vvt_opt3 & (0x03 << (x * 2));
                    opt = opt >> (x * 2);
                    generic_timer_setup(123, opt, pol, 105); /* do the bulk of the setup */
                    if (!conf_err) {
                        if (opt == 0) {  // MS3X PT4 spare input
                            vvt_intx = (vvt_intx & 0xfc) | (x + 1); // TC4 = cam2
                            tim_mask_run |= 0x10;
                        } else if (opt == 1) {  // MS3X PT2 cam input
                            vvt_intx = (vvt_intx & 0xf3) | (( x + 1) << 2); // TC2 = cam2
                            tim_mask_run |= 0x04;
                        } else if (opt == 2) {  // JS10 PT5 cam input
                            vvt_intx = (vvt_intx & 0xcf) | ((x + 1) << 4); // TC5 = cam2
                            tim_mask_run |= 0x20;
                        } else if (opt == 3) {  // Datalog PT6 cam input
                            vvt_intx = (vvt_intx & 0x3f) | ((x + 1) << 6); // TC6 = cam2
                            tim_mask_run |= 0x40;
                        }
                    }
                }
            }
        } else {
            conf_err = 120;
        }

        if (ram_window.pg24.vvt_opt2 & 0x1) {
            vvt_ctl_pid_init();
        }
        outpc.vvt_ang[0] = ram_window.pg24.vvt_min_ang[0];
        outpc.vvt_ang[1] = ram_window.pg24.vvt_min_ang[1];
        outpc.vvt_ang[2] = ram_window.pg24.vvt_min_ang[2];
        outpc.vvt_ang[3] = ram_window.pg24.vvt_min_ang[3];
    }
/* end VVT */

/* TCLU */
    RPAGE = 0xfb; // HARDCODED page 24
    if (ram_window.pg24.tclu_outpin & 0x1f) {
        unsigned char tmp_opt;

        tmp_opt = ram_window.pg24.tclu_enablepin & 0x1f;
        if (tmp_opt) {
            generic_digin_setup(&port_tcluen, &pin_tcluen, NULL, 125, tmp_opt, 107); 
        } else {
            port_tcluen = &dummyReg;
            pin_tcluen = 0;
        }

        tmp_opt = ram_window.pg24.tclu_brakepin & 0x1f;
        if (tmp_opt) {
            generic_digin_setup(&port_tclubr, &pin_tclubr, NULL, 125, tmp_opt, 107); 
        } else {
            port_tclubr = &dummyReg;
            pin_tclubr = 0;
        }

        tmp_opt = ram_window.pg24.tclu_outpin & 0x3f;
        if (tmp_opt) { // on/off out
            generic_digout_setup(&port_tcluout, &pin_tcluout, 125, tmp_opt, 107);
        } else {
            port_tcluout = &dummyReg;
            pin_tcluout = 0;
        }

    }
/* end TCLU */

/* TC - traction control */
    if (ram5.tc_opt & 1) {
        unsigned char tmp_opt;

        tmp_opt = ram5.tc_enin & 0x1f;
        if (tmp_opt) {
            generic_digin_setup(&port_tcenin, &pin_tcenin, NULL, 126, tmp_opt, 108); 
        }

        if (ram5.tc_opt & 0x10) {
            tmp_opt = ram5.tc_knob & 0x1f;
            if (tmp_opt) {
                generic_adc_setup(&port_tc_knob, 126, tmp_opt, 108);
            }
        }
    }
/* end TC */
    
    /* RTC real time clock (internal) */
    if ((ram4.opt142 & 0x03) == 1) {
    // I2C init
    /* need config errors / port checking here too */
        if (portusage.k[1]) {
            conf_err = 124;
            conf_err_feat = portusage.k[1];
        } else if (portusage.k[3]) {
            conf_err = 124;
            conf_err_feat = portusage.k[3];
        } else {
            DDRK &= ~2; // PK1 input  SDA. Pullup already enabled earlier
            DDRK |= 8; // PK3 output SCL
            PORTK |= 8; //SCL high
        }
    }
    i2cstate = 0;
    i2cstate2 = 0;
    i2csubstate = 0;
    i2cbyte = 0;

    pin_knk = 0;
    pin_knk_match = 0;
    port_knk = &dummyReg;

    knock_state = 0;
    knock_gain = ram5.knock_gain[0] & 0x3f;
    knock_chan = 0;
    knock_retry = 0;

/* advanced knock input */
    if (ram4.knk_option & 0x03) {
        unsigned char tmp_opt;

        /* validate values in ram, enforce sanity */
        if (ram4.knk_step1 == 0) {
            ram4.knk_step1 = 30;
        }
        if (ram4.knk_step2 == 0) {
            ram4.knk_step2 = 10;
        }
        if (ram4.knk_step_adv == 0) {
            ram4.knk_step_adv = 10;
        }

        if ((ram4.knk_option & 0x0c) == 0x0c) {
            tmp_opt = 15; // JS11
        } else {
            tmp_opt = ram4.knk_pin_out & 0x1f;
        }

        if (tmp_opt) {
            if ((MONVER >= 0x390) && (MONVER <= 0x39f)) {
                DDRK |= 0x80;
                pin_knock_out = 0x80;
                port_knock_out = &PORTK; // hardcoded to PK7 for deriv#3
            } else {
                generic_digout_setup(&port_knock_out, &pin_knock_out, 4, tmp_opt, 12);
            }
        } else {
            port_knock_out = &dummyReg;
            pin_knock_out = 1; // to allow RTC code to run
        }            

        if ((ram4.knk_option & 0x0c) == 0x04) { // analogue input
            generic_adc_setup(&port_knock_in, 4, ram4.knkport_an & 0x0f , 12);
        } else if ((ram4.knk_option & 0x0c) == 0x0c) { // internal SPI input
            MODRR |= 0x10;
            if (portusage.m[3]) {
                conf_err = 132;
                conf_err_feat = portusage.m[3];
            } else if (portusage.m[4]) {
                conf_err = 132;
                conf_err_feat = portusage.m[4];
            } else if (portusage.m[5]) {
                conf_err = 132;
                conf_err_feat = portusage.m[5];
            } else {
                portusage.m[3] = 12;
                portusage.m[4] = 12;
                portusage.m[5] = 12;
                port_inits_PTM |= 0x08;  // default is on
                port_inits_PTM &= ~0x10; // default is off
                port_inits_PTM &= ~0x20; // default is off
                /* set the ports although SPI0 setup should override this anyway */
                DDRM &= ~0x04; // PM2 in
                PTM |= 0x08;
                DDRM |= 0x08 | 0x10 | 0x20; // PM3,4,5 out
            }

            // SPI0 set up as master, CPOL=0, CPHA=1, SS = bit bash
            SPI0BR = 0x44; // slow is 0x36
            SPI0CR1 = 0x56; // enabled but ints off. ~CS control
            SPI0CR2 = 0x10; // was 0x00
            knock_state = 255;
            if ((ram5.knock_conf & 0x80) && (no_triggers != num_cyl) && (spkmode != 26)  && (spkmode != 27)) {
                /* not enough position information to know which cylinder */
                conf_err = 133;
            }
        } else { // digi

            /* for some reason, this isn't using generic IO yet */

            unsigned char tmp_opt;
            tmp_opt = ram4.knk_port & 0x0f;
            // knock input pins
            //"INVALID", "Tableswitch", "PE0/JS7", "PE1", "JS10", "JS11", "JS5 (ADC6)", "JS4 (ADC7)", "Nitrous In", "Launch in", "Datalog In", "EXT_MAP (ADC11)", "EGO2 (ADC12)", "Spare ADC (ADC13)","INVALID","INVALID"
            if (tmp_opt == 1) {  // TableSwitch
                if (portusage.h[6]) {
                    conf_err = 4;
                } else {
                    portusage.h[6] = 12;
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERH |= 0x80;
                        PPSH &= ~0x80;
                    }
                    port_knk = pPTH;
                    pin_knk = 0x40;
                }
            } else if (tmp_opt == 2) {  // PE0/JS7
                if (portusage.e[0]) {
                    conf_err = 4;
                } else {
                    portusage.e[0] = 12;
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        // can't set PORTE pulls independantly
                    }
                    port_knk = pPTE;
                    pin_knk = 0x01;
                }
            } else if (tmp_opt == 3) { // PE1
                if (portusage.e[1]) {
                    conf_err = 4;
                } else {
                    portusage.e[1] = 12;
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        // can't set PORTE pulls independantly
                    }
                    port_knk = pPTE;
                    pin_knk = 0x02;
                }
            } else if (tmp_opt == 4) { // JS10
                if (portusage.t[5]) {
                    conf_err = 4;
                } else {
                    portusage.t[5] = 12;
                    DDRT &= ~0x20;          // set PTT5 as input
                    OCPD |= 0x20;           // disconnect from logic
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERT |= 0x20;
                        PPST &= ~0x20;
                    }
                    port_knk = pPTT;
                    pin_knk = 0x20;
                }
            } else if (tmp_opt == 5) { // JS11
                //JS11 was PA0 on MS2/Extra. Now connected to PJ7
                if (portusage.j[7]) {
                    conf_err = 4;
                } else {
                    portusage.j[7] = 12;
                    DDRJ &= ~0x80;  // configure as input
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERJ |= 0x80;
                        PPSJ &= ~0x80;
                    }
                    port_knk = pPTJ;
                    pin_knk = 0x80;
                }
            } else if (tmp_opt == 6) {   // option AD6/JS5
                if (portusage.ad0l[6]) {
                    conf_err = 4;
                } else {
                    portusage.ad0l[6] = 12;
                    DDRAD0L &= ~0x40;       // ensure pin AD06 is input
                    ATD0DIENL |= 0x40;
    // pullup only. XE doesn't do pulldown for some reason.
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERAD0L |= 0x40;
                    }
                    port_knk = pPTAD0L;
                    pin_knk = 0x40;
                }
            } else if (tmp_opt == 7) {                // default AD7/JS4
                if (portusage.ad0l[7]) {
                    conf_err = 4;
                } else {
                    portusage.ad0l[7] = 12;
                    DDRAD0L &= ~0x80;       // enable pin AD06 as digital input
                    ATD0DIENL |= 0x80;
    // pullup
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERAD0L |= 0x80;
                    }
                    port_knk = pPTAD0L;
                    pin_knk = 0x80;
                }
            } else if (tmp_opt == 8) { // Nitrous In 
                if (portusage.h[7]) {
                    conf_err = 4;
                } else {
                    portusage.h[7] = 12;
                    DDRH &= ~0x80;  // configure as input
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERH |= 0x80;
                        PPSH &= ~0x80;   // pull up
                    }
                    port_knk = pPTH;
                    pin_knk = 0x80;
                }
            } else if (tmp_opt == 9) {      //  PK2 Launch in
                if (portusage.k[2]) {
                    conf_err = 4;
                } else {
                    portusage.k[2] = 12;
                    DDRK &= ~0x04;
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        // can't set PORTK pulls independently
                    }
                    port_knk = pPTK;
                    pin_knk = 0x04;
                }
            } else if (tmp_opt == 10) {  // Datalog In
                if (portusage.t[6]) {
                    conf_err = 4;
                } else {
                    portusage.t[6] = 12;
                    OCPD |= 0x40;
                    DDRT &= ~0x40;
                    port_knk = pPTT;
                    pin_knk = 0x40;
                    if ((ram4.knk_option & 0x60) != 0x20) {
    #ifdef PULLUP_WITHOUT_MS3X
                        PERT |= 0x40;
                        PPST &= ~0x40;
    #endif
                    }
                }
            } else if (tmp_opt == 11) {   // option ADC11
                if (portusage.ad0h[3]) {
                    conf_err = 4;
                } else {
                    portusage.ad0h[3] = 12;
                    DDRAD0H &= ~0x08;       // ensure pin AD06 is input
                    ATD0DIENH |= 0x08;
    // pullup only. XE doesn't do pulldown for some reason.
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERAD0H |= 0x08;
                    }
                    port_knk = pPTAD0H;
                    pin_knk = 0x08;
                }
            } else if (tmp_opt == 12) {   // option ADC12
                if (portusage.ad0h[4]) {
                    conf_err = 4;
                } else {
                    portusage.ad0h[4] = 12;
                    DDRAD0H &= ~0x10;       // ensure pin AD06 is input
                    ATD0DIENH |= 0x10;
    // pullup only. XE doesn't do pulldown for some reason.
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERAD0H |= 0x10;
                    }
                    port_knk = pPTAD0H;
                    pin_knk = 0x10;
                }
            } else if (tmp_opt == 13) {   // option ADC13
                if (portusage.ad0h[5]) {
                    conf_err = 4;
                } else {
                    portusage.ad0h[5] = 12;
                    DDRAD0H &= ~0x20;       // ensure pin AD06 is input
                    ATD0DIENH |= 0x20;
    // pullup only. XE doesn't do pulldown for some reason.
                    if ((ram4.knk_option & 0x60) != 0x20) {
                        PERAD0H |= 0x20;
                    }
                    port_knk = pPTAD0H;
                    pin_knk = 0x20;
                }
            }

            if (ram4.knk_option & 0x10) {
                pin_knk_match = pin_knk; // high input
            } else {
                pin_knk_match = 0; // low input
            }
        }
    }

    /* CEL */
    pin_cel = 0;
    port_cel = &dummyReg;   
    if (ram5.cel_opt & 0x01) {
        unsigned char tmp_opt;
        tmp_opt = ram5.cel_port & 0x1f;
        if (tmp_opt) {
            generic_digout_setup(&port_cel, &pin_cel, 151, tmp_opt, 117);
            *port_cel |= pin_cel; // set it on now as lamp test. Code will turn it off shortly.
        }
    }

    /* make this the last one to allow ADC sharing */
    sensors_init();

/* --------------- all feature port settings above this line ----------------- */

    // LEDs use the old redirection method and only if unused (Stepper idle elsewhere)
    if (!portusage.m[3]) {
        pPTMpin3 = pPTM;
    }
    if (!portusage.m[4]) {
        pPTMpin4 = pPTM;
    }
    if (!portusage.m[5]) {
        pPTMpin5 = pPTM;
    }
    // turn off leds
    *pPTMpin3 &= ~0x08;       // squirt LED
    *pPTMpin4 &= ~0x10;       // acc LED
    *pPTMpin5 &= ~0x20;       // warmup LED

    /* Injector outputs */
    TIMOC7M = 0;
    TIMTTOV = 0;
    TIMTIOS = 0xff;
    TIMTCTL1 = 0;
    TIMTCTL2 = 0;
    TIMTCTL3 = 0;
    TIMTCTL4 = 0;
    TIMTSCR2 = 0x80;        // enabled timer overflow
    TIMOCPD = 0xff;
    TIMTIE = 0;
    TIMTSCR1 |= 0x80;
    TIMTFLG1 = 0xff;
    if (ram4.sequential & 0x3) {
        if ((ram4.sequential & SEQ_FULL) && (cycle_deg == 3600) && ((ram4.EngStroke & 0x01) == 0) && (spkmode != 26) && (spkmode != 27)) {
            conf_err = 103;
        }
    }
    if ((ram4.sequential & SEQ_FULL) && (!(ram4.hardware & HARDWARE_MS3XFUEL)) && (ram4.no_cyl > 2)) { // can only run full seq on V3 outputs for 1 + 2 cyl
        conf_err = 104;
    }

    if ((ram4.sequential & SEQ_SEMI) && (!(ram4.hardware & HARDWARE_MS3XFUEL))) {
        if ((ram4.no_cyl == 4) && (ram4.Divider != 1) && (!(ram4.Alternate & 1))) { // hopefully this will change to 1 sim instead
            conf_err = 102;
        }

        if ((ram4.no_cyl == 3) || (ram4.no_cyl > 4)) { // can only run semi seq on V3 outputs for 1, 2, 4 cyl
            conf_err = 105;
        }
    }

    if (ram4.tsw_pin_rf && pin_tsw_rf && (!(*port_tsw_rf & pin_tsw_rf))) {      // Reqfuel switching
        calc_reqfuel(ram4.ReqFuel_alt);
    } else {
        calc_reqfuel(ram4.ReqFuel);
    }

    // Check if monitor port inits match ours
    {
        unsigned int ad;
        unsigned char a = 0, *p;
        EPAGE = 0x1f;
        p = port_inits;

        if (*((unsigned char *) 0x822) == 0xff) {
            // First boot after a reflash, ignore any changes due to default tuning data as likely to be wrong
            // User can load their MSQ on this first boot without the pin states getting changed.
            // Change that flag to a zero, so next boot we do change the pin state data
            for (ad = 0x800; ad < 0x828; ad++) {
                *p++ = *((unsigned char *) ad);
            }
            port_inits[0x22] = 0x00;    // next time is normal boot
            a = 1;              // force the code to reburn the port inits
        } else {
            // normal boot
            for (ad = 0x800; ad < 0x828; ad++) {
                if (*((unsigned char *) ad) != *p++) {
                    a = 1;
                }
            }
        }

        if (a) {
            // rewrite the port init sector with new values
            unsigned int *src_ad, dest_ad;
            // interrupts not yet enabled
            DISABLE_INTERRUPTS;
            dest_ad = 0x7c00;
            // erase the flash sector
            if ((FSTAT & 0x30) != 0) {
                FSTAT = 0x30;  // clear ACCERR or FPVIOL if set
            }
            FCCOBIX = 0;
            FCCOBHI = 0x12;     // erase D-flash sector command
            FCCOBLO = 0x10;     // global address of all D-flash

            FCCOBIX = 1;
            FCCOB = dest_ad;    // 256 byte sector to erase. Same as EPAGE=0x1f, address = 0x800

            FSTAT = 0x80;       // initiate flash command
            while (!(FSTAT & 0x80)) {;
            };                  // wait until flash command completed

            src_ad = (unsigned int *) &port_inits[0];

            for (a = 0; a < 5; a++) {
                // now write in the values
                if ((FSTAT & 0x30) != 0) {
                    FSTAT = 0x30;      // clear ACCERR or FPVIOL if set
                }
                FCCOBIX = 0;
                FCCOBHI = 0x11; // prog D-flash
                FCCOBLO = 0x10; // global addr

                FCCOBIX = 1;
                FCCOB = dest_ad;        //global addr

                FCCOBIX = 2;
                FCCOB = *src_ad;
                src_ad++;       // (words)

                FCCOBIX = 3;
                FCCOB = *src_ad;
                src_ad++;       // (words)

                FCCOBIX = 4;
                FCCOB = *src_ad;
                src_ad++;       // (words)

                FCCOBIX = 5;
                FCCOB = *src_ad;
                src_ad++;       // (words)

                FSTAT = 0x80;   // initiate flash command
                while (!(FSTAT & 0x80)) {;
                };              // wait until flash command completed
                dest_ad += 8;
            }
        }
    }

    // enable global interrupts
    ENABLE_INTERRUPTS;

#if defined(MS3PRO)
    if ((MONVER < 0x380) || (MONVER > 0x383)) {
        conf_err = 68;
        goto SKIP_INIT;
    }
#else // MS3 or derivative #3
    if ( (!((MONVER >= 0x300) && (MONVER <= 0x308)))
        && (!((MONVER >= 0x390) && (MONVER <= 0x393))) ) {
        conf_err = 68;
        goto SKIP_INIT;
    }
#endif

    // Check input voltage - if too low wait around
    while ((ATD0DR4 < MIN_VOLTS) && (outpc.seconds < 4)) {;
    };
    if (ATD0DR4 < MIN_VOLTS) {
        conf_err = 67;
        goto SKIP_INIT;
    }

    // Prime Pulse - shoot 1 prime pulse of length PrimeP ms x 10
    start_clt = outpc.clt;
    set_prime_ASE();

    /* Set default port and pin for each injector output once
     */
    for (ix = 0; ix < NUM_TRIGS; ix++) {
        inj_cnt_xref[ix] = ix; // normal condition
        inj_chan[ix] = ix;
        inj_status[ix] = 0;
    }

    /* Set up injector port/pin mappings. Also called when test mode is cleared. */
    inj_event_array_portpin(0);
    if (do_dualouts) {
        inj_event_array_portpin(INJ_FILL_STAGED);
    }

    if ((num_inj > 8) && (!(ram4.sequential & 0x03)) && ((!(ram4.Alternate & 0x01)) || (!(ram4.Alternate & 0x02)))) {
        /* > 8 cyl non-seq MS3X must use Alternating  Alternating cranking (or not enough timers)*/
        conf_err = 121;
    }

    for (ix = 0; ix < NUM_TRIGS; ix++) {
         inj_events_b[ix].time = inj_events_a[ix].time = next_inj[ix].time = 0;
         inj_events_b[ix].tooth = inj_events_a[ix].tooth = next_inj[ix].tooth = 0;
         inj_events_b[ix].port = inj_events_a[ix].port = next_inj[ix].port;
         inj_events_b[ix].pin = inj_events_a[ix].pin = next_inj[ix].pin;
    }

    // FIXME need to consider dual fuel switching here and pick up alternate outputs is required
    if (PrimeP && (conf_err == 0) && (outpc.tps < ram4.TPSWOT)) {  // in flood clear don't prime
        if (PrimeP > 655) {
            outpc.pw1 = 65535;  // max is 65ms, so can't overflow
        } else {
            outpc.pw1 = PrimeP * 100;   // ticks
        }
        outpc.pw2 = outpc.pw1;
        // Turn on fuel pump
        *port_fp |= pin_fp;

        // schedule injection events
        if (ram4.hardware & HARDWARE_MS3XFUEL) {
            int delay = ram4.primedelay * 2000;
            delay += 1;
            /* start timers */
            for (ix = 0; ix < num_inj_pri; ix++) {
                seq_pw[ix] = outpc.pw1;
                inj_cnt[ix] = delay + (ix * 700); // phased start to reduce timer usage on > 8cyl engines ~32ms between each pulse (max 0.5s total priming)
            }
        } else {
            int delay = ram4.primedelay * 2000;
            delay += 1;
            inj_cnt[8] = delay; // start ASAP

            /* INJ2 prime pulse... seperated for staged injection */
            if (!(ram4.staged & 0x7)) {
                inj_cnt[9] = delay; // start ASAP
            }
            // turn on inj led
            *pPTMpin3 |= 0x08;
            outpc.squirt |= 0x03;   // both injectors squirting
        }
    } else {
        // ensure all fuel outputs are zero
        if (ram4.hardware & HARDWARE_MS3XFUEL) { // MS3X
            for (ix = 0; ix < num_inj_pri; ix++) {
                *next_inj[ix].port &= ~next_inj[ix].pin;
            }
        } else { // V3
            PORTT &= ~0x0a;
        }
    }

    if (ram4.staged & 0x7) {
        setup_staging();
    }

    if (ram4.RevLimOption & 0x8) {
        RevLimRpm1 = ram4.RevLimNormal1;
        RevLimRpm2 = ram4.RevLimNormal2;
    } else {
        RevLimRpm1 = ram4.RevLimTPSbypassRPM;
        RevLimRpm2 = RevLimRpm1 + ram4.RevLimRpm2;
    }

    if ((ram4.feature3 & 0x01) && (outpc.tps < 0)) {
            tps0_auto = outpc.tpsadc;
    }

// change over to single conversion sequence
    ATD0CTL2 &= ~0x40;          // turn off fast flag clearing
    ATD0CTL5 = 0x10;            // single sequence. Next sequence started in 0.128ms
    flagbyte6 |= FLAGBYTE6_DONEINIT;

/* 'user defined'                                                           *  
 * Here are some variables defined to help the new programmer get started   *
 * search for 'user defined' in ms3_main.c for more notes             *
 *                                                                          *
 * Initialisation                                                           */
    user_ulong = 0;
    user_uint = 0;
    user_uchar = 0;
/* end user defined section                                                 */

  SKIP_INIT:
    return;
}

/* end of main init function ************************************************/

void vss_init()
{
    unsigned char tmp_opt;

    /* VSS input 1 */
    port_vss1 = (unsigned char *)&dummyReg;
    pin_vss1 = 0;
    tmp_opt = ram4.vss1_an & 0x1f;
    if (tmp_opt) {         // analogue
        generic_adc_setup((volatile unsigned short**)&port_vss1, 58, tmp_opt, 60);
    } else if (ram4.vss_opt & 0x0f) {   // digital inputs
        unsigned int fdratio;

        tmp_opt = ram4.vss_opt & 0x0f;

        if ((ram4.vss_pos & 0x03) == 0x02) {
            /* pulses per mile */
            vss1_coeff = 3218688000UL / ram4.vss1_can_scale;
        } else if ((ram4.vss_pos & 0x03) == 0x03) {
            /* pulses per km */
            vss1_coeff = 2000000000UL / ram4.vss1_can_scale;
        } else {
            /* use ratios, wheel diameter etc. */
            // scaling factor to give m/s * 10 (required so rolling average works)
            vss1_coeff = 62811;

            if (ram4.vss_pos & 1) { // vss1 on driveline
                fdratio = ram4.fdratio1;
            } else {
                fdratio = 100;
            }
            vss1_coeff =
                (vss1_coeff * (unsigned long) ram4.wheeldia1) /
                ((unsigned long) fdratio *
                 (unsigned long) ram4.reluctorteeth1);
        }

        if (tmp_opt == 0x0f) {
            vss1_coeff = (vss1_coeff * 50L * (unsigned long)ram4.canpwm_clk) / (ram4.canpwm_pre * (ram4.canpwm_div + 1));
        }

        vss1_stall = VSS_STALL_TIMEOUT + 1;

        if (tmp_opt < 0x0e) {
            generic_digin_setup(&port_vss1, &pin_vss1, NULL, 58, tmp_opt, 60);
        }
//     (tmp_opt == 0x0e)  // uses CAN VSS value
//     (tmp_opt == 0x0f)  // uses CAN PWMin
    }

    /* VSS input 2 */
    port_vss2 = (unsigned char *)&dummyReg;
    pin_vss2 = 0;
    tmp_opt = ram4.vss2_an & 0x1f;
    if (tmp_opt) {         // analogue
        generic_adc_setup((volatile unsigned short**)&port_vss2, 58, tmp_opt, 60);
    } else if (ram4.vss_opt & 0xf0) {
        unsigned int fdratio;

        if ((ram4.vss_pos & 0x03) == 0x02) {
            /* pulses per mile */
            vss2_coeff = 3218688000UL / ram4.vss2_can_scale;
        } else if ((ram4.vss_pos & 0x03) == 0x03) {
            /* pulses per km */
            vss2_coeff = 2000000000UL / ram4.vss2_can_scale;
        } else {
            /* use ratios, wheel diameter etc. */
            // scaling factor to give m/s * 10 (required so rolling average works)
            vss2_coeff = 62811;

            if (ram4.vss_pos & 4) { // vss2 on driveline
                fdratio = ram4.fdratio1;
            } else {
                fdratio = 100;
            }
            vss2_coeff =
                (vss2_coeff * (unsigned long) ram4.wheeldia2) /
                ((unsigned long) fdratio *
                 (unsigned long) ram4.reluctorteeth2);
        }

        tmp_opt = (ram4.vss_opt & 0xf0) >> 4;
        if (tmp_opt == 0x0f) {
            vss2_coeff = (vss2_coeff * 50L * (unsigned long)ram4.canpwm_clk) / (ram4.canpwm_pre * (ram4.canpwm_div + 1));
        }

        vss2_stall = VSS_STALL_TIMEOUT + 1;

        if (tmp_opt < 0x0e) {
            generic_digin_setup(&port_vss2, &pin_vss2, NULL, 58, tmp_opt, 60);
        }
//     (tmp_opt == 0x0e)  // uses CAN VSS value
//     (tmp_opt == 0x0f)  // uses CAN PWMin
    }

    // Shaft speed input 1*/
    if (ram4.ss_opt & 0x07) {
        unsigned char tmp_opt;
        pin_ss1 = 0;
        ss1_coeff =
            (unsigned long) 1200000 / (unsigned long) ram4.reluctorteeth3;
        ss1_stall = VSS_STALL_TIMEOUT + 1;
        tmp_opt = ram4.ss_opt & 0x0f;
        if (tmp_opt < 0x0e) {
            generic_digin_setup(&port_ss1, &pin_ss1, NULL, 60, tmp_opt, 62);
        }
    }

    /* Shaft speed input 2 */
    if (ram4.ss_opt & 0x70) {
        unsigned char tmp_opt;
        pin_ss1 = 0;
        ss2_coeff =
            (unsigned long) 1200000 / (unsigned long) ram4.reluctorteeth4;
        ss2_stall = VSS_STALL_TIMEOUT + 1;
        tmp_opt = (ram4.ss_opt & 0xf0) >> 4;
        if (tmp_opt < 0x0e) {
            generic_digin_setup(&port_ss2, &pin_ss2, NULL, 61, tmp_opt, 63);
        }
    }

    port_vssout = (volatile unsigned char *) &dummyReg;
    pin_vssout = 0;
    if (ram4.vssout_opt & 0x1f) {
        generic_digout_setup(&port_vssout, &pin_vssout, 78, ram4.vssout_opt & 0x1f, 53);
        *port_vssout &= ~pin_vssout;        // turn it off
    }

    if ((ram4.gear_method & 0x03) == 1) {         // rpm/vss
        gear_scale = (unsigned int)((19098L * ram4.fdratio1) / (ram4.wheeldia1 * 10L));
    } else if ((ram4.gear_method & 0x03) == 2) {         // analogue
        unsigned char tmp_opt;
        port_gearsel = (unsigned short *)&dummyReg;
        tmp_opt = ram4.gear_port_an & 0x1f;
        if (tmp_opt) {         // analogue
            generic_adc_setup(&port_gearsel, 77, tmp_opt, 64);
        }
    }
    vss1_time = 0;
    vss2_time = 0;
    vss1_teeth = 0;
    vss2_teeth = 0;
    vss1_time_sum = 0;
    vss2_time_sum = 0;
}

void spr_port_init()
{
    unsigned int ix;

    portpins *portusage;
    portusage = init_portusage_addr; // place on stack - only called from init

/* Here after everything else has assigned pins we can try to assign spr_ports */
    // reset those pointers to pins which are to be used as alternate outputs
// ports 0 = PM, 1 = PJ, 2 = PP, 3 = PT, 4 = PA, 5 = PB, 6 = PK
    outpc.port_status = 0;
    RPAGE = tables[23].rpg; // need access to the tuning data in paged ram
    for (ix = 0; ix < NPORT; ix++) {
        if (ram_window.pg23.spr_port[ix]) {
            unsigned char pin, port, err_chk;
            port = spr_port_hw[0][ix];
            pin = spr_port_hw[1][ix];
            // pin output from normal function goes to dumy reg in ram
            err_chk = 255;
            if (port == 0) {
                err_chk = portusage->m[pin];
            } else if (port == 1) {
                err_chk = portusage->j[pin];
            } else if (port == 2) {
                err_chk = portusage->p[pin];
            } else if (port == 3) {
                err_chk = portusage->t[pin];
            } else if (port == 4) {
                err_chk = portusage->a[pin];
            } else if (port == 5) {
                err_chk = portusage->b[pin];
            } else if (port == 6) {
                err_chk = portusage->k[pin];
            } else if (port == 7) {
                err_chk = portusage->canout1[pin];
            } else if (port == 8) {
                err_chk = portusage->canout2[pin];
            }
            if (err_chk != 255) {
                if (err_chk) {
                    // pin already used - config error!
                    conf_err = 43;
                    conf_err_port = port;
                    conf_err_pin = pin;
                    conf_err_feat = err_chk;
                } else {
                    unsigned char mask;
                    mask = spr_port_hw[2][ix];
                    set_spr_port((char) ix, ram_window.pg23.init_val[ix]);
                    lst_pval[ix] = ram_window.pg23.init_val[ix];
                    if (port == 0) {
                        DDRM |= mask;
                    } else if (port == 1) {
                        DDRJ |= mask;
                    } else if (port == 2) {
                        DDRP |= mask;
                        PWME &= ~mask;
                    } else if (port == 3) {
                        DDRT |= mask;
                        OCPD |= mask;
                        if (pin == 1) { // PT1
                            PWME &= ~0x01;
                            DDRP |= 0x01;
                            PTP |= 0x01;
                        } else if (pin == 3) { // PT3
                            PWME &= ~0x02;
                            DDRP |= 0x02;
                            PTP |= 0x02;
                        }
                    } else if (port == 4) {
                        DDRA |= mask;
                    } else if (port == 5) {
                        DDRB |= mask;
                    } else if (port == 6) {
                        DDRK |= mask;
                    }

                    // mark pins used
                    if (port == 0) {
                        portusage->m[pin] = 9;
                    } else if (port == 1) {
                        portusage->j[pin] = 9;
                    } else if (port == 2) {
                        portusage->p[pin] = 9;
                    } else if (port == 3) {
                        portusage->t[pin] = 9;
                    } else if (port == 4) {
                        portusage->a[pin] = 9;
                    } else if (port == 5) {
                        portusage->b[pin] = 9;
                    } else if (port == 6) {
                        portusage->k[pin] = 9;
                    } else if (port == 7) {
                        portusage->canout1[pin] = 9;
                    } else if (port == 8) {
                        portusage->canout2[pin] = 9;
                    }
                }
            }
        }
    }
}

void var_init()
{
    // Initialize variables
    flocker = 0;
    pwcalc1 = 0;                // us
    outpc.pw1 = pwcalc1;
    pwcalc2 = 0;                // us
    outpc.pw2 = pwcalc2;
    outpc.adv_deg = 0;          // crank deg x 10
    coil_dur = ram4.max_coil_dur;       // msx10
    coil_dur_set = coil_dur * 100;      // us
    outpc.coil_dur = (int) (coil_dur_set / 100);        // msx10
    outpc.fuelload = 1000;      // kPa x 10
    outpc.ignload = 1000;
    last_tps = outpc.tps;       // % x 10
    last_map = outpc.map;       // kPa x 10
    outpc.afrtgt1 = 147;        // afr x 10
    outpc.afrtgt2 = outpc.afrtgt1;
    outpc.aircor = 1000;
    outpc.vecurr1 = 1000;
    outpc.vecurr2 = outpc.vecurr1;
    outpc.barocor = 1000;
    outpc.warmcor = 100;
    outpc.cold_adv_deg = 0;     // crank deg x 10
    outpc.wbo2_en1 = 1;
    outpc.wbo2_en2 = 1;
    outpc.egocor1 = 1000;
    outpc.egocor2 = 1000;
    outpc.egocor[0] = 1000;
    outpc.egocor[1] = 1000;
    outpc.egocor[2] = 1000;
    outpc.egocor[3] = 1000;
    outpc.egocor[4] = 1000;
    outpc.egocor[5] = 1000;
    outpc.egocor[6] = 1000;
    outpc.egocor[7] = 1000;
    outpc.tpsfuelcut = 100;
    outpc.gammae = 100;
    outpc.tpsaccel = 0;
    tcrank_done = 0xFFFF;
    tcold_pos = 0xFFFF;
    FPdcounter = 0;
    FSens_Pd = 0;
    last_fsensdat = 0;
    outpc.fuelcor = 100;        // %
    ffspkdel = 0;               // degx10
    bad_ego_flag = 0;
    idle_advance_timer = 0;
    idle_ve_timer = 0;
    if ((ram4.EAEOption == 1) || (ram4.EAEOption == 2)) {
        WF1 = 0;
        WF2 = 0;
        AWA1 = 0;
        AWA2 = 0;
        SOA1 = 0;
        SOA2 = 0;
    }

    if (ram4.EAEOption == 2) {
        EAEdivider = divider * 100;
        injtime_EAElagcomp = 0;
    }

    injtime = 0;
    inj1cntdown.time_32_bits = 0;
    inj2cntdown.time_32_bits = 0;

    sec_timer = 0;
    stall_timeout = 18750 / num_cyl;    // calculate stall timeout once
    fc_counter = 0xff;
    adc_ctr = 10; // 10ms
    lowres_ctr = 0;
    tacho_targ = 0;
    spk_cutx = 0;
    spk_cuty = 0;
    wheeldec_ovflo = 0;
    rtsci = 0;
    bl_timer = 0;
    n2o_act_timer = 0;
    n2o2_act_timer = 0;
    outpc.n2o_addfuel = 0;
    outpc.n2o_retard = 0;
    outpc.nitrous1_duty = 0;
    outpc.nitrous2_duty = 0;
    fc_off_time = 0xfe;
    datax1.adc[0] = 0;
    datax1.adc[1] = 0;
    datax1.adc[2] = 0;
    datax1.adc[3] = 0;
    datax1.adc[4] = 0;
    datax1.adc[5] = 0;
    datax1.adc[6] = 0;
    datax1.adc[7] = 0;

    datax1.adc[8] = 0;
    datax1.adc[9] = 0;
    datax1.adc[10] = 0;
    datax1.adc[11] = 0;
    datax1.adc[12] = 0;
    datax1.adc[13] = 0;
    datax1.adc[14] = 0;
    datax1.adc[15] = 0;

    datax1.adc[16] = 0;
    datax1.adc[17] = 0;
    datax1.adc[18] = 0;
    datax1.adc[19] = 0;
    datax1.adc[20] = 0;
    datax1.adc[21] = 0;
    datax1.adc[22] = 0;
    datax1.adc[23] = 0;
    /* bogus default date */
    datax1.rtc_sec = 0;
    datax1.rtc_min = 0;
    datax1.rtc_hour = 0;
    datax1.rtc_day = 0;
    datax1.rtc_date = 1;
    datax1.rtc_month = 1;
    datax1.rtc_year = 2000;
    flagbyte9 |= FLAGBYTE9_GETRTC; /* fetch ASAP */
    datax1.FuelAdj = 0;
    datax1.SpkAdj = 0;
    datax1.IdleAdj = 0;
    datax1.SprAdj = 0;

    outpc.sensors[0] = 0;
    outpc.sensors[1] = 0;
    outpc.sensors[2] = 0;
    outpc.sensors[3] = 0;
    outpc.sensors[4] = 0;
    outpc.sensors[5] = 0;
    outpc.sensors[6] = 0;
    outpc.sensors[7] = 0;
    outpc.sensors[8] = 0;
    outpc.sensors[9] = 0;
    outpc.sensors[10] = 0;
    outpc.sensors[11] = 0;
    outpc.sensors[12] = 0;
    outpc.sensors[13] = 0;
    outpc.sensors[14] = 0;
    outpc.sensors[15] = 0;

    outpc.syncreason = 0;

    sd_phase = 0;
    outpc.sd_status = 0;
    sd_block = 0;
    sd_log_addr = 0x1000;
    sd_int_cmd = 0;
    sd_int_phase = 0;
    sd_lmms_last = 0;
    sd_lmms_last2 = 0;
    sd_stream_avg = 0x8000;
    outpc.stream_level = 0;
    sd_match_mask = 0xff;

    launch_timer = 0x7ffe;
    nitrous_timer = 0x7ffe;
    tb_timer = 0x7ffe;
    mmsDiv = 0;
    mmsDivn2o = 0;
    mmsDivwi = 0;
    outpc.water_duty = 0;
    water_pw = 0;
    water_pw_cnt = 0;
    outpc.duty_pwm[0] = 0;
    outpc.duty_pwm[1] = 0;
    outpc.duty_pwm[2] = 0;
    outpc.duty_pwm[3] = 0;
    outpc.duty_pwm[4] = 0;
    outpc.duty_pwm[5] = 0;

    cum_cycle = 0;              // cumulative cycle (fuel tooth) and tooth counters
    cum_tooth = 0;
    vss_time = 0;
    {
        unsigned int ix;
        for (ix = 0; ix < NUM_TRIGS ; ix++) {
            spk_trim[ix] = 0;
            fuel_trim[ix] = 0;
            dwl[ix] = 0;
        }
    }
    spk_trim_cnt = 0;
    fuel_trim_cnt = 0;
    accxyz_time = 0;
    sens_time = 10;
    shift_cut_phase = 0;
    shift_cut_timer = 0;
    log_offset = 0;
    tc_addfuel = 0;
    perfect_timer = 0;
    sliptimer = 0;
    mapsample_time = 0;
    tpssample_time = 0;
    als_timing = 0;
    als_addfuel = 0;
    als_timer = 0;
    als_iacstep = 32000;
    timer_usage = 0xff; // all inj timers free
    vvt_inj_timing_adj = 0;
    tclu_state = 0;
    tclu_timer = 0;
    knocksample_time = 0;
    flowsum[0] = 0;
    flowsum_accum[0] = 0;
    flowsum[1] = 0;
    flowsum_accum[1] = 0;
    spiwb_timer1 = 0;
    spiwb_timer2 = 0;
    spiwb_errcnt1 = 0;
    spiwb_errcnt2 = 0;
    map_deadman = 0;
    spk_crk_targ = 0;
    dwl_crk_targ = 0;
    xgspkq[0].sel = 0;
    xgspkq[0].cnt = 0;
    xgspkq[1].sel = 0;
    xgspkq[1].cnt = 0;
    xgdwellq[0].sel = 0;
    xgdwellq[0].cnt = 0;
    xgdwellq[1].sel = 0;
    xgdwellq[1].cnt = 0;
    ltt_but_state = 0;
    ltt_fl_state = 0;
    ltt_but_debounce = 0;
    tps_ring_cnt = 0;
    maplog_cnt = 0;
    maplog_max = 1;
    testmode_glob = 0;
    iactest_glob = 0;
    maxdwl = 255;
    rdwl[0] = 0;
    rdwl[1] = 0;
    rdwl[2] = 0;
    rdwl[3] = 0;
    rdwl[4] = 0;
    rdwl[5] = 0;
    rdwl[6] = 0;
    rdwl[7] = 0;
    tc_addfuel = 0;
    tc_nitrous = 100;
    tc_boost = 100;
    tc_boost_duty_delta = 0;
    flagbyte10 &= ~FLAGBYTE10_TC_N2O;
    adc_lmms = 0;
    srl_err_cnt = 0;
}

void sensors_init()
{
    int ix, s;
    unsigned char er;
    if (ram4.opt142 & 0x04) {
        er = 0; // allow ADC sharing
    } else {
        er = 118;
    } 
    for (ix = 0; ix <= 15 ; ix++) {
        s = ram5.sensor_source[ix] & 0x1f;
        if (s) {
            generic_adc_setup(&port_sensor[ix], er, s, 85 + s);
        }
    }
}

void pinport_init()
{
/* set all ports and pins to safe locations */
    port_stream = (unsigned short*)&dummyReg;
    port_launch_var = (unsigned short*)&dummyReg;
    mapport = (unsigned short*)&dummyReg;
    port_egt[0] = (unsigned short*)&dummyReg;
    port_egt[1] = (unsigned short*)&dummyReg;
    port_egt[2] = (unsigned short*)&dummyReg;
    port_egt[3] = (unsigned short*)&dummyReg;
    port_egt[4] = (unsigned short*)&dummyReg;
    port_egt[5] = (unsigned short*)&dummyReg;
    port_egt[6] = (unsigned short*)&dummyReg;
    port_egt[7] = (unsigned short*)&dummyReg;
    port_egt[8] = (unsigned short*)&dummyReg;
    port_egt[9] = (unsigned short*)&dummyReg;
    port_egt[10] = (unsigned short*)&dummyReg;
    port_egt[11] = (unsigned short*)&dummyReg;
    mafport = (unsigned short*)&dummyReg;
    port_sensor[0] = (unsigned short*)&dummyReg;
    port_sensor[1] = (unsigned short*)&dummyReg;
    port_sensor[2] = (unsigned short*)&dummyReg;
    port_sensor[3] = (unsigned short*)&dummyReg;
    port_sensor[4] = (unsigned short*)&dummyReg;
    port_sensor[5] = (unsigned short*)&dummyReg;
    port_sensor[6] = (unsigned short*)&dummyReg;
    port_sensor[7] = (unsigned short*)&dummyReg;
    port_sensor[8] = (unsigned short*)&dummyReg;
    port_sensor[9] = (unsigned short*)&dummyReg;
    port_sensor[10] = (unsigned short*)&dummyReg;
    port_sensor[11] = (unsigned short*)&dummyReg;
    port_sensor[12] = (unsigned short*)&dummyReg;
    port_sensor[13] = (unsigned short*)&dummyReg;
    port_sensor[14] = (unsigned short*)&dummyReg;
    port_sensor[15] = (unsigned short*)&dummyReg;
    port_gearsel = (unsigned short*)&dummyReg;
    baroport = (unsigned short*)&dummyReg;
    egoport[0] = (unsigned short*)&dummyReg;
    egoport[1] = (unsigned short*)&dummyReg;
    egoport[2] = (unsigned short*)&dummyReg;
    egoport[3] = (unsigned short*)&dummyReg;
    egoport[4] = (unsigned short*)&dummyReg;
    egoport[5] = (unsigned short*)&dummyReg;
    egoport[6] = (unsigned short*)&dummyReg;
    egoport[7] = (unsigned short*)&dummyReg;
    accXport = (unsigned short*)&dummyReg;
    accYport = (unsigned short*)&dummyReg;
    accZport = (unsigned short*)&dummyReg;

    port_tacho = &dummyReg;
    port_idleonoff = &dummyReg;
    port_sdled = &dummyReg;
    port_sdbut = &dummyReg;
    port_launch = &dummyReg;
    port_maxafr = &dummyReg;
    port_n2oin = &dummyReg;
    port_n2o1n = &dummyReg;
    port_n2o1f = &dummyReg;
    port_n2o2n = &dummyReg;
    port_n2o2f = &dummyReg;
    port_tsw_rf = &dummyReg;
    port_tsw_afr = &dummyReg;
    port_tsw_stoich = &dummyReg;
    port_boost_tsw = &dummyReg;
    port_tsf = &dummyReg;
    port_tss = &dummyReg;
    port_3step = &dummyReg;
    port_idlepwm = &dummyReg;
    port_idlepwm3 = &dummyReg;
    port_vss1 = &dummyReg;
    port_vss2  = &dummyReg;
    port_ss1 = &dummyReg;
    port_ss2 = &dummyReg;
    port_wipump = &dummyReg;
    port_wivalve = &dummyReg;
    port_wiin = &dummyReg;
    port_vssout = &dummyReg;
    port_ac_out = &dummyReg;
    port_ac_in = &dummyReg;
    port_fanctl_out  = &dummyReg;
    port_shift_cut_in = &dummyReg;
    port_shift_cut_out = &dummyReg;
    port_pwm[0] = &dummyReg;
    port_pwm[1] = &dummyReg;
    port_pwm[2] = &dummyReg;
    port_pwm[3] = &dummyReg;
    port_pwm[4] = &dummyReg;
    port_pwm[5] = &dummyReg;
    port_dualfuel = &dummyReg;
    port_tsw_ob = &dummyReg;
    port_knk = &dummyReg;
    port_alsin = &dummyReg;
    port_alsout = &dummyReg;
    port_knock_out = &dummyReg;
    port_knock_in = (unsigned short*)&dummyReg;
    port_ltt_but = &dummyReg;
    port_ltt_led = &dummyReg;

    pin_tacho = 0;
    pin_idleonoff = 0;
    pin_sdled = 0;
    pin_sdbut = 0;
    pin_launch = 0;
    pin_maxafr = 0;
    pin_n2oin = 0;
    pin_match_n2oin = 0;
    pin_n2o1n = 0;
    pin_n2o1f = 0;
    pin_n2o2n = 0;
    pin_n2o2f = 0;
    pin_tsw_rf = 0;
    pin_tsw_afr = 0;
    pin_tsw_stoich = 0;
    pin_boost_tsw = 0;
    pin_tsf = 0;
    pin_tss = 0;
    pin_3step = 0;
    pin_idlepwm = 0;
    pin_idlepwm3 = 0;
    pin_vss1 = 0;
    pin_vss2 = 0;
    pin_ss1 = 0;
    pin_ss2 = 0;
    pin_wipump = 0;
    pin_wivalve = 0;
    pin_wiin = 0;
    pin_match_wiin = 0;
    pin_vssout = 0;
    pin_ac_out = 0;
    pin_ac_in = 0;
    pin_fanctl_out = 0;
    pin_shift_cut_in = 0;
    pin_shift_cut_match = 0;
    pin_shift_cut_out = 0;
    pin_pwm[0] = 0;
    pin_pwm[1] = 0;
    pin_pwm[2] = 0;
    pin_pwm[3] = 0;
    pin_pwm[4] = 0;
    pin_pwm[5] = 0;
    pin_dualfuel = 0;
    pin_tsw_ob = 0;
    pin_knk = 0;
    pin_knk_match  = 0;
    pin_alsin = 0;
    pin_alsout = 0;
    pin_alsout = 0;
    pin_tcluen = 0;
    pin_tclubr = 0;
    pin_tcluout = 0;
    pin_knock_out = 0;
    pin_ltt_but = 0;
    pin_ltt_led = 0;
}

