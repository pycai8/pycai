extern void CHlsHandlerInit();
extern void CHlsHandlerUninit();
extern void CTsEncoderInit();
extern void CTsEncoderUninit();

__attribute__ ((constructor))
static void ComponentInit()
{
    CHlsHandlerInit();
    CTsEncoderInit();
}

__attribute__ ((destructor))
static void ComponentUninit()
{
    CHlsHandlerUninit();
    CTsEncoderUninit();
}

