#ifndef ES_UPDATE_H
#include "model.h"

#ifdef PHYTREX
#include "main.h"
#include <lwip_netconf.h>
#include <phytrex_homekit_flow.h>
#include <homekit/HAPAccessory.h>
#include <homekit/HAPAccessoryAPI.h>
#include "phytrex_model.h"
#include "phytrex_update.h"
#endif

void ES_phytrex_OtaHandler(void);


#define ES_UPDATE_H
#endif	/* ES_UPDATE_H */