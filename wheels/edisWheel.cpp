/* $Id: ms3_ign_wheel.c,v 1.118.2.2 2013/06/01 17:22:54 jsmcortina Exp $
 * Copyright 2007, 2008, 2009, 2010, 2011, 2012 James Murray and Kenneth Culver
 *
 * This file is a part of Megasquirt-3.
 *
 * ign_wheel_init()
    Origin: Kenneth Culver
    Major: James Murray / Kenneth Culver
    Majority: James Murray / Kenneth Culver
 *
 * You should have received a copy of the code LICENSE along with this source, please
 * ask on the www.msextra.com forum if you did not.
 *
*/

#include "ms3.h"
#include "edisWheel.h"

unsigned int EdisWheel::init()
{
        unsigned int j=0;
            trig_ang = 100; // 10BTDC - needs confirmation
        if (((ram4.spk_mode3 & 0xc0) == 0x80) || (ram4.sequential & SEQ_FULL)) { // use cam
            no_triggers = num_cyl;
            flagbyte5 |= FLAGBYTE5_CAM;
            cycle_deg = 7200;
        } else {
            no_triggers = num_cyl >> 1;
        }

        no_teeth = no_triggers;
        last_tooth = no_triggers;

        tmp_deg_per_tooth = (((unsigned int) 7200 / (char) num_cyl));
        for (j = 1; j <= no_triggers; j++) {
            deg_per_tooth[j - 1] = tmp_deg_per_tooth;
        }

        for (j = 0; j < no_triggers; j++) {
            trig_angs[j] = 100;
            trigger_teeth[j] = j + 1;
        }

        smallest_tooth_crk = tmp_deg_per_tooth;
        smallest_tooth_cam = 0;

        return j;
}

