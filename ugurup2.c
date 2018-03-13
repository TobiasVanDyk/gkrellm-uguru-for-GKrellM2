/* ugurup2.c	- requires GKrellM 2.0.0 or better */
/***************************************************************************
 *   2005 - Tobias van Dyk                                                 *
 *   vdykt@icon.co.za                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 // Based on
 /***************************************************************************
                          main.c  -  Read uGuru sensors
                             -------------------
    begin                : mån feb 28 12:08:43 CET 1972
    copyright            : (C) 2005 by olle sandberg
    email                : ollebull@gmail.com
 ***************************************************************************/
 /**************************************************************************
  demo3.c from http://members.dslextreme.com/users/billw/gkrellm/Plugins.html
 ***************************************************************************/


#include <gkrellm2/gkrellm.h>
#include <utime.h>
#include <dirent.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <asm/io.h>

#define DATA_BUFLEN 80

  /* CONFIG_NAME would be the name in the configuration tree, but this demo
  |  does not have a config.  See demo1.c for that.
  */
#define	CONFIG_NAME	"ugurup2"

  /* STYLE_NAME will be the theme subdirectory for custom images for this
  |  plugin and it will be the gkrellmrc style name for custom settings.
  */
#define	STYLE_NAME	"ugurup2"


// Two i/o-ports are used by uGuru
#define PORT_DATA 0xE4	// Mostly used to check if uGuru is busy
#define PORT_CMD 0xE0	// Used to tell uGuru what to read and read the actual data

// Every sensor down to the RPM are placed in bank 1.
// Temp sensors
#define SENS_CPUTEMP		0x2100     		// 255C max
#define SENS_SYSTEMP		0x2101		// 255C max
#define SENS_PWMTEMP		0x210F		// 255C max
// Voltage sensors
#define SENS_VCORE		0x2103		// 3.49V max
#define SENS_DDRVDD		0x2104		// 3.49V max
#define SENS_DDRVTT		0x210A		// 3.49V max
#define SENS_NBVDD		0x2108		// 3.49V max
#define SENS_SBVDD		0x210E		// 3.49V max
#define SENS_HTV			0x2102		// 3.49V max
#define SENS_AGP			0x2109		// 3.49V max
#define SENS_5V			0x2106		// 6.25V max
#define SENS_3V3			0x2105		// 4.36V max
#define SENS_5VSB		0x210B		// 6.25V max
#define SENS_3VDUAL		0x210D	   	// 4.36V max
// Fans. These are placed at bank 2
#define SENS_CPUFAN	    	0x2600		// 15300RPM max
#define SENS_NBFAN	   	0x2601		// 15300RPM max
#define SENS_SYSFAN	   	0x2602		// 15300RPM max
#define SENS_AUXFAN1    	0x2603		// 15300RPM max
#define SENS_AUXFAN2    	0x2604		// 15300RPM max

// Multipliers used to convert from the 8bit value to what the
// sensors should display. No multiplier for temps,
#define MLTP_3V49   		3.49f/255
#define MLTP_4V36   		4.36f/255
#define MLTP_6V25   		6.25f/255
#define MLTP_FAN    		15300/255
 

static GkrellmMonitor *monitor;
static GkrellmPanel   *panel;
static GkrellmTicks   *pGK;
static GkrellmDecal *text1_decal, *text2_decal, *text3_decal, *text4_decal;


gchar data1[DATA_BUFLEN], data2[DATA_BUFLEN], data3[DATA_BUFLEN], data4[DATA_BUFLEN];

static gint     style_id;

// Return TRUE if uGuru is detected
int uGuru_Detect(void);
// Put uGuru in ready state. uGuru will hold 0x08 at Data port and 0xAC at Command
// port after this.
int uGuru_Ready(void);
// End uGuru session
void uGuru_End(void);
// Read a sensor
unsigned char uGuru_ReadSensor(unsigned short SensorID);

// Return TRUE if uGuru is detected
int uGuru_Detect(void)
{
  	unsigned char portData, portCommand;
   
	// Get i/o acces to the uGuru ports
	ioperm(PORT_CMD,1,TRUE);
	ioperm(PORT_DATA,1,TRUE);

	// Read the ports
	portData = inb(PORT_DATA);
	portCommand =  inb(PORT_CMD);
	
	// ***** Detect uGuru
	// After a reboot uGuru will hold 0x00 at data and 0xAC at Command. When this program
	// have end the ports will hold 0x08 at Data and 0xAC at Command, that's why we need to test both
	// combinations at the Data port.
	if( ( (portData == 0x00 ) || portData == 0x08 ) && (portCommand == 0xAC ) )
	{
	  	// uGuru was detected so put uGuru in ready state and return TRUE.
		uGuru_Ready();	
		return TRUE;			
	}
	// uGuru was not detected
	{
	uGuru_Ready();
	return TRUE;
	}
}


// Put uGuru in ready state. uGuru will hold 0x08 at Data port and 0xAC at Command port after this.
int uGuru_Ready(void)
{
	int LockupCnt1 = 0;
	int LockupCnt2 = 0;
	
	// Wait until uGuru is in ready-state
	// The loop shouldn't be needed to execut more then one time
	while(inb(PORT_DATA) != 0x08)
	{	
		outb(0x00,PORT_DATA);            		// after 0x00 is written to Data port

		// Wait until 0x09 is read at Data port
		while( inb(PORT_DATA) != 0x09)
		{
		  	// Prevent a lockup
		  	if(LockupCnt2++ > 1000)
		   		return FALSE;
		}
		
		LockupCnt2 = 0;
		
		// Wait until 0xAC is read at Cmd port
 		while(inb(PORT_CMD) != 0xAC)
   		{
	  		// Prevent a lockup
	  		if(LockupCnt2++ > 1000)
		   		return FALSE;
		}

		// Prevent a lockup
		if(LockupCnt1++ > 1000)
		   		return FALSE;
	}
	
	return TRUE;
}

// Read a sensor
unsigned char uGuru_ReadSensor(unsigned short SensorID)
{
	unsigned char SensorResult;
	unsigned char Port_hAddr, Port_lAddr;
	int LockupCnt = 0;
	
	// Get the high and low byte of address
	Port_hAddr = ((0xFF00 & SensorID)>>8);
	Port_lAddr = (0x00FF & SensorID);
	
	// Ask to read from uGuru
	outb(Port_hAddr,PORT_DATA);				// Out BankID @ Data

	// I guess this is to se if uGuru is ready to take a command
	while(inb(PORT_DATA) != 0x08)				// In 0x08 @ Data
	{
	  	// Prevent a lockup
	  	if(LockupCnt++ > 1000)
		  	return FALSE;
	}
	// Whitch sensor to read?
	outb(Port_lAddr,PORT_CMD);				// Out Sensor ID @ Cmd

	LockupCnt = 0;
	// Wait until uGuru is ready to be read
	while(inb(PORT_DATA) != 0x01)
	{
	  	// Prevent a lockup
	  	if(LockupCnt++ > 1000)
		  	return FALSE;
	}
	
	// Read the sensor
	SensorResult = inb(PORT_CMD);

	// Put the chip in ready state
	uGuru_Ready();

	// Return the result of the sensor
	return SensorResult;

}


static gint
panel_expose_event(GtkWidget *widget, GdkEventExpose *ev)
	{
	gdk_draw_pixmap(widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
			ev->area.width, ev->area.height);
	return FALSE;
	}


static void
update_plugin()
	{
	
	unsigned char CPUtemp, SYStemp, PWMtemp;
    	float Vcore, DDRVdd, DDRVtt, NBVdd, SBVdd, HTVdd, AGPVdd, Vdd5V, Vdd3V3, Vdd5VSB, Vdd3VDual;
    	unsigned int CPUfan, NBfan, SYSfan, AUXfan1, AUXfan2;

	//if ((pGK->timer_ticks % 2) == 0)
	if (pGK->two_second_tick)
    	     {
		CPUtemp = uGuru_ReadSensor(SENS_CPUTEMP);
		SYStemp = uGuru_ReadSensor(SENS_SYSTEMP);
		PWMtemp = uGuru_ReadSensor(SENS_PWMTEMP);
		// Voltages
		Vcore = uGuru_ReadSensor(SENS_VCORE) * MLTP_3V49;
		DDRVdd = uGuru_ReadSensor(SENS_DDRVDD) * MLTP_3V49;
		DDRVtt = uGuru_ReadSensor(SENS_DDRVTT) * MLTP_3V49;
		NBVdd = uGuru_ReadSensor(SENS_NBVDD) * MLTP_3V49;
		SBVdd = uGuru_ReadSensor(SENS_SBVDD) * MLTP_3V49;
		HTVdd = uGuru_ReadSensor(SENS_HTV) * MLTP_3V49;
		AGPVdd = uGuru_ReadSensor(SENS_AGP) * MLTP_3V49;
		Vdd5V = uGuru_ReadSensor(SENS_5V) * MLTP_6V25;
		Vdd3V3 = uGuru_ReadSensor(SENS_3V3) * MLTP_4V36;
		Vdd5VSB = uGuru_ReadSensor(SENS_5VSB) * MLTP_6V25;
		Vdd3VDual = uGuru_ReadSensor(SENS_3VDUAL) * MLTP_4V36;
		// Fans
		CPUfan = uGuru_ReadSensor(SENS_CPUFAN) * MLTP_FAN;
		NBfan = uGuru_ReadSensor(SENS_NBFAN) * MLTP_FAN;
		SYSfan = uGuru_ReadSensor(SENS_SYSFAN) * MLTP_FAN;
		AUXfan1 = uGuru_ReadSensor(SENS_AUXFAN1) * MLTP_FAN;
		AUXfan2 = uGuru_ReadSensor(SENS_AUXFAN2) * MLTP_FAN;
        	
    		sprintf (data1,"cpu %d %d\n",CPUtemp,CPUfan);
		sprintf (data2,"nbg %d %d\n",SYStemp,NBfan);
		sprintf (data3,"sys %d %d\n",PWMtemp,SYSfan);
		sprintf (data4,"psu %.2f %.2f\n",Vdd5V,Vcore);
		
		gkrellm_draw_decal_text(panel, text1_decal, data1, -1);
		gkrellm_draw_decal_text(panel, text2_decal, data2, -1);
		gkrellm_draw_decal_text(panel, text3_decal, data3, -1);
		gkrellm_draw_decal_text(panel, text4_decal, data4, -1);
		
		gkrellm_draw_panel_layers(panel);
	
	     }
			
	}


static void
create_plugin(GtkWidget *vbox, gint first_create)
	{
	GkrellmStyle	*style;
	GkrellmTextstyle *ts, *ts_alt;
	int i;

	if (first_create) panel = gkrellm_panel_new0();

	style = gkrellm_meter_style(style_id);

	/* Each Style has two text styles.  The theme designer has picked the
	|  colors and font sizes, presumably based on knowledge of what you draw
	|  on your panel.  You just do the drawing.  You can assume that the
	|  ts font is larger than the ts_alt font.
	*/
	ts = gkrellm_meter_textstyle(style_id);
	ts_alt = gkrellm_meter_alt_textstyle(style_id);


	/* Create a text decal */
	text1_decal = gkrellm_create_decal_text(panel, "Ay", ts, style,
				-1,     /* x = -1 places at left margin */
				-1,     /* y = -1 places at top margin	*/
				-1);    /* w = -1 makes decal the panel width minus margins */
	
	text2_decal = gkrellm_create_decal_text(panel, "Ay", ts, style,
				-1,     /* x = -1 places at left margin */
				32,     /* y = 28 places at top margin-28	*/
				-1);    /* w = -1 makes decal the panel width minus margins */
	text3_decal = gkrellm_create_decal_text(panel, "Ay", ts, style,
				-1,     /* x = -1 places at left margin */
				49,     /* y = 42 places at top margin-42	*/
				-1);    /* w = -1 makes decal the panel width minus margins */
	text4_decal = gkrellm_create_decal_text(panel, "Ay", ts, style,
				-1,     /* x = -1 places at left margin */
				66,     /* y = 64 places at top margin-64	*/
				-1);    /* w = -1 makes decal the panel width minus margins */
	
	/* Configure the panel to hold the above created decals, and create it.	*/
	gkrellm_panel_configure(panel, NULL, style);
	gkrellm_panel_create(vbox, monitor, panel);

	
	if (first_create)
	    g_signal_connect(G_OBJECT (panel->drawing_area), "expose_event",
    	        G_CALLBACK(panel_expose_event), NULL);
		
	// Set up uGuru
    	uGuru_Detect() ;
	
	for (i = 0; i < 20; i++) {
             data1[i] = data2[i] = data3[i] = data4[i] = ' '; }
	data1[79] = data2[79] = data3[79] = data4[79] = '\0'; 
	
	}


/* The monitor structure tells GKrellM how to call the plugin routines.
*/
static GkrellmMonitor	plugin_mon	=
	{
	CONFIG_NAME,        /* Name, for config tab.    */
	0,                  /* Id,  0 if a plugin       */
	create_plugin,      /* The create function      */
	update_plugin,      /* The update function      */
	NULL,               /* The config tab create function   */
	NULL,               /* Apply the config function        */

	NULL,               /* Save user config   */
	NULL,               /* Load user config   */
	NULL,               /* config keyword     */

	NULL,               /* Undefined 2  */
	NULL,               /* Undefined 1  */
	NULL,               /* private      */

	MON_MAIL,           /* Insert plugin before this monitor */

	NULL,               /* Handle if a plugin, filled in by GKrellM     */
	NULL                /* path if a plugin, filled in by GKrellM       */
	};


  /* All GKrellM plugins must have one global routine named init_plugin()
  |  which returns a pointer to a filled in monitor structure.
  */

GkrellmMonitor *
gkrellm_init_plugin(void)
{
	pGK = gkrellm_ticks();
	/* If this call is made, the background and krell images for this plugin
	|  can be custom themed by putting bg_meter.png or krell.png in the
	|  subdirectory STYLE_NAME of the theme directory.  Text colors (and
	|  other things) can also be specified for the plugin with gkrellmrc
	|  lines like:  StyleMeter  STYLE_NAME.textcolor orange black shadow
	|  If no custom themeing has been done, then all above calls using
	|  style_id will be equivalent to style_id = DEFAULT_STYLE_ID.
	*/
	style_id = gkrellm_add_meter_style(&plugin_mon, STYLE_NAME);
	monitor = &plugin_mon;
	return &plugin_mon;
	}
