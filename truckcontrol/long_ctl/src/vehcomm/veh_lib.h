#include "asn_application.h"
#include "asn_internal.h"       /* for _ASN_DEFAULT_STACK_MAX */
#include "asn_SEQUENCE_OF.h"
#include "BSMCACC.h"

extern int vehcomm2BSM(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt);
extern int BSM2vehcomm(BSMCACC_t *BSMCACC, veh_comm_packet_t *comm_pkt);
