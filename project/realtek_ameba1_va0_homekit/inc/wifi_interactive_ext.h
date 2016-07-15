extern void cmd_wac(int argc, char **argv);
extern void cmd_hap(int argc, char **argv);
extern void cmd_mdns(int argc, char **argv);
extern void cmd_interface(int argc, char **argv);
extern void cmd_factory_reset(int argc, char **argv);
extern void cmd_time(int argc, char **argv);
extern void cmd_led(int argc, char **argv);
extern void cmd_mcuota(int argc, char **argv);
extern void cmd_mcuver(int argc, char **argv);
extern void cmd_mcustate(int argc, char **argv);
extern void cmd_mcusynch(int argc, char **argv);
extern void cmd_jpush(int argc, char **argv);
extern void cmd_ntp(int argc, char **argv);
extern void cmd_reportiot(int argc, char **argv);
extern void cmd_reportiotalarm(int argc, char **argv);
extern void cmd_ota(int argc, char **argv);

static const cmd_entry ext_cmd_table[] = {
	{"wac", cmd_wac},
	{"hap", cmd_hap},
	{"mdns", cmd_mdns},
	{"interface", cmd_interface},
	{"factory_reset", cmd_factory_reset},
	{"time", cmd_time},
	{"led", cmd_led},
	{"mcuota", cmd_mcuota},
	{"mcuver", cmd_mcuver},
	{"mcustate", cmd_mcustate},
	{"mcusynch", cmd_mcusynch},
	{"jpush", cmd_jpush},
	{"getntp", cmd_ntp},
	{"reportiot", cmd_reportiot},
	{"reportiotalarm", cmd_reportiotalarm},
	{"ota", cmd_ota},
	{"", NULL}
};
