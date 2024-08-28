/************************************************************************
 * COPYRIGHT (C) Ericsson 2012                                           *
 * The copyright to the computer program(s) herein is the property       *
 * of Telefonaktiebolaget LM Ericsson.                                   *
 * The program(s) may be used and/or copied only with the written        *
 * permission from Telefonaktiebolaget LM Ericsson or in accordance with *
 * the terms and conditions stipulated in the agreement/contract         *
 * under which the program(s) have been supplied.                        *
 *************************************************************************
 *************************************************************************
 * File: gtpv1_information_elements.c
 * Date: Mar 19, 2012
 * Author: LMI/LXR/SH Liam Fallon
 ************************************************************************/

/**
 * This module manages GTP V1 information elements
 */

#include <string.h>
#include <gtpv1_information_elements.h>

// All GTP V1 information elements
struct gtpv1_information_element gtpv1_information_elements[MAX_GTPV1_INFORMATION_ELEMENTS];

//
// This function initializes static data for GTP V1 messages
//
void init_gtpv1_information_elements()
{
	// Zero the message types
	for (int i = 0; i < MAX_GTPV1_INFORMATION_ELEMENTS; i++) {
		gtpv1_information_elements[i].id = 0;
		gtpv1_information_elements[i].grouped = 0;
		gtpv1_information_elements[i].header_length = 0;
		gtpv1_information_elements[i].body_length = 0;
		strcpy(gtpv1_information_elements[i].name, "");
	}

	// Set up the information elements
	gtpv1_information_elements[GTPV1_IE_CAUSE].id = GTPV1_IE_CAUSE;
	gtpv1_information_elements[GTPV1_IE_CAUSE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CAUSE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_CAUSE].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_CAUSE].name, "Cause");

	gtpv1_information_elements[GTPV1_IE_IMSI].id = GTPV1_IE_IMSI;
	gtpv1_information_elements[GTPV1_IE_IMSI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_IMSI].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_IMSI].body_length = 8;
	strcpy(gtpv1_information_elements[GTPV1_IE_IMSI].name, "International Mobile Subscriber Identity (IMSI)");

	gtpv1_information_elements[GTPV1_IE_RAI].id = GTPV1_IE_RAI;
	gtpv1_information_elements[GTPV1_IE_RAI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RAI].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_RAI].body_length = 6;
	strcpy(gtpv1_information_elements[GTPV1_IE_RAI].name, "Routeing Area Identity (RAI)");

	gtpv1_information_elements[GTPV1_IE_TLLI].id = GTPV1_IE_TLLI;
	gtpv1_information_elements[GTPV1_IE_TLLI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TLLI].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_TLLI].body_length = 4;
	strcpy(gtpv1_information_elements[GTPV1_IE_TLLI].name, "Temporary Logical Link Identity (TLLI)");

	gtpv1_information_elements[GTPV1_IE_P_TMSI].id = GTPV1_IE_P_TMSI;
	gtpv1_information_elements[GTPV1_IE_P_TMSI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_P_TMSI].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_P_TMSI].body_length = 4;
	strcpy(gtpv1_information_elements[GTPV1_IE_P_TMSI].name, "Packet TMSI (P-TMSI)");

	gtpv1_information_elements[GTPV1_IE_REORDERING_REQUIRED].id = GTPV1_IE_REORDERING_REQUIRED;
	gtpv1_information_elements[GTPV1_IE_REORDERING_REQUIRED].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_REORDERING_REQUIRED].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_REORDERING_REQUIRED].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_REORDERING_REQUIRED].name, "Reordering Required");

	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_TRIPLET].id = GTPV1_IE_AUTHENTICATION_TRIPLET;
	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_TRIPLET].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_TRIPLET].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_TRIPLET].body_length = 28;
	strcpy(gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_TRIPLET].name, "Authentication Triplet");

	gtpv1_information_elements[GTPV1_IE_MAP_CAUSE].id = GTPV1_IE_MAP_CAUSE;
	gtpv1_information_elements[GTPV1_IE_MAP_CAUSE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MAP_CAUSE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_MAP_CAUSE].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_MAP_CAUSE].name, "MAP Cause");

	gtpv1_information_elements[GTPV1_IE_P_TMSI_SIGNATURE].id = GTPV1_IE_P_TMSI_SIGNATURE;
	gtpv1_information_elements[GTPV1_IE_P_TMSI_SIGNATURE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_P_TMSI_SIGNATURE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_P_TMSI_SIGNATURE].body_length = 3;
	strcpy(gtpv1_information_elements[GTPV1_IE_P_TMSI_SIGNATURE].name, "P-TMSI Signature");

	gtpv1_information_elements[GTPV1_IE_MS_VALIDATED].id = GTPV1_IE_MS_VALIDATED;
	gtpv1_information_elements[GTPV1_IE_MS_VALIDATED].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MS_VALIDATED].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_MS_VALIDATED].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_MS_VALIDATED].name, "MS Validated");

	gtpv1_information_elements[GTPV1_IE_RECOVERY].id = GTPV1_IE_RECOVERY;
	gtpv1_information_elements[GTPV1_IE_RECOVERY].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RECOVERY].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_RECOVERY].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_RECOVERY].name, "Recovery");

	gtpv1_information_elements[GTPV1_IE_SELECTION_MODE].id = GTPV1_IE_SELECTION_MODE;
	gtpv1_information_elements[GTPV1_IE_SELECTION_MODE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_SELECTION_MODE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_SELECTION_MODE].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_SELECTION_MODE].name, "Selection Mode");

	gtpv1_information_elements[GTPV1_IE_TEI_DATA_I].id = GTPV1_IE_TEI_DATA_I;
	gtpv1_information_elements[GTPV1_IE_TEI_DATA_I].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TEI_DATA_I].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_TEI_DATA_I].body_length = 4;
	strcpy(gtpv1_information_elements[GTPV1_IE_TEI_DATA_I].name, "Tunnel Endpoint Identifier Data I");

	gtpv1_information_elements[GTPV1_IE_TEI_CONTROL_PLANE].id = GTPV1_IE_TEI_CONTROL_PLANE;
	gtpv1_information_elements[GTPV1_IE_TEI_CONTROL_PLANE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TEI_CONTROL_PLANE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_TEI_CONTROL_PLANE].body_length = 4;
	strcpy(gtpv1_information_elements[GTPV1_IE_TEI_CONTROL_PLANE].name, "Tunnel Endpoint Identifier Control Plane");

	gtpv1_information_elements[GTPV1_IE_TEI_DATA_II].id = GTPV1_IE_TEI_DATA_II;
	gtpv1_information_elements[GTPV1_IE_TEI_DATA_II].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TEI_DATA_II].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_TEI_DATA_II].body_length = 5;
	strcpy(gtpv1_information_elements[GTPV1_IE_TEI_DATA_II].name, "Tunnel Endpoint Identifier Data II");

	gtpv1_information_elements[GTPV1_IE_TEARDOWN_IND].id = GTPV1_IE_TEARDOWN_IND;
	gtpv1_information_elements[GTPV1_IE_TEARDOWN_IND].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TEARDOWN_IND].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_TEARDOWN_IND].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_TEARDOWN_IND].name, "Teardown Ind");

	gtpv1_information_elements[GTPV1_IE_NSAPI].id = GTPV1_IE_NSAPI;
	gtpv1_information_elements[GTPV1_IE_NSAPI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_NSAPI].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_NSAPI].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_NSAPI].name, "NSAPI");

	gtpv1_information_elements[GTPV1_IE_RANAP_CAUSE].id = GTPV1_IE_RANAP_CAUSE;
	gtpv1_information_elements[GTPV1_IE_RANAP_CAUSE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RANAP_CAUSE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_RANAP_CAUSE].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_RANAP_CAUSE].name, "RANAP Cause");

	gtpv1_information_elements[GTPV1_IE_RAB_CONTEXT].id = GTPV1_IE_RAB_CONTEXT;
	gtpv1_information_elements[GTPV1_IE_RAB_CONTEXT].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RAB_CONTEXT].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_RAB_CONTEXT].body_length = 9;
	strcpy(gtpv1_information_elements[GTPV1_IE_RAB_CONTEXT].name, "RAB Context");

	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_SMS].id = GTPV1_IE_RADIO_PRIORITY_SMS;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_SMS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_SMS].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_SMS].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_SMS].name, "Radio Priority SMS");

	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY].id = GTPV1_IE_RADIO_PRIORITY;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY].name, "Radio Priority");

	gtpv1_information_elements[GTPV1_IE_PACKET_FLOW_ID].id = GTPV1_IE_PACKET_FLOW_ID;
	gtpv1_information_elements[GTPV1_IE_PACKET_FLOW_ID].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_PACKET_FLOW_ID].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_PACKET_FLOW_ID].body_length = 2;
	strcpy(gtpv1_information_elements[GTPV1_IE_PACKET_FLOW_ID].name, "Packet Flow Id");

	gtpv1_information_elements[GTPV1_IE_CHARGING_CHARACTERISTICS].id = GTPV1_IE_CHARGING_CHARACTERISTICS;
	gtpv1_information_elements[GTPV1_IE_CHARGING_CHARACTERISTICS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CHARGING_CHARACTERISTICS].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_CHARGING_CHARACTERISTICS].body_length = 2;
	strcpy(gtpv1_information_elements[GTPV1_IE_CHARGING_CHARACTERISTICS].name, "Charging Characteristics");

	gtpv1_information_elements[GTPV1_IE_TRACE_REFERENCE].id = GTPV1_IE_TRACE_REFERENCE;
	gtpv1_information_elements[GTPV1_IE_TRACE_REFERENCE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TRACE_REFERENCE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_TRACE_REFERENCE].body_length = 2;
	strcpy(gtpv1_information_elements[GTPV1_IE_TRACE_REFERENCE].name, "Trace Reference");

	gtpv1_information_elements[GTPV1_IE_TRACE_TYPE].id = GTPV1_IE_TRACE_TYPE;
	gtpv1_information_elements[GTPV1_IE_TRACE_TYPE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TRACE_TYPE].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_TRACE_TYPE].body_length = 2;
	strcpy(gtpv1_information_elements[GTPV1_IE_TRACE_TYPE].name, "Trace Type");

	gtpv1_information_elements[GTPV1_IE_MS_NOT_REACHABLE_REASON].id = GTPV1_IE_MS_NOT_REACHABLE_REASON;
	gtpv1_information_elements[GTPV1_IE_MS_NOT_REACHABLE_REASON].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MS_NOT_REACHABLE_REASON].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_MS_NOT_REACHABLE_REASON].body_length = 1;
	strcpy(gtpv1_information_elements[GTPV1_IE_MS_NOT_REACHABLE_REASON].name, "MS Not Reachable Reason");

	gtpv1_information_elements[GTPV1_IE_CHARGING_ID].id = GTPV1_IE_CHARGING_ID;
	gtpv1_information_elements[GTPV1_IE_CHARGING_ID].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CHARGING_ID].header_length = 1;
	gtpv1_information_elements[GTPV1_IE_CHARGING_ID].body_length = 4;
	strcpy(gtpv1_information_elements[GTPV1_IE_CHARGING_ID].name, "Charging ID");

	gtpv1_information_elements[GTPV1_IE_END_USER_ADDRESS].id = GTPV1_IE_END_USER_ADDRESS;
	gtpv1_information_elements[GTPV1_IE_END_USER_ADDRESS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_END_USER_ADDRESS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_END_USER_ADDRESS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_END_USER_ADDRESS].name, "End User Address");

	gtpv1_information_elements[GTPV1_IE_MM_CONTEXT].id = GTPV1_IE_MM_CONTEXT;
	gtpv1_information_elements[GTPV1_IE_MM_CONTEXT].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MM_CONTEXT].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MM_CONTEXT].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MM_CONTEXT].name, "MM Context");

	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT].id = GTPV1_IE_PDP_CONTEXT;
	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT].name, "PDP Context");

	gtpv1_information_elements[GTPV1_IE_APN].id = GTPV1_IE_APN;
	gtpv1_information_elements[GTPV1_IE_APN].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_APN].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_APN].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_APN].name, "Access Point Name");

	gtpv1_information_elements[GTPV1_IE_PROTOCOL_CONFIGURATION_OPTIONS].id = GTPV1_IE_PROTOCOL_CONFIGURATION_OPTIONS;
	gtpv1_information_elements[GTPV1_IE_PROTOCOL_CONFIGURATION_OPTIONS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_PROTOCOL_CONFIGURATION_OPTIONS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_PROTOCOL_CONFIGURATION_OPTIONS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_PROTOCOL_CONFIGURATION_OPTIONS].name, "Protocol Configuration Options");

	gtpv1_information_elements[GTPV1_IE_GSN_ADDRESS].id = GTPV1_IE_GSN_ADDRESS;
	gtpv1_information_elements[GTPV1_IE_GSN_ADDRESS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_GSN_ADDRESS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_GSN_ADDRESS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_GSN_ADDRESS].name, "GSN Address");

	gtpv1_information_elements[GTPV1_IE_MSISDN].id = GTPV1_IE_MSISDN;
	gtpv1_information_elements[GTPV1_IE_MSISDN].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MSISDN].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MSISDN].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MSISDN].name, "MS International PSTN/ISDN Number (MSISDN)");

	gtpv1_information_elements[GTPV1_IE_QOS_PROFILE].id = GTPV1_IE_QOS_PROFILE;
	gtpv1_information_elements[GTPV1_IE_QOS_PROFILE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_QOS_PROFILE].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_QOS_PROFILE].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_QOS_PROFILE].name, "Quality of Service Profile");

	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_QUINTUPLET].id = GTPV1_IE_AUTHENTICATION_QUINTUPLET;
	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_QUINTUPLET].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_QUINTUPLET].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_QUINTUPLET].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_AUTHENTICATION_QUINTUPLET].name, "Authentication Quintuplet");

	gtpv1_information_elements[GTPV1_IE_TRAFFIC_FLOW_TEMPLATE].id = GTPV1_IE_TRAFFIC_FLOW_TEMPLATE;
	gtpv1_information_elements[GTPV1_IE_TRAFFIC_FLOW_TEMPLATE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TRAFFIC_FLOW_TEMPLATE].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_TRAFFIC_FLOW_TEMPLATE].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_TRAFFIC_FLOW_TEMPLATE].name, "Traffic Flow Template");

	gtpv1_information_elements[GTPV1_IE_TARGET_IDENTIFICATION].id = GTPV1_IE_TARGET_IDENTIFICATION;
	gtpv1_information_elements[GTPV1_IE_TARGET_IDENTIFICATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TARGET_IDENTIFICATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_TARGET_IDENTIFICATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_TARGET_IDENTIFICATION].name, "Target Identification");

	gtpv1_information_elements[GTPV1_IE_UTRAN_TRANSPARENT_CONTAINER].id = GTPV1_IE_UTRAN_TRANSPARENT_CONTAINER;
	gtpv1_information_elements[GTPV1_IE_UTRAN_TRANSPARENT_CONTAINER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_UTRAN_TRANSPARENT_CONTAINER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_UTRAN_TRANSPARENT_CONTAINER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_UTRAN_TRANSPARENT_CONTAINER].name, "UTRAN Transparent Container");

	gtpv1_information_elements[GTPV1_IE_RAB_SETUP_INFORMATION].id = GTPV1_IE_RAB_SETUP_INFORMATION;
	gtpv1_information_elements[GTPV1_IE_RAB_SETUP_INFORMATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RAB_SETUP_INFORMATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RAB_SETUP_INFORMATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RAB_SETUP_INFORMATION].name, "RAB Setup Information");

	gtpv1_information_elements[GTPV1_IE_EXTENSION_HEADER_TYPE_LIST].id = GTPV1_IE_EXTENSION_HEADER_TYPE_LIST;
	gtpv1_information_elements[GTPV1_IE_EXTENSION_HEADER_TYPE_LIST].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_EXTENSION_HEADER_TYPE_LIST].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_EXTENSION_HEADER_TYPE_LIST].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_EXTENSION_HEADER_TYPE_LIST].name, "Extension Header Type List");

	gtpv1_information_elements[GTPV1_IE_TRIGGER_ID].id = GTPV1_IE_TRIGGER_ID;
	gtpv1_information_elements[GTPV1_IE_TRIGGER_ID].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TRIGGER_ID].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_TRIGGER_ID].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_TRIGGER_ID].name, "Trigger Id");

	gtpv1_information_elements[GTPV1_IE_OMC_IDENTITY].id = GTPV1_IE_OMC_IDENTITY;
	gtpv1_information_elements[GTPV1_IE_OMC_IDENTITY].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_OMC_IDENTITY].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_OMC_IDENTITY].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_OMC_IDENTITY].name, "OMC Identity");

	gtpv1_information_elements[GTPV1_IE_RAN_TRANSPARENT_CONTAINER].id = GTPV1_IE_RAN_TRANSPARENT_CONTAINER;
	gtpv1_information_elements[GTPV1_IE_RAN_TRANSPARENT_CONTAINER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RAN_TRANSPARENT_CONTAINER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RAN_TRANSPARENT_CONTAINER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RAN_TRANSPARENT_CONTAINER].name, "RAN Transparent Container");

	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT_PRIORITIZATION].id = GTPV1_IE_PDP_CONTEXT_PRIORITIZATION;
	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT_PRIORITIZATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT_PRIORITIZATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT_PRIORITIZATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_PDP_CONTEXT_PRIORITIZATION].name, "PDP Context Prioritization");

	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_RAB_SETUP_INFORMATION].id = GTPV1_IE_ADDITIONAL_RAB_SETUP_INFORMATION;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_RAB_SETUP_INFORMATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_RAB_SETUP_INFORMATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_RAB_SETUP_INFORMATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_ADDITIONAL_RAB_SETUP_INFORMATION].name, "Additional RAB Setup Information");

	gtpv1_information_elements[GTPV1_IE_SGSN_NUMBER].id = GTPV1_IE_SGSN_NUMBER;
	gtpv1_information_elements[GTPV1_IE_SGSN_NUMBER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_SGSN_NUMBER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_SGSN_NUMBER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_SGSN_NUMBER].name, "SGSN Number");

	gtpv1_information_elements[GTPV1_IE_COMMON_FLAGS].id = GTPV1_IE_COMMON_FLAGS;
	gtpv1_information_elements[GTPV1_IE_COMMON_FLAGS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_COMMON_FLAGS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_COMMON_FLAGS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_COMMON_FLAGS].name, "Common Flags");

	gtpv1_information_elements[GTPV1_IE_APN_RESTRICTION].id = GTPV1_IE_APN_RESTRICTION;
	gtpv1_information_elements[GTPV1_IE_APN_RESTRICTION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_APN_RESTRICTION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_APN_RESTRICTION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_APN_RESTRICTION].name, "APN Restriction");

	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_LCS].id = GTPV1_IE_RADIO_PRIORITY_LCS;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_LCS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_LCS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_LCS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RADIO_PRIORITY_LCS].name, "Radio Priority LCS");

	gtpv1_information_elements[GTPV1_IE_RAT_TYPE].id = GTPV1_IE_RAT_TYPE;
	gtpv1_information_elements[GTPV1_IE_RAT_TYPE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RAT_TYPE].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RAT_TYPE].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RAT_TYPE].name, "RAT Type");

	gtpv1_information_elements[GTPV1_IE_USER_LOCATION_INFORMATION].id = GTPV1_IE_USER_LOCATION_INFORMATION;
	gtpv1_information_elements[GTPV1_IE_USER_LOCATION_INFORMATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_USER_LOCATION_INFORMATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_USER_LOCATION_INFORMATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_USER_LOCATION_INFORMATION].name, "User Location Information");

	gtpv1_information_elements[GTPV1_IE_MS_TIME_ZONE].id = GTPV1_IE_MS_TIME_ZONE;
	gtpv1_information_elements[GTPV1_IE_MS_TIME_ZONE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MS_TIME_ZONE].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MS_TIME_ZONE].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MS_TIME_ZONE].name, "MS Time Zone");

	gtpv1_information_elements[GTPV1_IE_IMEISV].id = GTPV1_IE_IMEISV;
	gtpv1_information_elements[GTPV1_IE_IMEISV].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_IMEISV].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_IMEISV].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_IMEISV].name, "IMEI(SV)");

	gtpv1_information_elements[GTPV1_IE_CAMEL_CHARGING_INFORMATION_CONTAINER].id = GTPV1_IE_CAMEL_CHARGING_INFORMATION_CONTAINER;
	gtpv1_information_elements[GTPV1_IE_CAMEL_CHARGING_INFORMATION_CONTAINER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CAMEL_CHARGING_INFORMATION_CONTAINER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_CAMEL_CHARGING_INFORMATION_CONTAINER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_CAMEL_CHARGING_INFORMATION_CONTAINER].name, "CAMEL Charging Information Container");

	gtpv1_information_elements[GTPV1_IE_MBMS_UE_CONTEXT].id = GTPV1_IE_MBMS_UE_CONTEXT;
	gtpv1_information_elements[GTPV1_IE_MBMS_UE_CONTEXT].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_UE_CONTEXT].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_UE_CONTEXT].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_UE_CONTEXT].name, "MBMS UE Context");

	gtpv1_information_elements[GTPV1_IE_TMGI].id = GTPV1_IE_TMGI;
	gtpv1_information_elements[GTPV1_IE_TMGI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_TMGI].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_TMGI].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_TMGI].name, "Temporary Mobile Group Identity (TMGI)");

	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS].id = GTPV1_IE_RIM_ROUTING_ADDRESS;
	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS].name, "RIM Routing Address");

	gtpv1_information_elements[GTPV1_IE_MBMS_PROTOCOL_CONFIGURATION_OPTIONS].id = GTPV1_IE_MBMS_PROTOCOL_CONFIGURATION_OPTIONS;
	gtpv1_information_elements[GTPV1_IE_MBMS_PROTOCOL_CONFIGURATION_OPTIONS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_PROTOCOL_CONFIGURATION_OPTIONS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_PROTOCOL_CONFIGURATION_OPTIONS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_PROTOCOL_CONFIGURATION_OPTIONS].name, "MBMS Protocol Configuration Options");

	gtpv1_information_elements[GTPV1_IE_MBMS_SERVICE_AREA].id = GTPV1_IE_MBMS_SERVICE_AREA;
	gtpv1_information_elements[GTPV1_IE_MBMS_SERVICE_AREA].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_SERVICE_AREA].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_SERVICE_AREA].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_SERVICE_AREA].name, "MBMS Service Area");

	gtpv1_information_elements[GTPV1_IE_SOURCE_RNC_PDCP_CONTEXT_INFO].id = GTPV1_IE_SOURCE_RNC_PDCP_CONTEXT_INFO;
	gtpv1_information_elements[GTPV1_IE_SOURCE_RNC_PDCP_CONTEXT_INFO].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_SOURCE_RNC_PDCP_CONTEXT_INFO].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_SOURCE_RNC_PDCP_CONTEXT_INFO].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_SOURCE_RNC_PDCP_CONTEXT_INFO].name, "Source RNC PDCP context info");

	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_TRACE_INFO].id = GTPV1_IE_ADDITIONAL_TRACE_INFO;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_TRACE_INFO].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_TRACE_INFO].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_TRACE_INFO].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_ADDITIONAL_TRACE_INFO].name, "Additional Trace Info");

	gtpv1_information_elements[GTPV1_IE_HOP_COUNTER].id = GTPV1_IE_HOP_COUNTER;
	gtpv1_information_elements[GTPV1_IE_HOP_COUNTER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_HOP_COUNTER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_HOP_COUNTER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_HOP_COUNTER].name, "Hop Counter");

	gtpv1_information_elements[GTPV1_IE_SELECTED_PLMN_ID].id = GTPV1_IE_SELECTED_PLMN_ID;
	gtpv1_information_elements[GTPV1_IE_SELECTED_PLMN_ID].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_SELECTED_PLMN_ID].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_SELECTED_PLMN_ID].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_SELECTED_PLMN_ID].name, "Selected PLMN ID");

	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_IDENTIFIER].id = GTPV1_IE_MBMS_SESSION_IDENTIFIER;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_IDENTIFIER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_IDENTIFIER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_IDENTIFIER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_IDENTIFIER].name, "MBMS Session Identifier");

	gtpv1_information_elements[GTPV1_IE_MBMS_2G_3G_INDICATOR].id = GTPV1_IE_MBMS_2G_3G_INDICATOR;
	gtpv1_information_elements[GTPV1_IE_MBMS_2G_3G_INDICATOR].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_2G_3G_INDICATOR].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_2G_3G_INDICATOR].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_2G_3G_INDICATOR].name, "MBMS 2G/3G Indicator");

	gtpv1_information_elements[GTPV1_IE_ENHANCED_NSAPI].id = GTPV1_IE_ENHANCED_NSAPI;
	gtpv1_information_elements[GTPV1_IE_ENHANCED_NSAPI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_ENHANCED_NSAPI].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_ENHANCED_NSAPI].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_ENHANCED_NSAPI].name, "Enhanced NSAPI");

	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_DURATION].id = GTPV1_IE_MBMS_SESSION_DURATION;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_DURATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_DURATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_DURATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_DURATION].name, "MBMS Session Duration");

	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MBMS_TRACE_INFO].id = GTPV1_IE_ADDITIONAL_MBMS_TRACE_INFO;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MBMS_TRACE_INFO].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MBMS_TRACE_INFO].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MBMS_TRACE_INFO].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MBMS_TRACE_INFO].name, "Additional MBMS Trace Info");

	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_REPETITION_NUMBER].id = GTPV1_IE_MBMS_SESSION_REPETITION_NUMBER;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_REPETITION_NUMBER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_REPETITION_NUMBER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_REPETITION_NUMBER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_SESSION_REPETITION_NUMBER].name, "MBMS Session Repetition Number");

	gtpv1_information_elements[GTPV1_IE_MBMS_TIME_TO_DATA_TRANSFER].id = GTPV1_IE_MBMS_TIME_TO_DATA_TRANSFER;
	gtpv1_information_elements[GTPV1_IE_MBMS_TIME_TO_DATA_TRANSFER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_TIME_TO_DATA_TRANSFER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_TIME_TO_DATA_TRANSFER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_TIME_TO_DATA_TRANSFER].name, "MBMS Time To Data Transfer");

	gtpv1_information_elements[GTPV1_IE_BSS_CONTAINER].id = GTPV1_IE_BSS_CONTAINER;
	gtpv1_information_elements[GTPV1_IE_BSS_CONTAINER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_BSS_CONTAINER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_BSS_CONTAINER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_BSS_CONTAINER].name, "BSS Container");

	gtpv1_information_elements[GTPV1_IE_CELL_IDENTIFICATION].id = GTPV1_IE_CELL_IDENTIFICATION;
	gtpv1_information_elements[GTPV1_IE_CELL_IDENTIFICATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CELL_IDENTIFICATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_CELL_IDENTIFICATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_CELL_IDENTIFICATION].name, "Cell Identification");

	gtpv1_information_elements[GTPV1_IE_PDU_NUMBERS].id = GTPV1_IE_PDU_NUMBERS;
	gtpv1_information_elements[GTPV1_IE_PDU_NUMBERS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_PDU_NUMBERS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_PDU_NUMBERS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_PDU_NUMBERS].name, "PDU Numbers");

	gtpv1_information_elements[GTPV1_IE_BSSGP_CAUSE].id = GTPV1_IE_BSSGP_CAUSE;
	gtpv1_information_elements[GTPV1_IE_BSSGP_CAUSE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_BSSGP_CAUSE].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_BSSGP_CAUSE].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_BSSGP_CAUSE].name, "BSSGP Cause");

	gtpv1_information_elements[GTPV1_IE_REQUIRED_MBMS_BEARER_CAPABILITIES].id = GTPV1_IE_REQUIRED_MBMS_BEARER_CAPABILITIES;
	gtpv1_information_elements[GTPV1_IE_REQUIRED_MBMS_BEARER_CAPABILITIES].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_REQUIRED_MBMS_BEARER_CAPABILITIES].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_REQUIRED_MBMS_BEARER_CAPABILITIES].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_REQUIRED_MBMS_BEARER_CAPABILITIES].name, "Required MBMS bearer capabilities");

	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS_DISCRIMINATOR].id = GTPV1_IE_RIM_ROUTING_ADDRESS_DISCRIMINATOR;
	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS_DISCRIMINATOR].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS_DISCRIMINATOR].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS_DISCRIMINATOR].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RIM_ROUTING_ADDRESS_DISCRIMINATOR].name, "RIM Routing Address Discriminator");

	gtpv1_information_elements[GTPV1_IE_LIST_OF_SET_UP_PFCS].id = GTPV1_IE_LIST_OF_SET_UP_PFCS;
	gtpv1_information_elements[GTPV1_IE_LIST_OF_SET_UP_PFCS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_LIST_OF_SET_UP_PFCS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_LIST_OF_SET_UP_PFCS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_LIST_OF_SET_UP_PFCS].name, "List of set-up PFCs");

	gtpv1_information_elements[GTPV1_IE_PS_HANDOVER_XID_PARAMETERS].id = GTPV1_IE_PS_HANDOVER_XID_PARAMETERS;
	gtpv1_information_elements[GTPV1_IE_PS_HANDOVER_XID_PARAMETERS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_PS_HANDOVER_XID_PARAMETERS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_PS_HANDOVER_XID_PARAMETERS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_PS_HANDOVER_XID_PARAMETERS].name, "PS Handover XID Parameters");

	gtpv1_information_elements[GTPV1_IE_MS_INFO_CHANGE_REPORTING_ACTION].id = GTPV1_IE_MS_INFO_CHANGE_REPORTING_ACTION;
	gtpv1_information_elements[GTPV1_IE_MS_INFO_CHANGE_REPORTING_ACTION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MS_INFO_CHANGE_REPORTING_ACTION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MS_INFO_CHANGE_REPORTING_ACTION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MS_INFO_CHANGE_REPORTING_ACTION].name, "MS Info Change Reporting Action");

	gtpv1_information_elements[GTPV1_IE_DIRECT_TUNNEL_FLAGS].id = GTPV1_IE_DIRECT_TUNNEL_FLAGS;
	gtpv1_information_elements[GTPV1_IE_DIRECT_TUNNEL_FLAGS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_DIRECT_TUNNEL_FLAGS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_DIRECT_TUNNEL_FLAGS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_DIRECT_TUNNEL_FLAGS].name, "Direct Tunnel Flags");

	gtpv1_information_elements[GTPV1_IE_CORRELATION_ID].id = GTPV1_IE_CORRELATION_ID;
	gtpv1_information_elements[GTPV1_IE_CORRELATION_ID].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CORRELATION_ID].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_CORRELATION_ID].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_CORRELATION_ID].name, "Correlation-ID");

	gtpv1_information_elements[GTPV1_IE_BEARER_CONTROL_MODE].id = GTPV1_IE_BEARER_CONTROL_MODE;
	gtpv1_information_elements[GTPV1_IE_BEARER_CONTROL_MODE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_BEARER_CONTROL_MODE].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_BEARER_CONTROL_MODE].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_BEARER_CONTROL_MODE].name, "Bearer Control Mode");

	gtpv1_information_elements[GTPV1_IE_MBMS_FLOW_IDENTIFIER].id = GTPV1_IE_MBMS_FLOW_IDENTIFIER;
	gtpv1_information_elements[GTPV1_IE_MBMS_FLOW_IDENTIFIER].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_FLOW_IDENTIFIER].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_FLOW_IDENTIFIER].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_FLOW_IDENTIFIER].name, "MBMS Flow Identifier");

	gtpv1_information_elements[GTPV1_IE_MBMS_IP_MULTICAST_DISTRIBUTION].id = GTPV1_IE_MBMS_IP_MULTICAST_DISTRIBUTION;
	gtpv1_information_elements[GTPV1_IE_MBMS_IP_MULTICAST_DISTRIBUTION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_IP_MULTICAST_DISTRIBUTION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_IP_MULTICAST_DISTRIBUTION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_IP_MULTICAST_DISTRIBUTION].name, "MBMS IP Multicast Distribution");

	gtpv1_information_elements[GTPV1_IE_MBMS_DISTRIBUTION_ACKNOWLEDGEMENT].id = GTPV1_IE_MBMS_DISTRIBUTION_ACKNOWLEDGEMENT;
	gtpv1_information_elements[GTPV1_IE_MBMS_DISTRIBUTION_ACKNOWLEDGEMENT].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MBMS_DISTRIBUTION_ACKNOWLEDGEMENT].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MBMS_DISTRIBUTION_ACKNOWLEDGEMENT].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MBMS_DISTRIBUTION_ACKNOWLEDGEMENT].name, "MBMS Distribution Acknowledgement");

	gtpv1_information_elements[GTPV1_IE_RELIABLE_INTER_RAT_HANDOVER_INFO].id = GTPV1_IE_RELIABLE_INTER_RAT_HANDOVER_INFO;
	gtpv1_information_elements[GTPV1_IE_RELIABLE_INTER_RAT_HANDOVER_INFO].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RELIABLE_INTER_RAT_HANDOVER_INFO].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RELIABLE_INTER_RAT_HANDOVER_INFO].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RELIABLE_INTER_RAT_HANDOVER_INFO].name, "Reliable INTER RAT HANDOVER INFO ");

	gtpv1_information_elements[GTPV1_IE_RFSP_INDEX].id = GTPV1_IE_RFSP_INDEX;
	gtpv1_information_elements[GTPV1_IE_RFSP_INDEX].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_RFSP_INDEX].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_RFSP_INDEX].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_RFSP_INDEX].name, "RFSP Index");

	gtpv1_information_elements[GTPV1_IE_FQDN].id = GTPV1_IE_FQDN;
	gtpv1_information_elements[GTPV1_IE_FQDN].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_FQDN].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_FQDN].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_FQDN].name, "Fully Qualified Domain Name (FQDN)");

	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_I].id = GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_I;
	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_I].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_I].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_I].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_I].name, "Evolved Allocation/Retention Priority I");

	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_II].id = GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_II;
	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_II].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_II].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_II].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_EVOLVED_ALLOCATION_RETENTION_PRIORITY_II].name, "Evolved Allocation/Retention Priority II");

	gtpv1_information_elements[GTPV1_IE_EXTENDED_COMMON_FLAGS].id = GTPV1_IE_EXTENDED_COMMON_FLAGS;
	gtpv1_information_elements[GTPV1_IE_EXTENDED_COMMON_FLAGS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_EXTENDED_COMMON_FLAGS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_EXTENDED_COMMON_FLAGS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_EXTENDED_COMMON_FLAGS].name, "Extended Common Flags");

	gtpv1_information_elements[GTPV1_IE_UCI].id = GTPV1_IE_UCI;
	gtpv1_information_elements[GTPV1_IE_UCI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_UCI].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_UCI].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_UCI].name, "User CSG Information (UCI)");

	gtpv1_information_elements[GTPV1_IE_CSG_INFORMATION_REPORTING_ACTION].id = GTPV1_IE_CSG_INFORMATION_REPORTING_ACTION;
	gtpv1_information_elements[GTPV1_IE_CSG_INFORMATION_REPORTING_ACTION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CSG_INFORMATION_REPORTING_ACTION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_CSG_INFORMATION_REPORTING_ACTION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_CSG_INFORMATION_REPORTING_ACTION].name, "CSG Information Reporting Action");

	gtpv1_information_elements[GTPV1_IE_CSG_ID].id = GTPV1_IE_CSG_ID;
	gtpv1_information_elements[GTPV1_IE_CSG_ID].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CSG_ID].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_CSG_ID].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_CSG_ID].name, "CSG ID");

	gtpv1_information_elements[GTPV1_IE_CMI].id = GTPV1_IE_CMI;
	gtpv1_information_elements[GTPV1_IE_CMI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CMI].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_CMI].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_CMI].name, "CSG Membership Indication (CMI)");

	gtpv1_information_elements[GTPV1_IE_AMBR].id = GTPV1_IE_AMBR;
	gtpv1_information_elements[GTPV1_IE_AMBR].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_AMBR].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_AMBR].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_AMBR].name, "Aggregate Maximum Bit Rate (AMBR)");

	gtpv1_information_elements[GTPV1_IE_UE_NETWORK_CAPABILITY].id = GTPV1_IE_UE_NETWORK_CAPABILITY;
	gtpv1_information_elements[GTPV1_IE_UE_NETWORK_CAPABILITY].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_UE_NETWORK_CAPABILITY].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_UE_NETWORK_CAPABILITY].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_UE_NETWORK_CAPABILITY].name, "UE Network Capability");

	gtpv1_information_elements[GTPV1_IE_UE_AMBR].id = GTPV1_IE_UE_AMBR;
	gtpv1_information_elements[GTPV1_IE_UE_AMBR].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_UE_AMBR].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_UE_AMBR].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_UE_AMBR].name, "UE-AMBR");

	gtpv1_information_elements[GTPV1_IE_APN_AMBR_WITH_NSAPI].id = GTPV1_IE_APN_AMBR_WITH_NSAPI;
	gtpv1_information_elements[GTPV1_IE_APN_AMBR_WITH_NSAPI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_APN_AMBR_WITH_NSAPI].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_APN_AMBR_WITH_NSAPI].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_APN_AMBR_WITH_NSAPI].name, "APN-AMBR with NSAPI");

	gtpv1_information_elements[GTPV1_IE_GGSN_BACK_OFF_TIME].id = GTPV1_IE_GGSN_BACK_OFF_TIME;
	gtpv1_information_elements[GTPV1_IE_GGSN_BACK_OFF_TIME].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_GGSN_BACK_OFF_TIME].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_GGSN_BACK_OFF_TIME].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_GGSN_BACK_OFF_TIME].name, "GGSN Back-Off Time");

	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION].id = GTPV1_IE_SIGNALLING_PRIORITY_INDICATION;
	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION].name, "Signalling Priority Indication");

	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION_WITH_NSAPI].id = GTPV1_IE_SIGNALLING_PRIORITY_INDICATION_WITH_NSAPI;
	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION_WITH_NSAPI].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION_WITH_NSAPI].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION_WITH_NSAPI].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_SIGNALLING_PRIORITY_INDICATION_WITH_NSAPI].name, "Signalling Priority Indication with NSAPI");

	gtpv1_information_elements[GTPV1_IE_HIGHER_BITRATES_THAN_16_MBPS_FLAG].id = GTPV1_IE_HIGHER_BITRATES_THAN_16_MBPS_FLAG;
	gtpv1_information_elements[GTPV1_IE_HIGHER_BITRATES_THAN_16_MBPS_FLAG].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_HIGHER_BITRATES_THAN_16_MBPS_FLAG].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_HIGHER_BITRATES_THAN_16_MBPS_FLAG].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_HIGHER_BITRATES_THAN_16_MBPS_FLAG].name, "Higher bitrates than 16 Mbps flag");

	gtpv1_information_elements[GTPV1_IE_MAX_MBR_APN_AMBR].id = GTPV1_IE_MAX_MBR_APN_AMBR;
	gtpv1_information_elements[GTPV1_IE_MAX_MBR_APN_AMBR].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_MAX_MBR_APN_AMBR].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_MAX_MBR_APN_AMBR].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_MAX_MBR_APN_AMBR].name, "Max MBR/APN-AMBR");

	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MM_CONTEXT_FOR_SRVCC].id = GTPV1_IE_ADDITIONAL_MM_CONTEXT_FOR_SRVCC;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MM_CONTEXT_FOR_SRVCC].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MM_CONTEXT_FOR_SRVCC].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MM_CONTEXT_FOR_SRVCC].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_ADDITIONAL_MM_CONTEXT_FOR_SRVCC].name, "Additional MM context for SRVCC");

	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_FLAGS_FOR_SRVCC].id = GTPV1_IE_ADDITIONAL_FLAGS_FOR_SRVCC;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_FLAGS_FOR_SRVCC].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_FLAGS_FOR_SRVCC].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_ADDITIONAL_FLAGS_FOR_SRVCC].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_ADDITIONAL_FLAGS_FOR_SRVCC].name, "Additional flags for SRVCC");

	gtpv1_information_elements[GTPV1_IE_STN_SR].id = GTPV1_IE_STN_SR;
	gtpv1_information_elements[GTPV1_IE_STN_SR].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_STN_SR].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_STN_SR].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_STN_SR].name, "STN-SR");

	gtpv1_information_elements[GTPV1_IE_C_MSISDN].id = GTPV1_IE_C_MSISDN;
	gtpv1_information_elements[GTPV1_IE_C_MSISDN].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_C_MSISDN].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_C_MSISDN].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_C_MSISDN].name, "C-MSISDN");

	gtpv1_information_elements[GTPV1_IE_EXTENDED_RANAP_CAUSE].id = GTPV1_IE_EXTENDED_RANAP_CAUSE;
	gtpv1_information_elements[GTPV1_IE_EXTENDED_RANAP_CAUSE].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_EXTENDED_RANAP_CAUSE].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_EXTENDED_RANAP_CAUSE].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_EXTENDED_RANAP_CAUSE].name, "Extended RANAP Cause");

	gtpv1_information_elements[GTPV1_IE_CHARGING_GATEWAY_ADDRESS].id = GTPV1_IE_CHARGING_GATEWAY_ADDRESS;
	gtpv1_information_elements[GTPV1_IE_CHARGING_GATEWAY_ADDRESS].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_CHARGING_GATEWAY_ADDRESS].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_CHARGING_GATEWAY_ADDRESS].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_CHARGING_GATEWAY_ADDRESS].name, "Charging Gateway Address");

	gtpv1_information_elements[GTPV1_IE_PRIVATE_EXTENSION].id = GTPV1_IE_PRIVATE_EXTENSION;
	gtpv1_information_elements[GTPV1_IE_PRIVATE_EXTENSION].grouped = 0;
	gtpv1_information_elements[GTPV1_IE_PRIVATE_EXTENSION].header_length = 3;
	gtpv1_information_elements[GTPV1_IE_PRIVATE_EXTENSION].body_length = 0;
	strcpy(gtpv1_information_elements[GTPV1_IE_PRIVATE_EXTENSION].name, "Private Extension");
}


