#ifndef AT_DEFINITIONS_H
#define AT_DEFINITIONS_H
// I hate AT commands

#define AT_PREFIX "AT"

// commands strings
#define ATC_ECHO     "E"
#define ATC_CPIN     "+CPIN"
#define ATC_CMEE     "+CMEE"
#define ATC_CMGF     "+CMGF"
#define ATC_CMGD     "+CMGD"
#define ATC_CNMP     "+CNMP"
#define ATC_CMNB     "+CMNB"
#define ATC_CGDCONT  "+CGDCONT"
#define ATC_COPS     "+COPS"
#define ATC_CNACT    "+CNACT"
#define ATC_CASSLCFG "+CASSLCFG"
#define ATC_CACFG    "+CACFG"
#define ATC_CAOPEN   "+CAOPEN"
#define ATC_CACLOSE  "+CACLOSE"
#define ATC_CASEND   "+CASEND"
#define ATC_CARECV   "+CARECV"
#define ATC_CAACK    "+CAACK"
#define ATC_CASTATE  "+CASTATE"

// responses
#define ATR_OK        "OK"
#define ATR_ERROR     "ERROR"
#define ATR_CME_ERROR "+CME ERROR:"

// state responses
// These are the responses we parse to track modem state
#define ATR_APP_PDP   "+APP PDP:"
#define ATR_CAOPEN    "+CAOPEN:"
#define ATR_CACLOSE   "+CACLOSE:"
#define ATR_CASTATE   "+CASTATE:"
#define ATR_NPD       "NORMAL POWER DOWN"

#endif // AT_DEFINITIONS_H
