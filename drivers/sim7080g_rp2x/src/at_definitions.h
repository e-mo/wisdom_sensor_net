#ifndef AT_DEFINITIONS_H
#define AT_DEFINITIONS_H

// Responses

enum {
	AT_PREFIX,

	// Commands
	ATC_ECHO,
	ATC_CPIN,
	ATC_CMEE,
	ATC_CMGF,
	ATC_CMGD,
	ATC_CNMP,
	ATC_CMNB,
	ATC_CGDCONT,
	ATC_COPS,
	ATC_CNACT,
	ATC_CASSLCFG,
	ATC_CACFG,
	ATC_CAOPEN,
	ATC_CACLOSE,
	ATC_CASEND,
	ATC_CARECV,
	ATC_CAACK,
	ATC_CASTATE,

	// Responses
	ATR_ERROR,
	ATR_CME_ERROR,

	ATR_OK,
	ATR_APP_PDP,
	ATR_CAOPEN,
	ATR_CACLOSE,
	ATR_CASTATE,
	ATR_NPD,

	AT_STRING_COUNT
};

static const char *at_string_lookup[AT_STRING_COUNT] = {
	[AT_PREFIX]     = "AT",

	// commands
	[ATC_ECHO]      = "E",
	[ATC_CPIN]      = "+CPIN",
	[ATC_CMEE]      = "+CMEE",
	[ATC_CMGF]      = "+CMGF",
	[ATC_CMGD]      = "+CMGD",
	[ATC_CNMP]      = "+CNMP",
	[ATC_CMNB]      = "+CMNB",
	[ATC_CGDCONT]   = "+CGDCONT",
	[ATC_COPS]      = "+COPS",
	[ATC_CNACT]     = "+CNACT",
	[ATC_CASSLCFG]  = "+CASSLCFG",
	[ATC_CACFG]     = "+CACFG",
	[ATC_CAOPEN]    = "+CAOPEN",
	[ATC_CACLOSE]   = "+CACLOSE",
	[ATC_CASEND]    = "+CASEND",
	[ATC_CARECV]    = "+CARECV",
	[ATC_CAACK]     = "+CAACK",
	[ATC_CASTATE]   = "+CASTATE",

	// responses
	[ATR_OK]        = "OK",
	[ATR_ERROR]     = "ERROR",
	[ATR_CME_ERROR] = "+CME ERROR:",

	// state responses
	// These are the responses we parse to track modem state
	[ATR_APP_PDP]   = "+APP PDP:",
	[ATR_CAOPEN]    = "+CAOPEN:",
	[ATR_CACLOSE]   = "+CACLOSE:",
	[ATR_CASTATE]   = "+CASTATE:",
	[ATR_NPD]       = "NORMAL POWER DOWN"
};

#endif // AT_DEFINITIONS_H
