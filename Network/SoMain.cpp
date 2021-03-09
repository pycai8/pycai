extern void CTcpServerInit();
extern void CTcpServerUninit();

extern void CTcpSessionInit();
extern void CTcpSessionUninit();

extern void CUdpHelperInit();
extern void CUdpHelperUninit();

__attribute__ ((constructor))
static void ComponentInit()
{
    CTcpServerInit();
    CTcpSessionInit();
    CUdpHelperInit();
}

__attribute__ ((destructor))
static void ComponentUninit()
{
    CTcpServerUninit();
    CTcpSessionUninit();
    CUdpHelperUninit();
}

