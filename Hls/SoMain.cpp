extern void CHlsHandlerInit();
extern void CHlsHandlerUninit();

__attribute__ ((constructor))
static void ComponentInit()
{
    CHlsHandlerInit();
}

__attribute__ ((destructor))
static void ComponentUninit()
{
    CHlsHandlerUninit();
}

