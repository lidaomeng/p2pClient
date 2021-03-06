/* soapClient.cpp
   Generated by gSOAP 2.8.14 from fctxt.h

Copyright(C) 2000-2013, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under ONE of the following licenses:
GPL or Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"

SOAP_SOURCE_STAMP("@(#) soapClient.cpp ver 2.8.14 2016-03-29 04:15:25 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getTaskProcessReport(struct soap *soap, const char *soap_endpoint, const char *soap_action, ns1__getTaskProcessReport *ns1__getTaskProcessReport_, ns1__getTaskProcessReportResponse *ns1__getTaskProcessReportResponse_)
{	struct __ns1__getTaskProcessReport soap_tmp___ns1__getTaskProcessReport;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.23.140.181:8080/peer/services/peerWS";
	if (soap_action == NULL)
		soap_action = "";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__getTaskProcessReport.ns1__getTaskProcessReport_ = ns1__getTaskProcessReport_;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getTaskProcessReport(soap, &soap_tmp___ns1__getTaskProcessReport);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__getTaskProcessReport(soap, &soap_tmp___ns1__getTaskProcessReport, "-ns1:getTaskProcessReport", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getTaskProcessReport(soap, &soap_tmp___ns1__getTaskProcessReport, "-ns1:getTaskProcessReport", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getTaskProcessReportResponse_)
		return soap_closesock(soap);
	ns1__getTaskProcessReportResponse_->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getTaskProcessReportResponse_->soap_get(soap, "ns1:getTaskProcessReportResponse", "ns1:getTaskProcessReportResponse");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getPackList(struct soap *soap, const char *soap_endpoint, const char *soap_action, ns1__getPackList *ns1__getPackList_, ns1__getPackListResponse *ns1__getPackListResponse_)
{	struct __ns1__getPackList soap_tmp___ns1__getPackList;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.23.140.181:8080/peer/services/peerWS";
	if (soap_action == NULL)
		soap_action = "";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__getPackList.ns1__getPackList_ = ns1__getPackList_;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getPackList(soap, &soap_tmp___ns1__getPackList);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__getPackList(soap, &soap_tmp___ns1__getPackList, "-ns1:getPackList", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getPackList(soap, &soap_tmp___ns1__getPackList, "-ns1:getPackList", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getPackListResponse_)
		return soap_closesock(soap);
	ns1__getPackListResponse_->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getPackListResponse_->soap_get(soap, "ns1:getPackListResponse", "ns1:getPackListResponse");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__sendTaskProcess(struct soap *soap, const char *soap_endpoint, const char *soap_action, ns1__sendTaskProcess *ns1__sendTaskProcess_, ns1__sendTaskProcessResponse *ns1__sendTaskProcessResponse_)
{	struct __ns1__sendTaskProcess soap_tmp___ns1__sendTaskProcess;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.23.140.181:8080/peer/services/peerWS";
	if (soap_action == NULL)
		soap_action = "";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__sendTaskProcess.ns1__sendTaskProcess_ = ns1__sendTaskProcess_;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__sendTaskProcess(soap, &soap_tmp___ns1__sendTaskProcess);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__sendTaskProcess(soap, &soap_tmp___ns1__sendTaskProcess, "-ns1:sendTaskProcess", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__sendTaskProcess(soap, &soap_tmp___ns1__sendTaskProcess, "-ns1:sendTaskProcess", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__sendTaskProcessResponse_)
		return soap_closesock(soap);
	ns1__sendTaskProcessResponse_->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__sendTaskProcessResponse_->soap_get(soap, "ns1:sendTaskProcessResponse", "ns1:sendTaskProcessResponse");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getDcp(struct soap *soap, const char *soap_endpoint, const char *soap_action, ns1__getDcp *ns1__getDcp_, ns1__getDcpResponse *ns1__getDcpResponse_)
{	struct __ns1__getDcp soap_tmp___ns1__getDcp;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.23.140.181:8080/peer/services/peerWS";
	if (soap_action == NULL)
		soap_action = "";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__getDcp.ns1__getDcp_ = ns1__getDcp_;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getDcp(soap, &soap_tmp___ns1__getDcp);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__getDcp(soap, &soap_tmp___ns1__getDcp, "-ns1:getDcp", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getDcp(soap, &soap_tmp___ns1__getDcp, "-ns1:getDcp", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getDcpResponse_)
		return soap_closesock(soap);
	ns1__getDcpResponse_->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getDcpResponse_->soap_get(soap, "ns1:getDcpResponse", "ns1:getDcpResponse");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___ns1__getExceptionReport(struct soap *soap, const char *soap_endpoint, const char *soap_action, ns1__getExceptionReport *ns1__getExceptionReport_, ns1__getExceptionReportResponse *ns1__getExceptionReportResponse_)
{	struct __ns1__getExceptionReport soap_tmp___ns1__getExceptionReport;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.23.140.181:8080/peer/services/peerWS";
	if (soap_action == NULL)
		soap_action = "";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__getExceptionReport.ns1__getExceptionReport_ = ns1__getExceptionReport_;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__getExceptionReport(soap, &soap_tmp___ns1__getExceptionReport);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__getExceptionReport(soap, &soap_tmp___ns1__getExceptionReport, "-ns1:getExceptionReport", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__getExceptionReport(soap, &soap_tmp___ns1__getExceptionReport, "-ns1:getExceptionReport", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__getExceptionReportResponse_)
		return soap_closesock(soap);
	ns1__getExceptionReportResponse_->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__getExceptionReportResponse_->soap_get(soap, "ns1:getExceptionReportResponse", "ns1:getExceptionReportResponse");
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapClient.cpp */
