/* 
 * File:   commands.h
 * Author: marcus
 *
 * Created on 3. Mai 2015, 16:26
 */

#ifndef COMMANDS_H
#define	COMMANDS_H

/*
** Kanalwert setzen
** Send: 'A' <channel> <value> <checksum>
*/

#define I2CCMD_SETCHANNEL        'A'
#define I2CCMD_GETCHANNEL        'J'
/*
** Setzt eine Szene für einen Taster
**
**  'B' <keycode> <scene> <checksum>
*/

#define I2CCMD_SETSCENEFORKEY    'B'

/*
** Holt die für einen Taster gesetzte Szene
**
**  'C' <keycode> -- <checksum>
*/

#define I2CCMD_GETSCENEFORKEY     'C'

/*
** Setzt den Wert eines Kanals in einer Szene
**
** 'D' <scene> <channel> <value> <transparancy> <dimtime> <checksum>
** wobei: transparency : 0 -- kanal wird nicht verändert
**                       1 -- kanal wird verwendet
*/

#define I2CCMD_SETSCENECHANNELVALUE 'D'

/*
** Liefert den Wert eines Kanals in einer Szene
**
** 'E' <scene> <channel> -- -- -- <checksum>
** wobei: transparency : 0 -- kanal wird nicht verändert
**                       1 -- kanal wird verwendet
** Reveive: <value> <transparency> <dimtime>
*/

#define I2CCMD_GETSCENECHANNELVALUE 'E'

/*
** Szene anfahren
**
** 'F' <scene> <checksum>
*/

#define I2CCMD_CALLSCENE          'F'

/*
** Setzt eine Szene auf einen Wert
** 'G' <scene> <value> <checksum>
*/

#define I2CCMD_SETSCENELEVEL   'G'


/*
** Liefert den Code des letzten gedrückten tasters
** Send:     'H' -- <checksum>
** Receive:  <keycode>
*/

#define I2CCMD_GETLASTKEY        'H'

/*
** Holt die Aktuell eingestellte Szene
**
**  Send   : 'I' -- <checksum>
**  Receive: <scene>
*/

#define I2CCMD_GETCURRENTSCENE    'I'


/*
** Gibt eine Information, ob die Eingestellten Kanalwerte der Aufgerufenen szene entsprechen.
*/

#define I2CCMD_GETSCENECHANGE     'K'

//#define SPICMD_SETSCENETRANSITIONTIME 'L'
//#define SPICMD_GETSCENETRANSITIONTIME 'M'

//#define SPICMD_DUMPBYTE 'O'   // oscar

//#define SPICMD_PREPMEM 'P'   //  den flashspeicher mit mustern beschreiben

//#define SPICMD_GETTESTPATTERN     'T'


#define I2CCMD_BLACKOUT            'Q'
#define I2CCMD_CHANNELMASK         'R'
#define I2CCMD_STORECURRENTSETTING 'S'

#define I2CCMD_GETLIVECHANNELVALUE 'V'
#define I2CCMD_GETCHANNELMASK      'W'

/*
 *    'a' - Buttonnumber - scene - mode - checksum
 */

//#define I2CCMD_SETBUTTONCONFIG      'a'

/*
 *  'b' - buttonnumber - checksum // return: 'b' - buttonnumber - scene - mode - checksum
 */

//#define I2CCMD_GETBUTTONCONFIG      'b'

/*
 * '#' - pagenumber - checksum // 256 bytes - checksum
 */


#define I2CCMD_GETMEMORYBLOCK       '#'   

#define I2CCMD_SETSCENECHANNELLEVEL         '1'
#define I2CCMD_SETSCENECHANNELDIMSPEED      '2' 
#define I2CCMD_SETSCENECHANNELTRANSPARENCY  '3'


/*
** Defaultwert
*/

//#define SPICMD_NONE        '\0'




#endif	/* COMMANDS_H */

