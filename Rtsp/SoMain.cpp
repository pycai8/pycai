extern void CRtspHandlerInit();
extern void CRtspHandlerUninit();

extern void CRtpSessionInit();
extern void CRtpSessionUninit();

extern void CRtcpSessionInit();
extern void CRtcpSessionUninit();

__attribute__ ((constructor))
static void ComponentInit()
{
    CRtspHandlerInit();
    CRtpSessionInit();
    CRtcpSessionInit();
}

__attribute__ ((destructor))
static void ComponentUninit()
{
    CRtspHandlerUninit();
    CRtpSessionUninit();
    CRtcpSessionUninit();
}

