/*
 * Generated by asn1c-0.9.26 (http://lionet.info/asn1c)
 * From ASN.1 module "J2735BSMESSAGE"
 * 	found in "BasicSafetyMessage-Volvo.txt"
 */

#ifndef	_TemporaryID_H_
#define	_TemporaryID_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TemporaryID */
typedef OCTET_STRING_t	 TemporaryID_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TemporaryID;
asn_struct_free_f TemporaryID_free;
asn_struct_print_f TemporaryID_print;
asn_constr_check_f TemporaryID_constraint;
ber_type_decoder_f TemporaryID_decode_ber;
der_type_encoder_f TemporaryID_encode_der;
xer_type_decoder_f TemporaryID_decode_xer;
xer_type_encoder_f TemporaryID_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _TemporaryID_H_ */
#include <asn_internal.h>