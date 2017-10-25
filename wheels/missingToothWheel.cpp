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
#include "ignWheel.h"

void MissingToothWheel::init()
{
        unsigned char err_count;
        unsigned char trig_tooth, trig_missing, miss_teeth;
        unsigned char base_teeth_per_trig = 1, base_trig_tooth, teeth_per_trig[8], odd_trig_teeth[8];
        signed int odd_trig_ang[8], base_trig_ang, oddf_ang;

        /* ----------------------  DIZZY MODE or Fuel only ------------------------ */
        if (((spkmode & 0xfe) == 2) || (spkmode == 31)) {  // dizzy (options 2,3) + fuel only
            teeth_per_trig[0] = 1;
            teeth_per_trig[1] = 1;
            teeth_per_trig[2] = 1;
            teeth_per_trig[3] = 1;
            odd_trig_ang[0] = 0;
            odd_trig_ang[1] = 0;
            odd_trig_ang[2] = 0;
            odd_trig_ang[3] = 0;
            if (num_cyl & 1) {
                no_triggers = num_cyl;  // (for 3, 5 cyl)
            } else if ((spkmode == 2) && ((ram4.spk_conf2 & 0x06) == 0x06)) {
                no_triggers = num_cyl;  // sig-PIP TFI
                cycle_deg = 7200;
                flagbyte5 |= FLAGBYTE5_CRK_DOUBLE;
            } else {
                no_triggers = num_cyl >> 1;     // even no. cylinders
            }

            trig_ang = ram4.adv_offset;
            if ((spkmode == 3) && (trig_ang < 200)) {
                conf_err = 9; // next-cyl trigret not allowed
            }

        /*********** oddfire dizzy + fuel  ********/
            if (ram4.ICIgnOption & 0x8) {
                no_triggers = num_cyl;  // oddfire needs to trigger on each one
                no_teeth = no_triggers;
                last_tooth = no_teeth;
                tmp_deg_per_tooth = (((unsigned int) 14400 / (char) num_cyl));  // degx10 between tooth 1 & 3
                for (j = 1; j <= no_teeth; j++) {
                    unsigned int jtmp1;
                    if ((j & 1) == 0) {        // these might be transposed
                        jtmp1 = ram4.OddFireang;        // short gap
                    } else {
                        jtmp1 = tmp_deg_per_tooth - ram4.OddFireang;    // long gap
                    }
                    deg_per_tooth[j - 1] = jtmp1;
                }

                if (ram4.OddFireang < (tmp_deg_per_tooth - ram4.OddFireang)) {
                    smallest_tooth_crk = ram4.OddFireang;
                } else {
                    smallest_tooth_crk = tmp_deg_per_tooth - ram4.OddFireang;
                }
                smallest_tooth_cam = 0;

        /*********** even dizzy (incl trigger return) ***********/
            } else {
                if ((ram4.no_cyl & 0x1f) == 1) {
                    no_triggers = 2;
                    if (((ram4.EngStroke & 1) == 0) && ((ram4.spk_mode3 & 0xc0) == 0x80)) {
                        tmp_deg_per_tooth = 7200; // 1 cyl 4 stroke COP (unusual)
                    } else {
                        tmp_deg_per_tooth = 3600; // normal 1 cyl arrangement
                        if (ram4.EngStroke & 1) { // 2-stroke
                            no_triggers = 1;
                        }
                    }
                } else if ((ram4.no_cyl & 0x1f) == 2) {
                    no_triggers = 2;
                    tmp_deg_per_tooth = 3600; // normal 2 cyl arrangement in this mode
                } else { // normal calc
                    tmp_deg_per_tooth =
                    (((unsigned int) 7200 / (char) num_cyl));
                }
                no_teeth = no_triggers;
                last_tooth = no_triggers;
                for (j = 1; j <= no_triggers; j++) {
                    deg_per_tooth[j - 1] = tmp_deg_per_tooth;
                }
            }

            smallest_tooth_crk = tmp_deg_per_tooth;
            smallest_tooth_cam = 0;
            if (spkmode == 3) {
                flagbyte5 |= FLAGBYTE5_CRK_DOUBLE;
            }


            /* ----------------------  EVEN WHEEL MODE ------------------------ */
        } else {                // real wheel mode
            if ( /*((ram4.spk_mode3 & 0xc0) == 0x80)        // COP selected  breaks 2 stroke COP
                || */((ram4.spk_config & 0x0c) == 0x0c)   //  or 2nd trig+missing
                || (((ram4.spk_config & 0x0c) == 0x04) && (ram4.spk_config & 0x02))     //  or cam speed missing tooth wheel
                || (((ram4.spk_config & 0x0c) == 0x08) && ((ram4.spk_config & 0xc0) == 0x00))  //  or non missing dual wheel with cam speed 2nd trig
                ) {
                no_triggers = num_cyl;  // same triggers as COP i.e. cover 720 crank degrees
                cycle_deg = 7200;
            } else { // triggers cover 360 crank degrees
                if (ram4.EngStroke & 0x01) {
                    no_triggers = num_cyl;     // For 2-stroke or rotary = no. cyl/rotors
                } else {
                    no_triggers = num_cyl >> 1; // for 4-stroke it is half
                }
            }
            // we also need to calc the no. teeth we see - do this a little lower

            if (((ram4.no_cyl & 0x1f) == 2) && (ram4.EngStroke & 1) && ((ram4.spk_mode3 & 0xc0) == 0x80)) { // if 2 cyl 2 stroke COP
                no_triggers = 2; // likely not needed due to above changes
            } else {
                // check COP validity
                if (((ram4.spk_mode3 & 0xc0) == 0x80) && (!(ram4.EngStroke & 1))) {       // if COP and 4-stroke then need 720 deg of info
                    if (((ram4.spk_config & 0xc) == 0x8)
                        && ((ram4.spk_config & 0xc0) == 0x40)) {
                        conf_err = 32;      // dual wheel and COP not allowed with only crank speed 2nd trigger (no phase info)
                    } else if (((ram4.spk_config & 0xc) == 0x4)
                               && ((ram4.spk_config & 2) == 0)) {
                        conf_err = 38;      // single wheel and COP not allowed at crank speed (no phase info)
                    }
                }
            }

            // using cam ?
            if ((ram4.spk_config & 0x08) == 0x08) {     // dual wheel or 2nd trig+missing
                flagbyte5 |= FLAGBYTE5_CAM;
                // rising+falling option removed
            }

            /* Use MAP sensor in lieu of cam sensor ? */
            if ((ram4.hardware & 0xc0) == 0x80) {
                if ((ram4.no_cyl > 2) || (!((ram4.spk_config & 0x0c) == 0x0c))
                    || (!((ram4.spk_mode3 & 0xe0) == 0x60))
                    || (ram4.mapsample_opt & 0x04)) {
                    /* only valid for 1,2 cyl with dual+missing and WCOP and windowed MAP */
                    conf_err = 152;
                } else {
                    int mt;
                    flagbyte5 &= ~FLAGBYTE5_CAM; /* not actually using hardware input */
                    /* calculate ADC threshold */
                    mt = ram5.map_phase_thresh - ram4.map0;
                    if (mt < 0) {
                        mt = 0;
                    }
                    mapadc_thresh = (mt * 1023UL) / (ram4.mapmax - ram4.map0);
                }
            }

            // Do selective division and check remainder for config error

            if (ram4.No_Teeth == 32) {
                // special case
                tmp_deg_per_tooth = 1125; // centi-degrees
                trig_ang = 10 * (ram4.Miss_ang + tmp_offset);
            } else {
            /* Now figure out how many degrees there are per tooth */
            /* only for even wheels, for uneven use special mode*/

        	    if (ram4.spk_config & 0x02) { // cam or crank speed
            		tmp_deg_per_tooth = (unsigned int)(((unsigned int)7200 / (unsigned char)ram4.No_Teeth));
                    if (7200 % ram4.No_Teeth) {
                        conf_err = 1;
                        return;
                    }
        	    } else {
            		tmp_deg_per_tooth = (unsigned int)(((unsigned int)3600 / (unsigned char)ram4.No_Teeth));
                    if (3600 % ram4.No_Teeth) {
                        conf_err = 1;
                        return;
                    }
        	    }
                trig_ang = ram4.Miss_ang + tmp_offset;
            }

            if ((ram4.spk_config & 0xc) == 0x8) {       // dual wheel
                if (ram4.spk_config & 0x02) {   // cam speed
                    if ((ram4.spk_config & 0xc0) == 0x40) {
                        no_teeth = ram4.No_Teeth >> 1;  //  2nd trig every crank rev
                    } else if ((ram4.spk_config & 0xc0) == 0x80) {
                        no_teeth = ram4.No_Teeth / num_cyl;  //  2nd trig every ignition event
                    } else {
                        no_teeth = ram4.No_Teeth;       // 2nd trig every cam rev
                    }
                } else {        // crank speed
                    if ((ram4.spk_config & 0xc0) == 0x40) {
                        no_teeth = ram4.No_Teeth;       //  2nd trig every crank rev
                    } else if ((ram4.spk_config & 0xc0) == 0x80) {
                        no_teeth = ram4.No_Teeth / (num_cyl >> 1);  //  2nd trig every ignition event
                    } else {
                        no_teeth = ram4.No_Teeth << 1;  // 2nd trig every cam rev
                    }
                }
                miss_teeth = 0;
            } else {
                if ((ram4.spk_config & 0xc) == 0xc) {
                    /* missing + extra */
                    no_teeth = ram4.No_Teeth << 1;
                } else {
                    no_teeth = ram4.No_Teeth;
                }
                miss_teeth = ram4.No_Miss_Teeth;
            }

            if ((no_teeth > MAXNUMTEETH) || (ram4.No_Teeth > MAXNUMTEETH)) {
                conf_err = 141;
                return;
            }

            base_teeth_per_trig = no_teeth / no_triggers;
            if ((no_teeth % no_triggers) && (num_cyl != 7)) { /* special case for 7 cyl in a bit */
                conf_err = 1;
            }
            // store degrees per tooth
            if ((ram4.spk_config & 0x0c) == 0x08) {
                last_tooth = no_teeth;  // if 2nd trig only then ignore missing teeth field

                /* Fill in array of tooth sizes for even wheels */
                for (j = 0; j < no_teeth; j++) {
                    deg_per_tooth[j] = tmp_deg_per_tooth;
                }

            } else {
                mid_last_tooth = ram4.No_Teeth - ram4.No_Miss_Teeth;
                last_tooth = no_teeth - ram4.No_Miss_Teeth;

                /* Fill in array of tooth sizes for even wheels */
                for (j = 0; j < mid_last_tooth; j++) {
                    deg_per_tooth[j] = tmp_deg_per_tooth;
                }
                deg_per_tooth[mid_last_tooth - 1] =
                    tmp_deg_per_tooth * (1 + ram4.No_Miss_Teeth);
                deg_per_tooth[mid_last_tooth] = 0;      // tooth doesn't exist

                if (no_teeth != ram4.No_Teeth) {
                    // must be 2nd + missing, go around again
                    for (j = ram4.No_Teeth; j < last_tooth; j++) {
                        deg_per_tooth[j] = tmp_deg_per_tooth;
                    }
                    deg_per_tooth[last_tooth - 1] =
                        tmp_deg_per_tooth * (1 + ram4.No_Miss_Teeth);
                }
            }
        }

        /* ------------------ common to dizzy and wheel ------------------- */

        // Now setup trigger teeth
        /* figure out trigger angle... get as close to -10 as possible.
         * calcs will count back from this tooth to get spark/dwell
         */


        trig_tooth = 1;
        while (trig_ang > -100) {       // 10ATDC
            trig_ang -= deg_per_tooth[trig_tooth - 1];
            trig_tooth++;
            if (trig_tooth > no_teeth) {
                trig_tooth = 1;
            }
        }

        if (ram4.ICIgnOption & 0x8) {   // oddfire
            if ((spkmode == 2) || (spkmode == 31)) { // oddfire distributor or fuel only
                teeth_per_trig[0] = 1;
                odd_trig_ang[0] = 0;
                teeth_per_trig[1] = 1;
                odd_trig_ang[1] = ram4.OddFireang;
                teeth_per_trig[2] = 1;
                odd_trig_ang[2] = 0;
                teeth_per_trig[3] = 1;
                odd_trig_ang[3] = ram4.OddFireang;
            } else {
                unsigned short odd_trig_offset;
                unsigned char odd_trig_tooth_offset = 0;

                if (ram4.No_Teeth == 32) {
                    // special case
                    oddf_ang = ram4.OddFireang * 10;
                } else {
                    oddf_ang = ram4.OddFireang;
                }

                odd_trig_tooth_offset = oddf_ang / tmp_deg_per_tooth;
                odd_trig_offset = oddf_ang % tmp_deg_per_tooth;

                if ((ram4.spk_conf2 & 0x18) == 0x10) { // oddfire paired (like vmax)
                    odd_trig_ang[0] = 0;
                    teeth_per_trig[0] = base_teeth_per_trig;
                    odd_trig_ang[1] = 0;
                    teeth_per_trig[1] = odd_trig_tooth_offset;
                    odd_trig_ang[1] = 0;
                    teeth_per_trig[2] = base_teeth_per_trig;
                    odd_trig_ang[2] = odd_trig_offset;
                    teeth_per_trig[3] = (base_teeth_per_trig << 1) - odd_trig_tooth_offset;
                    odd_trig_ang[3] = odd_trig_offset;

                } else if ((ram4.spk_conf2 & 0x18) == 0x00) { // alternating (more normal)
                    teeth_per_trig[0] = odd_trig_tooth_offset;
                    odd_trig_ang[0] = 0;
                    teeth_per_trig[1] = (base_teeth_per_trig << 1) - odd_trig_tooth_offset;
                    odd_trig_ang[1] = odd_trig_offset;
                    teeth_per_trig[2] = teeth_per_trig[0];
                    odd_trig_ang[2] = odd_trig_ang[0];
                    teeth_per_trig[3] = teeth_per_trig[1];
                    odd_trig_ang[3] = odd_trig_ang[1];

                } else { // custom user supplied angles
                    int oddfang_mult;

                    if ((ram5.oddfireangs[0] + ram5.oddfireangs[1]
                        + ram5.oddfireangs[2] + ram5.oddfireangs[3]) != 7200) {
                        conf_err = 149;
                    }

                    if (ram4.No_Teeth == 32) {
                        oddfang_mult = 10;
                    } else {
                        oddfang_mult = 1;
                    }

                    oddf_ang = ram5.oddfireangs[0] * oddfang_mult;
                    odd_trig_tooth_offset = oddf_ang / tmp_deg_per_tooth;
                    odd_trig_offset = oddf_ang % tmp_deg_per_tooth;

                    teeth_per_trig[0] = odd_trig_tooth_offset;
                    odd_trig_ang[1] = odd_trig_offset;

                    oddf_ang = ram5.oddfireangs[1] * oddfang_mult;
                    odd_trig_tooth_offset = oddf_ang / tmp_deg_per_tooth;
                    odd_trig_offset = oddf_ang % tmp_deg_per_tooth;

                    teeth_per_trig[1] = odd_trig_tooth_offset;
                    odd_trig_ang[2] = odd_trig_offset;

                    oddf_ang = ram5.oddfireangs[2] * oddfang_mult;
                    odd_trig_tooth_offset = oddf_ang / tmp_deg_per_tooth;
                    odd_trig_offset = oddf_ang % tmp_deg_per_tooth;

                    teeth_per_trig[2] = odd_trig_tooth_offset;
                    odd_trig_ang[3] = odd_trig_offset;

                    oddf_ang = ram5.oddfireangs[3] * oddfang_mult;
                    odd_trig_tooth_offset = oddf_ang / tmp_deg_per_tooth;
                    odd_trig_offset = oddf_ang % tmp_deg_per_tooth;

                    teeth_per_trig[3] = odd_trig_tooth_offset;
                    odd_trig_ang[0] = 0; // by definition;
                }
            }
        } else { // wholly even
            teeth_per_trig[0] = base_teeth_per_trig;
            teeth_per_trig[1] = base_teeth_per_trig;
            teeth_per_trig[2] = base_teeth_per_trig;
            teeth_per_trig[3] = base_teeth_per_trig;
            odd_trig_ang[0] = 0;
            odd_trig_ang[1] = 0;
            odd_trig_ang[2] = 0;
            odd_trig_ang[3] = 0;
        }

        if (num_cyl == 7) {
            int remainder, base_remainder, z, t = 0;
            /* common */
            teeth_per_trig[4] = base_teeth_per_trig;
            teeth_per_trig[5] = base_teeth_per_trig;
            teeth_per_trig[6] = base_teeth_per_trig;
            teeth_per_trig[7] = 0;
            odd_trig_ang[0] = 0; /* first one 'even' */
            odd_trig_teeth[0] = 0;
            odd_trig_ang[7] = 0;
            if ((ram4.EngStroke & 1) == 0) {
                /* 4 stroke */
                /* angle per event = 102.857 degrees (720.000/7) */
                base_remainder = 1028 % tmp_deg_per_tooth;
                remainder = 0;
                /* second and beyond might be over the next tooth */ 
                for (z = 1; z < 7; z++) {
                    remainder += base_remainder;
                    if (remainder > tmp_deg_per_tooth) {
                        t++;
                        remainder -= tmp_deg_per_tooth;
                    }
                    odd_trig_ang[z] = -remainder;
                    odd_trig_teeth[z] = t;
                    }
            } else {
                /* 2 stroke not supported*/
                conf_err = 1;
            }
        } else {
            // For all other cases, double up pattern due to (j & 7)
            teeth_per_trig[4] = teeth_per_trig[0];
            teeth_per_trig[5] = teeth_per_trig[1];
            teeth_per_trig[6] = teeth_per_trig[2];
            teeth_per_trig[7] = teeth_per_trig[3];
            odd_trig_ang[4] = odd_trig_ang[0];
            odd_trig_ang[5] = odd_trig_ang[1];
            odd_trig_ang[6] = odd_trig_ang[2];
            odd_trig_ang[7] = odd_trig_ang[3];
            odd_trig_teeth[0] = 0;
            odd_trig_teeth[1] = 0;
            odd_trig_teeth[2] = 0;
            odd_trig_teeth[3] = 0;
            odd_trig_teeth[4] = 0;
            odd_trig_teeth[5] = 0;
            odd_trig_teeth[6] = 0;
            odd_trig_teeth[7] = 0;
        }

        /* at this point I have my 1st trigger tooth, and my trigger angle,
         * so I'll go ahead and set the rest of my trigger teeth
         */

        err_count = 0;
        base_trig_tooth = trig_tooth;
        base_trig_ang = trig_ang;

      WHEEL_TOOTH_CALC:
        trig_tooth = base_trig_tooth;
        trig_missing = 0;

        for (j = 0; j < no_triggers; j++) {

            trig_angs[j] = trig_ang + odd_trig_ang[j & 7];
            trigger_teeth[j] = trig_tooth + odd_trig_teeth[j & 7];
        
            // check if we landed on a gap 
            if (mid_last_tooth && (trig_tooth > mid_last_tooth) && (trig_tooth <= ram4.No_Teeth)) {
                trig_missing = ram4.No_Teeth - trig_tooth + 1;
            } else if ((trig_tooth > last_tooth) && (trig_tooth <= no_teeth)) {
                trig_missing = no_teeth - trig_tooth + 1;
            } else if (trig_tooth > no_teeth) {
                trig_tooth -= no_teeth;
            }

            if ((trig_tooth > last_tooth) && (trig_tooth <= no_teeth)) {
                /* not good, need to adjust all teeth... set a flag
                 * to tell us to do that later
                 */
                trig_missing = no_teeth - trig_tooth + 1;
            } else if (trig_tooth > no_teeth) {
                trig_tooth -= no_teeth;
            }
            trig_tooth += teeth_per_trig[j & 7];  // for next time around loop
            if (trig_tooth > no_teeth) {
                trig_tooth -= no_teeth;
            }

        }

        if (trig_missing) {
            err_count++;
            if (err_count > ram4.No_Teeth) {
                conf_err = 25; // failed to calc teeth
                return;
            }

            trig_ang = base_trig_ang - (tmp_deg_per_tooth * trig_missing);
            base_trig_tooth += trig_missing;
            if (base_trig_tooth > no_teeth) {
                base_trig_tooth -= no_teeth;
            }
            goto WHEEL_TOOTH_CALC; // try again
        }

        if (ram4.No_Teeth == 32) {
            // special case for 32 teeth
            //now convert back to deci-degrees
            trig_ang /= 10;
            for (j = 0; j < no_triggers; j++) {
                trig_angs[j] /= 10;
            }
            for (j = 0; j < no_teeth; j++) {
                unsigned int tmp_ang;
                tmp_ang = deg_per_tooth[j-1];
                // pattern will be 11.2, 11.3 etc.
                if ((j & 1) & (tmp_ang % 10)) {
                    tmp_ang = (tmp_ang / 10) + 1;
                } else {
                    tmp_ang = tmp_ang / 10;
                }
                deg_per_tooth[j-1] = tmp_ang;
            }
            tmp_deg_per_tooth = 113;
        }

        smallest_tooth_crk = tmp_deg_per_tooth;
        if (ram4.spk_config & 0x08) {   // dual or dual+missing
            if ((ram4.spk_config & 0xc0) == 0x40) {
                smallest_tooth_cam = 3600;      // crank speed
            } else if ((ram4.spk_config & 0xc0) == 0x80) {
                smallest_tooth_cam = 7200 / num_cyl; // every ignition event
            } else {
                smallest_tooth_cam = 7200;      // cam speed
            }
        }
}

