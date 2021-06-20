extern void CHttpMjpgHandlerInit();
extern void CHttpMjpgHandlerUninit();
extern void CMjpgEncoderInit();
extern void CMjpgEncoderUninit();

__attribute__ ((constructor))
static void ComponentInit()
{
    CHttpMjpgHandlerInit();
    CMjpgEncoderInit();
}

__attribute__ ((destructor))
static void ComponentUninit()
{
    CHttpMjpgHandlerUninit();
    CMjpgEncoderUninit();
}

