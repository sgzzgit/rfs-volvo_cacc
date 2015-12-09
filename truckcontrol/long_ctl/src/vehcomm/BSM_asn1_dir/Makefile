ASN_MODULE_SOURCES=	\
	DSRCmsgID.c	\
	BSMblob.c	\
	BSMCACC.c	\
	CaccData.c	\
	VehicleSize.c	\
	MsgCount.c	\
	TemporaryID.c	\
	Latitude.c	\
	Longitude.c	\
	Elevation.c	\
	PositionalAccuracy.c	\
	TransmissionAndSpeed.c	\
	Heading.c	\
	SteeringWheelAngle.c	\
	AccelerationSet4Way.c	\
	Acceleration.c	\
	VerticalAcceleration.c	\
	YawRate.c	\
	BrakeSystemStatus.c	\
	ThrottlePosition.c	\
	VehicleWidth.c	\
	VehicleLength.c	\
	TransmissionState.c	\
	Velocity.c	\
	CACCFlags.c	\
	DDateTime.c	\
	DYear.c	\
	DMonth.c	\
	DDay.c	\
	DHour.c	\
	DMinute.c	\
	DSecond.c

ASN_MODULE_HEADERS=	\
	DSRCmsgID.h	\
	BSMblob.h	\
	BSMCACC.h	\
	CaccData.h	\
	VehicleSize.h	\
	MsgCount.h	\
	TemporaryID.h	\
	Latitude.h	\
	Longitude.h	\
	Elevation.h	\
	PositionalAccuracy.h	\
	TransmissionAndSpeed.h	\
	Heading.h	\
	SteeringWheelAngle.h	\
	AccelerationSet4Way.h	\
	Acceleration.h	\
	VerticalAcceleration.h	\
	YawRate.h	\
	BrakeSystemStatus.h	\
	ThrottlePosition.h	\
	VehicleWidth.h	\
	VehicleLength.h	\
	TransmissionState.h	\
	Velocity.h	\
	CACCFlags.h	\
	DDateTime.h	\
	DYear.h	\
	DMonth.h	\
	DDay.h	\
	DHour.h	\
	DMinute.h	\
	DSecond.h

ASN_MODULE_HEADERS+=INTEGER.h
ASN_MODULE_HEADERS+=BOOLEAN.h
ASN_MODULE_HEADERS+=asn_SEQUENCE_OF.h
ASN_MODULE_HEADERS+=asn_SET_OF.h
ASN_MODULE_HEADERS+=NativeEnumerated.h
ASN_MODULE_SOURCES+=INTEGER.c
ASN_MODULE_SOURCES+=asn_SEQUENCE_OF.c
ASN_MODULE_SOURCES+=asn_SET_OF.c
ASN_MODULE_SOURCES+=BOOLEAN.c
ASN_MODULE_SOURCES+=NativeEnumerated.c
ASN_MODULE_HEADERS+=NativeInteger.h
ASN_MODULE_SOURCES+=NativeInteger.c
ASN_MODULE_HEADERS+=constr_SEQUENCE.h
ASN_MODULE_SOURCES+=constr_SEQUENCE.c
ASN_MODULE_HEADERS+=asn_application.h
ASN_MODULE_HEADERS+=asn_system.h
ASN_MODULE_HEADERS+=asn_codecs.h
ASN_MODULE_HEADERS+=asn_internal.h
ASN_MODULE_HEADERS+=OCTET_STRING.h
ASN_MODULE_SOURCES+=OCTET_STRING.c
ASN_MODULE_HEADERS+=BIT_STRING.h
ASN_MODULE_SOURCES+=BIT_STRING.c
ASN_MODULE_SOURCES+=asn_codecs_prim.c
ASN_MODULE_HEADERS+=asn_codecs_prim.h
ASN_MODULE_HEADERS+=ber_tlv_length.h
ASN_MODULE_SOURCES+=ber_tlv_length.c
ASN_MODULE_HEADERS+=ber_tlv_tag.h
ASN_MODULE_SOURCES+=ber_tlv_tag.c
ASN_MODULE_HEADERS+=ber_decoder.h
ASN_MODULE_SOURCES+=ber_decoder.c
ASN_MODULE_HEADERS+=der_encoder.h
ASN_MODULE_SOURCES+=der_encoder.c
ASN_MODULE_HEADERS+=constr_TYPE.h
ASN_MODULE_SOURCES+=constr_TYPE.c
ASN_MODULE_HEADERS+=constraints.h
ASN_MODULE_SOURCES+=constraints.c
ASN_MODULE_HEADERS+=xer_support.h
ASN_MODULE_SOURCES+=xer_support.c
ASN_MODULE_HEADERS+=xer_decoder.h
ASN_MODULE_SOURCES+=xer_decoder.c
ASN_MODULE_HEADERS+=xer_encoder.h
ASN_MODULE_SOURCES+=xer_encoder.c
ASN_MODULE_HEADERS+=per_support.h
ASN_MODULE_SOURCES+=per_support.c
ASN_MODULE_HEADERS+=per_decoder.h
ASN_MODULE_SOURCES+=per_decoder.c
ASN_MODULE_HEADERS+=per_encoder.h
ASN_MODULE_SOURCES+=per_encoder.c
ASN_MODULE_HEADERS+=per_opentype.h
ASN_MODULE_SOURCES+=per_opentype.c
ASN_CONVERTER_SOURCES+=converter-sample.c


lib_LTLIBRARIES=libsomething.la
libsomething_la_SOURCES=$(ASN_MODULE_SOURCES) $(ASN_MODULE_HEADERS)

# This file may be used as an input for make(3)
# Remove the lines below to convert it into a pure .am file
TARGET = myDecoder
CFLAGS += -I. -DPDU=BSMCACC
#OBJS=${ASN_MODULE_SOURCES:.c=.o} ${ASN_CONVERTER_SOURCES:.c=.o}
OBJS=${ASN_MODULE_SOURCES:.c=.o}

all: $(TARGET)

$(TARGET): ${OBJS}
	$(CC) $(CFLAGS) -o $(TARGET) converter-sample.c ${OBJS} $(LDFLAGS) $(LIBS)
#	$(CC) $(CFLAGS) -o $(TARGET) ${OBJS} $(LDFLAGS) $(LIBS)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)

regen: regenerate-from-asn1-source

regenerate-from-asn1-source:
	asn1c -S/home/jspring/downloads/asn1c-0.9.26/skeletons -fnative-types BasicSafetyMessage-Volvo.txt
