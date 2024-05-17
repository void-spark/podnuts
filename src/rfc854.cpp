#include "rfc854.h"
/* See rfc854 */

bool RFC854::isUSASCIIGraphic( unsigned char c )
{
   return c >= 32 && c <= 126;
}

bool RFC854::isUSASCIIControl( unsigned char c )
{
   return c <= 31 || c == 127;
}

bool RFC854::isUSASCIIUncovered( unsigned char c )
{
   return c >= 128;
}

/*
 *    The NVT printer has an unspecified carriage width and page length
 *    and can produce representations of all 95 USASCII graphics (codes
 *    32 through 126).  Of the 33 USASCII control codes (0 through 31
 *    and 127), and the 128 uncovered codes (128 through 255), the
 *    following have specified meaning to the NVT printer:
 */

/*
 * Name    : NULL (NUL)
 * meaning : No Operation
 */
const unsigned char RFC854::NUL = 0;

/*
 * Name    : Line Feed (LF)
 * meaning : Moves the printer to the
 *           next print line, keeping the
 *           same horizontal position.
 */
const unsigned char RFC854::LF = 10;

/*
 * Name    : Carriage Return (CR)
 * meaning : Moves the printer to the left
 *           margin of the current line.
 */
const unsigned char RFC854::CR = 13;

/*
 * In addition, the following codes shall have defined, but not
 * required, effects on the NVT printer.  Neither end of a TELNET
 * connection may assume that the other party will take, or will
 * have taken, any particular action upon receipt or transmission
 * of these:
 */

/*
 * Name    : BELL (BEL)
 * meaning : Produces an audible or
 *           visible signal (which does
 *           NOT move the print head).
 */
const unsigned char RFC854::BEL = 7;

/*
 * Name    : Back Space (BS)
 * meaning : Moves the print head one
 *           character position towards
 *           the left margin.
 */
const unsigned char RFC854::BS = 8;

/*
 * Name    : Horizontal Tab (HT)
 * meaning : Moves the printer to the
 *           next horizontal tab stop.
 *           It remains unspecified how
 *           either party determines or
 *           establishes where such tab
 *           stops are located.
 */
const unsigned char RFC854::HT = 9;

/*
 * Name    : Vertical Tab (VT)
 * meaning : Moves the printer to the
 *           next vertical tab stop.  It
 *           remains unspecified how
 *           either party determines or
 *           establishes where such tab
 *           stops are located.
 */
const unsigned char RFC854::VT = 11;

/*
 * Name    : Form Feed (FF)
 * meaning : Moves the printer to the top
 *           of the next page, keeping
 *           the same horizontal position.
 */
const unsigned char RFC854::FF = 12;


/*   All TELNET commands consist of at least a two byte sequence:  the
 *   "Interpret as Command" (IAC) escape character followed by the code
 *   for the command.  The commands dealing with option negotiation are
 *   three byte sequences, the third byte being the code for the option
 *   referenced.  This format was chosen so that as more comprehensive use
 *   of the "data space" is made -- by negotiations from the basic NVT, of
 *   course -- collisions of data bytes with reserved command values will
 *   be minimized, all such collisions requiring the inconvenience, and
 *   inefficiency, of "escaping" the data bytes into the stream.  With the
 *   current set-up, only the IAC need be doubled to be sent as data, and
 *   the other 255 codes may be passed transparently.
 *
 *   The following are the defined TELNET commands.  Note that these codes
 *   and code sequences have the indicated meaning only when immediately
 *   preceded by an IAC.
 */

/*
 * Name    : SE
 * meaning : End of subnegotiation parameters.
 */
const unsigned char RFC854::SE = 240;

/*
 * Name    : NOP
 * meaning : No operation.
 */
const unsigned char RFC854::NOP = 241;

/*
 * Name    : Data Mark
 * meaning : The data stream portion of a Synch.
 *           This should always be accompanied
 *           by a TCP Urgent notification.
 */
const unsigned char RFC854::DM = 242;

/*
 * Name    : Break
 * meaning : NVT character BRK.
 */
const unsigned char RFC854::BREAK = 243;

/*
 * Name    : Interrupt Process
 * meaning : The function IP.
 */
const unsigned char RFC854::IP = 244;

/*
 * Name    : Abort output
 * meaning : The function AO.
 */
const unsigned char RFC854::AO = 245;

/*
 * Name    : Are You There
 * meaning : The function AYT.
 */
const unsigned char RFC854::AYT = 246;

/*
 * Name    : Erase character
 * meaning : The function EC.
 */
const unsigned char RFC854::EC = 247;

/*
 * Name    : Erase Line
 * meaning : The function EL.
 */
const unsigned char RFC854::EL = 248;

/*
 * Name    : Go ahead
 * meaning : The GA signal.
 */
const unsigned char RFC854::GA = 249;

/*
 * Name    : SB
 * meaning : Indicates that what follows is
 *           subnegotiation of the indicated
 *           option.
 */
const unsigned char RFC854::SB = 250;

/*
 * Name    : WILL (option code)
 * meaning : Indicates the desire to begin
 *           performing, or confirmation that
 *           you are now performing, the
 *           indicated option.
 */
const unsigned char RFC854::WILL = 251;

/*
 * Name    : WON'T (option code)
 * meaning : Indicates the refusal to perform,
 *           or continue performing, the
 *           indicated option.
 */
const unsigned char RFC854::WONT = 252;

/*
 * Name    : DO (option code)
 * meaning : Indicates the request that the
 *           other party perform, or
 *           confirmation that you are expecting
 *           the other party to perform, the
 *           indicated option.
 */
const unsigned char RFC854::DO = 253;

/*
 * Name    : DON'T (option code)
 * meaning : Indicates the demand that the
 *           other party stop performing,
 *           or confirmation that you are no
 *           longer expecting the other party
 *           to perform, the indicated option.
 */
const unsigned char RFC854::DONT = 254;

/*
 * Name    : IAC
 * meaning : Data Byte 255.
 */
const unsigned char RFC854::IAC = 255;
